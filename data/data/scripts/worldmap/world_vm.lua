--[[
The part of the world map script machine that places the models.

The decompiled scripts of the world map (wm0.lua, wm2.lua and wm3.lua) have a function for each type
of entity that runs when the entity is created ("event 0" of the type), and sets where it is with
SetActiveEntityMeshCoordinates, SetActiveEntityMeshCoordinatesInMesh and
SetActiveEntityDirectionAndFacing. The installer writes which function is it in
System.WorldMap.entity_functions[type][event].

Here these three functions and LoadModel only store what the scripts ask, and WorldVM.place_models runs
the functions of the types that have a simple one, and the models are created in the module with the
positions that it got. Variables that the scripts read (Read) are 0 for now, as there is no savemap
for them.
]]

WorldVM = WorldVM or {}

--- Types of entities whose "event 0" function only sets the position, and that are not the
-- vehicles. The functions of the other types run windows and wait for them, or depend on the story.
WorldVM.static_types = {
    [7] = true, [12] = true, [14] = true, [15] = true, [16] = true, [17] = true, [18] = true,
    [20] = true, [24] = true, [25] = true, [26] = true, [28] = true,
    -- The weapons. Their functions set the position by the story variables and don't wait.
    [10] = true, [11] = true, [29] = true, [30] = true,
}

--- Situation of the models (the one of WorldMapModels.situations) that is used in the maps that don't
-- depend on the story: the underwater one and the snowstorm one (WM0 has WorldVM.get_situation).
WorldVM.default_situation = {[0] = 0, [1] = 11, [2] = 12}

--- The situation of the models of WM0, from the story. It is the function 0x75cd4f of ff7.exe: the story
-- counter (ReadWord(0)) and two flags of the savemap (the bits 976 and 7220) give a
-- stage from 0 to 8, which is the situation (0 the start, 1 to 4 the Diamond Weapon, 5 and 6 the
-- Ultimate Weapon, 7 and 8 the Ruby Weapon). The game also uses 9 and 10 (the Warship and the cannon)
-- when it enters the world from some fields of Midgar, which is not handled.
-- @return The situation.
WorldVM.get_story_situation = function()
    local story = ReadWord(0)
    if story < 1000 then return 0 end
    local late = story >= 1620 and 1 or 0
    if story < 1580 then
        -- The bit 976 is bit 0 of the byte 122 (the cannon is gone, as the world scripts use it).
        return 1 + ReadBit(976) + late * 2
    end
    -- The bit 7220 is bit 4 of the byte 902.
    return 5 + late + ReadBit(7220) * 2
end

--- Writes the variables of the story that the debug data has (WorldMapDebug.variables), for testing.
WorldVM.apply_debug_variables = function()
    local variables = WorldMapDebug and WorldMapDebug.variables
    if not variables then return end
    for address, value in pairs(variables.byte or {}) do Write(address, value) end
    for address, value in pairs(variables.word or {}) do WriteWord(address, value) end
    for address, value in pairs(variables.bit or {}) do WriteBit(address, value) end
    for address, value in pairs(variables.special or {}) do WriteSpecial(address, value) end
end

--- Gets the situation of the models of a map.
-- WorldMapDebug.situation[map] has priority, for testing.
-- @param map_id Map of the module (0 WM0, 1 WM2, 2 WM3).
-- @return The situation.
WorldVM.get_situation = function(map_id)
    WorldVM.apply_debug_variables()
    local debug_situation = WorldMapDebug and WorldMapDebug.situation and WorldMapDebug.situation[map_id]
    if debug_situation then return debug_situation end
    if map_id == 0 then return WorldVM.get_story_situation() end
    return WorldVM.default_situation[map_id]
end

--- Types of the vehicles of WM0, by the name of their entity.
WorldVM.vehicle_types = {Highwind = 3, Chocobo = 4, TinyBronco = 5, Buggy = 6, Submarine = 13}

--- Size of a mesh in the world, in the units of the module (8182 is the size in the fixed point of
-- the game, and 4096 is 1).
-- The exits of the fields save the world coordinates in units of 8 (to fit 16 bits). To be checked in game.
WorldVM.EXIT_SCALE = 8
WorldVM.MESH_SIZE = 8182 / 4096

--- The position that the script gave to the entity that it is placing.
WorldVM.current = nil

LoadModel = function(model_type)
    return 0
end

SetActiveEntityMeshCoordinates = function(mesh_x, mesh_y)
    if WorldVM.current then
        WorldVM.current.mesh_x = mesh_x
        WorldVM.current.mesh_y = mesh_y
    end
    return 0
end

SetActiveEntityMeshCoordinatesInMesh = function(x, y)
    if WorldVM.current then
        WorldVM.current.in_x = x
        WorldVM.current.in_y = y
    end
    return 0
end

SetActiveEntityDirectionAndFacing = function(direction)
    if WorldVM.current then WorldVM.current.direction = direction end
    return 0
end

