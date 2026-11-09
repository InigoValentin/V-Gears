EntityContainer = EntityContainer or {}

local function show_vehicle_error(message)
    print(message)
    if not dialog:show_message(message) then
        return false
    end
    return true
end

local Vehicle = {}
Vehicle.__index = Vehicle

Vehicle.terrains = {}
Vehicle.board = function(self)
    show_vehicle_error("Vehicle.board must be implemented by the vehicle.")
    return false
end
Vehicle.unboard = function(self)
    show_vehicle_error("Vehicle.unboard must be implemented by the vehicle.")
    return false
end
Vehicle.on_interact = function(self)
    self:board()
end

--[[
Creates a vehicle that inherits the Vehicle interface.
@param entity_name Managed world-map entity name.
@param terrains Walkmap terrain type names this vehicle can travel.
@param rise_amount Height offset applied when boarding.
@return The vehicle script table.
]]
local function extend_vehicle(entity_name, terrains, rise_amount, unboarding_terrains)
    return setmetatable({
        entity_name = entity_name,
        terrains = terrains,
        unboarding_terrains = unboarding_terrains or terrains,
        rise_amount = rise_amount,
    }, Vehicle)
end

local function is_unboarding_terrain_allowed(vehicle)
    local terrain_name = world_map_module:get_terrain_type_under_entity(vehicle.entity_name)
    for _, allowed_terrain in ipairs(vehicle.unboarding_terrains) do
        if terrain_name == allowed_terrain then
            return true
        end
    end
    return false, terrain_name
end

--[[
Copies a terrain list so a Chocobo color change does not mutate a shared profile.
@param terrains Terrain type names to copy.
@return A new terrain list.
]]
local function copy_terrain_list(terrains)
    local copy = {}
    for _, terrain_name in ipairs(terrains) do
        table.insert(copy, terrain_name)
    end
    return copy
end

local yellow_chocobo_terrains = {
    "GRASS", "FOREST", "SWAMP", "DESERT", "WASTELAND", "SNOW", "RIVERSIDE",
    "COREL_BRIDGE", "WUTAI_BRIDGE", "HILL_SIDE", "BEACH", "CANYON", "MOUNTAIN_PASS",
    "GOLD_SAUCER_DESERT_BORDER", "BRIDGE_HEAD",
}
local green_chocobo_terrains = copy_terrain_list(yellow_chocobo_terrains)
table.insert(green_chocobo_terrains, "MOUNTAIN")
table.insert(green_chocobo_terrains, "CLIFF")
local blue_chocobo_terrains = copy_terrain_list(yellow_chocobo_terrains)
table.insert(blue_chocobo_terrains, "RIVER_CROSSING")
table.insert(blue_chocobo_terrains, "RIVER")
table.insert(blue_chocobo_terrains, "WATER")
local black_chocobo_terrains = copy_terrain_list(green_chocobo_terrains)
table.insert(black_chocobo_terrains, "RIVER_CROSSING")
table.insert(black_chocobo_terrains, "RIVER")
table.insert(black_chocobo_terrains, "WATER")
local gold_chocobo_terrains = {
    "GRASS", "FOREST", "MOUNTAIN", "SEA_DEEP_WATER", "RIVER_CROSSING", "RIVER", "WATER",
    "SWAMP", "DESERT", "WASTELAND", "SNOW", "RIVERSIDE", "CLIFF", "COREL_BRIDGE",
    "WUTAI_BRIDGE", "UNUSED_1", "HILL_SIDE", "BEACH", "SUBMARINE_PEN", "CANYON",
    "MOUNTAIN_PASS", "UNKNOWN_1", "WATERFALL", "UNUSED_2", "GOLD_SAUCER_DESERT", "JUNGLE",
    "SEA", "NORTHERND_CAVE", "GOLD_SAUCER_DESERT_BORDER", "BRIDGE_HEAD", "BACK_ENTRANCE",
    "UNUSED_3",
}
local chocobo_terrains_by_color = {
    YELLOW = yellow_chocobo_terrains,
    GREEN = green_chocobo_terrains,
    BLUE = blue_chocobo_terrains,
    BLACK = black_chocobo_terrains,
    GOLD = gold_chocobo_terrains,
}

