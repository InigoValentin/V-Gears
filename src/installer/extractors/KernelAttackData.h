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
 * Records of the commands and the attacks.
 */
namespace KernelData{

    /**
     * Menus that must be shown when selecting a command.
     */
    enum COMMAND_MENU{

        /**
         * No menu, execute command directly, without target selection.
         */
        NONE = 0,

        /**
         * Show magic menu.
         */
        MAGIC = 1,

        /**
         * Show summon menu.
         */
        SUMMON = 2,

        /**
         * Show item menu.
         */
        ITEM = 3,

        /**
         * Show enemy skill menu.
         */
        E_SKILL = 4,

        /**
         * Show throw menu.
         */
        THROW = 5,

        /**
         * Show limit menu.
         */
        LIMIT = 6,

        /**
         * No menu, skip to target selection.
         */
        TARGET = 7,

        /**
         * Show W-Magic menu
         */
        W_MAGIC = 8,

        /**
         * Show W-Summon menu
         */
        W_SUMMON = 9,

        /**
         * Show W-Item menu
         */
        W_ITEM = 10,

        /**
         * Show Coin menu
         */
        COIN = 11,

    };

    /**
     * Data for each command.
     *
     * As found in KERNEL.BIN, file 0, in order, up to camera_multiple.
     * Members after camera_multiple are not found literally in KERNEL.BIN,
     * but are derived from other members or found in other files of the
     * kernel (text files).
     */
    struct CommandData{

        /**
         * Cursor action when the command is selected.
         */
        u8 cursor_action;

        /**
         * Targeting mode. 1 byte.
         *
         * See {@see target} and {@see TargetModes} for more information.
         */
        u8 target_raw;

        /**
         * Unknown data (Always 0XFFFF). 2 bytes.
         */
        u16 unknown;

        /**
         * Camera movement ID for single target. 2 bytes.
         *
         * Determines the camera movement when the command is executed over
         * a single target.
         */
        u16 camera_single;

        /**
         * Camera movement ID for multiple target. 2 bytes.
         *
         * Determines the camera movement when the command is executed over multiple targets.
         */
        u16 camera_multiple;

        /**
         * Command ID.
         *
         * Not actually in the item file data, it's the order at which it
         * appears.
         */
        int id;

        /**
         * Command name.
         *
         * Not in the same file as the rest of the data, but on file 17.
         */
        std::string name;

        /**
         * Command description.
         *
         * Not in the same file as the rest of the data, but on file 9.
         */
        std::string description;

        /**
         * Target selection mode.
         *
         * Derived from {@see target_raw}.
         */
        Target target;

        /**
         * Menu name
         *
         * Derived from {@see cursor_action}.
         */
        std::string menu;

    };

    /**
     * Data for each attack.
     *
     * As found in KERNEL.BIN, file 1, in order, up to camera_multiple.
     * Members after special_raw are not found literally in KERNEL.BIN, but
     * are derived from other members.
     */
    struct AttackData{

        /**
         * Probability of the attack to land. 1 byte.
         */
        u8 accuracy;

        /**
         * Impact effect. 1 byte.
         *
         * The sprite shown when the attack hits.
         */
        u8 impact_effect;

        /**
         * Index of the animation played by the target when hit. 1 byte.
         */
        u8 hurt_anim;

        /**
         * Unknown data. 1 bytes.
         */
        u8 unknown;

        /**
         * MP cost. 2 bytes.
         */
        u16 mp;

        /**
         * Impact sound. 2 bytes.
         */
        u16 sound;

        /**
         * Camera movement ID for single target. 2 bytes.
         *
         * Determines the camera movement when the command is executed over
         * a single target.
         */
        u16 camera_single;

        /**
         * Camera movement ID for multiple target. 2 bytes.
         *
         * Determines the camera movement when the command is executed over
         * multiple targets.
         */
        u16 camera_multiple;

        /**
         * Targeting mode. 1 byte.
         *
         * See {@see target} and {@see TargetModes} for more information.
         */
        u8 target_raw;

        /**
         * Atack effect ID. 1 byte.
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
         * The attack power. 1 byte.
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
         * Attack ID.
         *
         * Not in the item file data, it's the order at which it appears.
         */
        int id;

        /**
         * Attack name.
         *
         * Not in the same file as the rest of the data, but on file 18.
         */
        std::string name;

        /**
         * Attack description.
         *
         * Not in the same file as the rest of the data, but on file 10.
         */
        std::string description;

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
         * What the attack restores when used.
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
         * Elements of the attack.
         *
         * Derived from {@see element_raw}.
         */
        std::vector<int> elements;

    };

    /**
     * List of summon names.
     *
     * There is no data file about key items in KERNEL.BIN, there are only
     * names (file 26).
     */
    struct SummonNameData{

        /**
         * Summon ID.
         *
         * Not actually in KERNEL.BIN, it;s the order at which they appear.
         */
        int id;

        /**
         * Summon name.
         */
        std::string name;
    };
}
