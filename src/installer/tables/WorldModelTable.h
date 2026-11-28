/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#pragma once

/**
 * The models of the world maps, and the animations that each one uses.
 *
 * The data comes from the tables of ff7.exe (the 1998 build, .data section at 0x95bfd0 to 0x95cad4).
 * A record has 20 bytes: a pointer to the name of the model, a pointer to its HRC file, the number of
 * animations, a value (scale), and a pointer to the animations in a list of names of A files. The
 * code that selects the models of a map (0x74f5dc) has a jump table of 13 cases, and each case picks
 * a part of the records. The animations of every model have the same number of bones as the model.
 */
namespace WorldModelTable{

    /**
     * A model of the world maps.
     */
    struct Model{

        /// Name of the HRC file, without extension.
        const char* hrc;

        /// Name that the game gives the model.
        const char* name;

        /// Value that the game stores with the model. It is probably its scale, in fixed point
        /// where 0x1000 is 1, but this is not confirmed.
        unsigned int scale_raw;

        /// Names of the A files (animations) of the model, without extension. The first one is
        /// the first in the table.
        const char* animations[11];

        /// Number of animations.
        unsigned int animation_count;
    };

    /**
     * The models that are in the tables of the game. They are in the order of their first appearance.
     */
    static const Model MODELS[] = {
        {"aja", "choko", 0x200, {"asc", "asd", "ase"}, 3},
        {"dva", "tybrco", 0x280, {"dya"}, 1},
        {"aba", "buggy", 0x200, {"age", "aha", "ahb", "ahc", "ahd", "ahe"}, 6},
        {"aaa", "barrier", 0xdac, {"aae"}, 1},
        {"bkd", "condor", 0xc80, {"bld"}, 1},
        {"coc", "rocket", 0x8fc, {"cpb"}, 1},
        {"cpc", "rocketbase", 0x8fc, {"cqb"}, 1},
        {"cfc", "goldsaucer", 0xaf0, {"cgc"}, 1},
        {"aia", "canon", 0x1000, {"aie"}, 1},
        {"cnb", "rock", 0x834, {"cob"}, 1},
        {"cgd", "highwind", 0x300, {"cic"}, 1},
        {"ddd", "sbmrn", 0x200, {"dfe"}, 1},
        {"dkc", "tail", 0x300, {"dla"}, 1},
        {"bna", "diaweapon", 0x400, {"btd", "bte", "bua", "bub", "buc"}, 5},
        {"eke", "warship", 0x1b8, {"ena"}, 1},
        {"cmb", "midgulcanon", 0x1000, {"cna"}, 1},
        {"cid", "highwind02", 0x300, {"cma"}, 1},
        {"dyb", "ultima", 0x200, {"eic", "eid", "eie", "eja", "ejb", "ejc", "ejd"}, 7},
        {"cqc", "ruby", 0x280, {"dda", "ddb", "ddc"}, 3},
        {"cec", "gernika", 0x1000, {"cfb"}, 1},
        {"eje", "underseemk", 0x1000, {"ekd"}, 1},
        {"ble", "coralkey", 0x300, {"bme"}, 1},
        {"dic", "seeobject", 0x800, {"djb"}, 1},
        {"dga", "sbmrn2", 0x200, {"dib"}, 1},
        {"bud", "emerald", 0x280, {"cea", "ceb"}, 2},
        {"djc", "snowpole", 0x300, {"dkb"}, 1},
        {"bbe", "cloud", 0x200, {"bid", "bie", "bja", "bjb", "bjc", "bjd", "bje", "bka", "bkb", "bkc"}, 10},
        {"dlb", "tifa", 0x200, {"dse", "dta", "dtb", "dtc", "dtd", "dte", "dua", "dub", "duc", "dud", "due"}, 11},
        {"ata", "cid", 0x200, {"aze", "baa", "bab", "bac", "bad", "bae", "bba", "bbb", "bbc", "bbd"}, 10},
    };

    static const unsigned int MODEL_COUNT = sizeof(MODELS) / sizeof(MODELS[0]);

    /**
     * The models that the game loads for a situation of the world map, in the order of the table.
     * The first 11 are the overworld in different moments of the story, and the last two are the
     * underwater map (WM2) and the snowstorm map (WM3).
     */
    struct ModelSet{

        /// What the set is for.
        const char* description;

        /// Names of the HRC files of its models, in the order of the game.
        const char* models[13];

        /// Number of models.
        unsigned int model_count;
    };

