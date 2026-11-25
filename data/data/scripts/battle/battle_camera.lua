--- Camera scripts of the battles (attacks, magic, enemy attacks).
--
-- The original game has a small script for the camera of each attack, in the files camdat0.bin,
-- camdat1.bin and camdat2.bin (the installer writes them to gamedata/battle_cameras.lua). Two points
-- move in a battle: the camera (position stream) and the point it looks at (look stream), each one
-- with its own script of bytes, found with (camera id * 3 + variant). The ids are the camera_single
-- and camera_multiple of the attacks. The interpreters are in ff7.exe (0x5c2341 and 0x5c3f45).
--
-- A script is a list of opcodes (0xd5-0xff, other values do nothing) with operands that are bytes or
-- 16 bit values (little endian, with sign). Positions are in the units of the original game, with x to
-- the side, y down and z in depth (the enemies are in the negative side), and the ones that follow an
-- actor are relative to a point of it, the offsets turned with the way it faces if the stream has
-- "rotate" on. The frame of the scene is converted like the positions of the formations:
--
--   scene = (x, z, -y) / 255
--
-- (the division is the one the installer uses for every coordinate of the battle). The facing angle of
-- an actor in ff7.exe is the one of the direction to its target plus half a turn (0x420d5b), so the
-- positive z of a turned offset is behind the actor. The streams run
-- one opcode after the other until one of them waits or places the point (the ones that do are
-- listed in YIELDS), once per tick, 15 ticks per second like the effects.
--
-- The state of the battle is read through an environment (see BattleCameraController in
-- battle_camera_controller.lua): env:actor(ref) gives the position of an actor of the scene, the way it
-- faces and its height, and env.formation the camera that was in use before the script.
BattleCamera = {}
BattleCamera.__index = BattleCamera

--- Scene units for each original unit.
BattleCamera.UNIT = 1 / 255

--- Ticks of the scripts in a second.
BattleCamera.TPS = 15

--- Operands (the bytes after the opcode) of each opcode: the numbers are the ones of the installer
-- (BattleCameraScripts.h).
BattleCamera.LENGTH = {
    pos = {
        [0xd5] = 2, [0xd6] = 0, [0xd7] = 2, [0xd8] = 9, [0xd9] = 0, [0xda] = 0, [0xdb] = 0, [0xdc] = 0,
        [0xdd] = 1, [0xde] = 1, [0xdf] = 0, [0xe0] = 2, [0xe1] = 0, [0xe2] = 1, [0xe3] = 9, [0xe4] = 8,
        [0xe5] = 8, [0xe6] = 7, [0xe7] = 8, [0xe9] = 8, [0xeb] = 9, [0xef] = 8, [0xf0] = 7, [0xf1] = 0,
        [0xf2] = 5, [0xf3] = 5, [0xf4] = 0, [0xf5] = 1, [0xf7] = 7, [0xf8] = 12, [0xf9] = 6, [0xfe] = 0,
        [0xff] = 0,
    },
    look = {
        [0xd8] = 9, [0xd9] = 0, [0xdb] = 0, [0xdc] = 0, [0xdd] = 1, [0xde] = 1, [0xdf] = 0, [0xe0] = 2,
        [0xe1] = 0, [0xe2] = 1, [0xe3] = 9, [0xe4] = 8, [0xe5] = 8, [0xe6] = 7, [0xe8] = 8, [0xea] = 8,
        [0xec] = 9, [0xf0] = 8, [0xf4] = 0, [0xf5] = 1, [0xf8] = 7, [0xf9] = 7, [0xfa] = 6, [0xfe] = 0,
        [0xff] = 0,
    },
}

--- Opcodes that end the processing of a tick (the ones that wait or place the point).
BattleCamera.YIELDS = {
    pos = {[0xef] = true, [0xf0] = true, [0xf4] = true, [0xf7] = true, [0xff] = true},
    look = {[0xf0] = true, [0xf4] = true, [0xf8] = true, [0xf9] = true, [0xff] = true},
}

--- Maximum opcodes in a tick, in case a script loops without waiting.
BattleCamera.MAX_OPCODES = 64

--- Gets the bytes of a script.
--
-- @param set Number of the camdat file (0 to 2).
-- @param kind "pos" or "look".
-- @param id Camera id.
-- @param variant 0 to 2.
-- @return The list of bytes, nil if there is none (ids of -1 or 0xffff do nothing).
function BattleCamera.get_script(set, kind, id, variant)
    if id == nil or id < 0 or id >= 0xffff then return nil end
    local data = Game.BattleCameras ~= nil and Game.BattleCameras[set] or nil
    if data == nil then return nil end
    local key = data[kind == "pos" and "position" or "look"][id * 3 + (variant or 0) + 1]
    if key == nil then return nil end
    return data.scripts[key]
