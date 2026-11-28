--[[
Stubs for the functions that the decompiled world map scripts (wm0.lua, wm2.lua and wm3.lua) call
and that are not implemented yet.

Each one does nothing and returns 0, and is reported the first time it is called, so the scripts
that use them keep running instead of failing with a Lua error. They are not an implementation:
the names come from the decompiler, and most of the ones called "kernel_unknown_*" have an unknown
purpose.

A function that is already defined somewhere else is never replaced.
]]

WorldMapStubs = WorldMapStubs or { reported = {} }

--- Creates the stub of a function.
-- @param name Name of the function.
-- @return The stub.
local function make_stub(name, quiet)
    return function(...)
        if not quiet and not WorldMapStubs.reported[name] then
            WorldMapStubs.reported[name] = true
            print("World map script function not implemented: " .. name)
        end
        return 0
    end
end

--- Bitwise and of two numbers. The scripts of the world map use it for flags, and the Lua of the
-- game has no bit library.
BitAnd = function(a, b)
    a = math.floor(a) % 4294967296
    b = math.floor(b) % 4294967296
    local result, bit = 0, 1
    for _ = 1, 32 do
        if a % 2 == 1 and b % 2 == 1 then result = result + bit end
        a = math.floor(a / 2)
        b = math.floor(b / 2)
        bit = bit * 2
    end
    return result
end

--- Bitwise or of two numbers.
BitOr = function(a, b)
    a = math.floor(a) % 4294967296
    b = math.floor(b) % 4294967296
    local result, bit = 0, 1
    for _ = 1, 32 do
        if a % 2 == 1 or b % 2 == 1 then result = result + bit end
        a = math.floor(a / 2)
        b = math.floor(b / 2)
        bit = bit * 2
    end
    return result
end

-- Functions of the decompiled scripts with a known name.
local stubbed_functions = {
    "Wait", "SetWaitFrames", "LoadModel", "SetActiveEntityDirectionAndFacing",
    "SetActiveEntityDirection", "SetActiveEntityMovespeed", "SetActiveEntityMeshCoordinates",
    "SetActiveEntityMeshCoordinatesInMesh", "SetLightMeshCoordinates",
    "SetLightMeshCoordinatesInMesh", "SetActivePoint", "WaitForWindowReady",
    "WaitForMessageAcknowledge", "SetWindowDimensions", "SetWindowMessage", "SetWindowParameters",
    "SetControlLock", "PlaySoundEffect", "EnterVehicle", "FadeOut", "FadeIn",
    "SetSubmarineColor", "SetChocoboType", "SetWorldProgress", "SetMeteorTexture", "Unknown333",
    "Unknown_AKAO", "SetBattleLock", "TriggerBattle", "UnknownFunction1B",
}
for _, name in ipairs(stubbed_functions) do
    if rawget(_G, name) == nil then
        -- The ones with a known name are only the things that the placement of the entities doesn't
        -- need (windows, waits, locks): not reported. The unknown ones are, to see what is missing.
        _G[name] = make_stub(name, true)
    end
end

-- The decompiler names the functions it can't identify "kernel_unknown_0x<opcode>". There are too
-- many to list, and a new installation can have more, so they are stubbed when they are used.
local previous_metatable = getmetatable(_G)
setmetatable(_G, {
    __index = function(table, key)
        if type(key) == "string" and key:match("^kernel_unknown_0x%x+$") then
            local stub = make_stub(key)
            rawset(table, key, stub)
            return stub
        end
        if previous_metatable and previous_metatable.__index then
            if type(previous_metatable.__index) == "function" then
                return previous_metatable.__index(table, key)
            end
            return previous_metatable.__index[key]
        end
        return nil
    end,
    __newindex = previous_metatable and previous_metatable.__newindex or nil,
})
