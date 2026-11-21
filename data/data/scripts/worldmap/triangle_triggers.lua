System.WorldMap = {
    triangle_triggers = {},
    triangle_functions = {},
    field_table = {},

    --[[
    Registers a callback for a WM0 mesh function.
    @param mesh_x World-map mesh X coordinate (0-35).
    @param mesh_y World-map mesh Y coordinate (0-27).
    @param function_id Triangle mesh function ID (0-4).
    @param callback Function called when the player enters the trigger.
    @return True when registered, false for invalid input.
    ]]
    register_triangle_trigger = function(self, mesh_x, mesh_y, function_id, callback)
        if type(mesh_x) ~= "number" or mesh_x < 0 or mesh_x > 35
          or type(mesh_y) ~= "number" or mesh_y < 0 or mesh_y > 27
          or type(function_id) ~= "number" or function_id < 0 or function_id > 4
          or type(callback) ~= "function" then
            return false
        end

        local mesh_triggers = self.triangle_triggers[mesh_y]
        if not mesh_triggers then
            mesh_triggers = {}
            self.triangle_triggers[mesh_y] = mesh_triggers
        end
        local function_triggers = mesh_triggers[mesh_x]
        if not function_triggers then
            function_triggers = {}
            mesh_triggers[mesh_x] = function_triggers
        end
        function_triggers[function_id] = callback
        return true
    end,

    --[[
    Dispatches an entered WM0 mesh function to its registered callback.
    @param self World-map system script table.
    @param mesh_index Linear mesh index (mesh_y * 36 + mesh_x).
    @param function_id Triangle mesh function ID.
    ]]
    on_triangle_enter = function(self, mesh_index, function_id)
        mesh_index = tonumber(mesh_index)
        function_id = tonumber(function_id)
        if not mesh_index or not function_id then
            return
        end

        local mesh_x = mesh_index % 36
        local mesh_y = math.floor(mesh_index / 36)
        local mesh_triggers = self.triangle_triggers[mesh_y]
        local function_triggers = mesh_triggers and mesh_triggers[mesh_x]
        local callback = function_triggers and function_triggers[function_id]
        if callback then
            callback(mesh_x, mesh_y, function_id)
            return
        end

        local mesh_functions = self.triangle_functions[mesh_index]
        local function_index = mesh_functions and mesh_functions[function_id]
        local world_function = function_index
          and self["world_function_" .. tostring(function_index)]
        if world_function then
            -- A place can only be entered by the vehicles that the settings say (WorldMapSettings.entry).
            local record = WorldVM and WorldVM.find_field_entry(world_function)
            if record and not WorldVM.can_enter(record, self.vehicle, self.landing) then
                return
            end
            world_function(self)
        else
            print(
                "No world-map function for mesh " .. mesh_index
                .. ", function " .. function_id
            )
        end
    end,
}

--[[
Memory of the world map scripts (Read, ReadWord, ReadBit and their Write functions).

The variables of the game are the savemap from 0xBA4, the same as the memory banks of the field
scripts: the address (or the bit, for ReadBit) goes through 256 bytes for each pair of banks, 1 and 2,
3 and 4, 11 and 12, 13 and 14, and 7 and 15. The first one of the pair is the one for bytes and bits
(Banks[1][127] and the bit 3 of it, BitBanks[1][127][3], are Read(127) and ReadBit(1019)), and the second
one the one for words (Banks[2][0], the story counter, is ReadWord(0)). That way what the fields give
(the vehicles, the progress of the story) is what the world has.

ReadLocal and ReadSpecial are not game variables: the first is the memory of the scripts and the second
what the engine gives them (the way in which the world was entered, in System.WorldMap.specials: 5 is 0
when entering from a field and 6 the FIELD.TBL record of the exit).
]]
local BYTE_BANKS = {[0] = 1, 3, 11, 13, 7}
local WORD_BANKS = {[0] = 2, 4, 12, 14, 15}