--- Runs something that only looks at what a script does, and undoes what it wrote to the variables
-- (it marks that it showed a window...).
-- @param action Function to run.
-- @return What the function returns.
WorldVM.run_sandboxed = function(action)
    local journal = {}
    local previous_journal = System.WorldMap.journal
    local saved_locals = System.WorldMap.locals
    local saved_specials = {}
    for address, value in pairs(System.WorldMap.specials) do saved_specials[address] = value end
    System.WorldMap.journal = journal
    System.WorldMap.locals = {}
    local a, b, c = action()
    for index = #journal, 1, -1 do
        local entry = journal[index]
        entry[1][entry[2]] = entry[3]
    end
    System.WorldMap.journal = previous_journal
    System.WorldMap.locals = saved_locals
    System.WorldMap.specials = saved_specials
    return a, b, c
end

--- Runs the function that creates an entity of a type, and gets where it puts it.
-- @param model_type Type of the entity.
-- @return A table with x and z (world units) and direction (0 to 255), nil if the type has no function
-- or it didn't set a position.
WorldVM.get_spawn = function(model_type)
    local functions = System.WorldMap.entity_functions
    local function_id = functions and functions[model_type] and functions[model_type][0]
    local script_function = function_id and System.WorldMap["world_function_" .. function_id]
    if not script_function then return nil end
    local spawn, ok, message = WorldVM.run_sandboxed(function()
        WorldVM.current = {}
        local call_ok, call_message = pcall(script_function, System.WorldMap)
        local result = WorldVM.current
        WorldVM.current = nil
        return result, call_ok, call_message
    end)
    if not ok then
        print("World map entity function of type " .. model_type .. " failed: " .. tostring(message))
        return nil
    end
    if not (spawn.mesh_x and spawn.mesh_y and spawn.in_x and spawn.in_y) then return nil end
    return {
        x = spawn.mesh_x * WorldVM.MESH_SIZE + spawn.in_x / 4096,
        z = spawn.mesh_y * WorldVM.MESH_SIZE + spawn.in_y / 4096,
        direction = spawn.direction or 0,
    }
end

--- Which field a function of a trigger enters (EnterFieldScene), without doing anything else of what it
-- does: it runs it as something that only looks (it undoes the writes to the variables).
-- @param script_function The function (world_function_N).
-- @return The FIELD.TBL record that it enters, nil if it doesn't enter one.
WorldVM.find_field_entry = function(script_function)
    local previous = EnterFieldScene
    local record = nil
    EnterFieldScene = function(field_table_id)
        record = record or tonumber(field_table_id)
        return 0
    end
    WorldVM.run_sandboxed(function()
        pcall(script_function, System.WorldMap)
    end)
    EnterFieldScene = previous
    return record
end

--- If a vehicle can enter a place (WorldMapSettings.entry).
-- @param record FIELD.TBL record of the place.
-- @param vehicle walk, chocobo, buggy, tiny_bronco, highwind or submarine.
-- @param landing True if the Highwind has just landed.
-- @return True if it can.
WorldVM.can_enter = function(record, vehicle, landing)
    if vehicle == nil or vehicle == "walk" then return true end
    local entry = WorldMapSettings and WorldMapSettings.entry
    if not entry then return true end
    if entry.exempt and entry.exempt[record] then return true end
    if vehicle == "highwind" then
        return landing == true and entry.highwind_landing ~= nil and entry.highwind_landing[record] == true
    end
    local allowed = entry.allowed and entry.allowed[vehicle]
    return allowed ~= nil and allowed[record] == true
end

--- Creates the models that the scripts place in a map.
-- @param map_id Map of the module (0 WM0, 1 WM2, 2 WM3).
WorldVM.place_models = function(map_id)
    local situation = WorldVM.get_situation(map_id)
    local set = situation and WorldMapModels.situations[situation]
    if not set then return end
    WorldVM.apply_debug_variables()
    for model_type, hrc in pairs(set) do
        local model = WorldMapModels.models[hrc]
        local exists = WorldVM.loaded_types == nil or WorldVM.loaded_types[model_type]
        if exists and WorldVM.static_types[model_type] and model then
            local spawn = WorldVM.get_spawn(model_type)
            if spawn then
                local rotation = model.rotation or {0, 0, 0}
                world_map_module:add_world_model(
                    model.name .. "_" .. model_type, hrc, spawn.x, spawn.z, model.scale, spawn.direction,
                    rotation[1], rotation[2], rotation[3]
                )
                -- The model plays its default animation (model.animation, the first one if it
                -- is not set: the scenery has a single static one). The others are for events.
                local entity_name = "WorldMap" .. model.name .. "_" .. model_type
                if model.altitude then world_map_module:set_entity_altitude(entity_name, model.altitude) end
                local entity = entity_manager:get_entity(entity_name)
                if entity ~= nil and entity:get_animation_count() > 0 then
                    entity:set_default_animation_index(model.animation or 0)
                    entity:play_animation_index(model.animation or 0)
                end
            end
        end
    end
end

