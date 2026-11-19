-- The models of the world map and where each type of entity of the world scripts uses which one.
--
-- The data is from the tables of ff7.exe (see src/installer/WorldModelTable.h): the models are the
-- HRC files of WORLD_US.LGP with the name and the scale the game gives them, and "situations" has
-- the model that each type of entity has in each of the 13 situations of the map (the argument of
-- LoadModel in the scripts is the type). The scale is the raw value of the game: 0x200 is 1.

WorldMapModels = {
    --- Name that the game gives each type of entity.
    type_names = {
        [0] = "cloud",
        [1] = "tifa",
        [2] = "cid",
        [3] = "highwind",
        [4] = "chocobo",
        [5] = "tiny bronco",
        [6] = "buggy",
        [7] = "canon",
        [8] = "warship",
        [9] = "tail",
        [10] = "diamond weapon",
        [11] = "ultimate weapon",
        [12] = "condor",
        [13] = "submarine",
        [14] = "gold saucer",
        [15] = "rocket",
        [16] = "rocket base",
        [17] = "gernika",
        [18] = "underwater mark",
        [19] = "chocobo (2)",
        [20] = "midgar cannon",
        [21] = "snowpole 1",
        [22] = "snowpole 2",
        [23] = "snowpole 3",
        [24] = "barrier",
        [25] = "rock",
        [26] = "coral key",
        [27] = "sea object",
        [28] = "submarine (2)",
        [29] = "ruby weapon",
        [30] = "emerald weapon",
    },

    --- HRC file (without extension), name and raw scale of each model.
    models = {
        aja = {name = "choko", scale = 0x200},
        dva = {name = "tybrco", scale = 0x280},
        aba = {name = "buggy", scale = 0x200},
        aaa = {name = "barrier", scale = 0xdac},
        bkd = {name = "condor", scale = 0xc80},
        coc = {name = "rocket", scale = 0x8fc},
        cpc = {name = "rocketbase", scale = 0x8fc},
        cfc = {name = "goldsaucer", scale = 0xaf0},
        -- "animation" is the position of the animation (from 0, in the order of the game) that the model
        -- plays when it is placed, 0 if it is not there. Ultima Weapon: 0 stands, 1 flies, 2 is rammed
        -- by the Highwind, 3 threatens a town, 4 hovers, 5 starts the battle, 6 a transition. Ruby
        -- Weapon: 0 idle (out of the ground), 1 only the head shown, 2 the transition from 1 to 0.
        -- Diamond Weapon: 0 stands, 1 walks, 2 and 3 turn left and right, 4 pose before the battle.
        -- Emerald Weapon: 0 idle, 1 moving. "altitude" is a fixed height of the world (not over the
        -- ground) where the entity is put, instead of the terrain.
        -- The rotation of the model, in degrees (x, y, z), is how it has to be turned to face the way
        -- that the direction of the script says. It was found by looking at the model.
        aia = {name = "canon", scale = 0x1000, rotation = {0, 180, 0}},
        cnb = {name = "rock", scale = 0x834},
        cgd = {name = "highwind", scale = 0x300},
        ddd = {name = "sbmrn", scale = 0x200},
        dkc = {name = "tail", scale = 0x300},
        bna = {name = "diaweapon", scale = 0x400},
        eke = {name = "warship", scale = 0x1b8},
        cmb = {name = "midgulcanon", scale = 0x1000},
        cid = {name = "highwind02", scale = 0x300},
        dyb = {name = "ultima", scale = 0x200, animation = 4},
        cqc = {name = "ruby", scale = 0x280, animation = 1},
        cec = {name = "gernika", scale = 0x1000},
        eje = {name = "underseemk", scale = 0x1000},
        ble = {name = "coralkey", scale = 0x300},
        dic = {name = "seeobject", scale = 0x800},
        dga = {name = "sbmrn2", scale = 0x200},
        bud = {name = "emerald", scale = 0x280, animation = 0, altitude = -0.9},
        djc = {name = "snowpole", scale = 0x300},
        bbe = {name = "cloud", scale = 0x200},
        dlb = {name = "tifa", scale = 0x200},
        ata = {name = "cid", scale = 0x200},
    },

    --- For each situation, the HRC file of the model of each type of entity.
    situations = {
        [0] = {[4] = "aja", [5] = "dva", [6] = "aba", [7] = "aia", [12] = "bkd", [14] = "cfc", [15] = "coc", [16] = "cpc", [24] = "aaa", [25] = "cnb"},
        [1] = {[3] = "cgd", [4] = "aja", [7] = "aia", [9] = "dkc", [10] = "bna", [12] = "bkd", [13] = "ddd", [14] = "cfc", [15] = "coc", [16] = "cpc", [19] = "aja", [24] = "aaa", [25] = "cnb"},
        [2] = {[3] = "cgd", [4] = "aja", [9] = "dkc", [10] = "bna", [12] = "bkd", [13] = "ddd", [14] = "cfc", [15] = "coc", [16] = "cpc", [19] = "aja", [20] = "cmb", [24] = "aaa", [25] = "cnb"},
        [3] = {[3] = "cid", [4] = "aja", [7] = "aia", [9] = "dkc", [10] = "bna", [12] = "bkd", [13] = "ddd", [14] = "cfc", [15] = "coc", [16] = "cpc", [19] = "aja", [24] = "aaa", [25] = "cnb"},
        [4] = {[3] = "cid", [4] = "aja", [9] = "dkc", [10] = "bna", [12] = "bkd", [13] = "ddd", [14] = "cfc", [15] = "coc", [16] = "cpc", [19] = "aja", [20] = "cmb", [24] = "aaa", [25] = "cnb"},
        [5] = {[3] = "cgd", [4] = "aja", [9] = "dkc", [11] = "dyb", [12] = "bkd", [13] = "ddd", [14] = "cfc", [15] = "coc", [16] = "cpc", [19] = "aja", [20] = "cmb", [24] = "aaa", [25] = "cnb"},
        [6] = {[3] = "cid", [4] = "aja", [9] = "dkc", [11] = "dyb", [12] = "bkd", [13] = "ddd", [14] = "cfc", [15] = "coc", [16] = "cpc", [19] = "aja", [20] = "cmb", [24] = "aaa", [25] = "cnb"},
        [7] = {[3] = "cgd", [4] = "aja", [9] = "dkc", [12] = "bkd", [13] = "ddd", [14] = "cfc", [15] = "coc", [16] = "cpc", [19] = "aja", [20] = "cmb", [25] = "cnb", [29] = "cqc"},
        [8] = {[3] = "cid", [4] = "aja", [9] = "dkc", [12] = "bkd", [13] = "ddd", [14] = "cfc", [15] = "coc", [16] = "cpc", [19] = "aja", [20] = "cmb", [25] = "cnb", [29] = "cqc"},
        [9] = {[7] = "aia", [8] = "eke"},
        [10] = {[7] = "aia", [8] = "eke"},
        [11] = {[13] = "ddd", [17] = "cec", [18] = "eje", [26] = "ble", [27] = "dic", [28] = "dga", [30] = "bud"},
        [12] = {[21] = "djc", [22] = "djc", [23] = "djc"},
    },
}
