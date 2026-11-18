--- Camera scripts of the battles (attacks, magic, enemy attacks).
--
-- The original game has a small script for the camera of each attack, in the files camdat0.bin,
-- camdat1.bin and camdat2.bin (the installer writes them to gamedata/battle_cameras.lua). Two points
-- move in a battle: the camera (position stream) and the point it looks at (look stream), each one
-- with its own script of bytes, found with (camera id * 3 + variant). The ids are the camera_single
-- and camera_multiple of the attacks. The interpreters are in ff7.exe (0x5c2341 and 0x5c3f45).
--
-- A script is a list of opcodes (0xd5-0xff, other values do nothing) with operands that are bytes or
-- 16 bit values (little endian, with sign). Positions are in the units of the original game, in a
-- frame with x to the side, y down and z forward (toward the enemies, for the party), and the ones
-- that follow an actor are relative to a point of it, the offsets turned with the way it faces if
-- the stream has "rotate" on. The frame of the scene is converted with
--
--   scene = (x, -z, -y) / 255      (a reflection: the original frame is left handed)
--
-- (the division is the one the installer uses for every coordinate of the battle). The streams run
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
    return x * k, -z * k, -y * k
end

--- Converts a position of the original frame (absolute) to the scene.
function BattleCamera:absolute(x, y, z)
    return vec(to_scene_offset(x, y, z))
end

--- Position of a point of an actor, plus an offset of the original frame.
--
-- @param rotate If the offset turns with the way the actor faces.
function BattleCamera:anchor(ref, point, ox, oy, oz, rotate)
    local actor = self.env:actor(ref)
    if actor == nil then return nil end
    -- The points of the original are bones. All of the used ones (0 and 10) are near the middle of
    -- the body, so they are the middle of the actor.
    local px, py, pz = actor.pos[1], actor.pos[2], actor.pos[3] + actor.height * 0.5
    local k = BattleCamera.UNIT
    local dx, dy, dz
    if rotate then
        -- x is to the left of the way it faces (reflection), z forward, y down.
        local fx, fy = actor.forward[1], actor.forward[2]
        dx = (-fy) * ox * k + fx * oz * k
        dy = fx * ox * k + fy * oz * k
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
        age = 0, rotate = rotate,
    }
end

--- Places a stream at a point of an actor, now.
function BattleCamera:place(stream, ref, point, ox, oy, oz, rotate)
    local p = self:anchor(ref, point, ox, oy, oz, rotate)
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
        elseif kind == "pos" and op == 0xf1 then
            stream.rotate = false
        elseif kind == "look" and op == 0xdb then
            stream.rotate = false
        elseif kind == "pos" and op == 0xda then
            stream.linear = false
        elseif kind == "pos" and op == 0xdb then
            stream.linear = true
        elseif op == 0xe1 or op == 0xd9 or op == 0xdd then
            -- The camera that was in use.
            if op == 0xdd then u8(stream) end
            local source = kind == "pos" and env.formation.pos or env.formation.look
            stream.pos = {source[1], source[2], source[3]}
        elseif op == 0xe2 then
            local source = kind == "pos" and env.formation.pos or env.formation.look
            start_move(stream, {source[1], source[2], source[3]}, u8(stream))
        elseif op == 0xe6 then
            local x, y, z = s16(stream), s16(stream), s16(stream)
            start_move(stream, self:absolute(x, y, z), u8(stream))
        elseif (kind == "pos" and op == 0xf9) or (kind == "look" and op == 0xfa) then
            local x, y, z = s16(stream), s16(stream), s16(stream)
            stream.pos = self:absolute(x, y, z)
        elseif op == 0xe3 or op == 0xd8 or op == 0xe4 or op == 0xe5 then
            -- Move to a point of an actor, with an offset: actor (e3 and d8), point, x, y, z, ticks.
            local ref
            if op == 0xe3 then ref = actor_from_byte(u8(stream))
            elseif op == 0xd8 then u8(stream) ref = "targets"
            elseif op == 0xe4 then ref = "attacker"
            else ref = "current" end
            local point = u8(stream)
            local x, y, z = s16(stream), s16(stream), s16(stream)
            local ticks = u8(stream)
            local target = self:anchor(ref, point, x, y, z, stream.rotate)
            if target ~= nil then start_move(stream, target, ticks) end
        elseif (kind == "pos" and (op == 0xe7 or op == 0xe9 or op == 0xeb))
          or (kind == "look" and (op == 0xe8 or op == 0xea or op == 0xec)) then
            -- Follow a point of an actor with an offset, for some ticks.
            local ref
            if op == 0xeb or op == 0xec then ref = actor_from_byte(u8(stream))
            elseif op == 0xe7 or op == 0xe8 then ref = "attacker"
            else ref = "current" end
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
            else ref = "current" end
            local point = u8(stream)
            local x, y, z = s16(stream), s16(stream), s16(stream)
            self:place(stream, ref, point, x, y, z, stream.rotate)
        else
            -- Opcodes that don't change the camera (events of the battle, the orbits f2 and f3, the
            -- moves to the data of an actor e0, 0xf8 of the position): skip their operands.
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
        else
            local p = self:anchor(task.ref, task.point, task.off[1], task.off[2], task.off[3], task.rotate)
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

