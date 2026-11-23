-- Debug data used for testing, remove or don't include this script for actual gameplay.

Inventory.add_item(6, 5)
Inventory.add_item(73, 4)
Inventory.add_item(56, 3)
Inventory.add_item(0, 2)
Inventory.add_item(1, 4)
Inventory.add_item(42, 3)
Inventory.add_item(1, 4)
Inventory.add_item(71, 4)
Inventory.add_item(259, 2)
Inventory.add_item(260, 2)
Inventory.add_item(261, 2)
Inventory.add_item(262, 2)
Inventory.add_item(263, 2)
Inventory.add_item(264, 2)
Inventory.add_item(265, 2)
Inventory.add_item(266, 2)
Inventory.add_item(267, 2)
Inventory.add_item(268, 2)
Inventory.add_item(13, 2)
Inventory.add_item(129, 1)
Inventory.add_item(130, 1)
Inventory.add_item(131, 1)
Inventory.add_item(181, 1)
Inventory.add_item(14, 4)
Inventory.add_item(58, 3)
Inventory.add_item(240, 4)
Inventory.add_item(289, 4)
Inventory.add_item(290, 1)
Inventory.add_item(294, 2)
Inventory.add_key_item(0)
Inventory.add_key_item(1)
Inventory.add_key_item(5)
Inventory.add_key_item(18)
Inventory.add_key_item(19)
Inventory.add_key_item(20)
Inventory.add_key_item(21)
Inventory.add_key_item(22)
Inventory.add_key_item(33)
Inventory.add_key_item(38)
--[[for i = 0, 50 do
    Inventory.add_key_item(i)
end]]
Inventory.money = 1234567
Inventory.sort(Inventory.ORDER.NAME)
--[[Characters[0].stats.hp.current = 12345
Characters[0].stats.hp.base = 54321
]]
Characters[0].stats.dex.base = 11
Characters[0].stats.dex.total = 11
-- Cloud has enough MP for Comet (70 MP) and Comet2 (110 MP).
Characters[0].stats.mp.base = 400
Characters[0].stats.mp.max = 400
Characters[0].stats.mp.current = 400
Characters[0].armor.id = 268
-- The slots of this armor are linked in pairs: 0-1, 2-3, 4-5 and 6-7. Fire (0) is paired with MP
-- Absorb, Ice (2) with MP Turbo, Restore (4) with Quadra Magic, and Lightning (6) with nothing. Contain (4 spells) is in the weapon. Before, Lightning (6) was with Contain, which
-- is not a support materia (so nothing). All is not equipped any more. Slots of the weapon, which has
-- one pair (0-1): Double Cut with Steal as well (his Attack steals too).
Characters[0].armor.materia = {
    [0] = {id = 49, ap = 100000},  -- Fire
    [1] = {id = 27, ap = 100000},  -- MP Absorb
    [2] = {id = 50, ap = 100000},  -- Ice
    [3] = {id = 26, ap = 120000},  -- MP Turbo
    [4] = {id = 53, ap = 100000},  -- Restore
    [5] = {id = 35, ap = 200000},  -- Quadra Magic
    [6] = {id = 39, ap = 100000},  -- Throw mastered: Coin (Tifa has Throw level 1)
}
-- Attack commands to test: Double Cut gives 2x-Cut (AP below 100000) or 4x-Cut (100000 or more), and Slash-All
-- gives Slash-All (AP below 130000) or Flash (130000 or more). Cloud has Double Cut (4x-Cut) paired with Steal
-- as well, and Tifa has Slash-All (Flash). Only the last attack command of a character is used.
Characters[0].weapon.materia[0] = {id = 15, ap = 100000} -- Double Cut (4x-Cut)
-- Magic for testing. Materia IDs: Fire 49, Ice 50, Earth 51, Lightning 52, Restore 53, Heal 54,
-- Revive 55, Poison 60. AP is enough to reach the highest level.
Characters[0].weapon.materia[1] = {id = 34, ap = 200000} -- Steal as well (paired with Double Cut: his Attack steals too)
Characters[2].weapon.materia[0] = {id = 50, ap = 100000}
Characters[2].weapon.materia[1] = {id = 52, ap = 100000}
Characters[2].armor.materia[0] = {id = 54, ap = 100000}
Characters[2].armor.materia[1] = {id = 55, ap = 100000}
-- Aerith has no materia.
Characters[3].weapon.materia = {}
Characters[3].armor.id = 268
Characters[3].armor.materia = {}
-- Status magic: Seal 56 (Sleepel, Silence), Mystify 57 (Confu, Berserk), Transform 58 (Mini, Toad),
-- Time 65 (Haste, Slow, Stop), Destruct 68 (DeBarrier, DeSpell, Death). Cloud already has Barrier
-- (62) in his armor.
Characters[0].weapon.materia[2] = {id = 36, ap = 100000} -- Steal (mastered: Mug). Use ap = 0 for plain Steal.
Characters[0].weapon.materia[3] = {id = 41, ap = 0} -- Deathblow (D.blow), instead of Sense (37)
Characters[2].armor.materia[2] = {id = 68, ap = 100000}
Characters[2].armor.materia[3] = {id = 57, ap = 100000}
-- Limit bars to test the Limit command: everybody has the first limit and a bar of about 85% (217 of 255).
-- The HP and MP are filled (the game caps them to the maximums when the battle starts).
for _, c in ipairs({0, 2, 3}) do
    Characters[c].limit.learned[1][1] = true
    Characters[c].limit.bar = 217
    Characters[c].stats.hp.current = 99999
    Characters[c].stats.mp.current = 99999
