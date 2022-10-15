--- Game static data
--
-- Contains game data, not to be modified during a playthrough.
Game = {

    --- Data for character growth calculation on level up.
    Growth = {

        --- Bonuses given to different stats when leveling up.
        bonus = {

            --- Bonus given to primary stats when leveling up.
            --
            -- 12 values (indexes 0-11), one of the values is chosen at random.
            primary = {},

            --- Bonus given to HP when leveling up.
            --
            -- 12 values (indexes 0-11), one of them is chosen at random.
            hp = {},

            --- Bonus given to HP when leveling up.
            --
            -- 12 values (indexes 0-11), one of them is chosen at random.
            mp = {}
        },

        --- Curves for stat growth.
        --
        -- Used to determine stat growth for each character. There are 64
        -- curves (indexes 0-63). Each curve has 8 segments, with "base" and
        -- "gradient" values. Segments are used when reaching levels in
        -- brackets:
        -- Lv. 2-11: Segment 0.
        -- Lv. 12-21: Segment 1.
        -- Lv. 22-31: Segment 2.
        -- Lv. 32-41: Segment 3.
        -- Lv. 42-51: Segment 4.
        -- Lv. 52-61: Segment 5.
        -- Lv. 62-81: Segment 6.
        -- Lv. 82-99: Segment 7.
        curves = {}
    },

    --- Command definitions.
    Commands = {},

    --- Attack definitions.
    Attacks = {},

    --- Character definitions.
    Characters = {},

    --- Item definitions.
    --
    -- Only items: weapons, armors, accessories, key items and materia are not
    -- stored here.
    Items = {},

    --- Weapon definitions.
    Weapons = {},

    --- Weapon definitions.
    Armors = {},

    --- Accessory definitions.
    Accessories = {},

    --- Materia definitions.
    Materia = {},

    --- Key items definitions.
    Key_Items = {},

    --- Summon names definitions.
    Summon_Names = {},
}

--- Playable characters
Characters = {}
for i = 1, 11 do
    Characters[i] = {}
end

--- The party inventory.
--
-- Items can be accessed with indexes 0 - MAX_INVENTORY_SLOTS. Money can be accessed with the
-- index "money." It has the following functions:
--   add_item(item, quantity)
--   remove_item(item, quantity)
--   get_item_quantity(item)
--   add_money(amount)
--   remove_money(amount)
--   money_to_2_banks(b1, b2, a1, a2)
Inventory = {}
Inventory.money = 0



FFVII = {}



FFVII.BattleType = {
    ATB = 0,
    WAIT = 1,
}



FFVII.Data = {
    progress_game = 0,
    moving_linear = false,
    moving_jump = false,

    money = 0,

    love_point_aeris = 50,
    love_point_tifa = 30,
    love_point_yuffie = 10,
    love_point_barret = 0,
    battle_love_aeris = 100,
    battle_love_tifa = 100,
    battle_love_yuffie = 100,
    battle_love_barret = 100,
    elevtr1_floor = 1,
    m1_door1_open = false,
    m1_door2_open = false,
    m1_jessie_unstuck = true,
    m1_bomb_set = false,
    m1_226 = false,
    m1_times_up = false,
    item_sector1_chest1 = false,

    -- TODO: From here on, what are these for?
    movieFrameNumber = 0,
    temp_6_4 = 0,
    expectedTriangleId = 0,
}



FFVII.Script = {
    is_executing = false,
}



FFVII.MenuSettings = {
    available = false,
    pause_available = false,

    battle_type = FFVII.BattleType.ATB,
    battle_speed = 10,
}



FFVII.ItemStorage = {
}

--- Current party.
--
-- @todo: Change to Party, don't include it in FFVII, which is to be deleted.
FFVII.Party = {
    [1] = nil,
    [2] = nil,
    [3] = nil
}

--- Memory banks.
--
-- Used to store gama data.
-- 15 banks (indexes 0-14) with 255 addresses (indexes 0-254) each. Memory banks 4, 5, 7, 8 and 9
-- are temporary and are not saved to the savemap.
Banks = {}
for i = 0, 14 do
    Banks[i] = {}
    for j = 0, 255 do
        Banks[i][j] = 0
    end
end


--- Memory banks for individual bits.
--
-- @todo Delete this, and implement bit operations over Banks.
BitBanks = {}
for i = 0, 15 do
    BitBanks[i] = {}
    for j = 0, 255 do
        BitBanks[i][j] = {}
        for k = 0, 7 do
            BitBanks[i][j][k] = 0
        end
    end
end


