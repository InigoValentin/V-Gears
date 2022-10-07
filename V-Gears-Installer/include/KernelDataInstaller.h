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

#include "common/BinGZipFile.h"
#include "common/TypeDefine.h"
#include "Characters.h"

class KernelDataInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] path Absolute path to the KERNEL.BIN file
         */
        KernelDataInstaller(std::string path);

        /**
         * Destructor.
         */
        ~KernelDataInstaller();

        /**
         * Reads the items from the kernel.
         *
         * @return The number of items read.
         */
        int ReadItems();

        /**
         * Saves item data to an xml file
         *
         * @param file[in] Absolute path to the target XML file.
         */
        void WriteItems(std::string file);

    private:

        enum KERNEL_SECTIONS{

            /**
             * Command data section in KERNEL.BIN.
             */
            KERNEL_COMMAND_DATA = 0,

            /**
             * Attack data section in KERNEL.BIN.
             */
            KERNEL_ATTACK_DATA,

            /**
             * Battle and growth data section in KERNEL.BIN.
             */
            KERNEL_BATTLE_AND_GROWTH_DATA,

            /**
             * Initialization data section in KERNEL.BIN.
             */
            KERNEL_INITIALIZATION_DATA,

            /**
             * Item data section in KERNEL.BIN.
             */
            KERNEL_ITEM_DATA,

            /**
             * Weapon data section in KERNEL.BIN.
             */
            KERNEL_WEAPON_DATA,

            /**
             * Armor data section in KERNEL.BIN.
             */
            KERNEL_ARMOR_DATA,

            /**
             * Accessory data section in KERNEL.BIN.
             */
            KERNEL_ACCESSORY_DATA,

            /**
             * Materia data section in KERNEL.BIN.
             */
            KERNEL_MATERIA_DATA,

            /**
             * Command descriptions section in KERNEL.BIN.
             */
            KERNEL_COMMAND_DESCRIPTIONS,

            /**
             * Magic descriptions section in KERNEL.BIN.
             */
            KERNEL_MAGIC_DESCRIPTIONS,

            /**
             * Item descriptions section in KERNEL.BIN.
             */
            KERNEL_ITEM_DESCRIPTIONS,

            /**
             * Weapon descriptions section in KERNEL.BIN.
             */
            KERNEL_WEAPON_DESCRIPTIONS,

            /**
             * Armor descriptions section in KERNEL.BIN.
             */
            KERNEL_ARMOR_DESCRIPTIONS,

            /**
             * Accessory descriptions section in KERNEL.BIN.
             */
            KERNEL_ACCESSORY_DESCRIPTIONS,

            /**
             * Materia descriptions section in KERNEL.BIN.
             */
            KERNEL_MATERIA_DESCRIPTIONS,

            /**
             * Key Item descriptions section in KERNEL.BIN.
             */
            KERNEL_KEY_ITEM_DESCRIPTIONS,

            /**
             * Command Names section in KERNEL.BIN.
             */
            KERNEL_COMMAND_NAMES,

            /**
             * Magic Names section in KERNEL.BIN.
             */
            KERNEL_MAGIC_NAMES,

            /**
             * Item Names section in KERNEL.BIN.
             */
            KERNEL_ITEM_NAMES,

            /**
             * Weapon Names section in KERNEL.BIN.
             */
            KERNEL_WEAPON_NAMES,

            /**
             * Armor Names section in KERNEL.BIN.
             */
            KERNEL_ARMOR_NAMES,

            /**
             * Accessory Names section in KERNEL.BIN.
             */
            KERNEL_ACCESSORY_NAMES,

            /**
             * Materia Names section in KERNEL.BIN.
             */
            KERNEL_MATERIA_NAMES,

            /**
             * Key Item Names section in KERNEL.BIN.
             */
            KERNEL_KEY_ITEM_NAMES,

            /**
             * Battle and Battle-Screen Text section in KERNEL.BIN.
             */
            KERNEL_BATTLE_AND_BATTLE_SCREEN_TEXT,

            /**
             * Summon Attack Names section in KERNEL.BIN.
             */
            KERNEL_SUMMON_ATTACK_NAMES


        };

        enum ELEMENT{

            /**
             * Fire element.
             */
            FIRE = 0,


            /**
             * Ice element.
             */
            ICE,


            /**
             * Bolt element.
             */
            BOLT,


            /**
             * Earth element.
             */
            EARTH,


            /**
             * Poison element.
             */
            POISON,


            /**
             * Gravity element.
             */
            GRAVITY,


            /**
             * Water element.
             */
            WATER,


            /**
             * Wind element.
             */
            WIND,


            /**
             * Holy element.
             */
            HOLY,


            /**
             * Restorative element.
             */
            RESTORATIVE,


            /**
             * Cut element.
             */
            CUT,


            /**
             * Hit element.
             */
            HIT,


            /**
             * Punch element.
             */
            PUNCH,


            /**
             * Shoot element.
             */
            SHOOT,


            /**
             * Shout element.
             */
            SHOUT,


            /**
             * Hidden element.
             */
            HIDDEN,
        };

        /**
         * List of target selection options.
         */
        struct Target{

            /**
             * The target can be selected.
             *
             * If the target of the attack is selectable, at least between some subset of possible
             * targets, this must be true.
             */
            bool selection_enabled;

            /**
             * The default target is an enemy.
             *
             * When true, the cursor starts in an enemy row. When false, it starts in the ally row.
             * It may or may not be possible to change it.
             */
            bool default_enemy;

            /**
             * The default target is a group.
             *
             * When true, the cursor starts will all the members of a row selected. When false, it
             * starts with only one target selected. It may or may not be possible to change it.
             */
            bool default_multiple;

            /**
             * The selection can change between groups and individuals.
             *
             * When true, the target can be changed between groups and individuals When false,
             * only {@see default_multiple} determines if the target is an individual or a group.
             */
            bool toggle_multiple;

            /**
             * The target group can be changed.
             *
             * When true, the target cannot be changed between enemies and allies. In this case
             * {@see default_enemy} will determine the group that can be targeted.
             */
            bool fixed_row;

            /**
             * The command is short ranged.
             *
             * When true, if the target or the caster is not in the front of their row, the target
             * will take half damage. Also, when true, an enemy covered by another enemy can't be
             * targeted.
             */
            bool short_range;

            /**
             * Targets all, enemies and allies.
             *
             * When true, all allies and enemies are targets, and the selection can't be changed.
             */
            bool all_rows;

            /**
             * The target is random.
             *
             * When multiple targets are selected, one will be selected at random to be the
             * receiving target. Cursor will cycle among all viable targets.
             */
            bool random;

        };

        /**
         * Determines what an item affects on when used from the menu.
         */
        struct TargetCondition{

            /**
             * The item affects a party member or group HP.
             */
            bool party_hp;

            /**
             * The item affects a party member or group MP.
             */
            bool party_mp;

            /**
             * The item affects a party member or group status.
             */
            bool party_status;
        };

        /**
         * List of altered status.
         */
        enum STATUS{

            /**
             * The character is dead.
             */
            DEATH = 0,

            /**
             * The character has critical HP.
             */
            NEAR_DEATH,

            /**
             * The character is asleep.
             */
            SLEEP,

            /**
             * The character is poisoned.
             */
            POISONED,

            /**
             * The character is sad.
             */
            SADNESS,

            /**
             * The character is furious.
             */
            FURY,

            /**
             * The character is confused.
             */
            CONFU,

            /**
             * The character is silenced.
             */
            SILENCE,

            /**
             * The character is slow.
             */
            HASTE,

            /**
             * The character is
             */
            SLOW,

            /**
             * The character is stopped.
             */
            STOP,

            /**
             * The character is a frog.
             */
            FROG,

            /**
             * The character is small.
             */
            SMALL,

            /**
             * The character is going to be petrified.
             */
            SLOW_NUMB,

            /**
             * The character is petrified.
             */
            PETRIFY,

            /**
             * The character is regenerating health.
             */
            REGEN,

            /**
             * The character has a physical barrier.
             */
            BARRIER,

            /**
             * The character has a magical barrier
             */
            M_BARRIER,

            /**
             * The character reflects magical attacks.
             */
            REFLECT,

            /**
             * The character has dual status.
             *
             * @todo What does it do.
             */
            DUAL,

            /**
             * The character has a shield.
             */
            SHIELD,

            /**
             * The character has a death sentence.
             */
            D_SENTENCE,

            /**
             * The character is being manipulated.
             */
            MANIPULATE,

            /**
             * The character is berserk.
             */
            BERSERK,

            /**
             * The character is invincible.
             */
            PEERLESS,

            /**
             * The character is paralyzed.
             */
            PARALYSIS,

            /**
             * The character is blinded.
             */
            DARKNESS,

            /**
             * The character has dual drain.
             */
            DUAL_DRAIN,

            /**
             * The character has deathforce.
             */
            DEATH_FORCE,

            /**
             * The character has resist.
             */
            RESIST,

            /**
             * Cait' Sith lucky girl mode.
             */
            LUCKY_GIRL,

            /**
             * The character is imprisioned.
             */
            IMPRISONED
        };

        /**
         * How a status effect modifier attack or item can act.
         */
        enum STATUS_EFFECT{

            /**
             * Inflicts status changes.
             */
            INFLICT = 0,

            /**
             * Cures status changes.
             */
            CURE = 1,

            /**
             * Toggles status changes.
             *
             * If the target doesn't have the status change, inflict it. If the target already has
             * the status change, cure it.
             */
            TOGGLE = 2
        };

        struct StatusEffect{

            /**
             * The list of status to inflict or cure.
             */
            std::vector<int> status;

            /**
             * The chance of landing a status effect (out of 63).
             */
            u16 chance;

            /**
             * The status change mode.
             */
            STATUS_EFFECT mode;

        };

        /**
         * Data for each items
         *
         * As found in KERNEL.BIN, file 4, in order, up to special. Members after special are not
         * found literally in KERNEL.BIN, but are derived from other members or found in other
         * files of the kernel (text files).
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
             * Contains bits indicating if the item is {@see selleable}, {@see useable_battle} and
             * {@see useable_menu}.
             */
            u16 restrict_raw;

            /**
             * Targeting mode. 1 byte.
             *
             * See {@see target} and {@TargetModes} for more information.
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
             * This byte is divided into two nybbles (four bits). Upper nybble determines what
             * considerations are made in calculating damage such as physical/magical, allowing
             * criticals, how to calculate accuracy, etc. There are also three sets of known
             * formulae that are paired with the upper nybble values. These are selected in the
             * Lower Nybble and determine how to calculate pre-defense damage
             */
            u8 damage_raw;

            /**
             * The item power. 1 byte.
             *
             * Used for damage/healing calculation.
             */
            u8 power;

            /**
             * Item affected condition when used in menu. 1 byte.
             *
             * See {@condition} and {@ItemCondition} for more information.
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
             * The first six bits are the chance of inflicting/curing (out of 63). If the seventh
             * bit is set, the status is cured instead of inflicted. If the eighth bit is set, then
             * the status is toggled (cured if present, inflicted if not present).
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
             * Not actually in the item file data, it's the order at which it appears.
             */
            int id;

            /**
             * Item name.
             *
             * Not found in the same file as the rest of the data, but on file 19.
             */
            std::string name;

            /**
             * Item description.
             *
             * Not found in the same file as the rest of the data, but on file 11.
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
             * What the item affects when.
             *
             * Derived from {@see condition_raw}.
             */
            TargetCondition condition;

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
         * Items read from the kernel
         */
        std::vector<ItemData> items_;

        /**
         * The kernel file.
         */
        BinGZipFile kernel_;

};
