--- Meshes of the battle effects that change shape, like the ground of Ifrit or the mountain of Ramuh.
--
-- The vertices of the original game are in ff7.exe: a mesh has the vertices of its first shape and
-- a list of key shapes. The vertices move toward a key shape in a straight line over some ticks,
-- one step per tick, and the triangles don't change. There are two ways to say when each key is
-- reached:
--   * every `key_ticks` ticks the next key starts (Ifrit, function 0x594022), or
--   * a schedule, a list of {key, ticks, hold}: move to the key (a number of the list of keys, from
--     1) in `ticks` ticks and wait `hold` ticks (Ramuh, function 0x598f74, with the durations of
--     the table at 0x8ba970).
--
-- The data is written by the installer to game/battle/effects/effect_meshes.lua (Game.EffectMeshes) and drawn
-- with the dynamic meshes of the battle manager: one for the triangles with the texture and one for
-- the ones with colours at the vertices and no texture.
EffectMesh = {}
EffectMesh.__index = EffectMesh

--- Gets the data of a mesh.
--
-- @param name Name of the mesh, like "ifrit_ground".
-- @return The data, nil if it isn't installed.
function EffectMesh.get_data(name)
    return Game ~= nil and Game.EffectMeshes ~= nil and Game.EffectMeshes[name] or nil
end

--- Divides like the original code: whole numbers, rounding toward zero.
local function divide(a, b)
    if a >= 0 then return math.floor(a / b) end
    return -math.floor(-a / b)
end

