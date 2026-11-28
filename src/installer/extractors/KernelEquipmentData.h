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
 * Records of the weapons, armors and accessories.
 */
namespace KernelData{

    /**
     * Data for each weapon
     *
     * As found in KERNEL.BIN, file 5, in order, up to special. Members
     * after special are not found literally in KERNEL.BIN, but are derived
     * from other members or found in other kernel files (text files).
     */
    struct WeaponData{

        /**
         * Targeting mode. 1 byte.
         *
         * See {@see target} and {@see TargetModes} for more information.
         */
        u8 target_raw;

        /**
         * Effect ID, unused (Always 0XFF). 1 byte.
         */
        u8 unused_0;

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
         * Unused data (Always 0XFF). 1 byte.
         */
        u8 unused_1;

        /**
         * The weapon power. 1 byte.
         *
         * Used for damage calculation.
         */
        u8 power;

        /**
         * The status effect the weapon induces. 1 byte.
         */
        u8 status;

        /**
         * Weapon materia growth multiplier. 1 byte.
         */
        u8 growth;

        /**
         * Weapon critical chance. 1 byte.
         */
        u8 critical;

        /**
         * Weapon accuracy. 1 byte.
         */
        u8 accuracy;

        /**
         * Weapon battle model. 1 byte.
         *
         * This byte is divided into two nybbles (four bits). Upper nybble
         * determines the attack animation modifier, and it's only used for
         * Barret and Vincent. The lower nybble is actually the index of the
         * weapon model.
         */
        u8 model_raw;

        /**
         * Unused data (Always 0XFF). 1 byte.
         */
        u8 unused_2;

        /**
         * Consider it unused.
         *
         * @todo Verify.
         */
        u8 high_sound;

        /**
         * Camera movement ID. 2 bytes.
         *
         * Used for both single and multiple targeted attacks with this
         * weapon. Always 0xFFFF.
         */
        u16 camera;

        /**
         * Characters that can equip the weapon. 2 bytes.
         *
         * Contains bits indicating the characters that can equip the weapon
         * {@see character} for more info.
         */
        u16 equip_raw;

        /**
         * Weapon element ids.
         */
        u16 element_raw;

        /**
         * Unused data (Always 0XFFFF). 1 byte.
         */
        u8 unused_3;

        /**
         * Stat the weapon raises.
         *
         * {@see stat_bonus} for more info.
         */
        u8 stat_raw[4];

        /**
         * The amount the raised stats are raised by.
         *
         * {@see stat_bonus} for more info.
         */
        u8 stat_bonus_raw[4];

        /**
         * Materia slots.
         *
         * {@see slots} for more info.
         */
        u8 slots_raw[8];

        /**
         * Sound effect id for normal hit.
         */
        u8 sound;

        /**
         * Sound effect id for critical hit.
         */
        u8 sound_critical;

        /**
         * Sound effect id for miss hit.
         */
        u8 sound_miss;

        /**
         * Impcat effect ID. 1 byte.
         *
         * Used for animation in battle.
         */
        u8 effect;

        /**
         * Special flags. 2 bytes.
         *
         * @todo document and parse with info from
         * https://wiki.ffrtt.ru/index.php?title=FF7/Battle/Special_Attack_Flags
         */
        u16 special_raw;

        /**
         * Weapon restrictions. 2 bytes.
         *
         * Contains bits indicating if the item is {@see selleable},
         * {@see useable_battle} and {@see useable_menu} and
         * {@see throwable}.
         */
        u16 restrict_raw;

        /**
         * Weapon ID.
         *
         * Not actually in the item file data, it's the order at which it
         * appears.
         */
        int id;

        /**
         * Weapon name.
         *
         * Not in the same file as the rest of the data, but on file 20.
         */
        std::string name;

        /**
         * Weapon description.
         *
         * Not in the same file as the rest of the data, but on file 12.
         */
        std::string description;

        /**
         * Indicates if the weapon can be sold.
         *
         * True if the bit 0 in {@see restrict_raw} is 0.
         */
        bool sellable;

        /**
         * Indicates if the weapon can be used in battle.
         *
         * True if the bit 2 in {@see restrict_raw} is 0.
         */
        bool useable_battle;

