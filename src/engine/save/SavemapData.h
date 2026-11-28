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

/**
 * The records of a savemap, and their limits.
 */
struct SavemapData{

    /**
     * Maximum number of characters.
     */
    static constexpr unsigned int MAX_CHARACTERS = 11;

    /**
     * Maximum number of inventory slots.
     */
    static constexpr unsigned int MAX_ITEM_SLOTS = 500;

    /**
     * Maximum number of inventory slots for key items.
     */
    static constexpr unsigned int MAX_KEY_ITEM_SLOTS = 100;

    /**
     * Maximum number of materia slots.
     */
    static constexpr unsigned int MAX_MATERIA_SLOTS = 500;

    /**
     * Maximum number of materia slots in the stash.
     */
    static constexpr unsigned int MAX_STASH_SLOTS = 500;

    /**
     * Maximum number of materia slots in a weapon or armor.
     */
    static constexpr unsigned int MAX_EQUIP_SLOTS = 10;

    /**
     * Maximum number of skills in an Enemy Skill materia.
     */
    static constexpr unsigned int MAX_ENEMY_SKILLS = 32;

    /**
     * Maximum limit level.
     */
    static constexpr unsigned int MAX_LIMIT_LEVELS = 4;

    /**
     * Maximum limit techniques per level.
     */
    static constexpr unsigned int MAX_LIMIT_TECHNIQUES = 4;

    /**
     * Level at which the limit level is full.
     */
    static constexpr unsigned int MAX_LIMIT_BAR = 254;

    /**
     * Maximum number of party member.
     */
    static constexpr unsigned int MAX_PARTY_MEMBERS = 3;

    /**
     * Number of data banks.
     */
    static constexpr unsigned int BANK_COUNT = 16;

    /**
     * Number of addresses in each data bank.
     */
    static constexpr unsigned int BANK_ADDRESS_COUNT = 256;

    /**
     * Max colour component value (they are stored in a byte).
     */
    static constexpr unsigned int MAX_COLOUR = 255;

    /**
     * Location information.
     *
     * Used for fields and world map.
     */
    struct Location{

        /**
         * X coordinate.
         */
        float x;

        /**
         * Y coordinate.
         */
        float y;

        /**
         * Z coordinate.
         *
         * Usually unused.
         */
        float z;

        /**
         * Triangle in the walkmesh;
         */
        unsigned int triangle;

        /**
         * Facing direction.
         */
        unsigned int angle;

        /**
         * Current field map.
         *
         * If null or empty, it means the player is in in the world map.
         */
        std::string field;

        /**
         * Location name. Used for the saveslot.
         */
        std::string name;
    };

    /**
     * A materia structure.
     */
    struct Materia{

        /**
         * Materia ID.
         */
        int id;

        /**
         * Total AP in the materia.
         */
        unsigned int ap;

        /**
         * Indicates if the materia is an enemy skill materia.
         */
        bool enemy_skill;

        /**
         * Learned enemy skills (only for Enemy Skill materia).
         */
        bool enemy_skill_learned[MAX_ENEMY_SKILLS];
    };

    /**
     * Item structure.
     */
    struct Item{

        /**
         * Item ID.
         */
        int id;

        /**
         * Item stock.
         */
        unsigned int quantity;
    };

    /**
     * A character entri in the savemap.
     */
    struct Character{

        /**
         * A character stat.
         */
        struct Stat{

            /**
             * Stat base value.
             */
            unsigned int base;

            /**
             * For STR, VIT, DEX and LCK, it means the source bonus. For HP and MP, it means
             * the current values.
             */
            unsigned int extra;
        };

        /**
         * A weapon or armor.
         */
        struct Equipment{

            /**
             * Item ID of the weapon or armor.
             */
            u16 id;

            /**
             * Materia in the weapon/armor.
             */
            Materia materia[MAX_EQUIP_SLOTS];
        };

        /**
         * Character ID, sequential.
         */
        unsigned int id;

        /**
         * Character ID, can change with the history. -1 to not use.
         */
        int char_id;

        /**
         * Indicates if the character has been unlocked for the PHS.
         */
        bool enabled;

        /**
         * Indicates if the character is locked in or out of the PHS.
         */
        bool locked;

        /**
         * Character name.
         */
        std::string name;

        /**
         * Character level.
         */
        unsigned int level;

        /**
         * Enemies killed by the character.
         */
        unsigned int kills;

        /**
         * Indicates if the character is in the back_row;
         */
        bool back_row;

        /**
         * Total experience.
         */
        unsigned int exp;

        /**
         * Experience required to reach next level.
         */
        unsigned int exp_to_next;

        /**
         * STR stat.
         */
        Stat str;

        /**
         * VIT stat.
         */
        Stat vit;

        /**
         * MAG stat.
         */
        Stat mag;

        /**
         * SPR stat.
         */
        Stat spr;

        /**
         * DEX stat.
         */
        Stat dex;

        /**
         * LCK stat.
         */
        Stat lck;

        /**
         * HP stat.
         */
        Stat hp;

        /**
         * MP stat.
         */
        Stat mp;

        /**
         * Current limit level.
         */
        unsigned int limit_level;

        /**
         * Current limit bar.
         */
        unsigned int limit_bar;

        /**
         * Learned limits.
         *
         * Grouped by level and technique.
         */
        bool limits_learned[MAX_LIMIT_LEVELS][MAX_LIMIT_TECHNIQUES];

        /**
         * Uses of techniques in each limit level.
         */
        unsigned int limit_uses[MAX_LIMIT_LEVELS];

        /**
         * Equipped weapon;
         */
        Equipment weapon;

        /**
         * Equipped weapon;
         */
        Equipment armor;

        /**
         * Equipped accessory ID. -1 if none equipped.
         */
        int accessory;

        /**
         * Current status list.
         */
        std::vector<u8> status;
    };

    /**
     * Names of the corners of the windows in the saved games, in the order of their colours.
     */
    static const char* const CORNER_NAMES[4];
};