end

local function vec(x, y, z) return {x, y, z} end

local function new_stream(kind, script)
    return {
        kind = kind, script = script, pc = 1, wait = 0, rotate = false, linear = false,
        pos = vec(0, 0, 0), tasks = {}, finished = script == nil, yielded = false,
    }
end

--- Creates the camera of a pair of scripts.
--
-- @param position Script of the position (list of bytes, can be nil).
-- @param look Script of the look at point.
-- @param env See above.
function BattleCamera.new(position, look, env)
    local self = setmetatable({}, BattleCamera)
    self.env = env
    self.cam = new_stream("pos", position)
    self.look = new_stream("look", look)
    local f = env.formation
    self.cam.pos = {f.pos[1], f.pos[2], f.pos[3]}
    self.look.pos = {f.look[1], f.look[2], f.look[3]}
    -- The camera the script started on (opcode d9). The opcode dd changes the one of e1 and e2, not it.
    self.base = f
    return self
end

--- Is everything of the script done? (the scripts stay on their last opcode forever)
function BattleCamera:is_finished()
    return self.cam.finished and self.look.finished and #self.cam.tasks == 0 and #self.look.tasks == 0
end

-- Operand readers. A "reader" has the script and the position of the next byte.
local function u8(s)
    local v = s.script[s.pc]
    s.pc = s.pc + 1
    return v or 0
end

local function s16(s)
    local lo, hi = u8(s), u8(s)
    local v = hi * 256 + lo
    if v >= 32768 then v = v - 65536 end
    return v
end

--- Converts a vector of the original frame to the scene, as an offset.
local function to_scene_offset(x, y, z)
    local k = BattleCamera.UNIT
    return x * k, z * k, -y * k
end

--- Converts a position of the original frame (absolute) to the scene.
function BattleCamera:absolute(x, y, z)
    return vec(to_scene_offset(x, y, z))
end

--- Position of a point of an actor, plus an offset of the original frame.
--
-- As in ff7.exe, the offsets of the position of the camera grow with the size of the model (0x5c6290) and
-- turn with the way it faces if the stream has "rotate" on, and the ones of the point it looks at do
-- neither: they only move down by the height that the model has for it (0x5c5a78). None of that applies to
-- the center of a group of targets.
--
-- @param kind Stream of the offset, "pos" or "look".
-- @param rotate If the offset turns with the way the actor faces.
function BattleCamera:anchor(ref, point, ox, oy, oz, rotate, kind)
    local actor = self.env:actor(ref)
    if actor == nil then return nil end
    -- The points are ends of bones of the model. Without that data (an old install), the middle of the actor.
    local px, py, pz
    if actor.point ~= nil then px, py, pz = actor.point(point) end
    if px == nil then px, py, pz = actor.pos[1], actor.pos[2], actor.pos[3] + actor.height * 0.5 end
    if actor.group then
        rotate = false
    end
    if kind == "look" then
        oy = oy + (actor.look_height or 0)
        rotate = false
    elseif actor.size ~= nil then
        local function scale(value, size)
            -- A Small actor halves the factors, but they don't go under 256.
            if actor.small then size = math.max(256, math.floor(size / 2)) end
            return math.floor(value * size / 256)
        end
        ox, oy, oz = scale(ox, actor.size[1]), scale(oy, actor.size[2]), scale(oz, actor.size[3])
    end
    local k = BattleCamera.UNIT
    local dx, dy, dz
    if rotate then
        -- x is to the left of the way it faces, z behind it, y down (the rotation of 0x5c6290).
        local fx, fy = actor.forward[1], actor.forward[2]
        dx = (-fy) * ox * k - fx * oz * k
        dy = fx * ox * k - fy * oz * k
        dz = -oy * k
    else
        dx, dy, dz = to_scene_offset(ox, oy, oz)
    end
    return vec(px + dx, py + dy, pz + dz)
end

--- The reference of an actor of a script (the byte of the opcodes that take it): 0-2 the party and
-- 4-9 the enemies.
local function actor_from_byte(b)
    if b >= 0 and b <= 2 then return {side = "party", index = b + 1} end
    if b >= 4 and b <= 9 then return {side = "enemies", index = b - 3} end
    return "targets"
end

