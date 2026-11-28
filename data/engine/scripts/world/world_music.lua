--[[
Music of the world map. The data (tracks, overrides and conditions) is in world_settings.lua
(WorldMapSettings.music); this has the logic that picks the track and plays it.

WorldMusic.update is called by the module when a map starts, when the vehicle changes, and every half
second (the story can change the music while the map is on).
]]

WorldMusic = WorldMusic or {current = nil}

--- Calls a function of the settings that can fail without stopping the game.
local function safe(func, ...)
    if type(func) ~= "function" then return nil end
    local ok, result = pcall(func, ...)
    if ok then return result end
    return nil
end

--- Gets the track of a map and a vehicle.
-- @param map_id 0 for WM0, 1 for WM2 (underwater), 2 for WM3 (snow).
-- @param vehicle walk, chocobo, buggy, tiny_bronco, highwind or submarine.
-- @return The track or nil.
WorldMusic.choose = function(map_id, vehicle)
    local music = WorldMapSettings and WorldMapSettings.music
    if not music then return nil end
    if map_id == 1 then return music.underwater end
    if map_id == 2 then return music.snow end

    -- The Chocobo always has priority.
    if vehicle == "chocobo" and music.chocobo then return music.chocobo end

    -- Overrides of the story: the vehicles don't change them.
    for _, override in ipairs(music.overrides or {}) do
        if safe(override.when) then return override.track end
    end

    -- The vehicles with their own track.
    if vehicle == "highwind" and music.highwind and not safe(music.cloud_away) then
        return music.highwind
    end
    if vehicle ~= "walk" and vehicle ~= "highwind" and vehicle ~= "chocobo" and music[vehicle] then
        return music[vehicle]
    end

    -- The default one of the story.
    local from_story = music.danger_from_story
    if music.danger and from_story and ReadWord(0) >= from_story then return music.danger end
    return music.default
end

--- Plays the track of the moment if it is not the one that is playing.
-- @param map_id Map of the module.
-- @param vehicle Name of the vehicle.
WorldMusic.update = function(map_id, vehicle)
    local track = WorldMusic.choose(map_id, vehicle)
    if track == nil or track == WorldMusic.current then return end
    WorldMusic.current = track
    audio_manager:play_music(tostring(track))
end

--- Forgets the track that was playing (the next update plays it again).
WorldMusic.reset = function()
    WorldMusic.current = nil
end