    static const ModelSet MODEL_SETS[] = {
        {"overworld, start (choko..rock)", {"aja", "dva", "aba", "aaa", "bkd", "coc", "cpc", "cfc", "aia", "cnb"}, 10},
        {"overworld", {"aja", "aja", "cgd", "ddd", "bkd", "coc", "cpc", "cfc", "aia", "dkc", "aaa", "bna", "cnb"}, 13},
        {"overworld", {"aja", "aja", "cgd", "ddd", "bkd", "coc", "cpc", "cfc", "cmb", "dkc", "aaa", "bna", "cnb"}, 13},
        {"overworld", {"aja", "aja", "cid", "ddd", "bkd", "coc", "cpc", "cfc", "aia", "dkc", "aaa", "bna", "cnb"}, 13},
        {"overworld", {"aja", "aja", "cid", "ddd", "bkd", "coc", "cpc", "cfc", "cmb", "dkc", "aaa", "bna", "cnb"}, 13},
        {"overworld", {"aja", "aja", "cgd", "ddd", "bkd", "coc", "cpc", "cfc", "cmb", "dkc", "aaa", "dyb", "cnb"}, 13},
        {"overworld", {"aja", "aja", "cid", "ddd", "bkd", "coc", "cpc", "cfc", "cmb", "dkc", "aaa", "dyb", "cnb"}, 13},
        {"overworld", {"aja", "aja", "cgd", "ddd", "bkd", "coc", "cpc", "cfc", "cmb", "dkc", "cqc", "cnb"}, 12},
        {"overworld", {"aja", "aja", "cid", "ddd", "bkd", "coc", "cpc", "cfc", "cmb", "dkc", "cqc", "cnb"}, 12},
        {"overworld", {"eke", "aia"}, 2},
        {"overworld", {"eke", "aia"}, 2},
        {"underwater (WM2)", {"ddd", "cec", "eje", "ble", "dic", "dga", "bud"}, 7},
        {"snowstorm (WM3)", {"djc", "djc", "djc"}, 3},
    };

    static const unsigned int MODEL_SET_COUNT = sizeof(MODEL_SETS) / sizeof(MODEL_SETS[0]);


    /**
     * The model that each type of entity of the world scripts uses, for each situation of the map
     * (the same ones as MODEL_SETS). The type is the argument of the LoadModel call (opcode 0x300)
     * of the scripts, and nullptr means that the situation has no model for it.
     *
     * The table is in ff7.exe at 0x95f5d0 (32 bytes for every situation, the byte is the slot of the
     * model in the set plus one, and the game reads it at 0x75edd0 and 0x75ec57). The types 0, 1
     * and 2 are the player: Cloud, Tifa and Cid.
     */
    static const char* const TYPE_MODELS[13][32] = {
        {nullptr, nullptr, nullptr, nullptr, "aja", "dva", "aba", "aia", nullptr, nullptr, nullptr, nullptr, "bkd", nullptr, "cfc", "coc", "cpc", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "aaa", "cnb", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, "cgd", "aja", nullptr, nullptr, "aia", nullptr, "dkc", "bna", nullptr, "bkd", "ddd", "cfc", "coc", "cpc", nullptr, nullptr, "aja", nullptr, nullptr, nullptr, nullptr, "aaa", "cnb", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, "cgd", "aja", nullptr, nullptr, nullptr, nullptr, "dkc", "bna", nullptr, "bkd", "ddd", "cfc", "coc", "cpc", nullptr, nullptr, "aja", "cmb", nullptr, nullptr, nullptr, "aaa", "cnb", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, "cid", "aja", nullptr, nullptr, "aia", nullptr, "dkc", "bna", nullptr, "bkd", "ddd", "cfc", "coc", "cpc", nullptr, nullptr, "aja", nullptr, nullptr, nullptr, nullptr, "aaa", "cnb", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, "cid", "aja", nullptr, nullptr, nullptr, nullptr, "dkc", "bna", nullptr, "bkd", "ddd", "cfc", "coc", "cpc", nullptr, nullptr, "aja", "cmb", nullptr, nullptr, nullptr, "aaa", "cnb", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, "cgd", "aja", nullptr, nullptr, nullptr, nullptr, "dkc", nullptr, "dyb", "bkd", "ddd", "cfc", "coc", "cpc", nullptr, nullptr, "aja", "cmb", nullptr, nullptr, nullptr, "aaa", "cnb", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, "cid", "aja", nullptr, nullptr, nullptr, nullptr, "dkc", nullptr, "dyb", "bkd", "ddd", "cfc", "coc", "cpc", nullptr, nullptr, "aja", "cmb", nullptr, nullptr, nullptr, "aaa", "cnb", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, "cgd", "aja", nullptr, nullptr, nullptr, nullptr, "dkc", nullptr, nullptr, "bkd", "ddd", "cfc", "coc", "cpc", nullptr, nullptr, "aja", "cmb", nullptr, nullptr, nullptr, nullptr, "cnb", nullptr, nullptr, nullptr, "cqc", nullptr, nullptr},
        {nullptr, nullptr, nullptr, "cid", "aja", nullptr, nullptr, nullptr, nullptr, "dkc", nullptr, nullptr, "bkd", "ddd", "cfc", "coc", "cpc", nullptr, nullptr, "aja", "cmb", nullptr, nullptr, nullptr, nullptr, "cnb", nullptr, nullptr, nullptr, "cqc", nullptr, nullptr},
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "aia", "eke", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "aia", "eke", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "ddd", nullptr, nullptr, nullptr, "cec", "eje", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "ble", "dic", "dga", nullptr, "bud", nullptr},
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "djc", "djc", "djc", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
    };

    /**
     * Names that the game uses for the types of entities of the world scripts, from its model tables.
     * The ones without a name are not used.
     */
    static const char* const TYPE_NAMES[32] = {
        "cloud", "tifa", "cid", "highwind", "chocobo", "tiny bronco", "buggy", "canon", "warship",
        "tail", "diamond weapon", "ultimate weapon", "condor", "submarine", "gold saucer", "rocket",
        "rocket base", "gernika", "underwater mark", "chocobo (2)", "midgar cannon", "snowpole 1",
        "snowpole 2", "snowpole 3", "barrier", "rock", "coral key", "sea object", "submarine (2)",
        "ruby weapon", "emerald weapon", nullptr
    };

}