        /**
         * Indicates if the weapon can be used in the menu.
         *
         * True if the bit 4 in {@see restrict_raw} is 0.
         */
        bool useable_menu;

        /**
         * Indicates if the weapon can be sold.
         *
         * True if the bit 8 in {@see restrict_raw} is 0.
         */
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
         * Attack animation ID.
         *
         * It's the upper nybble in {@see model_raw}. It's only used for
         * Barret and Vincent.
         */
        u8 animation_index;

        /**
         * Weapon model ID.
         *
         * It's the lower nybble in {@see model_raw}.
         */
        u8 model;

        /**
         * The weapon battle model index.
         *
         * It's the lower nybble in {@see model_raw}.
         */
        std::vector<u8> equip;

        /**
         * Bonus in stats.
         *
         * Derived from {@see stat_raw} and {@see stat_bonus_raw}.
         */
        std::vector<StatBonus> stat_bonus;

        /**
         * Materia slots.
         *
         * Derived and simplified from {@see slots_raw}.
         */
        std::vector<u8> materia_slots;

        /**
         * Elements of the weapon.
         *
         * Derived from {@see element_raw}.
         */
        std::vector<int> elements;

    };

    /**
     * Data for each armor
     *
     * As found in KERNEL.BIN, file 6, in order, up to unknown_3. Members
     * after unknown_3 are not found literally in KERNEL.BIN, but are
     * derived from other members or found in other kernel files (text
     * files).
     */
    struct ArmorData{

        /**
         * Unknown (Always 0XFF). 1 byte.
         */
        u8 unknown_0;

        /**
         * Elemental resistance mode. 1 byte.
         *
         * For all elements in {@see element_raw}:
         * If 0: Absorbs.
         * If 1: Nullifies.
         * If 2: Halves.
         * If any other value: normal.
         */
        u8 element_defense_mode;

        /**
         * Defense stat. 1 byte.
         */
        u8 defense;

        /**
         * Magic defense stat. 1 byte.
         */
        u8 m_defense;

        /**
         * Evasion stat. 1 byte.
         */
        u8 evasion;

        /**
         * Magic evasion stat. 1 byte.
         */
        u8 m_evasion;

        /**
         * The status effect the armor protects against. 1 byte.
         */
        u8 status;

        /**
         * Unknown (Always 0XFFFF). 1 byte.
         */
        u16 unknown_1;

        /**
         * Materia slots.
         *
         * {@see slots} for more info.
         */
        u8 slots_raw[8];

        /**
         * Armor materia growth multiplier. 1 byte.
         */
        u8 growth;

        /**
         * Characters that can equip the armor. 2 bytes.
         *
         * Contains bits indicating the characters that can equip the armor
         * {@see character} for more info.
         */
        u16 equip_raw;

        /**
         * Elements the armor protects agains.
         */
        u16 element_raw;

        /**
         * Unknown (Always 0XFF). 1 byte.
         */
        u16 unknown_2;

        /**
         * Stat the weapon raises.
         *
         * {@see stat_bonus} for more info.
         */
        u8 stat_raw[4];

        /**
         * The amount the raised stats are raised by.
         *
         * {@see stat_bonus} for more info.
         */
        u8 stat_bonus_raw[4];

        /**
         * Weapon restrictions. 2 bytes.
         *
         * Contains bits indicating if the item is {@see selleable},
         * {@see useable_battle} and {@see useable_menu} and
         * {@see throwable}.
         */
        u16 restrict_raw;

        /**
         * Unknown (Always 0XFFFF). 1 byte.
         */
        u16 unknown_3;

        /**
         * Armor ID.
         *
         * Not in the item file data, it's the order at which it appears.
         */
        int id;

        /**
         * Armor name.
         *
         * Not in the same file as the rest of the data, but on file 20.
         */
        std::string name;

        /**
         * Armor description.
         *
         * Not in the same file as the rest of the data, but on file 12.
         */
        std::string description;

        /**
         * Indicates if the armor can be sold.
         *
         * True if the bit 0 in {@see restrict_raw} is 0.
         */
        bool sellable;

