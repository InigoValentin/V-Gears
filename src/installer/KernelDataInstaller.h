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
#include "SaveMap.h"

class KernelDataInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] path Path selected in the installation as source directory.
         */
        KernelDataInstaller(std::string path);

        /**
         * Destructor.
         */
        ~KernelDataInstaller();

        /**
         * Reads items and materia prices from the executable.
         */
        void ReadPrices();

        /**
         * Reads the command data from the kernel.
         *
         * @return The number of commands read.
         */
        int ReadCommands();

        /**
         * Saves command data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteCommands(std::string file_name);

        /**
         * Reads the attack data from the kernel.
         *
         * @return The number of attacks read.
         */
        int ReadAttacks();

        /**
         * Saves command data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteAttacks(std::string file_name);

        /**
         * Reads character data from the kernel.
         *
         * @return The number of characters read.
         */
        int ReadCharacters();

        /**
         * Saves character data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteCharacters(std::string file_name);

        /**
         * Reads growth data from the kernel.
         */
        void ReadGrowth();

        /**
         * Saves growth data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteGrowth(std::string file_name);

        /**
         * Reads the items from the kernel.
         *
         * @return The number of items read.
         */
        int ReadItems();

        /**
         * Saves item data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteItems(std::string file_name);

        /**
         * Reads the weapon info from the kernel.
         *
         * @return The number of weapons read.
         */
        int ReadWeapons();

        /**
         * Saves weapon data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteWeapons(std::string file_name);

        /**
         * Reads the armor info from the kernel.
         *
         * @return The number of armor read.
         */
        int ReadArmors();

        /**
         * Saves armor data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteArmors(std::string file_name);

        /**
         * Reads the accessory info from the kernel.
         *
         * @return The number of accessories read.
         */
        int ReadAccessories();

        /**
         * Saves accessory data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteAccessories(std::string file_name);

        /**
         * Reads materia info from the kernel.
         *
         * @return The number of accessories read.
         */
        int ReadMateria();

        /**
         * Saves materia data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteMateria(std::string file_name);

        /**
         * Reads key item info from the kernel.
         *
         * @return The number of key items read.
         */
        int ReadKeyItems();

        /**
         * Saves key item data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteKeyItems(std::string file_name);

        /**
         * Reads summon name info from the kernel.
         *
         * @return The number of summons read.
         */
        int ReadSummonNames();

        /**
         * Saves summon names data to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteSummonNames(std::string file_name);

        /**
         * Reads the initial savemap from the kernel.
         */
        void ReadInitialSaveMap();

        /**
         * Saves the initial savemap to a Lua script.
         *
         * @param[in] file_name Absolute path to the target Lua file.
         */
        void WriteInitialSaveMap(std::string file_name);

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
             * Attack descriptions section in KERNEL.BIN.
             */
            KERNEL_ATTACK_DESCRIPTIONS,

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
             * Atack Names section in KERNEL.BIN.
             */
            KERNEL_ATTACK_NAMES,

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
        enum RESTORE_TYPE{

            /**
             * The item affects a party member or group HP.
             */
            RESTORE_HP = 0,

            /**
             * The item affects a party member or group MP.
             */
            RESTORE_MP = 1,

            /**
             * The item affects a party member or group status.
             */
            RESTORE_STATUS = 2,

            /**
             * The item is not restorative.
             */
            RESTORE_NONE = 3
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
         * Data for each command.
         *
         * As found in KERNEL.BIN, file 0, in order, up to camera_multiple. Members after
         * camera_multiple are not found literally in KERNEL.BIN, but are derived from other
         * members or found in other files of the kernel (text files).
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
             * Determines the camera movement when the command is executed over a single target.
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
             * Not actually in the item file data, it's the order at which it appears.
             */
            int id;

            /**
             * Command name.
             *
             * Not found in the same file as the rest of the data, but on file 17.
             */
            std::string name;

            /**
             * Command description.
             *
             * Not found in the same file as the rest of the data, but on file 9.
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
         * As found in KERNEL.BIN, file 1, in order, up to camera_multiple. Members after
         * special_raw are not found literally in KERNEL.BIN, but are derived from other
         * members.
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
             *
             * Determines the camera movement when the command is executed over a single target.
             */
            u16 mp;

            /**
             * Impact sound. 2 bytes.
             *
             * Determines the camera movement when the command is executed over multiple targets.
             */
            u16 sound;

            /**
             * Camera movement ID for single target. 2 bytes.
             *
             * Determines the camera movement when the command is executed over a single target.
             */
            u16 camera_single;

            /**
             * Camera movement ID for multiple target. 2 bytes.
             *
             * Determines the camera movement when the command is executed over multiple targets.
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
             * This byte is divided into two nybbles (four bits). Upper nybble determines what
             * considerations are made in calculating damage such as physical/magical, allowing
             * criticals, how to calculate accuracy, etc. There are also three sets of known
             * formulae that are paired with the upper nybble values. These are selected in the
             * Lower Nybble and determine how to calculate pre-defense damage
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
             * See {@see restore_type} and {@see RESTORE_TYPE} for more information.
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
             * Attack ID.
             *
             * Not actually in the item file data, it's the order at which it appears.
             */
            int id;

            /**
             * Attack name.
             *
             * Not found in the same file as the rest of the data, but on file 18.
             */
            std::string name;

            /**
             * Attack description.
             *
             * Not found in the same file as the rest of the data, but on file 10.
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
             * The restore type.
             *
             * See {@see restore_type} and {@see RESTORE_TYPE} for more information.
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
         * Materia slot types.
         */
        enum SLOT_TYPE{

            /**
             * A non linked slot.
             */
            SLOT_UNLINKED = 0,

            /**
             * A linked slot.
             *
             * If the slot is in an even position, it's linked to the one in the right. If it's in
             * an odd position, it;s linked to the one in the Left. Remember that slot positions
             * are 0-index.
             */
            SLOT_LINKED = 1
        };

        /**
         * Bonus in stat givven by weapons or armor.
         */
        struct StatBonus{

            /**
             * ID of the raised stat.
             */
            u8 stat;

            /**
             * Stat value raise.
             */
            u8 bonus;
        };

        /**
         * Data for each weapon
         *
         * As found in KERNEL.BIN, file 5, in order, up to special. Members after special are not
         * found literally in KERNEL.BIN, but are derived from other members or found in other
         * files of the kernel (text files).
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
             * This byte is divided into two nybbles (four bits). Upper nybble determines what
             * considerations are made in calculating damage such as physical/magical, allowing
             * criticals, how to calculate accuracy, etc. There are also three sets of known
             * formulae that are paired with the upper nybble values. These are selected in the
             * Lower Nybble and determine how to calculate pre-defense damage
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
             * This byte is divided into two nybbles (four bits). Upper nybble determines the
             * attack animation modifier, and it's only used for Barret and Vincent. The lower
             * nybble is actually the index of the weapon model.
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
             * Used for both single and multiple targeted attacks with this weapon. Always 0xFFFF.
             */
            u16 camera;

            /**
             * Characters that can equip the weapon. 2 bytes.
             *
             * Contains bits indicating the characters that can equip the weapon {@see character}
             * for more info.
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
             * Contains bits indicating if the item is {@see selleable}, {@see useable_battle} and
             * {@see useable_menu}and {@see throwable}.
             */
            u16 restrict_raw;

            /**
             * Weapon ID.
             *
             * Not actually in the item file data, it's the order at which it appears.
             */
            int id;

            /**
             * Weapon name.
             *
             * Not found in the same file as the rest of the data, but on file 20.
             */
            std::string name;

            /**
             * Weapon description.
             *
             * Not found in the same file as the rest of the data, but on file 12.
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
             * It's the upper nybble in {@see model_raw}. It's only used for Barret and Vincent.
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
         * As found in KERNEL.BIN, file 6, in order, up to unknown_3. Members after unknown_3 are
         * not found literally in KERNEL.BIN, but are derived from other members or found in other
         * files of the kernel (text files).
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
             * Contains bits indicating the characters that can equip the armor {@see character}
             * for more info.
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
             * Contains bits indicating if the item is {@see selleable}, {@see useable_battle} and
             * {@see useable_menu}and {@see throwable}.
             */
            u16 restrict_raw;

            /**
             * Unknown (Always 0XFFFF). 1 byte.
             */
            u16 unknown_3;

            /**
             * Armor ID.
             *
             * Not actually in the item file data, it's the order at which it appears.
             */
            int id;

            /**
             * Armor name.
             *
             * Not found in the same file as the rest of the data, but on file 20.
             */
            std::string name;

            /**
             * Armor description.
             *
             * Not found in the same file as the rest of the data, but on file 12.
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
         * As found in KERNEL.BIN, file 7, in order, up to unknown_3. Members after unknown_3 are
         * not found literally in KERNEL.BIN, but are derived from other members or found in other
         * files of the kernel (text files).
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
             * Contains bits indicating the characters that can equip the armor {@see character}
             * for more info.
             */
            u16 equip_raw;

            /**
             * Weapon restrictions. 2 bytes.
             *
             * Contains bits indicating if the item is {@see selleable}, {@see useable_battle} and
             * {@see useable_menu}and {@see throwable}.
             */
            u16 restrict_raw;

            /**
             * Accessory ID.
             *
             * Not actually in the item file data, it's the order at which it appears.
             */
            int id;

            /**
             * Accessory name.
             *
             * Not found in the same file as the rest of the data, but on file 20.
             */
            std::string name;

            /**
             * Accessory description.
             *
             * Not found in the same file as the rest of the data, but on file 12.
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
         * As found in KERNEL.BIN, file 8, in order, up to attributes. Members after attributes are
         * not found literally in KERNEL.BIN, but are derived from other members or found in other
         * files of the kernel (text files).
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
             * 2 nybbles. The upper one is the subtype, and it's ignored in V-Gears. The lower one
             * the actual type.
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
             * Not actually in the item file data, it's the order at which it appears.
             */
            int id;

            /**
             * Accessory name.
             *
             * Not found in the same file as the rest of the data, but on file 20.
             */
            std::string name;

            /**
             * Accessory description.
             *
             * Not found in the same file as the rest of the data, but on file 12.
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
             * -1 means infinite times. Derived from {@see type_raw} and {@see attribute}.
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
             * It can be the number of times the ability can be executed, or the chance to
             * activate...
             */
            int ability_parameters[5];
        };

        /**
         * Data for each key item.
         *
         * There is no data file about key items in KERNEL.BIN, there are only names (file 24) and
         * descriptions (file 16)
         */
        struct KeyItemData{

            /**
             * Key item ID.
             *
             * Not actually in KERNEL.BIN, it;s the order at which they appear.
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

        /**
         * List of summon names.
         *
         * There is no data file about key items in KERNEL.BIN, there are only names (file 26).
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

        /**
         * Character data.
         *
         * Contains information about a character. Contained in the file 3 of the kernel, one
         * entry for each character. Each section is 56 bytes long. The character ID is not
         * actually in the data, but it's the order at which they appear. The character name is
         * also not in the dataand  it must be filled manually.
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
         * Althought any of them can be used for any stat, this is their intended use.
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
             * Experience curve. Base must be ignored when calculating. Gradient is quadratic.
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
            u8 base[8];

        };

        /**
         * Battle data.
         *
         * Contains several pieces of data related to character growth. Contained in the file 3 of
         * the kernel, after the character data (starts at offset 0x01F8). The kernel contains more
         * data, but it's not needed for V-Gears (random table, scene lookup...)
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

        /**
         * Retrieves the 3-letter name of a basic stat from it's id.
         *
         * @param[in] id The stat ID.
         * @return Stat name abbreviation. If the stat ID is unknown, "stat" will be returned.
         */
        std::string GetBasicStatName(int id);

        /**
         * Items read from the kernel.
         */
        std::vector<CommandData> commands_;

        /**
         * Attacks read from the kernel.
         */
        std::vector<AttackData> attacks_;

        /**
         * Character info read from the kernel.
         */
        std::vector<CharacterData> characters_;

        GrowthData growth_;

        /**
         * Items read from the kernel.
         */
        std::vector<ItemData> items_;

        /**
         * Weapons read from the kernel.
         */
        std::vector<WeaponData> weapons_;

        /**
         * Armors read from the kernel.
         */
        std::vector<ArmorData> armors_;

        /**
         * Accessories read from the kernel.
         */
        std::vector<AccessoryData> accessories_;

        /**
         * Materia read from the kernel.
         */
        std::vector<MateriaData> materia_;

        /**
         * Key items read from the kernel.
         */
        std::vector<KeyItemData> key_items_;

        /**
         * Summon names read from the kernel.
         */
        std::vector<SummonNameData> summon_names_;

        /**
         * The initial savemap structure.
         */
        SaveMap savemap_;

        /**
         * The kernel file.
         */
        BinGZipFile kernel_;

        /**
         * Path to ff7.exe file.
         */
        std::string exe_path_;

        /**
         * Item and materia shop prices.
         *
         * Indexes 0 - 127: Items.
         * Indexes 128 - 255: Weapons.
         * Indexes 256 - 287: Armors.
         * Indexes 288 - 319: Accessories
         * Indxes 320 - 415: Materia.
         */
        u32 prices_[416];

        /**
         * Attack names, required for other sections.
         */
        std::string attack_names_[128];

        /**
         * Command names, required for other sections.
         */
        std::string command_names_[128];

};