--- Journal of the writes, when System.WorldMap.journal is a table: WorldVM undoes the changes of the
-- functions that it runs only to see what they do.
local function remember(container, key)
    local journal = System.WorldMap.journal
    if journal then journal[#journal + 1] = {container, key, container[key]} end
end

--- Where a byte address of the game variables is.
-- @return The bank for bytes, the one for words and the address in them. nil if it is not in a bank.
local function locate(address)
    local index = math.floor(address / 256)
    if index < 0 or index > 4 then return nil end
    return BYTE_BANKS[index], WORD_BANKS[index], address % 256
end

--- Reads a byte of the game variables.
Read = function(address)
    local bank, _, offset = locate(address)
    return bank and Banks[bank][offset] or 0
end

--- Reads a word of the game variables.
ReadWord = function(address)
    local _, bank, offset = locate(address)
    return bank and Banks[bank][offset] or 0
end

--- Reads a bit of the game variables, as the address in bits.
ReadBit = function(bit_address)
    local bank, _, offset = locate(math.floor(bit_address / 8))
    return bank and BitBanks[bank][offset][bit_address % 8] or 0
end

--- Writes a byte of the game variables.
Write = function(address, value)
    local bank, _, offset = locate(address)
    if not bank then return end
    remember(Banks[bank], offset)
    Banks[bank][offset] = value
end

--- Writes a word of the game variables.
WriteWord = function(address, value)
    local _, bank, offset = locate(address)
    if not bank then return end
    remember(Banks[bank], offset)
    Banks[bank][offset] = value
end

--- Writes a bit of the game variables, as the address in bits.
WriteBit = function(bit_address, value)
    local bank, _, offset = locate(math.floor(bit_address / 8))
    if not bank then return end
    remember(BitBanks[bank][offset], bit_address % 8)
    BitBanks[bank][offset][bit_address % 8] = value ~= 0 and 1 or 0
end

System.WorldMap.locals = {}
System.WorldMap.specials = {}

--- Reads a value of the memory of the scripts.
ReadLocal = function(address)
    return System.WorldMap.locals[address] or 0
end

--- Writes a value of the memory of the scripts.
WriteLocal = function(address, value)
    System.WorldMap.locals[address] = value
end

--- Reads a value that the engine gives to the scripts.
ReadSpecial = function(address)
    return System.WorldMap.specials[address] or 0
end

--- Writes a value that the engine gives to the scripts.
WriteSpecial = function(address, value)
    System.WorldMap.specials[address] = value
end

--- The map of the module (0 WM0, 1 WM2, 2 WM3) that a field exit to the world map goes to.
-- It is what the game does with the record: some of them are the underwater map (the submarine, 30, 31,
-- 42 and 56) and from 60 on the snowstorm one.
-- @param point_name Spawn point name of the exit (it has the coordinates).
-- @param record FIELD.TBL record of the exit.
-- @return The map.
local function world_map_for_record(record)
    if record == 30 or record == 31 or record == 42 or record == 56 then return 1 end
    if record >= 60 then return 2 end
    return 0
end

--- Goes from a field to the world map by a field exit.
-- @param record FIELD.TBL record of the exit. The scripts of the world know where the player and the
-- vehicles are by it.
enter_world_map = function(record, point_name)
    -- The "wm<N>" of the fields is one less than the record that the scripts of the world use (the exit of
    -- Fort Condor is wm5 and its record is 6, the one of the submarine station wm41 and its record 42).
    record = record + 1
    -- Every field script starts with EntityContainer = {}, which deletes the scripts of the vehicles.
    local vehicles = loadfile("./data/scripts/worldmap/vehicles.lua")
    if vehicles then vehicles() else print("Could not reload the scripts of the vehicles") end
    System.WorldMap.arrival = record
    System.WorldMap.arrival_name = point_name
    console("camera_free true")
    MenuSettings.pause_available = true
    world_map_module:enter(world_map_for_record(record))
end

EnterFieldScene = function(field_table_id, scenario)
    local field_table = System.WorldMap.field_table[tonumber(field_table_id)]
    local entry = field_table and field_table[tonumber(scenario)]
    if not entry then
        print("No installed field entry for FIELD.TBL record " .. tostring(field_table_id))
        return 0
    end

    System.MapChanger.world_map_entry = entry
    console("camera_free false")
    load_field_map_request(entry.field, "")
    return 0
end