EntityContainer["WorldMapHighwind"] = extend_vehicle(
    "WorldMapHighwind",
    {
        "GRASS", "FOREST", "MOUNTAIN", "SEA_DEEP_WATER", "RIVER_CROSSING", "RIVER",
        "WATER", "SWAMP", "DESERT", "WASTELAND", "SNOW", "RIVERSIDE", "CLIFF",
        "COREL_BRIDGE", "WUTAI_BRIDGE", "HILL_SIDE", "BEACH", "SUBMARINE_PEN", "CANYON",
        "MOUNTAIN_PASS", "UNKNOWN_1", "WATERFALL", "GOLD_SAUCER_DESERT", "JUNGLE", "SEA",
        "NORTHERND_CAVE", "GOLD_SAUCER_DESERT_BORDER", "BRIDGE_HEAD", "BACK_ENTRANCE",
    },
    0.75,
    {"GRASS", "NORTHERND_CAVE"}
)
EntityContainer["WorldMapHighwind"].board = function(self)
    local player_entity = entity_manager:get_player_entity()
    local vehicle_entity = entity_manager:get_entity(self.entity_name)
    if not player_entity or not vehicle_entity then
        return false
    end
    local player_name = player_entity:get_name()
    if player_name ~= "WorldMapCloud" and player_name ~= "WorldMapChocobo" then
        show_vehicle_error("Leave your current vehicle before boarding another.")
        return false
    end
    vehicle_entity:move_by(0, self.rise_amount, 0)
    player_entity:set_visible(false)
    player_entity:set_solid(false)
    player_entity:set_talkable(false)
    world_map_module:clear_vehicle_terrains()
    for _, terrain_name in ipairs(self.terrains) do
        world_map_module:add_vehicle_terrain(terrain_name)
    end
    entity_manager:set_player_entity(self.entity_name)
    return true
end
EntityContainer["WorldMapHighwind"].unboard = function(self)
    if not is_unboarding_terrain_allowed(self) then
        show_vehicle_error("The Highwind cannot be unboarded on this terrain.")
        return false
    end
    if not world_map_module:unboard_vehicle(self.entity_name) then
        return false
    end

    local vehicle_entity = entity_manager:get_entity(self.entity_name)
    if vehicle_entity then
        vehicle_entity:move_by(0, -self.rise_amount, 0)
    end
    return true
end

EntityContainer["WorldMapBuggy"] = extend_vehicle(
    "WorldMapBuggy",
    {
        "GRASS", "FOREST", "SWAMP", "DESERT", "WASTELAND", "SNOW", "RIVERSIDE",
        "COREL_BRIDGE", "WUTAI_BRIDGE", "HILL_SIDE", "BEACH", "RIVER_CROSSING", "CANYON",
        "MOUNTAIN_PASS", "GOLD_SAUCER_DESERT", "JUNGLE", "GOLD_SAUCER_DESERT_BORDER",
        "BRIDGE_HEAD", "BACK_ENTRANCE",
    },
    0,
    {
        "GRASS", "FOREST", "WASTELAND", "SNOW", "SWAMP", "DESERT", "COREL_BRIDGE",
        "HILL_SIDE", "BEACH", "CANYON", "MOUNTAIN_PASS", "JUNGLE",
    }
)
EntityContainer["WorldMapBuggy"].board = function(self)
    local player_entity = entity_manager:get_player_entity()
    local vehicle_entity = entity_manager:get_entity(self.entity_name)
    if not player_entity or not vehicle_entity then
        return false
    end
    if player_entity:get_name() ~= "WorldMapCloud" then
        show_vehicle_error("Leave your current vehicle before boarding another.")
        return false
    end
    player_entity:set_visible(false)
    player_entity:set_solid(false)
    player_entity:set_talkable(false)
    world_map_module:clear_vehicle_terrains()
    for _, terrain_name in ipairs(self.terrains) do
        world_map_module:add_vehicle_terrain(terrain_name)
    end
    entity_manager:set_player_entity(self.entity_name)
    return true
end
EntityContainer["WorldMapBuggy"].unboard = function(self)
    if not is_unboarding_terrain_allowed(self) then
        show_vehicle_error("The Buggy cannot be unboarded on this terrain.")
        return false
    end
    return world_map_module:unboard_vehicle(self.entity_name)
end