--- The types of entities that the init function of the map (function 1) creates with LoadModel,
-- for the story and the flags that there are. For WM0 it also gives where the player appears when the
-- world is entered from a field (SetActiveEntity... of that function).
-- @return A table with the types as keys, and the position of the player (as get_spawn) or nil.
WorldVM.scan_init = function()
    local init = System.WorldMap.world_function_1
    local types = {}
    if not init then return nil end
    local previous_load_model = LoadModel
    LoadModel = function(model_type)
        types[model_type] = true
        return 0
    end
    local player, ok, message = WorldVM.run_sandboxed(function()
        WorldVM.current = {}
        local call_ok, call_message = pcall(init, System.WorldMap)
        local result = WorldVM.current
        WorldVM.current = nil
        return result, call_ok, call_message
    end)
    LoadModel = previous_load_model
    if not ok then
        print("World map init function failed: " .. tostring(message))
        return nil
    end
    local position = nil
    if player.mesh_x and player.mesh_y and player.in_x and player.in_y then
        position = {
            x = player.mesh_x * WorldVM.MESH_SIZE + player.in_x / 4096,
            z = player.mesh_y * WorldVM.MESH_SIZE + player.in_y / 4096,
        }
    end
    return types, position
end

--- The position that a field exit gives in the world map.
-- The exit saves x, y (world units of the exit, 16 bits) by the name of its spawn point.
-- @param name Spawn point name given by the field.
-- @return {x, z} in the units of the module, or nil.
WorldVM.arrival_point = function(name)
    if not name or name == "" then return nil end
    if WorldVM.arrivals == nil then
        local chunk = loadfile("./data/scripts/worldmap/world_arrivals.lua")
        WorldArrivals = nil
        if chunk then pcall(chunk) end
        WorldVM.arrivals = WorldArrivals or false
    end
    local exit = WorldVM.arrivals and WorldVM.arrivals[name]
    -- The exits to the world have 0, 0 (all of the ones of the game): the position is in the scripts.
    if not exit or (exit.x == 0 and exit.y == 0) then return nil end
    return {x = exit.x * WorldVM.EXIT_SCALE / 4096, z = exit.y * WorldVM.EXIT_SCALE / 4096}
end

--- Prepares a map before its entities are created: gives the scripts how the world was entered, finds out
-- which entities exist for the story and the flags, and sets where they start.
--
-- The way in is the FIELD.TBL record of the field exit that was used (System.WorldMap.arrival, or
-- WorldMapDebug.arrival to test), 0 if the world wasn't entered from a field. The vehicles that the
-- story hasn't given are hidden, except the ones that the debug data has a spawn for (WorldMapDebug.spawn),
-- which also keep that position. The other ones are where their scripts put them, or at the default of
-- the module if the script doesn't say (their last position is not saved yet).
-- @param map_id Map of the module (0 WM0, 1 WM2, 2 WM3).
WorldVM.prepare = function(map_id)
    WorldVM.apply_debug_variables()
    -- The speeds of walking and of each vehicle (world_settings.lua).
    if WorldMapSettings and WorldMapSettings.speed then
        for name, speed in pairs(WorldMapSettings.speed) do world_map_module:set_speed(name, speed) end
    end
    -- The distance and the angle of the camera for walking and for each vehicle.
    if WorldMapSettings and WorldMapSettings.camera then
        for name, camera in pairs(WorldMapSettings.camera) do
            world_map_module:set_camera(name, camera.distance, camera.pitch)
        end
    end
    System.WorldMap.specials = {}
    -- The arrival of a real field exit has priority over the spawn points of the debug data.
    local real_arrival = System.WorldMap.arrival
    local arrival = real_arrival or (WorldMapDebug and WorldMapDebug.arrival) or 0
    System.WorldMap.arrival = nil
    System.WorldMap.specials[5] = 0
    System.WorldMap.specials[6] = arrival
    local types, player = WorldVM.scan_init()
    WorldVM.loaded_types = types
    -- The field exit gives the coordinates itself (installer: world_arrivals.lua), they are what counts.
    local point = real_arrival and WorldVM.arrival_point(System.WorldMap.arrival_name)
    System.WorldMap.arrival_name = nil
    if point then
        print("World map arrival: the exit gives " .. point.x .. ", " .. point.z
            .. (player and (" (the scripts of the record say " .. player.x .. ", " .. player.z .. ")") or ""))
        player = point
    elseif arrival ~= 0 then
        print("World map arrival: FIELD.TBL record " .. arrival .. (player
            and (", the player appears at " .. player.x .. ", " .. player.z)
            or ", the scripts don't place the player"))
    end
    if map_id ~= 0 then return end
    local debug_spawn = WorldMapDebug and WorldMapDebug.spawn or {}
    for name, model_type in pairs(WorldVM.vehicle_types) do
        local forced = debug_spawn[name] ~= nil
        if types and not types[model_type] and not forced then
            world_map_module:set_vehicle_hidden(name)
        elseif not forced or real_arrival then
            local spawn = WorldVM.get_spawn(model_type)
            if spawn then world_map_module:set_spawn(name, spawn.x, spawn.z) end
        end
    end
    if arrival ~= 0 and player and (debug_spawn.Cloud == nil or real_arrival) then
        world_map_module:set_spawn("Cloud", player.x, player.z)
    end
end
