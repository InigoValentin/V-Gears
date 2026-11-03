EntityContainer = EntityContainer or {}

--[[
Boards a world-map vehicle and hides the previous player entity.
@param entity_name Name of the vehicle entity.
@param rise_amount Height to raise the vehicle while boarding.
]]
local function board_vehicle(entity_name, rise_amount)
    local player_entity = entity_manager:get_player_entity()
    local vehicle_entity = entity_manager:get_entity(entity_name)
    if not player_entity or not vehicle_entity then
        return
    end

    local player_name = player_entity:get_name()
    local boarding_from_foot = player_name == "WorldMapCloud"
    local chocobo_to_highwind
        = player_name == "WorldMapChocobo" and entity_name == "WorldMapHighwind"
    if not boarding_from_foot and not chocobo_to_highwind then
        print("Leave your current vehicle before boarding another.")
        return
    end

    if rise_amount ~= 0 then
        vehicle_entity:move_by(0, rise_amount, 0)
    end
    player_entity:set_visible(false)
    player_entity:set_solid(false)
    player_entity:set_talkable(false)
    entity_manager:set_player_entity(entity_name)
end

EntityContainer["WorldMapHighwind"] = {
    on_interact = function(self)
        board_vehicle("WorldMapHighwind", 0.75)
    end,
}

EntityContainer["WorldMapBuggy"] = {
    on_interact = function(self)
        board_vehicle("WorldMapBuggy", 0)
    end,
}

EntityContainer["WorldMapChocobo"] = {
    on_interact = function(self)
        board_vehicle("WorldMapChocobo", 0)
    end,
}

EntityContainer["WorldMapSubmarine"] = {
    on_interact = function(self)
        board_vehicle("WorldMapSubmarine", 0)
    end,
}

EntityContainer["WorldMapTinyBronco"] = {
    on_interact = function(self)
        print("Tiny Bronco interaction is not implemented yet.")
    end,
}