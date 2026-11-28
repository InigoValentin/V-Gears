--- Interpreter of the enemy AI scripts of the original game.
--
-- The scripts are bytecode for a stack machine (see the opcode list below). Memory is addressed by
-- bit: addresses below 0x2000 are variables of the actor that owns the script, 0x2000 to 0x3FFF
-- are values of the battle shared by every actor and 0x4000 to 0x433F are values of each actor
-- (HP, MP, level, statuses...), selected with an actor mask.
--
-- The interpreter doesn't know about the battle: it asks a host for every global and actor value,
-- and notifies it of actions and texts. The host must have the functions:
--
--   host:read_global(address, size)         Value of a global address.
--   host:write_global(address, size, value)
--   host:read_actor(actor, field, size)     Value of an actor field (address minus 0x4000).
--   host:write_actor(actor, field, size, value)
--   host:action(owner, type, id)             An action was requested, see BattleAI.ACTION. The targets
--                                            are the last value written to the global 0x2070.
--   host.owner                               Actor running the script (set by the interpreter).
--   host:text(owner, bytes)                 A text has to be shown (FF7 encoded text).
--   host:mp_cost(id)                        MP cost of an attack.
--   host:memory_bank(write, offset, value)  Access to the game memory banks (1 and 2).
--
-- Value sizes are 0 (bit), 1 (byte), 2 (word) and 3 (dword).

BattleAI = {}

--- Types of actions requested with the action opcode.
BattleAI.ACTION = {
    ENEMY_ATTACK = 0x20,
    RUN_SCRIPT = 0x22,
    SHOW_TEXT = 0x24,
}

--- Slots of the scripts of an enemy.
BattleAI.SLOT = {
    INIT = 0, MAIN = 1, COUNTER_GENERAL = 2, COUNTER_DEATH = 3, COUNTER_PHYSICAL = 4,
    COUNTER_MAGIC = 5, BATTLE_END = 6, PRE_TURN = 7, EVENT_1 = 8,
}

--- Bytes used to store each value size.
BattleAI.SIZE_BYTES = {[0] = 1, 1, 2, 4}

--- Maximum number of instructions in a script run, to stop endless loops.
BattleAI.MAX_STEPS = 20000

--- Persistent variables (addresses 0x0000 to 0x03FF) of every actor, by actor index. They keep
-- their values between scripts.
BattleAI.variables = {}

--- Clears the persistent variables, at the beginning of a battle.
function BattleAI.reset()
    BattleAI.variables = {}
end

--- Gets the table of bytes with the variables of an actor.
local function variables_of(actor)
    local vars = BattleAI.variables[actor]
    if vars == nil then
        vars = {}
        BattleAI.variables[actor] = vars
    end
    return vars
end

--- Reads a value of memory with the byte array of the variables of an actor.
local function read_bytes(bytes, address, size)
    local byte_index = math.floor(address / 8)
    if size == 0 then
        return math.floor((bytes[byte_index] or 0) / 2 ^ (address % 8)) % 2
    end
    local value = 0
    for i = BattleAI.SIZE_BYTES[size] - 1, 0, -1 do
        value = value * 256 + (bytes[byte_index + i] or 0)
    end
    return value
end

--- Writes a value to the byte array of the variables of an actor.
local function write_bytes(bytes, address, size, value)
    local byte_index = math.floor(address / 8)
    if size == 0 then
        local bit = 2 ^ (address % 8)
        local current = bytes[byte_index] or 0
        local has = math.floor(current / bit) % 2
        if (value ~= 0) and has == 0 then current = current + bit
        elseif (value == 0) and has == 1 then current = current - bit
        end
        bytes[byte_index] = current
        return
    end
    value = math.floor(value)
    if value < 0 then value = value + 2 ^ 32 end
    for i = 0, BattleAI.SIZE_BYTES[size] - 1 do
        bytes[byte_index + i] = value % 256
        value = math.floor(value / 256)
    end
end

--- Counts the actors of a mask in order, calling a function with their indexes.
local function each_actor(mask, fn)
    mask = math.floor(mask)
    for actor = 0, 31 do
        if math.floor(mask / 2 ^ actor) % 2 == 1 then fn(actor) end
    end
end

--- Bitwise operations on non negative integers (Lua 5.1 has none).
local function band(a, b)
    local result, bit = 0, 1
    a, b = math.floor(a), math.floor(b)
    while a > 0 and b > 0 do
        if a % 2 == 1 and b % 2 == 1 then result = result + bit end
        a, b, bit = math.floor(a / 2), math.floor(b / 2), bit * 2
    end
    return result