--- Gets the moves of a mesh from the durations of the executable (function 0x598f74 of Ramuh).
--
-- The function goes through the words T, starting at 0 with nothing to wait: when the wait is over it
-- takes T[i] as the new wait and, if i is even and the wait is not 0, aims at the shape i / 2 for that
-- time, and then i goes up. The shape moves only while i is odd (the odd words are the waits between
-- moves) and the effect ends when i gets to 8.
--
-- @param tail The words (16 are kept by the installer).
-- @return List of {start, ticks, key} (key from 1).
function EffectMesh.schedule_from_tail(tail)
    local moves = {}
    local i, remaining, time = 0, 0, 0
    while true do
        while remaining == 0 do
            if i >= 8 then return moves end
            remaining = tail[i + 1]
            if remaining ~= 0 and i % 2 == 0 then
                moves[#moves + 1] = {start = time, ticks = remaining, key = math.floor(i / 2) + 1}
            end
            i = i + 1
        end
        time = time + 1
        remaining = remaining - 1
    end
end

--- Creates a mesh.
--
-- @param data Data of the mesh (see EffectMesh.get_data).
-- @param params Table with: texture (name of the image), key_ticks (ticks between key shapes, if there
-- is no schedule), schedule (list of {key, ticks, hold}), blend (0 opaque, 1 additive, 2 alpha) and uv (position and size of the image in
-- the texture page of the original game, {u, v, w, h}, 0, 0, 128, 128 by default).
-- @return The mesh, nil if it couldn't be created.
function EffectMesh.new(data, params)
    local name = battle_manager:create_dynamic_mesh(params.texture or "", params.blend or 0)
    if name == nil or name == "" then return nil end
    local self = setmetatable({}, EffectMesh)
    self.data = data
    self.name = name
    self.key_ticks = params.key_ticks or 4
    local uv = params.uv or {}
    self.uv_u, self.uv_v, self.uv_w, self.uv_h = uv.u or 0, uv.v or 0, uv.w or 128, uv.h or 128
    -- Triangles with colours at the vertices have a mesh of their own, with no texture.
    if data.gouraud ~= nil and #data.gouraud > 0 then
        self.gouraud_name = battle_manager:create_dynamic_mesh("", 0)
    end
    -- The schedule is a list of moves with the tick they start.
    if params.schedule == "tail" then
        -- The durations are the 16 words that follow the table of key shapes in the executable.
        self.moves = EffectMesh.schedule_from_tail(data.tail)
    elseif params.schedule ~= nil then
        self.moves = {}
        local start = 0
        for _, step in ipairs(params.schedule) do
            self.moves[#self.moves + 1] = {start = start, ticks = step.ticks, key = step.key}
            start = start + step.ticks + (step.hold or 0)
        end
    end
    self.current = {}
    self.delta = {}
    for i = 1, #data.vertices do
        self.current[i] = data.vertices[i]
        self.delta[i] = 0
    end
    self.ticks = 0
    return self
end

--- Sets the step of every vertex to reach a key shape in some ticks.
function EffectMesh:aim(key, ticks)
    local target = self.data.keys[key]
    for i = 1, #self.current do
        self.delta[i] = divide(target[i] - self.current[i], ticks)
    end
end

--- Moves the vertices until the mesh has done a number of ticks.
--
-- @param ticks Ticks since the mesh started.
function EffectMesh:advance(ticks)
    while self.ticks < ticks do
        local t = self.ticks
        local moving = false
        if self.moves ~= nil then
            for _, move in ipairs(self.moves) do
                if t == move.start then self:aim(move.key, move.ticks) end
                if t >= move.start and t < move.start + move.ticks then moving = true end
            end
        elseif t < #self.data.keys * self.key_ticks then
            if t % self.key_ticks == 0 then self:aim(math.floor(t / self.key_ticks) + 1, self.key_ticks) end
            moving = true
        end
        if moving then
            for i = 1, #self.current do self.current[i] = self.current[i] + self.delta[i] end
        end
        self.ticks = t + 1
    end
end

--- Draws the mesh.
--
-- @param to_scene Function that gets the position of a point of the mesh in the effect and
-- returns it in the scene (x, y, z).
-- @param scale Size of the mesh (1 is the original one): a number, or a table with the factor of x, y
-- and z (negative to flip an axis).
-- @param brightness Factor of the grey level of the textured triangles (1 by default).
function EffectMesh:draw(to_scene, scale, brightness)
    local current = self.current
    local sx, sy, sz = scale, scale, scale
    if type(scale) == "table" then sx, sy, sz = scale[1], scale[2], scale[3] end
    brightness = brightness or 1
    local function position(vertex)
        local v = vertex * 3
        return to_scene(current[v + 1] * sx, current[v + 2] * sy, current[v + 3] * sz)
    end
    -- Triangles with texture. Their grey level is 128 for the original colour.
    local triangles = self.data.triangles
    local out, n = {}, 0
    for t = 0, math.floor(#triangles / 10) - 1 do
        local b = t * 10
        local grey = triangles[b + 10] / 128 * brightness
        for k = 0, 2 do
            local x, y, z = position(triangles[b + 1 + k])
            out[n + 1], out[n + 2], out[n + 3] = x, y, z
            out[n + 4] = (triangles[b + 4 + 2 * k] - self.uv_u) / self.uv_w
            out[n + 5] = (triangles[b + 5 + 2 * k] - self.uv_v) / self.uv_h
            out[n + 6], out[n + 7], out[n + 8], out[n + 9] = grey, grey, grey, 1
            n = n + 9
        end
    end
    battle_manager:set_dynamic_mesh(self.name, out)
    -- Triangles with colours.
    if self.gouraud_name ~= nil then
        local g = self.data.gouraud
        out, n = {}, 0
        for t = 0, math.floor(#g / 12) - 1 do
            local b = t * 12
            for k = 0, 2 do
                local x, y, z = position(g[b + 1 + k])
                out[n + 1], out[n + 2], out[n + 3] = x, y, z
                out[n + 4], out[n + 5] = 0, 0
                out[n + 6], out[n + 7], out[n + 8], out[n + 9] = g[b + 4 + 3 * k] / 255, g[b + 5 + 3 * k] / 255, g[b + 6 + 3 * k] / 255, 1
                n = n + 9
            end
        end
        battle_manager:set_dynamic_mesh(self.gouraud_name, out)
    end
end

--- Speed of the shards in the first tick for each unit of the cross product of two edges: 0x60 / 4096
-- (the code of the executable multiplies by 0x60 and shifts 12 bits, function 0x591455).
EffectMesh.SHATTER_SPEED = 96 / 4096

--- Draws the shards of a mesh that breaks (the ice of Shiva, function 0x591e14 of the executable).
--
-- Only the triangles of the first list (data.shards of them) are kept; the quads disappear when the
-- ice breaks. Every triangle goes along its normal (the cross product of two of its edges, divided
-- by 4096 and multiplied by 96 and with the sign changed: that is its speed in the first tick) and it loses an eighth of
-- the speed every tick, so after n ticks it has moved speed * (1 - 0.875 ^ n) / 0.125.
--
-- @param to_scene Function (x, y, z) -> scene, as in draw, for the points after the turn and the flip.
-- @param scale Size of the mesh.
-- @param brightness Factor of the grey level.
-- @param rotation Degrees around X, Y and Z, nil for none (the turn of the whole mesh).
-- @param ticks Ticks since it started to break, nil if it is whole.
function EffectMesh:draw_shatter(to_scene, scale, brightness, rotation, ticks)
    local data = self.data
    local verts = data.vertices
    local triangles = data.triangles
    local count = math.floor(#triangles / 10)
    local limit = (ticks ~= nil and data.shards ~= nil) and data.shards or count
    local factor = 0
    if ticks ~= nil then factor = (1 - 0.875 ^ ticks) / 0.125 end
    local out, n = {}, 0
    for t = 0, limit - 1 do
        local b = t * 10
        local ids = {triangles[b + 1], triangles[b + 2], triangles[b + 3]}
        local dx, dy, dz = 0, 0, 0
        if ticks ~= nil then
            local function vertex(i) return verts[i * 3 + 1], verts[i * 3 + 2], verts[i * 3 + 3] end
            local x0, y0, z0 = vertex(ids[1])
            local x1, y1, z1 = vertex(ids[2])
            local x2, y2, z2 = vertex(ids[3])
            local e1x, e1y, e1z, e2x, e2y, e2z = x1 - x0, y1 - y0, z1 - z0, x2 - x1, y2 - y1, z2 - z1
            local cx, cy, cz = e1y * e2z - e1z * e2y, e1z * e2x - e1x * e2z, e1x * e2y - e1y * e2x
            local speed = EffectMesh.SHATTER_SPEED
            dx, dy, dz = -cx * speed * factor, -cy * speed * factor, -cz * speed * factor
        end
        local grey = triangles[b + 10] / 128 * brightness
        for k = 0, 2 do
            local v = ids[k + 1] * 3
            local x, y, z = verts[v + 1] + dx, verts[v + 2] + dy, verts[v + 3] + dz
            if rotation ~= nil then x, y, z = EffectSprite.rotate(x, y, z, rotation[1], rotation[2], rotation[3]) end
            -- The effect is seen from the other side than the data (a turn of 180 degrees around the
            -- vertical axis).
            local px, py, pz = to_scene(-x * scale, y * scale, -z * scale)
            out[n + 1], out[n + 2], out[n + 3] = px, py, pz
            out[n + 4] = (triangles[b + 4 + 2 * k] - self.uv_u) / self.uv_w
            out[n + 5] = (triangles[b + 5 + 2 * k] - self.uv_v) / self.uv_h
            out[n + 6], out[n + 7], out[n + 8], out[n + 9] = grey, grey, grey, 1
            n = n + 9
        end
    end
    battle_manager:set_dynamic_mesh(self.name, out)
end

--- Removes the mesh.
function EffectMesh:remove()
    if self.name ~= nil then battle_manager:remove_dynamic_mesh(self.name) end
    if self.gouraud_name ~= nil then battle_manager:remove_dynamic_mesh(self.gouraud_name) end
    self.name, self.gouraud_name = nil, nil
end
