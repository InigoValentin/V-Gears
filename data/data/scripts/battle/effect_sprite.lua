--- Sprites of the battle effects, made from the sprite files (.SPT) of the original game.
--
-- A sprite file is a list of frames, and a frame a list of quads (Game.EffectAnimations, written by
-- the installer). The quads are not billboards: they lie on a plane of the space of the effect, that
-- the effect rotates with angles (a spinning lightning is a quad that rotates around the vertical
-- axis). The corners of a quad are in units of 8 (the draw function, 0x598a7a, shifts them 3 bits).
--
-- Quads are drawn with dynamic meshes, one for every texture, that are filled again every frame.
EffectSprite = {}

--- Gets the frames of a sprite file.
--
-- @param name Name of the file, like "summon_lamu_jo_c04".
-- @return List of frames, nil if it isn't installed.
function EffectSprite.get_frames(name)
    return Game ~= nil and Game.EffectAnimations ~= nil and Game.EffectAnimations[name] or nil
end

--- Rotates a point, with angles in degrees around X, then Y, then Z.
function EffectSprite.rotate(x, y, z, ax, ay, az)
    if ax ~= 0 then
        local c, s = math.cos(math.rad(ax)), math.sin(math.rad(ax))
        y, z = y * c - z * s, y * s + z * c
    end
    if ay ~= 0 then
        local c, s = math.cos(math.rad(ay)), math.sin(math.rad(ay))
        x, z = x * c + z * s, -x * s + z * c
    end
    if az ~= 0 then
        local c, s = math.cos(math.rad(az)), math.sin(math.rad(az))
        x, y = x * c - y * s, x * s + y * c
    end
    return x, y, z
end

EffectSpriteBatch = {}
EffectSpriteBatch.__index = EffectSpriteBatch

--- Creates a batch of quads that share a texture.
--
-- @param texture Name of the image.
-- @param width Width of the texture of the original game, in pixels.
-- @param height Height of the texture.
-- @param blend 1 for additive, 2 for alpha blending, 0 for opaque.
function EffectSpriteBatch.new(texture, width, height, blend)
    local name = battle_manager:create_dynamic_mesh(texture, blend or 1)
    if name == nil or name == "" then return nil end
    local self = setmetatable({}, EffectSpriteBatch)
    self.name, self.width, self.height = name, width, height
    self.list, self.count = {}, 0
    return self
end

--- Adds a quad.
--
-- @param c1 Corner of the top left, a table with x, y, z in the scene.
-- @param c2 Top right, c3 bottom left and c4 bottom right.
-- @param u0 Position in the texture (pixels): left, v0 top, u1 right and v1 bottom.
-- @param colour Table with r, g, b, a (0 to 1).
function EffectSpriteBatch:add_quad(c1, c2, c3, c4, u0, v0, u1, v1, colour)
    local list, n = self.list, self.count
    local w, h = self.width, self.height
    local function vertex(c, u, v)
        list[n + 1], list[n + 2], list[n + 3] = c[1], c[2], c[3]
        list[n + 4], list[n + 5] = u / w, v / h
        list[n + 6], list[n + 7], list[n + 8], list[n + 9] = colour[1], colour[2], colour[3], colour[4]
        n = n + 9
    end
    vertex(c1, u0, v0) vertex(c2, u1, v0) vertex(c3, u0, v1)
    vertex(c2, u1, v0) vertex(c4, u1, v1) vertex(c3, u0, v1)
    self.count = n
end

--- Adds a triangle with colours (no texture).
--
-- @param c1 First corner, a table with x, y, z in the scene. c2 and c3 the others.
-- @param colour Table with r, g, b, a (0 to 1).
function EffectSpriteBatch:add_triangle(c1, c2, c3, colour)
    local list, n = self.list, self.count
    for _, c in ipairs({c1, c2, c3}) do
        list[n + 1], list[n + 2], list[n + 3] = c[1], c[2], c[3]
        list[n + 4], list[n + 5] = 0, 0
        list[n + 6], list[n + 7], list[n + 8], list[n + 9] = colour[1], colour[2], colour[3], colour[4]
        n = n + 9
    end
    self.count = n
end

--- Shows the quads added since the last time, and starts again.
function EffectSpriteBatch:flush()
    -- The list can have vertices of a previous frame at the end.
    for i = self.count + 1, #self.list do self.list[i] = nil end
    battle_manager:set_dynamic_mesh(self.name, self.list)
    self.count = 0
end

--- Removes the mesh of the batch.
function EffectSpriteBatch:remove()
    if self.name ~= nil then battle_manager:remove_dynamic_mesh(self.name) end
    self.name = nil
end