--- Builds the environment of the camera for an action.
local function build_env(side, actor, target_side, targets, formation)
    local ui = UiContainer.BattleUi
    local function entity_of(s, index)
        local e = ui:get_entity(s, index)
        return e
    end
    -- Center of the targets (their entities) and of the attacker.
    local function center_of(s, list)
        local sx, sy, sz, n = 0, 0, 0, 0
        local height = 0
        for _, index in ipairs(list) do
            local e = entity_of(s, index)
            if e ~= nil then
                local x, y, z, h = entity_info(e)
                sx, sy, sz, n, height = sx + x, sy + y, sz + z, n + 1, math.max(height, h)
            end
        end
        if n == 0 then return nil end
        return {sx / n, sy / n, sz / n}, height
    end
    local attacker_entity = entity_of(side, actor)
    local attacker_pos, attacker_height
    if attacker_entity ~= nil then
        local x, y, z, h = entity_info(attacker_entity)
        attacker_pos, attacker_height = {x, y, z}, h
    end
    local target_pos, target_height = center_of(target_side, targets)
    local function direction(from, to)
        if from == nil or to == nil then return {0, -1} end
        local dx, dy = to[1] - from[1], to[2] - from[2]
        local l = math.sqrt(dx * dx + dy * dy)
        if l < 0.0001 then return {0, -1} end
        return {dx / l, dy / l}
    end
    local env = {formation = formation}
    function env:actor(ref)
        if ref == "attacker" then
            if attacker_pos == nil then return nil end
            return {pos = attacker_pos, height = attacker_height, forward = direction(attacker_pos, target_pos)}
        elseif ref == "current" or ref == "targets" then
            if target_pos == nil then return nil end
            if ref == "current" then
                -- The first target.
                local e = entity_of(target_side, targets[1])
                if e ~= nil then
                    local x, y, z, h = entity_info(e)
                    return {pos = {x, y, z}, height = h, forward = direction({x, y, z}, attacker_pos)}
                end
            end
            return {pos = target_pos, height = target_height, forward = direction(target_pos, attacker_pos)}
        elseif type(ref) == "table" then
            local e = entity_of(ref.side == "party" and Battle.TargetGroup.PARTY or Battle.TargetGroup.ENEMIES, ref.index)
            if e == nil then return nil end
            local x, y, z, h = entity_info(e)
            local other = ref.side == "party" and target_pos or attacker_pos
            return {pos = {x, y, z}, height = h, forward = direction({x, y, z}, other)}
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
