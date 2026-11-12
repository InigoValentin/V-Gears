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
Characters[0].armor.id = 268
Characters[0].armor.materia = {
    [0] = {id = 62, ap = 15340},
    [2] = {id = 44, ap = 0, skills = {}}, -- E. Skill
    [3] = {id = 78, ap = 520},
    [4] = {id = 36, ap = 0},
    [5] = {id = 36, ap = 50000},
    [6] = {id = 0, ap = 0},
    [7] = {id = 26, ap = 0},
}
Characters[0].weapon.materia[0] = {id = 64, ap = 15340}
for i = 1, 24 do -- E. Skill list
    if i == 1 or i == 6 or i == 9 or i == 13 or i == 14 or i == 15 or i == 23 or i == 24 then
        Characters[0].armor.materia[2].skills[i] = true
    else
        Characters[0].armor.materia[2].skills[i] = false
    end
end
-- Magic for testing. Materia IDs: Fire 49, Ice 50, Earth 51, Lightning 52, Restore 53, Heal 54,
-- Revive 55, Poison 60. AP is enough to reach the highest level.
Characters[0].weapon.materia[1] = {id = 49, ap = 100000}
Characters[0].armor.materia[1] = {id = 53, ap = 100000}
Characters[2].weapon.materia[0] = {id = 50, ap = 100000}
Characters[2].weapon.materia[1] = {id = 52, ap = 100000}
Characters[2].armor.materia[0] = {id = 54, ap = 100000}
Characters[2].armor.materia[1] = {id = 55, ap = 100000}
Characters[3].weapon.materia[0] = {id = 53, ap = 100000}
Characters[3].weapon.materia[1] = {id = 49, ap = 100000}
Characters[3].armor.materia[0] = {id = 60, ap = 100000}
Characters[3].armor.materia[1] = {id = 51, ap = 100000}
-- Status magic: Seal 56 (Sleepel, Silence), Mystify 57 (Confu, Berserk), Transform 58 (Mini, Toad),
-- Time 65 (Haste, Slow, Stop), Destruct 68 (DeBarrier, DeSpell, Death). Cloud already has Barrier
-- (62) in his armor.
Characters[0].weapon.materia[2] = {id = 65, ap = 100000}
Characters[0].weapon.materia[3] = {id = 56, ap = 100000}
Characters[2].armor.materia[2] = {id = 68, ap = 100000}
Characters[2].armor.materia[3] = {id = 57, ap = 100000}
Characters[3].armor.materia[2] = {id = 58, ap = 100000}
-- Summons: Choco/Mog 74, Shiva 75, Ifrit 76, Ramuh 77, Titan 78, Odin 79, Leviathan 80, Bahamut 81,
-- Kjata 82, Alexander 83, Phoenix 84, Neo Bahamut 85, Hades 86, Typhoon 87, Bahamut ZERO 88,
-- Knights of Round 89, Master Summon 90. They cost between 14 and 250 MP.
Characters[0].weapon.materia[4] = {id = 75, ap = 100000} -- Shiva
Characters[0].weapon.materia[5] = {id = 76, ap = 100000} -- Ifrit
Characters[2].weapon.materia[2] = {id = 77, ap = 100000} -- Ramuh
Characters[2].weapon.materia[3] = {id = 74, ap = 100000} -- Choco/Mog
Characters[3].weapon.materia[2] = {id = 84, ap = 100000} -- Phoenix
Characters[3].weapon.materia[3] = {id = 81, ap = 100000} -- Bahamut
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