        /**
         * Indicates if the armor can be used in battle.
         *
         * True if the bit 2 in {@see restrict_raw} is 0.
         */
        bool useable_battle;

        /**
         * Indicates if the armor can be used in the menu.
         *
         * True if the bit 4 in {@see restrict_raw} is 0.
         */
        bool useable_menu;

        /** Whether the item can be thrown (Throw command). */
        bool throwable;

        /**
         * The weapon battle model index.
         *
         * It's the lower nybble in {@see model_raw}.
         */
        std::vector<u8> equip;

        /**
         * Bonus in stats.
         *
         * Derived from {@see stat_raw} and {@see stat_bonus_raw}.
         */
        std::vector<StatBonus> stat_bonus;

        /**
         * Materia slots.
         *
         * Derived and simplified from {@see slots_raw}.
         */
        std::vector<u8> materia_slots;

        /**
         * Elements of the weapon.
         *
         * Derived from {@see element_raw}.
         */
        std::vector<int> elements;

    };

    /**
     * Data for each accessory
     *
     * As found in KERNEL.BIN, file 7, in order, up to unknown_3. Members
     * after unknown_3 are not found literally in KERNEL.BIN, but are not
     * found literally in KERNEL.BIN, but are derived from other members or
     * found in other files of the kernel (text files).
     */
    struct AccessoryData{

        /**
         * Stat the weapon raises.
         *
         * {@see stat_bonus} for more info.
         */
        u8 stat_raw[2];

        /**
         * The amount the raised stats are raised by.
         *
         * {@see stat_bonus} for more info.
         */
        u8 stat_bonus_raw[2];

        /**
         * Elemental resistance mode. 1 byte.
         *
         * For all elements in {@see element_raw}:
         * If 0: Absorbs.
         * If 1: Nullifies.
         * If 2: Halves.
         * If any other value: normal.
         */
        u8 element_defense_mode;

        /**
         * The special status effect the accesory induces.
         *
         * 0: Auto haste.
         * 1: Auto berserk.
         * 2: Curse.
         * 3: Auto reflect.
         * 4: Steal rate +.
         * 5: Manipulate rate +.
         * 6: Barrier + MBarrier.
         * Other: Nothing.
         */
        u8 effect;

        /**
         * Elements the armor protects against. 2 bytes.
         *
         * The defense type is defined in {@see element_defense_mode}.
         */
        u16 element_raw;

        /**
         * The status effect the armor protects against. 4 bytes.
         */
        u32 status_raw;

        /**
         * Characters that can equip the armor. 2 bytes.
         *
         * Contains bits indicating the characters that can equip the armor
         * {@see character} for more info. 
         * for more info.
         */
        u16 equip_raw;

        /**
         * Weapon restrictions. 2 bytes.
         *
         * Contains bits indicating if the item is {@see selleable},
         * {@see useable_battle} and {@see useable_menu} and
         * {@see throwable}.
         */
        u16 restrict_raw;

        /**
         * Accessory ID.
         *
         * Not in the item file data, it's the order at which itappears.
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
         * Indicates if the accessory can be sold.
         *
         * True if the bit 0 in {@see restrict_raw} is 0.
         */
        bool sellable;

        /**
         * Indicates if the accessory can be used in battle.
         *
         * True if the bit 2 in {@see restrict_raw} is 0.
         */
        bool useable_battle;

        /**
         * Indicates if the accessory can be used in the menu.
         *
         * True if the bit 4 in {@see restrict_raw} is 0.
         */
        bool useable_menu;

        /** Whether the item can be thrown (Throw command). */
        bool throwable;

        /**
         * The weapon battle model index.
         *
         * It's the lower nybble in {@see model_raw}.
         */
        std::vector<u8> equip;

        /**
         * Bonus in stats.
         *
         * Derived from {@see stat_raw} and {@see stat_bonus_raw}.
         */
        std::vector<StatBonus> stat_bonus;

        /**
         * Elements of the accesory.
         *
         * Derived from {@see element_raw}.
         */
        std::vector<int> elements;

        /**
         * Statuses the accessory protects against.
         *
         * Derived from {@see status_raw}.
         */
        std::vector<int> status;

    };
}
