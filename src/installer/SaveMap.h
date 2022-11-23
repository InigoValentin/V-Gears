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

#include "common/TypeDefine.h"

/**
 * The structure of a savemap.
 */
struct SaveMap{

    /**
     * Contains the window colours, grouped by corner. 12 bytes.
     */
    struct WindowColor{

        /**
         * Each corner of the window. Three colour components. 3 bytes.
         */
        struct Corner{

            /**
             * Red component of the corner. 1 byte.
             */
            u8 red;

            /**
             * Green component of the window. 1 byte.
             */
            u8 green;

            /**
             * BLue component of the window. 1 byte.
             */
            u8 blue;
        };

        /**
         * Top left corner. 3 bytes.
         */
        Corner top_left;

        /**
         * Top right corner. 3 bytes.
         */
        Corner top_right;

        /**
         * Bottom left corner. 3 bytes.
         */
        Corner bottom_left;

        /**
         * Bottom right corner. 3 bytes.
         */
        Corner bottom_right;
    };

    /**
     * A materia. 4 bytes.
     *
     * @todo
     */
    struct Materia{

        /**
         * Materia index. 1 byte.
         */
        u8 id;

        /**
         * Materia AP. 3 bytes, not four.
         *
         * For Enemy Skill materia, ap is not the actual AP, but each byte set indicates a learned
         * skill.
         */
        u32 ap;

    };

    /**
     * Type of inventory item record.
     */
    enum class INVENTORY_TYPE{

        /**
         * A normal item.
         */
        ITEM = 0,

        /**
         * A weapon.
         */
        WEAPON,

        /**
         * An armor.
         */
        ARMOR,

        /**
         * An accessory.
         */
        ACCESSORY
    };

    /**
     * Items, weapons, armors and accessories in inventory. 2 byte.
     *
     * The first 7 bits are the item quantity. The 9 last bits are the index.
     */
    struct Item{

        /**
         * The item bytes, as read from KERNEL.BIN in the save map.
         *
         * The first 7 bits are the item quantity. The 9 last bits are the index. All the other
         * members can be derived from this. Indexex 0-127 are items, 128-255 are weapons, 256-287
         * are weapons and 288-319 are accessories.
         */
        u16 item_raw;

        /**
         * Item index.
         */
        int item;

        /**
         * Item quantity.
         */
        int quantity;
    };

    /**
     * Information about the player location in a field map. 7 bytes.
     */
    struct FieldPosition{

        /**
         * X coordinate on the map. 2 bytes.
         */
        u16 x;

        /**
         * Y coordinate on the map. 2 bytes.
         */
        u16 y;

        /**
         * Triangle of the walkmesh. 2 bytes.
         */
        u16 triangle;

        /**
         * Player orientation. 1 byte.
         */
        u8 orientation;
    };

    /**
     * Character record. 132 bytes.
     */
    struct Character{

        /**
         * Character identifier 1 byte.
         *
         * It's not the standard character ID. Used only for Cait Sith and Vincent, to distinguish
         * them from Young Cloud and Sephiroth. V-Gears mostly ignores this.
         */
        u8 identifier;

        /**
         * Character level. 1 byte.
         */
        u8 level;

        /**
         * Character strength stat. 1 byte.
         */
        u8 str;

        /**
         * Character vitality stat. 1 byte.
         */
        u8 vit;

        /**
         * Character magic stat. 1 byte.
         */
        u8 mag;

        /**
         * Character spirit stat. 1 byte.
         */
        u8 spr;

        /**
         * Character dexterity stat. 1 byte.
         */
        u8 dex;

        /**
         * Character luck stat. 1 byte.
         */
        u8 lck;

        /**
         * Character strength bonus from sources. 1 byte.
         */
        u8 str_bonus;

        /**
         * Character vitality bonus from sources. 1 byte.
         */
        u8 vit_bonus;

        /**
         * Character magic bonus from sources. 1 byte.
         */
        u8 mag_bonus;

        /**
         * Character spirit bonus from sources. 1 byte.
         */
        u8 spr_bonus;

        /**
         * Character dexterity bonus from sources. 1 byte.
         */
        u8 dex_bonus;

        /**
         * Character luck bonus from sources. 1 byte.
         */
        u8 lck_bonus;

        /**
         * Current limit level (1-4). 1 byte.
         */
        u8 limit_level;

        /**
         * Limit bar fill status (0x00: empty, 0xFF, full)
         */
        u8 limit_bar;

        /**
         * Character name. FF Text format, ended in 0xFF. 12 bytes.
         */
        u8 name_raw[12];

        /**
         * Equipped weapon ID. 1 byte.
         */
        u8 weapon;

        /**
         * Equipped armor ID. 1 byte.
         */
        u8 armor;

        /**
         * Equipped accessory ID. 0xFF if none. 1 byte.
         */
        u8 accessory;

        /**
         * Fury/sadness flags. 1 byte.
         *
         * 0x10: Sadness.
         * 0x20: Fury.
         * Other: None.
         */
        u8 status_raw;

