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
 * Records of the items.
 */
namespace KernelData{

    /**
     * Data for each items
     *
     * As found in KERNEL.BIN, file 4, in order, up to special. Members
     * after special are not found literally in KERNEL.BIN, but are derived
     * from other members or found in other files (text files).
     */
    struct ItemData{

        /**
         * Unknown data (Always 0XFFFF). 4 bytes.
         */
        u32 unknown_0;

        /**
         * Unknown data (Always 0XFFFF). 4 bytes.
         */
        u32 unknown_1;

        /**
         * Camera movement ID. 2 bytes.
         *
         * Used for items useable in battle, determines the camera movement.
         */
        u16 camera;

        /**
         * Item restrictions. 2 bytes.
         *
         * Contains bits indicating if the item is {@see selleable},
         * {@see useable_battle} and {@see useable_menu}.
         */
        u16 restrict_raw;

        /**
         * Targeting mode. 1 byte.
         *
         * See {@see target} and {@see TargetModes} for more information.
         */
        u8 target_raw;

        /**
         * Item effect ID. 1 byte.
         *
         * Used for animation in battle.
         */
        u8 effect;

        /**
         * Damage formula. 1 byte.
         *
         * This byte is divided into two nybbles (four bits). Upper nybble
         * determines what considerations are made in calculating damage
         * such as physical/magical, allowing criticals, how to calculate
         * accuracy, etc. There are also three sets of known formulae that
         * are paired with the upper nybble values. These are selected in
         * the Lower Nybble and determine how to calculate pre-defense
         * damage.
         */
        u8 damage_raw;

        /**
         * The item power. 1 byte.
         *
         * Used for damage/healing calculation.
         */
        u8 power;

        /**
         * The restore type.
         *
         * See {@see restore_type} and {@see RESTORE_TYPE}.
         */
        u8 condition_raw;

        /**
         * Information about status change mode and chance. 1 bytes.
         */
        u8 status_change_raw;

        /**
         * Special flags. 1 byte.
         *
         * @todo Document and retrieve with info from
         * https://wiki.ffrtt.ru/index.php?title=FF7/Battle/Attack_Special_Effects
         */
        u8 additional_effects_raw;

        /**
         * Special flags. 1 byte.
         *
         * @todo Document and retrieve with info from
         * https://wiki.ffrtt.ru/index.php?title=FF7/Item_data
         */
        u8 additional_effects_mod_raw;

        /**
         * Information about what status can be inflicted or cured. 4 bytes.
         *
         * The first six bits are the chance of inflicting/curing (out of
         * 63). If the seventh bit is set, the status is cured instead of
         * inflicted. If the eighth bit is set, then the status is toggled
         * (cured if present, inflicted if not present).
         */
        u32 status_raw;

        /**
         * Information about the item elements. 2 bytes.
         */
        u16 element_raw;

        /**
         * Special flags. 2 bytes.
         *
         * @todo document and parse with info from
         * https://wiki.ffrtt.ru/index.php?title=FF7/Battle/Special_Attack_Flags
         */
        u16 special_raw;

        /**
         * Item ID.
         *
         * Not in the item file data, it's the order at which it appears.
         */
        int id;

        /**
         * Item name.
         *
         * Not in the same file as the rest of the data, but on file 19.
         */
        std::string name;

        /**
         * Item description.
         *
         * Not in the same file as the rest of the data, but on file 11.
         */
        std::string description;

        /**
         * Indicates if the item can be sold.
         *
         * True if the bit 0 in {@see restrict_raw} is 0.
         */
        bool sellable;

        /**
         * Indicates if the item can be used in battle.
         *
         * True if the bit 2 in {@see restrict_raw} is 0.
         */
        bool useable_battle;

        /**
         * Indicates if the item can be used in the menu.
         *
         * True if the bit 4 in {@see restrict_raw} is 0.
         */
        bool useable_menu;

        /** Whether the item can be thrown (Throw command). */
        bool throwable;

        /**
         * Target selection mode.
         *
         * Derived from {@see target_raw}.
         */
        Target target;

        /**
         * The damage formula.
         *
         * It's the upper nybble in {@see damage_raw}.
         */
        u8 damage_formula;

        /**
         * The damage modifier.
         *
         * It's the lower nybble in {@see damage_raw}.
         */
        u8 damage_modifier;

        /**
         * What the item restores when used.
         *
         * Same as {@see condition_raw}, except for the wrong cases.
         */
        RESTORE_TYPE restore_type;

        /**
         * Status effects.
         *
         * Derived from {@see status_change_raw} and {@see status_raw}.
         */
        StatusEffect status;

        /**
         * Elements of the item.
         *
         * Derived from {@see element_raw}.
         */
        std::vector<int> elements;

    };

    /**
     * Data for each key item.
     *
     * There is no data file about key items in KERNEL.BIN, there are only
     * names (file 24) and descriptions (file 16)
     */
    struct KeyItemData{

        /**
         * Key item ID.
         *
         * Not in KERNEL.BIN, it's the order at which they appear.
         */
        int id;

        /**
         * Key item name.
         */
        std::string name;

        /**
         * Key item description.
         */
        std::string description;
    };
}
