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
--[[Inventory.add_key_item(0)
Inventory.add_key_item(1)
Inventory.add_key_item(5)
Inventory.add_key_item(18)
Inventory.add_key_item(19)
Inventory.add_key_item(20)
Inventory.add_key_item(21)
Inventory.add_key_item(22)
Inventory.add_key_item(33)
Inventory.add_key_item(38)]]
for i = 0, 50 do
    Inventory.add_key_item(i)
end
Inventory.money = 1234567
Inventory.sort(Inventory.ORDER.NAME)
Characters[0].stats.hp.current = 12345
Characters[0].stats.hp.base = 54321
Characters[0].armor.id = 268
Characters[0].armor.materia = {
    [0] = {id = 62, ap = 5340},
    [2] = {id = 72, ap = 5},
    [3] = {id = 78, ap = 520},
    [4] = {id = 36, ap = 0},
    [5] = {id = 36, ap = 50000},
    [6] = {id = 0, ap = 0},
    [7] = {id = 26, ap = 0},
}
