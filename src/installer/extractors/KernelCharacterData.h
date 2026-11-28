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

#include <string>
#include "common/TypeDefine.h"

/*
 * Records of the characters and their growth.
 */
namespace KernelData{

    /**
     * Character data.
     *
     * Contains information about a character. Contained in the file 3 of
     * the kernel, one entry for each character. Each section is 56 bytes
     * long. The character ID is not actually in the data, but it's the
     * order at which they appear. The character name is also not in the
     * data and it must be filled manually.
     */
    struct CharacterData{

        /**
         * Strength stat growth curve. 1 byte.
         */
        u8 curve_str;

        /**
         * Vitality stat growth curve. 1 byte.
         */
        u8 curve_vit;

        /**
         * Magic stat growth curve. 1 byte.
         */
        u8 curve_mag;

        /**
         * Spirit stat growth curve. 1 byte.
         */
        u8 curve_spr;

        /**
         * Dexterity stat growth curve. 1 byte.
         */
        u8 curve_dex;

        /**
         * Luck stat growth curve. 1 byte.
         */
        u8 curve_lck;

        /**
         * HP stat growth curve. 1 byte.
         */
        u8 curve_hp;

        /**
         * MP stat growth curve. 1 byte.
         */
        u8 curve_mp;

        /**
         * Experience-for-next-level growth curve. 1 byte.
         */
        u8 curve_exp;

        /**
         * Ignored data. 1 byte.
         */
        u8 unused_0;

        /**
         * Starting level. 1 byte.
         */
        u8 initial_level;

        /**
         * Ignored data. 1 byte.
         */
        u8 unused_1;

        /**
         * 1-1 limit slot command. 1 byte.
         */
        u8 limit_1_1;

        /**
         * 1-2 limit slot command. 1 byte.
         */
        u8 limit_1_2;

        /**
         * 1-3 limit slot command. 1 byte, unused.
         */
        u8 limit_1_3;

        /**
         * 2-1 limit slot command. 1 byte.
         */
        u8 limit_2_1;

        /**
         * 2-2 limit slot command. 1 byte.
         */
        u8 limit_2_2;

        /**
         * 2-3 limit slot command. 1 byte, unused.
         */
        u8 limit_2_3;

        /**
         * 3-1 limit slot command. 1 byte.
         */
        u8 limit_3_1;

        /**
         * 3-2 limit slot command. 1 byte.
         */
        u8 limit_3_2;

        /**
         * 3-3 limit slot command. 1 byte, unused.
         */
        u8 limit_3_3;

        /**
         * 4-1 limit slot command. 1 byte.
         */
        u8 limit_4_1;

        /**
         * 4-2 limit slot command. 1 byte, unused.
         */
        u8 limit_4_2;

        /**
         * 4-3 limit slot command. 1 byte, unused.
         */
        u8 limit_4_3;

        /**
         * Kills required to unlock limit level 2. 2 bytes.
         */
        u16 limit_2_kills;

        /**
         * Kills required to unlock limit level 3. 2 bytes.
         */
        u16 limit_3_kills;

        /**
         * Kills required to unlock limit 1-2. 2 bytes.
         */
        u16 limit_1_2_uses;

        /**
         * Kills required to unlock limit 1-3. 2 bytes, unused.
         */
        u16 limit_1_3_uses;

        /**
         * Kills required to unlock limit 2-2. 2 bytes.
         */
        u16 limit_2_2_uses;

        /**
         * Kills required to unlock limit 2-3. 2 bytes, unused.
         */
        u16 limit_2_3_uses;

        /**
         * Kills required to unlock limit 3-2. 2 bytes.
         */
        u16 limit_3_2_uses;

        /**
         * Kills required to unlock limit 3-3. 2 bytes, unused.
         */
        u16 limit_3_3_uses;

        /**
         * HP divisor for limit level 1.
         */
        u32 limit_1_div;

        /**
         * HP divisor for limit level 2.
         */
        u32 limit_2_div;

        /**
         * HP divisor for limit level 3.
         */
        u32 limit_3_div;

        /**
         * HP divisor for limit level 4.
         */
        u32 limit_4_div;

        /**
         * Character ID.
         */
        int id;

        /**
         * Character default name.
         */
        std::string name;

    };

    /**
     * Types of stat curves.
     *
     * Althought any of them can be used for any stat, this is their
     * intended use.
     */
    enum STAT_CURVE_TYPE{

        /**
         * Normal curve, used for primary stat.
         */
        CURVE_PRIMARY = 0,

        /**
         * HP curve. Base values must be multiplied by 40 when calculating.
         */
        CURVE_HP,

        /**
         * MP curve. Base values must be multiplied by 2 when calculating.
         */
        CURVE_MP,

        /**
         * Experience curve. Base must be ignored when calculating. Gradient
         * is quadratic.
         */
        CURVE_EXP

    };

    /**
     * Stat curve.
     *
     * determines value growth
     */
    struct StatCurve{

        /**
         * Curve ID.
         */
        int id;

        /**
         * Type of curve.
         */
        STAT_CURVE_TYPE type;

        /**
         * Curve gradients.
         */
        u8 gradient[8];

        /**
         * Curve bases.
         */
        s8 base[8];

    };

    /**
     * Battle data.
     *
     * Contains several pieces of data related to character growth.
     * Contained in the file 3 of the kernel, after the character data
     * (starts at offset 0x01F8). The kernel contains more data, but it's
     * not needed for V-Gears (random table, scene lookup...)
     */
    struct GrowthData{

        /**
         * Random bonus to primary stats, 12 entries of 1 byte.
         */
        u8 bonus_stat[12];

        /**
         * Random bonus to HP stat, 12 entries of 1 byte.
         */
        u8 bonus_hp[12];

        /**
         * Random bonus to MP stat, 12 entries of 1 byte.
         */
        u8 bonus_mp[12];

        /**
         * Stat curves. 64 curves of 16 bit each.
         *
         * 37 for primary stats, 9 for HP, 9 form MP, 9 for experience.
         */
        StatCurve curves[64];
    };
}
