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
#include <vector>
#include "common/TypeDefine.h"
#include "installer/extractors/KernelDataTypes.h"

/*
 * Records of the materia.
 */
namespace KernelData{

    /**
     * Types of materia.
     */
    enum class MATERIA_TYPE{

        /**
         * Magic materia.
         */
        MAGIC = 1,

        /**
         * Support materia.
         */
        SUPPORT,

        /**
         * Command materia.
         */
        COMMAND,

        /**
         * Independent materia.
         */
        INDEPENDENT,

        /**
         * Summon materia.
         */
        SUMMON
    };

    /**
     * Stat bonuses given by equipping materia.
     *
     * They can be negative.
     */
    struct MateriaStatBonus{

        /**
         * Strength bonus. Absolute.
         */
        int str;

        /**
         * Vitality bonus. Absolute.
         */
        int vit;

        /**
         * Magic bonus. Absolute.
         */
        int mag;

        /**
         * Spirit bonus. Absolute.
         */
        int spr;

        /**
         * Dexterity bonus. Absolute.
         */
        int dex;

        /**
         * Luck bonus. Absolute.
         */
        int lck;

        /**
         * Max HP bonus. Percentile.
         */
        int hp;

        /**
         * Max MP bonus. Percentile.
         */
        int mp;

        /**
         * Indicates if at least one of the stats is changed by the materia.
         */
        bool change;
    };

    /**
     * Data for each materia.
     *
     * As found in KERNEL.BIN, file 8, in order, up to attributes. Members
     * after attributes are not found literally in KERNEL.BIN, but are
     * derived from other members or found in other files of the kernel
     * (text files).
     */
    struct MateriaData{

        /**
         * AP amounts at which the materia levels up. 4 x 1 byte.
         *
         * It can be used to determine how many levels the materia has.
         */
        u16 level_up_ap[4];

        /**
         * Index of stat modifier. 1 byte.
         */
        u8 stat_raw;

        /**
         * Status effects. 3 bytes, not 4!
         */
        u32 status_raw;

        /**
         * Materia element. 1 byte.
         */
        u8 element;

        /**
         * Materia type. 1 byte.
         *
         * 2 nybbles. The upper one is the subtype, and it's ignored in
         * V-Gears. The lower one the is actual type.
         */
        u8 type_raw;

        /**
         * Materia attributes. 6 x 1 byte.
         *
         * They are mostly ignored in V-Gears.
         */
        u8 attribute[6];

        /**
         * Accessory ID.
         *
         * Not in the item file data, it's the order at which it appears.
         */
        int id;

        /**
         * Accessory name.
         *
         * Not in the same file as the rest of the data, but on file 20.
         */
        std::string name;

        /**
         * Accessory description.
         *
         * Not in the same file as the rest of the data, but on file 12.
         */
        std::string description;

        /**
         * Materia type.
         *
         * Derived from {@see type_raw}.
         */
        int type;

        /**
         * Materia type.
         *
         * Derived from {@see type_raw}.
         */
        int sub_type;

        /**
         * Stat bonuses for equipping the materia.
         *
         * Derived from {@see stat_raw}.
         */
        MateriaStatBonus stats;

        /**
         * Statuses the materia inflicts.
         *
         * Derived from {@see status_raw}.
         */
        std::vector<int> status;

        /**
         * Commands the materia enables in battle.
         *
         * Derived from {@see type_raw}.
         */
        std::vector<int> command;

        /**
         * Attacks the materia enables in battle, at each level.
         *
         * Derived from {@see type_raw} and {@see attribute}.
         */
        int attack[6];

        /**
         * Number of times per battle an attack can be used, per level.
         *
         * -1 means infinite times. Derived from {@see type_raw} and
         * {@see attribute}.
         */
        int attack_times[6];

        /**
         * Ability confered by the materia.
         *
         * Can be provided by independent and independent materia.
         */
        std::string ability;

        /**
         * Parameter for the ability, varies per level.
         *
         * It can be the number of times the ability can be executed, or the
         * chance to activate...
         */
        int ability_parameters[5];
    };
}
