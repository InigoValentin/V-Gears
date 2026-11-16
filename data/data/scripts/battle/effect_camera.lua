--- Cameras of the battle effects (summons).
--
-- The original game has a small script for the camera of each effect, with the opcodes of a
-- small interpreter inside ff7.exe (the Shiva one is at 0x58e57d). Two points move in the space
-- of the effect: the camera (stream 0x40) and the point it looks at (stream 0x80). The script is
-- a list of 16 bit words: the opcode in the low 6 bits and the stream in bit 0x40 or 0x80.
--
--   0, 9  wait N ticks (N)
--   1     set the position (x, y, z)
--   2     move to a position in N ticks, in a straight line (N, x, y, z)
--   3     orbit: the camera goes to a position, but it stays in the line of the look point, and
--         the distance to the look point changes smoothly (N, x, y, z)
--   4, 5  accelerated moves, played as the ones of opcode 2 and 3 (not used by Shiva)
--   6     two arguments, ignored. 8 one argument, ignored. 7 ends the script. A word of -1 ends
--         the script too.
--
-- The interpreter takes a new opcode when one of the two streams has nothing to do, then moves
-- both points one tick. Units are the ones of the original game (the scene ones are 0.0032 of them).
EffectCamera = {}
EffectCamera.__index = EffectCamera

-- Arguments of each opcode.
EffectCamera.ARGS = {[0] = 1, [1] = 3, [2] = 4, [3] = 4, [4] = 4, [5] = 5, [6] = 2, [7] = 0, [8] = 1, [9] = 1}

local function copy(v) return {v[1], v[2], v[3]} end
local function length(v) return math.sqrt(v[1] * v[1] + v[2] * v[2] + v[3] * v[3]) end

local function new_stream()
    return {pos = {0, 0, 0}, dur = 0, mode = 0, delta = {0, 0, 0}, start = {0, 0, 0}, dist = 0, dist_step = 0}
end

--- Creates the camera of a script.
--
-- @param words List of the 16 bit words of the script.
function EffectCamera.new(words)
    local self = setmetatable({}, EffectCamera)
    self.words = words
    self.index = 1
    self.finished = false
    self.cam = new_stream()
    self.look = new_stream()
    self.previous = {cam = copy(self.cam.pos), look = copy(self.look.pos)}
    return self
end

--- Runs the next opcode of the script.
function EffectCamera:fetch()
    local w = self.words[self.index]
    if w == nil or w == -1 then self.finished = true return end
    local op = w % 64
    local count = EffectCamera.ARGS[op]
    if count == nil then self.finished = true return end
    local a, b, c, d, e = self.words[self.index + 1], self.words[self.index + 2], self.words[self.index + 3],
      self.words[self.index + 4], self.words[self.index + 5]
    self.index = self.index + 1 + count
    local s, other
    if (w % 128) >= 64 then s, other = self.cam, self.look else s, other = self.look, self.cam end
    s.mode = op
    if op == 0 or op == 9 then
        s.dur = math.max(0, a or 0)
    elseif op == 1 then
        s.pos = {a, b, c}
        -- Until the camera is placed for the first time, the camera of the battle is the one in use.
        if s == self.cam then self.started = true end
    elseif op == 2 or op == 4 then
        local n = math.max(1, a)
        s.dur = n
        s.delta = {(b - s.pos[1]) / n, (c - s.pos[2]) / n, (d - s.pos[3]) / n}
        s.mode = 2
    elseif op == 3 or op == 5 then
        local n = math.max(1, a)
        local target = op == 3 and {b, c, d} or {c, d, e}
        s.dur = n
        s.mode = 3
        s.start = copy(s.pos)
        s.delta = {(target[1] - s.pos[1]) / n, (target[2] - s.pos[2]) / n, (target[3] - s.pos[3]) / n}
        local d0 = length({other.pos[1] - s.pos[1], other.pos[2] - s.pos[2], other.pos[3] - s.pos[3]})
        -- Where the look point will be at the end, if it's moving in a straight line.
        local final = copy(other.pos)
        if other.mode == 2 then
            for i = 1, 3 do final[i] = other.pos[i] + other.delta[i] * other.dur end
        end
        local d1 = length({target[1] - final[1], target[2] - final[2], target[3] - final[3]})
        s.dist = d0
        s.dist_step = (d1 - d0) / n
    elseif op == 7 then
        self.finished = true
    end
end

--- Moves one of the points one tick.
local function step_stream(s, other)
    if s.dur <= 0 then return end
    if s.mode == 2 then
        for i = 1, 3 do s.pos[i] = s.pos[i] + s.delta[i] end
    elseif s.mode == 3 then
        for i = 1, 3 do s.start[i] = s.start[i] + s.delta[i] end
        s.dist = s.dist + s.dist_step
        local dir = {other.pos[1] - s.start[1], other.pos[2] - s.start[2], other.pos[3] - s.start[3]}
        local l = length(dir)
        if l > 0 then
            for i = 1, 3 do s.pos[i] = other.pos[i] - dir[i] / l * s.dist end
        end
    end
    s.dur = s.dur - 1
end

--- Advances the camera one tick of the effect.
function EffectCamera:tick()
    self.previous = {cam = copy(self.cam.pos), look = copy(self.look.pos)}
    local was_started = self.started
    while not self.finished and (self.cam.dur <= 0 or self.look.dur <= 0) do self:fetch() end
    -- The camera is moved first, with the look point of the previous tick.
    step_stream(self.cam, self.look)
    step_stream(self.look, self.cam)
    -- The first position doesn't move from nowhere.
    if self.started and not was_started then
        self.previous = {cam = copy(self.cam.pos), look = copy(self.look.pos)}
    end
end

--- Gets the position of the camera and the look point.
--
-- @param fraction How far (0 to 1) we are from the last tick to the next one, to move smoothly.
-- @return Two tables with x, y, z in the units of the original game.
function EffectCamera:get(fraction)
    local cam, look = {}, {}
    for i = 1, 3 do
        cam[i] = self.previous.cam[i] + (self.cam.pos[i] - self.previous.cam[i]) * fraction
        look[i] = self.previous.look[i] + (self.look.pos[i] - self.previous.look[i]) * fraction
    end
    return cam, look
end

--- Gets a camera script.
--
-- The scripts are read from ff7.exe by the installer (gamedata/effect_cameras.lua), and they are
-- named by their address in the executable, like "0x8ae268" (Shiva, one target). Without them
-- (another version of the executable) the effects play with the camera of the battle.
--
-- @param name Name of the script.
-- @return The list of words, nil if there is no such script.
function EffectCamera.get_script(name)
    return Game ~= nil and Game.EffectCameras ~= nil and Game.EffectCameras[name] or nil
end