        /**
         * Character row. 1 byte.
         *
         * 0xFF: Front row (default).
         * 0xFE: Back row.
         */
        u8 row_raw;

        /**
         * Progress of the level bar. 1 byte.
         *
         * Cosmetic only. V-Gears ignores it.
         */
        u8 level_progress;

        /**
         * Learned limits. 2 bytes.
         *
         * XORed values:
         * 0x0001: 1-1 Learned.
         * 0x0002: 1-2 Learned.
         * 0x0008: 2-1 Learned.
         * 0x0010: 2-2 Learned.
         * 0x0040: 3-1 Learned.
         * 0x0080: 3-2 Learned.
         * 0x0200: 4-1 Learned.
         */
        u16 learned_limits_raw;

        /**
         * Number of monster kills. 2 bytes.
         */
        u16 kills;

        /**
         * Number of times the limit 1-1 has been used. 3 x 2 bytes.
         *
         * 0: Uses of limit 1-1.
         * 1: Uses of limit 2-1.
         * 2: Uses of limit 3-1.
         */
        u16 limit_uses[3];

        /**
         * Current HP. 2 bytes.
         */
        u16 hp;

        /**
         * Base HP. 2 bytes.
         *
         * The max HP before materia alterations.
         */
        u16 base_hp;

        /**
         * Current MP. 2 bytes.
         */
        u16 mp;

        /**
         * Base MP. 2 bytes.
         *
         * The max MP before materia alterations.
         */
        u16 base_mp;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown;

        /**
         * Max HP. 2 bytes.
         *
         * The max HP after materia alterations.
         */
        u16 max_hp;

        /**
         * Max MP. 2 bytes.
         *
         * The max MP after materia alterations.
         */
        u16 max_mp;

        /**
         * Total gained experience. 4 bytes.
         */
        u32 exp;

        /**
         * Materia equipped in the weapon. 8 x 4 bytes.
         */
        Materia weapon_materia[8];

        /**
         * Materia equipped in the armor. 8 x 4 bytes.
         */
        Materia armor_materia[8];

        /**
         * Remaining experience to reach next level. 4 bytes.
         */
        u32 exp_to_next;

        /**
         * Character name.
         *
         * Derived from {@see name_raw}.
         */
        std::string name;

        /**
         * Indicates if the character is in fury state.
         *
         * Derived from {@see status_raw}.
         */
        bool fury;

        /**
         * Indicates if the character is in saddness state.
         *
         * Derived from {@see status_raw}.
         */
        bool sadness;

        /**
         * Indicates if the character is in the back row.
         */
        bool back_row;

        /**
         * Indicates which limits the character have learned.
         */
        bool limits_learned[4][2];

    };

    /**
     * Each of the memory banks dumped to the savemap. 256 bytes.
     */
    struct MemoryBank{

        /**
         * Bank data. 256 x 1 byte.
         */
        u8 byte[256];
    };

    /**
     * Selections related to the PHS. 2 bytes.
     */
    struct PhsSelection{

        /**
         * The mask, as in the savemap.
         *
         * The first 9 bits, when set, represents characters, sorted by id.
         */
        u16 mask;

        /**
         * Character seelction with the mask applied. Selected character IDs are true.
         */
        bool character[9];
    };

    /**
     * User defined settings. 21 bytes.
     */
    struct UserSettings{

        /**
         * Key mappings. 16 bytes.
         */
        struct KeyMapping{

            /**
             * L2 key mapping. 1 byte.
             */
            u8 l2;

            /**
             * R2 key mapping. 1 byte.
             */
            u8 r2;

            /**
             * L1 key mapping. 1 byte.
             */
            u8 l1;

            /**
             * R1 key mapping. 1 byte.
             */
            u8 r1;

            /**
             * Triangle key mapping. 1 byte.
             */
            u8 triangle;

            /**
             * Circle key mapping. 1 byte.
             */
            u8 circle;

            /**
             * Cross key mapping. 1 byte.
             */
            u8 cross;

            /**
             * Square key mapping. 1 byte.
             */
            u8 square;

            /**
             * Select key mapping. 1 byte.
             */
            u8 select;

            /**
             * Unknown key mapping. 1 byte.
             */
            u8 unknown_0;

            /**
             * Unknown key mapping. 1 byte.
             */
            u8 unknown_1;

            /**
             * Start key mapping. 1 byte.
             */
            u8 start;

            /**
             * Up key mapping. 1 byte.
             */
            u8 up;

            /**
             * Right key mapping. 1 byte.
             */
            u8 right;

            /**
             * Down key mapping. 1 byte.
             */
            u8 down;

            /**
             * Left key mapping. 1 byte.
             */
            u8 left;
        };

        /**
         * Battle ATB speed, 0x00: Fast, 0xFF: Slow. 1 byte.
         */
        u8 battle_speed;

        /**
         * Battle message speed, 0x00: Fast, 0xFF: Slow. 1 byte.
         */
        u8 battle_message_speed;