end

local function bor(a, b)
    local result, bit = 0, 1
    a, b = math.floor(a), math.floor(b)
    while a > 0 or b > 0 do
        if a % 2 == 1 or b % 2 == 1 then result = result + bit end
        a, b, bit = math.floor(a / 2), math.floor(b / 2), bit * 2
    end
    return result
end

local function bnot(a, bits)
    return 2 ^ bits - 1 - math.floor(a)
end

BattleAI.band = band
BattleAI.bor = bor

--- The context of a script run: the owner, the host and the stack.
local Context = {}
Context.__index = Context

--- Reads the value of a memory address for an owner or an actor.
--
-- @param actor Actor whose actor fields or variables are read.
function Context:read(address, size, actor)
    if address < 0x2000 then
        return read_bytes(variables_of(actor), address, size)
    elseif address < 0x4000 then
        return self.host:read_global(address, size)
    end
    return self.host:read_actor(actor, address - 0x4000, size)
end

--- Writes a value to a memory address, of an actor or its variables.
function Context:write(address, size, value, actor)
    if address < 0x2000 then
        write_bytes(variables_of(actor), address, size, value)
    elseif address < 0x4000 then
        self.host:write_global(address, size, value)
    else
        self.host:write_actor(actor, address - 0x4000, size, value)
    end
end

--- Gets the number a stack entry stands for.
--
-- Addresses give their value, and entries that stand for several actors the value of the first.
function Context:value_of(entry)
    if entry.kind == "value" then return entry.v end
    if entry.kind == "address" then
        if entry.mask ~= nil then return self:first_value(entry) end
        return self:read(entry.address, entry.size, self.owner)
    end
    return self:first_value(entry)
end

--- Gets the value of the first actor of a masked entry.
function Context:first_value(entry)
    local result = 0
    for actor = 0, 31 do
        if math.floor(entry.mask / 2 ^ actor) % 2 == 1 then
            return self:read(entry.address, entry.size, actor)
        end
    end
    return result
end

--- Pushes a value.
function Context:push(entry)
    self.top = self.top + 1
    self.stack[self.top] = entry
end

--- Pops an entry from the stack.
function Context:pop()
    local entry = self.stack[self.top]
    if entry == nil then error("AI script: the stack is empty") end
    self.stack[self.top] = nil
    self.top = self.top - 1
    return entry
end

--- Pushes a plain number.
function Context:push_value(size, value)
    self:push({kind = "value", size = size, v = value})
end

--- Gets the actor mask an entry stands for (values are masks, addresses give their value).
function Context:mask_of(entry)
    return math.floor(self:value_of(entry))
end

--- Compares two entries with an operator, for the comparison opcodes.
--
-- If one of them has an actor mask, the result is the mask of the actors that pass the comparison.
-- Otherwise it's 1 or 0.
local function compare(op, a, b)
    if op == 0x40 then return a == b
    elseif op == 0x41 then return a ~= b
    elseif op == 0x42 then return a >= b
    elseif op == 0x43 then return a <= b
    elseif op == 0x44 then return a > b
    elseif op == 0x45 then return a < b
    end
    return false
end

function Context:compare(op, second, first)
    -- "first" is the first pop (the right operand), "second" the second pop.
    local left_masked = second.mask ~= nil and (second.kind == "multi" or second.kind == "address")
    local right_masked = first.mask ~= nil and (first.kind == "multi" or first.kind == "address")
    if not left_masked and not right_masked then
        return {kind = "value", size = 0, v = compare(op, self:value_of(second), self:value_of(first)) and 1 or 0}
    end
    local mask = left_masked and second.mask or first.mask
    if left_masked and right_masked then mask = band(second.mask, first.mask) end
    local passed = 0
    each_actor(mask, function(actor)
        local a = left_masked and self:read(second.address, second.size, actor) or self:value_of(second)
        local b = right_masked and self:read(first.address, first.size, actor) or self:value_of(first)
        if compare(op, a, b) then passed = passed + 2 ^ actor end
    end)
    return {kind = "value", size = 2, v = passed}
end