EntityContainer["WorldMapChocobo"] = extend_vehicle(
    "WorldMapChocobo",
    copy_terrain_list(chocobo_terrains_by_color.YELLOW),
    0,
    {
        "GRASS", "FOREST", "WASTELAND", "SNOW", "SWAMP", "DESERT", "COREL_BRIDGE",
        "HILL_SIDE", "BEACH", "CANYON", "MOUNTAIN_PASS", "JUNGLE",
    }
)
EntityContainer["WorldMapChocobo"].color = "YELLOW"
EntityContainer["WorldMapChocobo"].set_color = function(self, color)
    if type(color) ~= "string" then
        return false
    end

    local color_name = string.upper(color)
    local terrains = chocobo_terrains_by_color[color_name]
    if not terrains then
        show_vehicle_error("Unknown Chocobo color: " .. color_name)
        return false
    end

    self.color = color_name
    self.terrains = copy_terrain_list(terrains)
    local player_entity = entity_manager:get_player_entity()
    if player_entity and player_entity:get_name() == self.entity_name then
        world_map_module:clear_vehicle_terrains()
        for _, terrain_name in ipairs(self.terrains) do
            world_map_module:add_vehicle_terrain(terrain_name)
        end
    end
    return true
end
EntityContainer["WorldMapChocobo"].board = function(self)
    local player_entity = entity_manager:get_player_entity()
    local vehicle_entity = entity_manager:get_entity(self.entity_name)
    if not player_entity or not vehicle_entity then
        return false
    end
    if player_entity:get_name() ~= "WorldMapCloud" then
        show_vehicle_error("Leave your current vehicle before boarding another.")
        return false
    end
    player_entity:set_visible(false)
    player_entity:set_solid(false)
    player_entity:set_talkable(false)
    world_map_module:clear_vehicle_terrains()
    for _, terrain_name in ipairs(self.terrains) do
        world_map_module:add_vehicle_terrain(terrain_name)
    end
    entity_manager:set_player_entity(self.entity_name)
    return true
end
EntityContainer["WorldMapChocobo"].unboard = function(self)
    if not is_unboarding_terrain_allowed(self) then
        show_vehicle_error("The Chocobo cannot be unboarded on this terrain.")
        return false
    end
    return world_map_module:unboard_vehicle(self.entity_name)
end

EntityContainer["WorldMapSubmarine"] = extend_vehicle(
    "WorldMapSubmarine",
    {"SEA_DEEP_WATER", "SEA", "SUBMARINE_PEN"},
    0,
    {"SUBMARINE_PEN"}
)
EntityContainer["WorldMapSubmarine"].board = function(self)
    local player_entity = entity_manager:get_player_entity()
    local vehicle_entity = entity_manager:get_entity(self.entity_name)
    if not player_entity or not vehicle_entity then
        return false
    end
    if player_entity:get_name() ~= "WorldMapCloud" then
        show_vehicle_error("Leave your current vehicle before boarding another.")
        return false
    end
    player_entity:set_visible(false)
    player_entity:set_solid(false)
    player_entity:set_talkable(false)
    world_map_module:clear_vehicle_terrains()
    for _, terrain_name in ipairs(self.terrains) do
        world_map_module:add_vehicle_terrain(terrain_name)
    end
    entity_manager:set_player_entity(self.entity_name)
    return true
end
EntityContainer["WorldMapSubmarine"].unboard = function(self)
    if not is_unboarding_terrain_allowed(self) then
        show_vehicle_error("The Submarine cannot be unboarded on this terrain.")
        return false
    end
    return world_map_module:unboard_vehicle(self.entity_name)
end

EntityContainer["WorldMapTinyBronco"] = extend_vehicle(
    "WorldMapTinyBronco",
    {
        "RIVERSIDE", "BEACH", "RIVER_CROSSING", "RIVER", "WATER",
    },
    0,
    {"BEACH", "RIVERSIDE"}
)
EntityContainer["WorldMapTinyBronco"].board = function(self)
    local player_entity = entity_manager:get_player_entity()
    local vehicle_entity = entity_manager:get_entity(self.entity_name)
    if not player_entity or not vehicle_entity then
        return false
    end
    if player_entity:get_name() ~= "WorldMapCloud" then
        show_vehicle_error("Leave your current vehicle before boarding another.")
        return false
    end
    player_entity:set_visible(false)
    player_entity:set_solid(false)
    player_entity:set_talkable(false)
    world_map_module:clear_vehicle_terrains()
    for _, terrain_name in ipairs(self.terrains) do
        world_map_module:add_vehicle_terrain(terrain_name)
    end
    entity_manager:set_player_entity(self.entity_name)
    return true
end
EntityContainer["WorldMapTinyBronco"].unboard = function(self)
    if not is_unboarding_terrain_allowed(self) then
        show_vehicle_error("The Tiny Bronco cannot be unboarded on this terrain.")
        return false
    end
    return world_map_module:unboard_vehicle(self.entity_name)
end