--- Starts a move to a point in some ticks.
local function start_move(stream, target, ticks)
    stream.tasks[#stream.tasks + 1] = {
        type = "move", start = {stream.pos[1], stream.pos[2], stream.pos[3]}, target = target,
        ticks = math.max(1, ticks), age = 0, linear = stream.linear,
    }
end

--- Starts the follow of an actor for some ticks.
local function start_follow(stream, ref, point, ox, oy, oz, ticks, rotate)
    stream.tasks[#stream.tasks + 1] = {
        type = "follow", ref = ref, point = point, off = {ox, oy, oz}, ticks = math.max(1, ticks),
        age = 0, rotate = rotate, kind = stream.kind,
    }
end

--- Starts an orbit of the position around the point the camera looks at (opcode f8 of the position,
-- task 0x5c3c7d of ff7.exe).
--
-- All the values are the ones of the script: angles in 4096ths of a turn, distances in the original frame.
--
-- @param speed Angle added each tick.
-- @param radius Horizontal distance to the point, in eighths.
-- @param angle Starting angle.
-- @param radius_speed Added to the radius each tick.
-- @param y_speed Added to the height (down) of the camera each tick.
local function start_orbit(stream, speed, radius, angle, radius_speed, y_speed, ticks)
    stream.tasks[#stream.tasks + 1] = {
        type = "orbit", speed = speed, radius = radius, angle = angle, radius_speed = radius_speed,
        y_speed = y_speed, ticks = math.max(1, ticks), age = 0,
    }
end

--- The reference of the "current actor" of the opcodes (byte 0xbe40e4 of ff7.exe): the target, or
-- the group of targets if there are several or after the opcode df.
function BattleCamera:current_ref()
    return self.group and "targets" or "current"
end

--- Position of one of the two views a model has for the cameras (opcode e0): the position of the
-- camera or the point it looks at, depending on the stream. It doesn't turn or grow with the model. The
-- height of the first view is over the ground, and the one of the second over the actor.
--
-- @param view 0 or 1.
-- @return Nil if there is no data, like for a group of targets.
function BattleCamera:view_point(ref, view, kind)
    local actor = self.env:actor(ref)
    if actor == nil or actor.views == nil or actor.views[view + 1] == nil then return nil end
    local offset = actor.views[view + 1][kind]
    local x, y, z = to_scene_offset(offset[1], offset[2], offset[3])
    if view ~= 0 then z = z + actor.pos[3] end
    return vec(actor.pos[1] + x, actor.pos[2] + y, z)
end

--- Places a stream at a point of an actor, now.
function BattleCamera:place(stream, ref, point, ox, oy, oz, rotate)
    local p = self:anchor(ref, point, ox, oy, oz, rotate, stream.kind)
    if p ~= nil then stream.pos = p end
end

