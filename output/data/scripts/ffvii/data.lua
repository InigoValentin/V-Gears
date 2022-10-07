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



FFVII.Party = {
}

FFVII.Banks = {}
for i = 1, 15 do
    FFVII.Banks[i] = {}
    for j = 1, 256 do
        FFVII.Banks[i][j] = 0
    end
end

FFVII.BitBanks = {}
for i = 1, 16 do
    FFVII.BitBanks[i] = {}
    for j = 1, 256 do
        FFVII.BitBanks[i][j] = {}
        for k = 1, 8 do
            FFVII.BitBanks[i][j][k] = 0
        end
    end
end