end
-- Cloud: limit level 4 (Omnislash, the first technique of the level) and a full bar.
Characters[0].limit.current = 4
Characters[0].limit.learned[4] = Characters[0].limit.learned[4] or {}
Characters[0].limit.learned[4][1] = true
Characters[0].limit.bar = 255
-- One kill from limit level 2 (120 kills): the first enemy he defeats teaches him Blade Beam.
Characters[0].kills = 119
-- Barret (Mindblow) and Red XIII (Lunatic High): limit level 1 with the two techniques and a full bar. The ids
-- are in begin_menu.lua option 7 (party Barret, Red XIII, Aerith). Their other levels: Barret 2 Grenade Bomb and
-- Hammerblow, 3 Satellite Beam; Red XIII 2 Blood Fang and Stardust Ray, 3 Howling Moon (on himself) and Earth Rave.
for _, c in ipairs({1, 4}) do
    Characters[c].limit.current = 1
    Characters[c].limit.learned[1] = Characters[c].limit.learned[1] or {}
    Characters[c].limit.learned[1][1] = true
    Characters[c].limit.learned[1][2] = true
    Characters[c].limit.bar = 255
    Characters[c].stats.hp.current = 99999
    Characters[c].stats.mp.current = 99999
end
-- Aerith: limit level 3 (Planet Protector and Pulse of Life), with a full bar.
Characters[3].limit.current = 3
Characters[3].limit.learned[3] = Characters[3].limit.learned[3] or {}
Characters[3].limit.learned[3][1] = true
Characters[3].limit.learned[3][2] = true
Characters[3].limit.bar = 255
-- Summons: Choco/Mog 74, Shiva 75, Ifrit 76, Ramuh 77, Titan 78, Odin 79, Leviathan 80, Bahamut 81,
-- Kjata 82, Alexander 83, Phoenix 84, Neo Bahamut 85, Hades 86, Typhoon 87, Bahamut ZERO 88,
-- Knights of Round 89, Master Summon 90. They cost between 14 and 250 MP.
Characters[0].weapon.materia[4] = {id = 75, ap = 100000} -- Shiva
Characters[0].weapon.materia[6] = {id = 44, ap = 0, skills = {}} -- E. Skill
Characters[0].weapon.materia[7] = {id = 69, ap = 100000} -- Contain (mastered: Freeze, Break, Tornado, Flare)
for i = 1, 24 do -- E. Skill list
    if i == 1 or i == 6 or i == 9 or i == 13 or i == 14 or i == 15 or i == 23 or i == 24 then
        Characters[0].weapon.materia[6].skills[i] = true
    else
        Characters[0].weapon.materia[6].skills[i] = false
    end