--- Runs the opcodes of a stream until it waits.
function BattleCamera:run(stream)
    local kind = stream.kind
    local lengths = BattleCamera.LENGTH[kind]
    local yields = BattleCamera.YIELDS[kind]
    local env = self.env
    for _ = 1, BattleCamera.MAX_OPCODES do
        if stream.finished then return end
        local op = stream.script[stream.pc]
        if op == nil then stream.finished = true return end
        stream.pc = stream.pc + 1
        local length = lengths[op]
        if length == nil then
            -- It isn't an opcode: nothing happens, but the tick is over.
            return
        end
        -- Opcodes that are the same in both streams (the ones with the same number and meaning).
        if op == 0xff then
            stream.pc = stream.pc - 1
            stream.finished = true
            return
        elseif op == 0xf5 then
            local n = u8(stream)
            if n >= 128 then n = n - 256 end
            stream.wait = n
        elseif op == 0xf4 then
            if stream.wait ~= 0 then
                stream.wait = stream.wait - 1
                stream.pc = stream.pc - 1
                return
            end
        elseif op == 0xfe then
            if stream.wait == 0 and stream.script[stream.pc] == 0xc0 then
                stream.pc = 1
                stream.wait = 0
            end
        elseif op == 0xdc then
            stream.rotate = true
        elseif op == 0xdf then
            -- From now on, the "current actor" of both streams is the group of targets.
            self.group = true
        elseif kind == "pos" and op == 0xf1 then
            stream.rotate = false
        elseif kind == "look" and op == 0xdb then
            stream.rotate = false
        elseif kind == "pos" and op == 0xda then
            stream.linear = false
        elseif kind == "pos" and op == 0xdb then
            stream.linear = true
        elseif op == 0xdd then
            -- Chooses the camera of the formation that e1 and e2 use, for both streams.
            local index = u8(stream)
            local chosen = env.formation_at ~= nil and env.formation_at(index) or nil
            if chosen ~= nil then env.formation, self.formation_index = chosen, index end
        elseif op == 0xe1 or op == 0xd9 then
            -- The camera of the formation (e1), or the one that was in use before the script (d9).
            local view = op == 0xd9 and self.base or env.formation
            local source = kind == "pos" and view.pos or view.look
            stream.pos = {source[1], source[2], source[3]}
        elseif op == 0xe2 then
            local source = kind == "pos" and env.formation.pos or env.formation.look
            start_move(stream, {source[1], source[2], source[3]}, u8(stream))
        elseif op == 0xe6 then
            local x, y, z = s16(stream), s16(stream), s16(stream)
            start_move(stream, self:absolute(x, y, z), u8(stream))
        elseif op == 0xe0 then
            -- Move to a view of the current actor: view, ticks.
            local view, ticks = u8(stream), u8(stream)
            local target = self:view_point(self:current_ref(), view, kind)
            if target ~= nil then start_move(stream, target, ticks) end
        elseif (kind == "pos" and op == 0xf9) or (kind == "look" and op == 0xfa) then
            local x, y, z = s16(stream), s16(stream), s16(stream)
            stream.pos = self:absolute(x, y, z)
        elseif op == 0xe3 or op == 0xd8 or op == 0xe4 or op == 0xe5 then
            -- Move to a point of an actor, with an offset: actor (e3 and d8), point, x, y, z, ticks.
            local ref
            if op == 0xe3 then ref = actor_from_byte(u8(stream))
            elseif op == 0xd8 then ref = {target = u8(stream) + 1}
            elseif op == 0xe4 then ref = "attacker"
            else ref = self:current_ref() end
            local point = u8(stream)
            local x, y, z = s16(stream), s16(stream), s16(stream)
            local ticks = u8(stream)
            local target = self:anchor(ref, point, x, y, z, stream.rotate, stream.kind)
            if target ~= nil then start_move(stream, target, ticks) end
        elseif (kind == "pos" and (op == 0xe7 or op == 0xe9 or op == 0xeb))
          or (kind == "look" and (op == 0xe8 or op == 0xea or op == 0xec)) then
            -- Follow a point of an actor with an offset, for some ticks.
            local ref
            if op == 0xeb or op == 0xec then ref = actor_from_byte(u8(stream))
            elseif op == 0xe7 or op == 0xe8 then ref = "attacker"
            else ref = self:current_ref() end
            local point = u8(stream)
            local x, y, z = s16(stream), s16(stream), s16(stream)
            local ticks = u8(stream)
            start_follow(stream, ref, point, x, y, z, ticks, stream.rotate)
        elseif (kind == "pos" and (op == 0xef or op == 0xf0 or op == 0xf7))
          or (kind == "look" and (op == 0xf0 or op == 0xf8 or op == 0xf9)) then
            -- Place at a point of an actor, with an offset.
            local ref
            if (kind == "pos" and op == 0xef) or (kind == "look" and op == 0xf0) then ref = actor_from_byte(u8(stream))
            elseif (kind == "pos" and op == 0xf0) or (kind == "look" and op == 0xf9) then ref = "attacker"
            else ref = self:current_ref() end
            local point = u8(stream)
            local x, y, z = s16(stream), s16(stream), s16(stream)
            self:place(stream, ref, point, x, y, z, stream.rotate)
        elseif kind == "pos" and op == 0xf8 then
            local speed, radius, angle = s16(stream), s16(stream), s16(stream)
            local radius_speed, y_speed, ticks = s16(stream), s16(stream), s16(stream)
            start_orbit(stream, speed, radius, angle, radius_speed, y_speed, ticks)
        else
            -- Opcodes that don't change the camera (events of the battle) and the ones no script of the
            -- game uses (like f2 and f3): skip their operands.
            stream.pc = stream.pc + length
        end
        if yields[op] and op ~= 0xff and op ~= 0xf4 then return end
    end
end

