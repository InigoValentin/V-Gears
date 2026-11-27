--- Helper function that loads a world map through fade in and fadeout
--
-- @param map The world map to load. 0 is the normal world map, 1 is the submarine map, and 2 is the
-- snowstorm map.
-- @param x X coordinate in the map for the PC.
-- @param y Y coordinate in the map for the PC.
load_world_map_request = function(map, x, y)
    if map >= 0 and map <= 2  then

        -- TODO: Set propper names
        -- System["MapChanger"].map_name = map_name
        -- System["MapChanger"].point_name = point_name
        player_lock(true) -- disable menu and pc movement while load map
        script:request_end_sync(Script.UI, "Fade", "fade_out", 0)
        world_map(map, x, y)
        player_lock(false) -- enable menu and pc movement after load map
        --self.current_map_name = self.map_name
        --self.map_name = ""
        script:request_end_sync(Script.UI, "Fade", "fade_in", 0)
    end
end