end
Characters[2].weapon.materia[2] = {id = 77, ap = 100000} -- Ramuh
Characters[2].weapon.materia[3] = {id = 40, ap = 0} -- Morph, instead of Choco/Mog (74)
Characters[2].weapon.materia[4] = {id = 14, ap = 130000} -- Slash-All (Flash)
Characters[2].weapon.materia[6] = {id = 16, ap = 40000} -- Cover (100%: takes the attacks for allies with critical HP)
Characters[2].armor.materia[4] = {id = 39, ap = 0} -- Throw (level 1; mastered it is Coin)
Characters[3].weapon.materia[0] = {id = 39, ap = 100000} -- Throw mastered: Aerith has Coin
-- Double commands: Aerith W-Magic (with Fire and Restore to cast), Tifa W-Item, Cloud W-Summon (instead of Mega All, with Shiva).
Characters[3].weapon.materia[1] = {id = 19, ap = 100000} -- W-Magic
Characters[3].armor.materia[0] = {id = 49, ap = 100000} -- Fire
Characters[3].armor.materia[1] = {id = 53, ap = 100000}
Characters[3].armor.materia[2] = {id = 43, ap = 0} -- Mime
Characters[2].weapon.materia[5] = {id = 42, ap = 0} -- Manipulate
Characters[2].armor.materia[5] = {id = 21, ap = 100000} -- W-Item
Characters[0].weapon.materia[5] = {id = 20, ap = 100000} -- W-Summon
Inventory.add_item(129, 3) -- Mythril Saber (to throw)
Inventory.add_item(130, 2)
-- Items to try: Potion, Hi-Potion, Ether, Phoenix Down, Antidote, Echo Screen, Hyper, Tranquilizer,
-- Remedy, Speed Drink, Grenade, Shrapnel, Molotov.
for _, item in ipairs({{0, 6}, {1, 4}, {3, 4}, {7, 4}, {8, 3}, {12, 3}, {13, 2}, {14, 2}, {15, 2}, {17, 3}, {20, 5}, {21, 3}, {53, 3}}) do
    Inventory.add_item(item[1], item[2])
end
-- Uncomment to have enough MP to cast anything, summons included.
--[[
for _, c in ipairs({0, 2, 3}) do
    Characters[c].stats.mp.base = 999
    Characters[c].stats.mp.max = 999
    Characters[c].stats.mp.current = 999
end
]]
Materia.add(4)
Materia.add(44)
Materia.add(12)
Materia.add(15, 2000)
Materia.add(24)

-- Where the entities of the overworld appear when it is loaded, in world units (x, z). Without this the
-- game would use the spawn points of the story (not known yet), so they are all around the start of the
-- test map, near Kalm. Remove an entry to use the default of that entity.
WorldMapDebug = WorldMapDebug or {}
WorldMapDebug.spawn = {
    Cloud = {42.0, 36.5},
    Chocobo = {42.5, 37.5},
    Highwind = {43.0, 36.5},
    Buggy = {42.0, 36.0},
    Submarine = {41.5, 36.5},
    TinyBronco = {42.0, 37.37},
}

-- Story of the overworld, for testing the models that depend on it. Without anything here the game
-- takes the situation (the set of models of the map, 0 to 12) from the story, like the original does:
-- the story counter (word 0 of the game variables, Banks[2][0] in the fields) and two flags (the bits
-- 976 and 7220) give 0 the start (counter under 1000), 1 to 4 the Diamond Weapon (1000 to 1579; +1 with
-- the bit 976, +2 from 1620 on), 5 to 8 the Ultimate Weapon and the Ruby Weapon (from 1580; +1 from
-- 1620, +2 with the bit 7220). The situation can be forced for a map with WorldMapDebug.situation[map]
-- (1 to 4 Diamond Weapon, with the Midgar cannon in 2 and 4 and the second Highwind in 3 and 4; 5 and
-- 6 Ultimate Weapon; 7 and 8 Ruby Weapon; 11 the underwater map; 12 the snowstorm one).
--
-- The variables are the ones of the savemap that the world scripts read, and the fields write: they
-- are written when the map is loaded. "word" and "byte" are by the address in bytes (the story counter
-- is word 0), "bit" by the address in bits (the vehicles are the bits 1016 to 1021: 1016 the Buggy, 1018
-- the Tiny Bronco, 1020 the Highwind, 1008 the Chocobo, 6787 and 6802 the Submarine, 6803 the Diamond
-- Weapon appears) and "special" the values that the engine gives to the world (6 is the FIELD.TBL
-- record of the exit by which the world was entered).
--
-- WorldMapDebug.arrival is that record, to test entering the world from a field exit: the vehicles
-- and the player are where their scripts put them for it, except the ones with a spawn above.
-- The map has to be loaded again to change any of this. Example: the story at 1400 with the Buggy, the
-- Tiny Bronco and the Highwind.
--WorldMapDebug.situation = {[0] = 3}
--WorldMapDebug.arrival = 51
WorldMapDebug.variables = {}
--WorldMapDebug.variables = {
--    word = {[0] = 1400},
--    bit = {[1016] = 1, [1018] = 1, [1020] = 1},
--}