--- Advances the tasks of a stream one tick.
function BattleCamera:run_tasks(stream)
    local alive = {}
    for _, task in ipairs(stream.tasks) do
        task.age = task.age + 1
        if task.type == "move" then
            if task.age >= task.ticks then
                stream.pos = {task.target[1], task.target[2], task.target[3]}
            else
                local f
                if task.linear then f = (task.age) / task.ticks
                else f = (1 - math.cos(math.pi * (task.age - 1) / task.ticks)) / 2 end
                stream.pos = {
                    task.start[1] + (task.target[1] - task.start[1]) * f,
                    task.start[2] + (task.target[2] - task.start[2]) * f,
                    task.start[3] + (task.target[3] - task.start[3]) * f,
                }
            end
        elseif task.type == "orbit" then
            task.angle = task.angle + task.speed
            local a = task.angle * 2 * math.pi / 4096
            local r = task.radius * 8 * BattleCamera.UNIT
            local center = self.look.pos
            stream.pos = {
                center[1] + math.sin(a) * r,
                center[2] + math.cos(a) * r,
                stream.pos[3] - task.y_speed * BattleCamera.UNIT,
            }
            task.radius = task.radius + task.radius_speed
        else
            local p = self:anchor(task.ref, task.point, task.off[1], task.off[2], task.off[3], task.rotate, task.kind)
            if p ~= nil then stream.pos = p end
        end
        if task.age < task.ticks then alive[#alive + 1] = task end
    end
    stream.tasks = alive
end

--- Advances the camera one tick.
function BattleCamera:tick()
    for _, stream in ipairs({self.cam, self.look}) do
        self:run(stream)
        self:run_tasks(stream)
    end
end

--- The position of the camera and the point it looks at, in the scene.
function BattleCamera:get()
    return self.cam.pos, self.look.pos
end

--- Controller of the camera of the battle.
--
-- Starts the camera of an action when it begins, moves the camera while the action goes on and takes it
-- back to the camera of the formation when it ends. Nothing moves if the camera is set to manual
-- in the config menu (see {@see Battle.is_camera_auto}).
BattleCameraController = {
    --- Number of the file camdatN.bin that the original uses for each battle layout (0 normal,
    -- 1 preemptive, 2 back attack, 3 side attack, ... 8 locked): the table at 0x7b8528 of ff7.exe,
    -- indexed with the type of the battle (byte 0x999e12), that has the same numbers as the layouts.
    FILE_BY_LAYOUT = {[0] = 0, 0, 1, 2, 2, 2, 2, 2, 0},

    --- Ticks in a second of the way back to the camera of the formation. The movement (half of the
    -- distance in every tick) is the one of the executable, but its rate isn't known: it was too
    -- fast with 15, the rate of the scripts, and with 10.
    RETURN_TPS = 5,

    --- The camera that is running, nil if none.
    camera = nil,

    --- Seconds that are left to the next tick, and the last two results (for the interpolation).
    accumulated = 0,
    previous = nil,
    current = nil,

    --- Ids that were started for debugging: set to a number to force an id.
    forced_id = nil,
}

--- Position of an entity and its height, in the scene.
local function entity_info(entity)
    local x, y, z = entity:get_position()
    local info = BattleCameraController.heights[entity:get_name()]
    if info == nil then
        -- The height is the one of the highest bone.
        local top = z
        for bone = 0, 24 do
            local ok, bx, by, bz = pcall(entity.get_bone_position, entity, bone)
            if ok and bz ~= nil and bz > top then top = bz end
        end
        info = math.max(top - z, 1.5)
        BattleCameraController.heights[entity:get_name()] = info
    end
    return x, y, z, info
end
BattleCameraController.heights = {}

--- Data of the battle models for the cameras, by model name. False for the models without it.
BattleCameraController.models = {}

--- Gets the camera data of a model, written by the installer. Nil if there is none.
local function model_data(name)
    if name == nil then return nil end
    local data = BattleCameraController.models[name]
    if data == nil then
        local chunk = loadfile("./data/scripts/battle_models/" .. name .. "_camera.lua")
        if chunk ~= nil then chunk() end
        data = BattleModelCameras ~= nil and BattleModelCameras[name] or false
        BattleCameraController.models[name] = data
    end
    return data or nil
end

--- Describes an actor of the scene for the camera.
--
-- @param entity Its entity.
-- @param side Its side, and index its place in it.
-- @param forward The way it faces.
local function actor_info(entity, side, index, forward)
    local x, y, z, height = entity_info(entity)
    local info = {pos = {x, y, z}, height = height, forward = forward({x, y, z})}
    local ui = UiContainer.BattleUi
    local data = model_data(ui:get_model_name(side, index))
    if data ~= nil then
        info.size, info.look_height, info.views = data.size, data.look_height, data.views
        info.small = ui:has_status(side, index, BattleStatus.SMALL)
        -- Position of a point of the model, nil if the model doesn't have it.
        info.point = function(point)
            local bone = data.points[point + 1]
            if bone == nil then return nil end
            local bx, by, bz
            if bone == 0 then bx, by, bz = entity:get_bone_position(0)
            elseif data.lengths[bone - 1] ~= nil then bx, by, bz = entity:get_bone_end(bone - 1, data.lengths[bone - 1])
            end
            if bx == nil or (bx == 0 and by == 0 and bz == 0) then return nil end
            return bx, by, bz
        end
    end
    return info
end

--- Builds the environment of the camera for an action.
local function build_env(side, actor, target_side, targets, formation)
    local ui = UiContainer.BattleUi
    local function entity_of(s, index)
        local e = ui:get_entity(s, index)
        return e
    end
    -- Middle of the area the targets are in, on the ground.
    local function ground_center(s, list)
        local min_x, max_x, min_y, max_y
        for _, index in ipairs(list) do
            local e = entity_of(s, index)
            if e ~= nil then
                local x, y = entity_info(e)
                min_x, max_x = math.min(min_x or x, x), math.max(max_x or x, x)
                min_y, max_y = math.min(min_y or y, y), math.max(max_y or y, y)
            end
        end
        if min_x == nil then return nil end
        return {(min_x + max_x) / 2, (min_y + max_y) / 2, 0}
    end
    local attacker_entity = entity_of(side, actor)
    local attacker_pos
    if attacker_entity ~= nil then
        local x, y, z = entity_info(attacker_entity)
        attacker_pos = {x, y, z}
    end
    local target_pos = ground_center(target_side, targets)
    local function direction(from, to)
        if from == nil or to == nil then return {0, -1} end
        local dx, dy = to[1] - from[1], to[2] - from[2]
        local l = math.sqrt(dx * dx + dy * dy)
        if l < 0.0001 then return {0, -1} end
        return {dx / l, dy / l}
    end
    local function toward(to) return function(from) return direction(from, to) end end
    local env = {formation = formation}
    function env:actor(ref)
        if ref == "attacker" then
            if attacker_entity == nil then return nil end
            return actor_info(attacker_entity, side, actor, toward(target_pos))
        elseif ref == "current" and #targets == 1 then
            local e = entity_of(target_side, targets[1])
            if e ~= nil then return actor_info(e, target_side, targets[1], toward(attacker_pos)) end
        elseif ref == "current" or ref == "targets" then
            -- The center of the group of targets, on the ground (0x5bfe39).
            local center = ground_center(target_side, targets)
            if center == nil then return nil end
            return {
                group = true, pos = center, height = 0, forward = direction(center, attacker_pos),
                point = function() return center[1], center[2], center[3] end,
            }
        elseif type(ref) == "table" and ref.target ~= nil then
            -- One of the targets, by its place in the list (opcode d8).
            local index = targets[ref.target] or targets[1]
            local e = entity_of(target_side, index)
            if e ~= nil then return actor_info(e, target_side, index, toward(attacker_pos)) end
        elseif type(ref) == "table" then
            local ref_side = ref.side == "party" and Battle.TargetGroup.PARTY or Battle.TargetGroup.ENEMIES
            local e = entity_of(ref_side, ref.index)
            if e == nil then return nil end
            return actor_info(e, ref_side, ref.index, toward(ref.side == "party" and target_pos or attacker_pos))
        end
        return nil
    end
    return env
end

--- Starts the camera of an action.
--
-- @param side Side of the actor.
-- @param actor Index of the actor.
-- @param target_side Side of the targets.
-- @param targets List of indexes of the targets.
-- @param camera_single Id for a single target (nil or 0xffff for none).
-- @param camera_multiple Id for several targets.
-- @return True if there is a camera.
function BattleCameraController:start(side, actor, target_side, targets, camera_single, camera_multiple)
    self:stop(false)
    self.from_script = false
    if Battle.is_camera_auto ~= nil and not Battle.is_camera_auto() then return false end
    if targets == nil or #targets == 0 then return false end
    local id = #targets > 1 and camera_multiple or camera_single
    if self.forced_id ~= nil then id = self.forced_id end
    local variant = math.random(0, 2)
    local layout = battle_manager:get_layout()
    local file = self.FILE_BY_LAYOUT[layout] or 0
    local position = BattleCamera.get_script(file, "pos", id, variant)
    local look = BattleCamera.get_script(file, "look", id, variant)
    if position == nil and look == nil then return false end
    -- A camera that was going back to the one of the formation is replaced by this one (if there is
    -- none, it goes on: an action with no camera must not leave it where it was).
    self.returning = nil
    self.last = nil
    local index = UiContainer.BattleUi.camera_index or 0
    local px, py, pz, lx, ly, lz = battle_manager:get_camera(index)
    local formation = {pos = {px, py, pz}, look = {lx, ly, lz}}
    local env = build_env(side, actor, target_side, targets, formation)
    self.camera = BattleCamera.new(position, look, env)
    self.camera_index = index
    self.accumulated = 0
    self.previous = nil
    self.current = nil
    print("Battle camera " .. tostring(id) .. " variant " .. tostring(variant) .. " file " .. tostring(file))
    return true
end

--- Starts the camera of the victory: shots of the party at fixed places, which depend on the camera
-- file of the layout of the battle (camera id -3 of ff7.exe, set at 0x42dbd7).
--
-- @return True if there is a camera (an old install doesn't have its data).
function BattleCameraController:start_victory()
    local variant = math.random(0, 2)
    local file = self.FILE_BY_LAYOUT[battle_manager:get_layout()] or 0
    local data = Game.BattleCameras ~= nil and Game.BattleCameras[file] or nil
    if data == nil or data.victory_position == nil then return false end
    local position = data.scripts[data.victory_position[variant + 1]]
    local look = data.scripts[data.victory_look[variant + 1]]
    if position == nil and look == nil then return false end
    self:reset()
    local index = UiContainer.BattleUi.camera_index or 0
    local px, py, pz, lx, ly, lz = battle_manager:get_camera(index)
    -- These scripts don't use any actor.
    local env = {formation = {pos = {px, py, pz}, look = {lx, ly, lz}}, actor = function() return nil end}
    self.camera = BattleCamera.new(position, look, env)
    self.camera_index = index
    print("Battle camera: victory, variant " .. tostring(variant) .. " file " .. tostring(file))
    return true
end

--- Starts one of the camera scripts that ff7.exe has (camera id -2): the ones of the start of the
-- battles, and the ones that the animation scripts of some enemies use. They go to a camera of the
-- formation, which they can change.
--
-- @param id Number of the script.
-- @return True if there is a camera.
function BattleCameraController:start_script(id)
    local data = Game.BattleCameras ~= nil and Game.BattleCameras.intro or nil
    if data == nil or id == nil or id < 0 then return false end
    local position = data.scripts[data.position[id + 1]]
    local look = data.scripts[data.look[id + 1]]
    if position == nil and look == nil then return false end
    local function formation_at(index)
        if index < 0 or index > 2 then return nil end
        local px, py, pz, lx, ly, lz = battle_manager:get_camera(index)
        return {pos = {px, py, pz}, look = {lx, ly, lz}}
    end
    local index = UiContainer.BattleUi.camera_index or 0
    local env = {formation = formation_at(index), formation_at = formation_at, actor = function() return nil end}
    self:reset()
    self.camera = BattleCamera.new(position, look, env)
    self.camera_index = index
    self.from_script = true
    return true
end

--- Starts the camera of the start of a battle: from some other place to a camera of the formation.
-- The formation chooses the script.
--
-- @return True if there is a camera.
function BattleCameraController:start_intro()
    self:reset()
    UiContainer.BattleUi.camera_index = 0
    local id = battle_manager:get_intro_camera()
    if not self:start_script(id) then return false end
    print("Battle camera: start of the battle, script " .. tostring(id))
    return true
end

--- Changes the camera of the formation the battle rests on, moving to it.
--
-- @param index 0 to 2.
function BattleCameraController:set_view(index)
    if index < 0 or index > 2 then return end
    local previous = UiContainer.BattleUi.camera_index or 0
    UiContainer.BattleUi.camera_index = index
    self.camera_index = index
    if self.camera ~= nil or previous == index then return end
    local px, py, pz, lx, ly, lz = battle_manager:get_camera(previous)
    local from = self.returning ~= nil and self.returning.to or nil
    local cam, look = from ~= nil and from[1] or {px, py, pz}, from ~= nil and from[2] or {lx, ly, lz}
    px, py, pz, lx, ly, lz = battle_manager:get_camera(index)
    self.returning = {target = {{px, py, pz}, {lx, ly, lz}}, cam = cam, look = look, index = index}
    self.accumulated = 0
end

--- Is a script of an animation still moving the camera? It goes on even with no action.
function BattleCameraController:is_script_running()
    return self.camera ~= nil and self.from_script == true and not self.camera:is_finished()
end

--- Moves the camera of the start of the battle, and leaves the one of the formation when it ends.
--
-- @param dt Seconds since the last call.
-- @return True while it goes on.
function BattleCameraController:update_intro(dt)
    if self.camera == nil then return false end
    self:update(dt)
    if not self.camera:is_finished() then return true end
    local index = self.camera_index or 0
    self:reset()
    battle_manager:set_camera(index)
    return false
end

--- Forgets the camera in use, without moving anything.
function BattleCameraController:reset()
    self.camera = nil
    self.from_script = false
    self.returning = nil
    self.last = nil
    self.previous = nil
    self.current = nil
    self.accumulated = 0
end

--- Stops the camera and goes back to the one of the formation.
--
-- The original doesn't jump: every tick the camera and the point it looks at cover half of the
-- distance that is left to the camera of the formation (function 0x5c64cc), until they get there.
function BattleCameraController:stop(restore)
    if self.camera == nil then return end
    self.camera = nil
    if restore == false or self.last == nil then
        print("Battle camera: stopped without easing (" .. tostring(restore) .. ", " .. tostring(self.last ~= nil) .. ")")
        self.returning = nil
        if restore ~= false then battle_manager:set_camera(self.camera_index or 0) end
        return
    end
    local index = self.camera_index or 0
    local px, py, pz, lx, ly, lz = battle_manager:get_camera(index)
    self.returning = {target = {{px, py, pz}, {lx, ly, lz}}, cam = self.last.cam, look = self.last.look, index = index}
    self.accumulated = 0
    print("Battle camera: going back to the camera " .. tostring(index))
end

--- Moves the camera. Call it every frame.
--
-- @param dt Seconds since the last call.
function BattleCameraController:update(dt)
    if self.returning ~= nil and self.camera == nil then
        local r = self.returning
        -- Every tick the points go half of the way (r.to is where they are after the last tick, and
        -- r.from where they were before), and between ticks they are drawn between the two.
        if r.from == nil then
            r.from = {{r.cam[1], r.cam[2], r.cam[3]}, {r.look[1], r.look[2], r.look[3]}}
            r.to = {{r.cam[1], r.cam[2], r.cam[3]}, {r.look[1], r.look[2], r.look[3]}}
        end
        self.accumulated = self.accumulated + dt * self.RETURN_TPS
        while self.accumulated >= 1 and self.returning ~= nil do
            self.accumulated = self.accumulated - 1
            local done = true
            for i = 1, 2 do
                for k = 1, 3 do
                    r.from[i][k] = r.to[i][k]
                    local diff = (r.target[i][k] - r.to[i][k]) / 2
                    r.to[i][k] = r.to[i][k] + diff
                    if math.abs(diff) > BattleCamera.UNIT then done = false end
                end
            end
            if done then
                self.returning = nil
                battle_manager:set_camera(r.index)
                print("Battle camera: back at the camera " .. tostring(r.index))
            end
        end
        if self.returning ~= nil then
            local f = self.accumulated
            local function mix(i, k) return r.from[i][k] + (r.to[i][k] - r.from[i][k]) * f end
            battle_manager:set_camera_position(mix(1, 1), mix(1, 2), mix(1, 3), mix(2, 1), mix(2, 2), mix(2, 3))
        end
        return
    end
    if self.camera == nil then return end
    self.accumulated = self.accumulated + dt * BattleCamera.TPS
    while self.accumulated >= 1 do
        self.accumulated = self.accumulated - 1
        self.camera:tick()
        -- A script chose another camera of the formation: the battle stays on it.
        local view = self.camera.formation_index
        if view ~= nil then
            self.camera_index = view
            UiContainer.BattleUi.camera_index = view
        end
        local cam, look = self.camera:get()
        self.previous = self.current
        self.current = {cam = {cam[1], cam[2], cam[3]}, look = {look[1], look[2], look[3]}}
    end
    if self.current == nil then return end
    local c, p, f = self.current, self.previous or self.current, self.accumulated
    local function mix(a, b) return {p[a][1] + (c[a][1] - p[a][1]) * f, p[a][2] + (c[a][2] - p[a][2]) * f, p[a][3] + (c[a][3] - p[a][3]) * f} end
    local cam, look = mix("cam"), mix("look")
    self.last = {cam = {cam[1], cam[2], cam[3]}, look = {look[1], look[2], look[3]}}
    battle_manager:set_camera_position(cam[1], cam[2], cam[3], look[1], look[2], look[3])
end