        /**
         * Several XORed settings. 1 byte.
         *
         * Sound: mono (0x00); stereo (0x01).
         * Controller: normal (0x00); customize (0x04).
         * Cursor: initial (0x00); memory (0x10).
         * ATB: Active (0x00); Recommended (0x40); Wait (0x80).
         */
        u8 mask_1;

        /**
         * Several XORed settings. 1 byte.
         *
         * Camera angle: Auto (0x00); Fix (0x01)
         * Magic order: Restore attack indirect (0x00)
         * Magic order: Restore indirect attack (0x04)
         * Magic order: Attack indirect restore (0x08)
         * Magic order: Attack restore indirect (0x0C)
         * Magic order: Indirect restore attack (0x10)
         * Magic order: Indirect attack restore (0x14)
         * Extra battle window displaying information: Inactive (0x00); Active (0x40)
         */
        u8 mask_2;

        /**
         * Key mapping. 16 bytes.
         */
        KeyMapping mapping;

        /**
         * Message speed, 0x00: Fast, 0xFF: Slow. 1 byte.
         */
        u8 message_speed;
    };

    // TODO: Maybe the header needs to be skipped.
    // PSX: 512 bytes.
    // PC: 9 bytes.

    /**
     * Savemap checksum. 4 bytes.
     *
     * Unused in V-Gears. Never generated.
     */
    u32 checksum;

    /**
     * Bytes used for the save preview. 68 bytes
     *
     * Unused in V-Gears, can be skipped.
     */
    u8 preview[68];

    /**
     * Bytes for window color. 12 bytes.
     */
    WindowColor window_colour;

    /**
     * Character records. 9 x 132 bytes.
     *
     * 0: Cloud.
     * 1: Barret.
     * 2: Tifa.
     * 3: Aeris.
     * 4: Red XIII.
     * 5: Yuffie.
     * 6: Cait Sith / Young Cloud.
     * 7: Vincent / Sephiroth.
     */
    Character characters[9];

    /**
     * IDs of characters in the party. 3 x 1 byte.
     */
    u8 party[3];

    /**
     * Unused data. Always 0xFF. 1 byte.
     */
    u8 unused_0;

    /**
     * Inventory list. 320 x 2 bytes,
     *
     * It includes items, unequipped weapons, unequipped armor and unequipped accessories.
     */
    Item inventory[320];

    /**
     * Unequipped materia list. 200 x 4 bytes.
     */
    Materia materia[200];

    /**
     * Materia stolen by Yuffie. 48 x 4 bytes.
     */
    Materia stolen_materia[48];

    /**
     * Unknown data. Always 0xFFFFFFFF. 4 bytes.
     */
    u32 unknown_0;

    /**
     * Current Gil. 4 bytes.
     */
    u32 gil;

    /**
     * Total played time, in seconds. 4 bytes.
     */
    u32 time;

    /**
     * Countdown time, in seconds. 4 bytes.
     */
    u32 countdown;

    /**
     * Unknown data. 3 bytes.
     */
    u8 unknown_1[3];

    /**
     * Total played time, fractions of seconds. 4 bytes.
     *
     * Ignored by V-Gears.
     */
    u32 time_fractions;

    /**
     * Countdown time, fractions of seconds. 4 bytes.
     *
     * Ignored by V-Gears.
     */
    u32 countdown_fractions;

    /**
     * Current game module, redundant. 4 bytes.
     *
     * Ignored by V-Gears.
     */
    u32 module_redundant;

    /**
     * Current game module. 4 bytes.
     *
     * 1: Game saved in a field.
     * 3: Game saved in the world map.
     */
    u16 module;

    /**
     * Saving location. 2 bytes.
     *
     * @todo Does this indicte the current field? The coordinates in a field? If so, 2 nybbles?
     */
    u16 location;

    /**
     * Unused data. Always 0xFF. 1 byte.
     */
    u8 unused_1;

    /**
     * Position information on a field map. 5 bytes.
     */
    FieldPosition field_position;

    /**
     * Unknown data. 3 bytes.
     */
    u8 unknown_2[3];

    /**
     * Information to seed encounter data.
     *
     * V-Gears ignores this.
     */
    u8 encounter_step;

    /**
     * Information to seed encounter data.
     *
     * V-Gears ignores this.
     */
    u8 encounter_offset;

    /**
     * Unused data. Always 0xFF. 1 byte.
     */
    u8 unused_2;

    /**
     * Memory banks dumped in the savemap. 5 x 256 bytes.
     */
    MemoryBank bank[5];

    /**
     * Characters locked in the PHS. 2 bytes.
     *
     * Locked characters can be switched to or from the party.
     */
    PhsSelection phs_lock;

    /**
     * Characters available in the PHS. 2 bytes.
     *
     * Available characters appear in the PHS.
     */
    PhsSelection phs_available;

    /**
     * Unknown data. 48 bytes.
     */
    u8 unknown_3[48];

    /**
     * UserSettings. 21 bytes.
     */
    UserSettings user_settings;

    /**
     * Unknown data. 8 bytes.
     */
    u8 unknown_4[8];
};
