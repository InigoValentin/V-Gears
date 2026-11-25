-- Settings of the world map that are meant to be edited.

local BLOCK = 8182 / 4096 -- units of a mesh block (a map block is 2 of them)
local BASE = BLOCK * 0.45 -- the speed that the game has by default for walking

--- Movement speeds, in world units per second (about 0.9 is the default walking speed).
-- A name that is removed uses the default of the module (walking 0.9, the vehicles twice that).
local speed = {
    walk = BASE,
    chocobo = BASE * 1.5,
    buggy = BASE * 1.2,
    tiny_bronco = BASE * 1.2,
    -- The Highwind and the submarine (in WM2 and when diving) when they move forward.
    highwind = BASE * 2,
    submarine = BASE * 1.2,
}

-- The camera that the module had: 2.7 units from the vehicle, 33.7 degrees over the ground (the offset
-- 1.5 up and 2.25 back). It is the distance that was good for the fastest vehicle.
local DISTANCE = 2.0
local PITCH = 33.7
-- A bit more parallel to the ground, for the vehicles that fly.
local FLIGHT_PITCH = 20
WorldMapSettings = {
    speed = speed,

    --- World units per second for each point of SetActiveEntityMovespeed in the scenes of the Warship
    -- (the Warship sails at 10 to 40 in them). From ff7.exe: a mesh is 8192 units and the ship moves
    -- speed / 8192 of a mesh each frame, which is this value with 30 frames per second (a unit of 30 / 4096).
    scene_speed_unit = 30 / 4096,
    --- Frames of the scene scripts for each second. The distance does not depend on it (it is for each frame, in
    -- the exe), only how long the waits and the movement take. 15 is what the user remembers of the original.
    scene_fps = 15,

    --- Camera: distance to the vehicle (world units) and angle over the ground (degrees), by vehicle.
    -- submarine_underwater is the submarine in WM2 and submarine the one that floats in WM0.
    camera = {
        walk = {distance = 1.35, pitch = PITCH},
        chocobo = {distance = DISTANCE, pitch = PITCH},
        buggy = {distance = DISTANCE, pitch = PITCH},
        tiny_bronco = {distance = DISTANCE, pitch = PITCH},
        highwind = {distance = DISTANCE, pitch = FLIGHT_PITCH},
        submarine = {distance = DISTANCE, pitch = PITCH},
        submarine_underwater = {distance = DISTANCE, pitch = FLIGHT_PITCH},
        -- The entity of a scripted scene (the Warship).
        scene = {distance = 1.25, pitch = 25},
    },

    --- Music of the world map (the logic is in world_music.lua). Numbers are the tracks of audio/musics.
    -- Written from the description of the game by the user, and from the scripts of the world (wm0.lua);
    -- the pieces that are not known yet are marked TODO and are the ones to fix.
    music = {
        -- On foot (and the vehicles that have no music of their own).
        default = 13, -- world_map
        -- TODO: the track that replaces the default after some point of disc two (70, world_map_danger),
        -- and the value of the story counter (ReadWord(0)) where it starts. nil means it never starts.
        danger = 70,
        danger_from_story = nil,

        -- The Chocobo always has its own track (assumed, not checked).
        chocobo = 50, -- chocobo_ride
        -- The Highwind (WM0) has its own track, except when the music is an override of the story or
        -- when cloud_away() is true.
        highwind = 46, -- airship
        -- TODO: the Buggy, the Tiny Bronco and the submarine of WM0 have no known music: they use
        -- the default one. Put the track here when it is known (nil = the default).
        buggy = nil,
        tiny_bronco = nil,
        submarine = nil,

        -- WM2 (underwater) and WM3 (snow): one track that never changes.
        -- TODO: 84 and 76 are from memory (84 is the "submarine" track of the installer), check them.
        underwater = 84,
        snow = 76,

        --- Overrides of the story, in order of priority. The first one whose when() is true gives the
        -- track, and the vehicles (except the Chocobo) don't change it.
        overrides = {
            -- While the Diamond Weapon is around. The conditions are the ones of the world script that
            -- calls Unknown_AKAO(6) (bit 6803 set, 7212 set, 7209 not set); that 6 is guessed to mean 69.
            -- TODO: check that the music ends with the Diamond Weapon (what ends it, when 7209 is set?).
            {name = "diamond_weapon", track = 69, when = function()
                return ReadBit(6803) ~= 0 and ReadBit(7212) ~= 0 and ReadBit(7209) == 0
            end},
            -- Before the last visit to Midgar. The script calls Unknown_AKAO(7) when the story counter is
            -- exactly 1596; that 7 is guessed to mean 71.
            -- TODO: the real range of the story counter (it may last more than that value).
            {name = "before_last_midgar", track = 71, when = function()
                return ReadWord(0) == 1596
            end},
        },

        --- True while Cloud is not in the party: the Highwind doesn't change the music then.
        -- TODO: find out when (the story counter range, or a flag). For now it is never.
        cloud_away = function()
            return false
        end,
    },

    --- Which vehicles can enter the places of the map (towns, caves...). The numbers are the FIELD.TBL
    -- records, the first parameter of EnterFieldScene (1 Midgar, 2 Kalm, 3 the chocobo farm, 7 Junon, 15 North
    -- Corel, 18 Costa del Sol, 59 the northern cave...). On foot everything can be entered.
    entry = {
        --- Records that any vehicle can enter. The scripted events and the submarine station are here
        -- so that nothing gets stuck.
        -- TODO: check the list (50-52 are the ship fields, 56 the submarine station): add the other
        -- records that are story events and not places, if some vehicle must be able to trigger them.
        exempt = {[50] = true, [51] = true, [52] = true, [56] = true},

        --- Exceptions to "only on foot", by vehicle: records that it can enter.
        allowed = {
            chocobo = {[3] = true}, -- the chocobo farm
            -- Corel, Junon and Costa del Sol.
            -- TODO: check the records of Corel: 15 is North Corel (ncorel), 14 Mt. Corel (mtcrl_0) and 16
            -- the Corel desert (desert2) could be it too.
            buggy = {[7] = true, [15] = true, [18] = true},
        },

        --- Records that the Highwind enters when it lands on the NORTHERND_CAVE terrain (it cannot enter
        -- anything else, and it never enters while it is flying).
        highwind_landing = {[59] = true}, -- the northern cave (las0_1)
    },
}