--- Runs a script.
--
-- @param code The bytecode, as a string.
-- @param owner Index of the actor that owns the script.
-- @param host The host of the interpreter.
-- @return True if it finished, false if it was stopped (error or too many instructions).
function BattleAI.run(code, owner, host)
    -- The host needs to know who is running the script to answer about "self".
    host.owner = owner
    local ctx = setmetatable({owner = owner, host = host, stack = {}, top = 0}, Context)
    local pc = 0
    local steps = 0
    local length = #code
    local function byte(i) return code:byte(i + 1) end
    local function word(i) return byte(i) + byte(i + 1) * 256 end
    while pc < length do
        steps = steps + 1
        if steps > BattleAI.MAX_STEPS then return false, "too many instructions" end
        local op = byte(pc)
        local hi, lo = math.floor(op / 16), op % 16
        if hi == 0 and lo <= 3 then
            -- Push the value at an address. Actor fields remember their address, so they can be
            -- masked with actors later.
            local address = word(pc + 1)
            ctx:push({kind = "value", size = lo, v = ctx:read(address, lo, owner), address = address})
            pc = pc + 3
        elseif hi == 1 and lo <= 3 then
            ctx:push({kind = "address", size = lo, address = word(pc + 1)})
            pc = pc + 3
        elseif op >= 0x60 and op <= 0x62 then
            local size = op - 0x5F
            local value = 0
            for i = size - 1, 0, -1 do value = value * 256 + byte(pc + 1 + i) end
            ctx:push_value(size, value)
            pc = pc + 1 + size
        elseif op >= 0x30 and op <= 0x37 then
            if op == 0x37 then
                local a = ctx:pop()
                ctx:push_value(2, bnot(ctx:value_of(a), 16))
            else
                local b = ctx:pop()
                local a = ctx:pop()
                local x, y = ctx:value_of(a), ctx:value_of(b)
                local r
                if op == 0x30 then r = x + y
                elseif op == 0x31 then r = x - y
                elseif op == 0x32 then r = x * y
                elseif op == 0x33 then r = y ~= 0 and math.floor(x / y) or 0
                elseif op == 0x34 then r = y ~= 0 and x % y or 0
                elseif op == 0x35 then r = band(x, y)
                else r = bor(x, y)
                end
                ctx:push_value(math.max(a.size or 2, b.size or 2, 2), r)
            end
            pc = pc + 1
        elseif op >= 0x40 and op <= 0x45 then
            local first = ctx:pop()
            local second = ctx:pop()
            ctx:push(ctx:compare(op, second, first))
            pc = pc + 1
        elseif op >= 0x50 and op <= 0x52 then
            if op == 0x52 then
                local a = ctx:pop()
                ctx:push_value(0, ctx:value_of(a) == 0 and 1 or 0)
            else
                local b = ctx:pop()
                local a = ctx:pop()
                local x, y = ctx:value_of(a) ~= 0, ctx:value_of(b) ~= 0
                local r
                if op == 0x50 then r = x and y else r = x or y end
                ctx:push_value(0, r and 1 or 0)
            end
            pc = pc + 1
        elseif op == 0x70 then
            -- Jump if the value is zero.
            local a = ctx:pop()
            if ctx:value_of(a) == 0 then pc = word(pc + 1) else pc = pc + 3 end
        elseif op == 0x71 then
            -- Jump if the value is different from the top of the stack (switch cases).
            local a = ctx:pop()
            local top = ctx.stack[ctx.top]
            if top == nil or ctx:value_of(a) ~= ctx:value_of(top) then pc = word(pc + 1) else pc = pc + 3 end
        elseif op == 0x72 then
            pc = word(pc + 1)
        elseif op == 0x73 then
            return true
        elseif op == 0x74 or op == 0x75 then
            ctx:pop()
            pc = pc + 1
        elseif op >= 0x80 and op <= 0x87 then
            if op == 0x80 then
                -- The first pop is masked by the second: an address or a value of an actor field
                -- is for those actors, two plain values are ANDed.
                local first = ctx:pop()
                local second = ctx:pop()
                local mask = ctx:mask_of(second)
                if first.kind == "address" then
                    ctx:push({kind = "address", size = first.size, address = first.address, mask = mask})
                elseif first.address ~= nil and (first.address >= 0x4000 or first.address < 0x2000) then
                    ctx:push({kind = "multi", size = first.size, address = first.address, mask = mask})
                else
                    ctx:push_value(math.max(first.size or 2, 2), band(ctx:value_of(first), mask))
                end
            elseif op == 0x81 then
                ctx:push_value(2, host:random(65536))
            elseif op == 0x82 then
                -- A random one of the bits that are on.
                local mask = ctx:mask_of(ctx:pop())
                local bits = {}
                each_actor(mask, function(actor) table.insert(bits, actor) end)
                if #bits == 0 then ctx:push_value(2, 0)
                else ctx:push_value(2, 2 ^ bits[host:random(#bits) + 1])
                end
            elseif op == 0x83 then
                local a = ctx:pop()
                local value = math.floor(ctx:value_of(a))
                if a.size == 1 or a.size == 0 then
                    local count = 0
                    each_actor(value, function() count = count + 1 end)
                    ctx:push_value(1, count)
                else
                    -- The first bit that is on, as a value with only that bit.
                    local result = 0
                    for bit = 0, 31 do
                        if math.floor(value / 2 ^ bit) % 2 == 1 then result = 2 ^ bit break end
                    end
                    ctx:push_value(2, result)
                end
            elseif op == 0x84 or op == 0x85 then
                -- The actors with the highest (or lowest) value of a masked entry.
                local a = ctx:pop()
                local best, mask = nil, 0
                if a.mask ~= nil then
                    each_actor(a.mask, function(actor)
                        local value = ctx:read(a.address, a.size, actor)
                        if best == nil or (op == 0x84 and value > best) or (op == 0x85 and value < best) then
                            best, mask = value, 2 ^ actor
                        elseif value == best then
                            mask = mask + 2 ^ actor
                        end
                    end)
                end
                ctx:push_value(2, mask)
            elseif op == 0x86 then
                local a = ctx:pop()
                ctx:push_value(2, host:mp_cost(math.floor(ctx:value_of(a))))
            else
                local a = ctx:pop()
                ctx:push_value(2, 2 ^ math.floor(ctx:value_of(a)))
            end
            pc = pc + 1
        elseif op == 0x90 then
            -- Store the value on top of the address below it. Entries with an actor mask store
            -- the value in every actor of the mask.
            local value = ctx:pop()
            local target = ctx:pop()
            if target.kind ~= "address" then
                -- Some scripts push the value first: swap if so.
                if value.kind == "address" then value, target = target, value end
            end
            if target.kind == "address" then
                local v = ctx:value_of(value)
                if target.mask ~= nil then
                    each_actor(target.mask, function(actor) ctx:write(target.address, target.size, v, actor) end)
                else
                    ctx:write(target.address, target.size, v, owner)
                end
            end
            pc = pc + 1
        elseif op == 0x91 then
            ctx:pop()
            pc = pc + 1
        elseif op == 0x92 then
            local id = ctx:pop()
            local kind = ctx:pop()
            host:action(owner, math.floor(ctx:value_of(kind)), math.floor(ctx:value_of(id)))
            pc = pc + 1
        elseif op == 0x93 then
            -- A text, in the FF7 encoding, finished with 0xFF.
            local stop = pc + 1
            while stop < length and byte(stop) ~= 0xFF do stop = stop + 1 end
            host:text(owner, code:sub(pc + 2, stop))
            pc = stop + 1
        elseif op == 0x94 then
            -- Copy the state of some actors to others. Not used by the battle yet.
            ctx:pop()
            ctx:pop()
            pc = pc + 1
        elseif op == 0x95 then
            -- Access to the memory banks 1 and 2 of the game: read or write, at an offset.
            local offset = ctx:pop()
            local write = ctx:pop()
            host:memory_bank(ctx:value_of(write) == 1, ctx:value_of(offset))
            pc = pc + 1
        elseif op == 0x96 then
            ctx:pop()
            ctx:pop()
            ctx:push_value(2, 0)
            pc = pc + 1
        elseif op == 0xA0 then
            -- Debug text, with one value for each "%d". Skipped.
            local stop = pc + 2
            while stop < length and byte(stop) ~= 0 do stop = stop + 1 end
            local text = code:sub(pc + 3, stop)
            for _ in text:gmatch("%%d") do ctx:pop() end
            pc = stop + 1
        elseif op == 0xA1 then
            ctx:pop()
            ctx:pop()
            pc = pc + 1
        else
            -- Anything else does nothing.
            pc = pc + 1
        end
    end
    return true
end

--- Runs a script protected from errors.
--
-- @return True if it finished correctly.
function BattleAI.run_safe(code, owner, host)
    local ok, result, reason = pcall(BattleAI.run, code, owner, host)
    if not ok then
        print("AI script error: " .. tostring(result))
        return false
    end
    if result == false then print("AI script stopped: " .. tostring(reason)) end
    return result
end
