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

/**
 * A savemap.
 */
class Savemap{

    public:

        /**
         * Constructor. Initializes an empty savemap.
         */
        Savemap();

        /**
         * Destructor.
         */
        ~Savemap();

        /**
         * Assignment operator.
         *
         * Copies a savemap into another, including control data.
         *
         * @param[in] to_copy The savemap to copy from.
         */
        void operator = (const Savemap &to_copy);

        /**
         * Reads the savemap data from a file.
         *
         * @param[in] file_name The name of the file to read.
         */
        void Read(std::string file_name);

        /**
         * Writes the savemap to a file.
         *
         * @param[in] slot The slot the savemap is to be written to.
         * @param[in] file_name Name of the file the savemap is to be written to.
         */
        void Write(int slot, std::string file_name);

        /**
         * Retrieves a control string for a savemap.
         *
         * Not a checksum, it does not depend on the actual data. It never mutates, is used to
         * differenciate savemaps when asking to override.
         */
        std::string GetControlKey() const;

        /**
         * Sets the control string of the savemap.
         *
         * @param[in] control The control string.
         */
        void SetControlKey(const std::string control);

        /**
         * Sets the window colours in the current savemap.
         *
         * @param[in] t_l_r Top-left corner, red component.
         * @param[in] t_l_g Top-left corner, green component.
         * @param[in] t_l_b Top-left corner, blue component.
         * @param[in] t_r_r Top-right corner, red component.
         * @param[in] t_r_g Top-right corner, green component.
         * @param[in] t_r_b Top-right corner, blue component.
         * @param[in] b_r_r Bottom-right corner, red component.
         * @param[in] b_r_g Bottom-right corner, green component.
         * @param[in] b_r_b Bottom-right corner, blue component.
         * @param[in] b_l_r Bottom-left corner, red component.
         * @param[in] b_l_g Bottom-left corner, green component.
         * @param[in] b_l_b Bottom-left corner, blue component.
         */
        void SetWindowColours(
          const unsigned int t_l_r, const unsigned int t_l_g, const unsigned int t_l_b,
          const unsigned int t_r_r, const unsigned int t_r_g, const unsigned int t_r_b,
          const unsigned int b_r_r, const unsigned int b_r_g, const unsigned int b_r_b,
          const unsigned int b_l_r, const unsigned int b_l_g, const unsigned int b_l_b
        );

        /**
         * Sets the money of the savemap.
         *
         * @param[in] money The current money.
         */
        void SetMoney(const unsigned int money);

        /**
         * Sets the total playtime of the savemap.
         *
         * @param[in] seconds The total playtime, in seconds.
         */
        void SetGameTime(const unsigned int seconds);

        /**
         * Sets the time of the curent timer.
         *
         * @param[in] seconds The current countdown timer time, in seconds.
         */
        void SetCountdownTime(const unsigned int seconds);

        /**
         * Marks a key item as owned or non-owned in the savemap.
         *
         * @param[in] item Key item id.
         * @param[in] owned True to mark it as owned, false to mark is as non-owned.
         */
        void SetKeyItem(const unsigned int item, const bool owned);

        /**
         * Sets the current party in the savemap.
         *
         * @param[in] member_1 The first party member ID. -1 to leave empty.
         * @param[in] member_2 The second party member ID. -1 to leave empty.
         * @param[in] member_3 The third party member ID. -1 to leave empty.
         */
        void SetParty(const int member_1, const int member_2, const int member_3);

        /**
         * Sets an item in a inventory slot in the savemap.
         *
         * @param[in] slot Inventory slot.
         * @param[in] id Item ID.
         * @param[in] quantity Item quantity. Set to 0 to mark as no item in the specified slot.
         */
        void SetItem(const unsigned int slot, const unsigned int id, const unsigned int quantity);

        /**
         * Sets a materia in a materia inventory slot in the savemap.
         *
         * For Enemy Skill materia, don't use this, use {@see SetESkillMateria}.
         *
         * @param[in] slot Materia inventory slot.
         * @param[in] id Materia ID. Set to -1 to mark it as an empty slot.
         * @param[in] ap Total AP of the materia.
         */
        void SetMateria(const unsigned int slot, const int id, const unsigned int ap);

        /**
         * Sets a skill as learned in an Enemy Skill materia inventory slot in the savemap.
         *
         * Calling this will make the materia in the slot to be an Enemy Skill materia and set the
         * AP to 0. To delete it, call {@see SetMateria} with the same slot and id -1.
         *
         * @param[in] slot Materia inventory slot.
         * @param[in] skill Enemy Skill ID (sequential, from 0).
         * @param[in] learned TRue to mark as learned, false to mark as not learned.
         */
        void SetESkillMateria(const unsigned slot, const unsigned int skill, const bool learned);

        /**
         * Sets a materia in a materia inventory slot in the savemap.
         *
         * For Enemy Skill materia, don't use this, use {@see SetEnemySkillMateriaStash}.
         *
         * @param[in] slot Materia inventory slot.
         * @param[in] id Materia ID. Set to -1 to mark it as an empty slot.
         * @param[in] ap Total AP of the materia.
         */
        void SetMateriaStash(const unsigned int slot, const int id, const unsigned int ap);

        /**
         * Sets a skill as learned in an Enemy Skill materia stash slot in the savemap.
         *
         * Calling this will make the materia in the slot to be an Enemy Skill materia and set the
         * AP to 0. To delete it, call {@see SetMateriaStash} with the same slot and id -1.
         *
         * @param[in] slot Materia inventory slot.
         * @param[in] skill Enemy Skill ID (sequential, from 0)
         * @param[in] learned TRue to mark as learned, false to mark as not learned.
         */
        void SetESkillMateriaStash(
          const unsigned slot, const unsigned int skill, const bool learned
        );

        /**
         * Sets the current location in the savemap.
         *
         * @param[in] x X coordinate.
         * @param[in] y Y coordinate.
         * @param[in] z Z coordinate. It's optional, set it to -1 to ignore it.
         * @param[in] triangle Walkmesh triangle ID.
         * @param[in] angle Facing direction.
         * @param[in] field Field map ID, or empty for the world map.
         * @param[in] name Name of the location to show in the save slot.
         */
        void SetLocation(
          const float x, const float y, const float z,
          const unsigned int triangle, const int angle, std::string field, std::string name
        );

        /**
         * Sets a setting value in the savemap.
         *
         * @param[in] key Setting key.
         * @param[in] value Setting value.
         * @todo Does nothing, implement when settings are working.
         */
        void SetSetting(const unsigned int key, const unsigned int value);

        /**
         * Sets a character basic information in the savemap.
         *
         * @param[in] id Character ID.
         * @param[in] char_id Character identifier.
         * @param[in] name Character name.
         * @param[in] enabled If enabled in the PHS.
         * @param[in] locked If locked in or out the current party.
         * @param[in] level Character level.
         * @param[in] kills Total kills.
         * @param[in] back_row If the character is in the back row.
         * @param[in] exp Total experience.
         * @param[in] exp_to_next Experience to reach next level.
         * @param[in] limit_level Currently selected limit level.
         * @param[in] limit_bar Current limit bar fill status
         * @param[in] weapon Equipped weapon ID.
         * @param[in] armor Equipped armor ID.
         * @param[in] accessory Equipped accessory ID. -1 if none equipped.
         */
        void SetCharacterInfo(
          const unsigned int id, const int char_id, const std::string name,
          const bool enabled, const bool locked,
          const unsigned int level, const unsigned int kills,
          const bool back_row, const unsigned int exp, const unsigned int exp_to_next,
          const unsigned int limit_level, const unsigned int limit_bar,
          const unsigned int weapon, const unsigned int armor, const int accessory
        );

        /**
         * Sets a character stat values in the savemap.
         *
         * @param[in] id Character ID.
         * @param[in] stat Stat ID. See {@see STAT}.
         * @param[in] base Base value of the stat.
         * @param[in] extra Extra value of the stat. For STR, VIT, MAG, SPR, DEX and LCK, it means
         * the increment by means of sources. For HP and MP, the current value.
         */
        void SetCharacterStat(
          const unsigned int id, const unsigned int stat,
          const unsigned int base, const unsigned int extra
        );

        /**
         * Sets a character limit learned status in the savemap.
         *
         * @param[in] id Character ID.
         * @param[in] level Limit level.
         * @param[in] technique Limit level technique ID.
         * @param[in] learned TRue to mark as learned, false to mark as non-learned.
         * @param[in] uses Times the technique has been used.
         */
        void SetCharacterLimitLearned(
          const unsigned int id, const unsigned int level,
          const unsigned int technique, const bool learned, const unsigned int uses
        );

        /**
         * Sets a materia in a character weapon or armor slot in the savemap.
         *
         * For Enemy Skill materia, don't use this, use {@see SetCharacterESkillMateria}.
         *
         * @param[in] id Character ID.
         * @param[in] weapon True to set the materia in the weapon, false to set it in the armor.
         * @param[in] slot Weapon or armor slot.
         * @param[in] materia Materia ID. Set to -1 to mark it as an empty slot.
         * @param[in] ap Total AP of the materia.
         */
        void SetCharacterMateria(
          const unsigned int id, const bool weapon, const unsigned int slot,
          const int materia, const unsigned int ap
        );

        /**
         * Sets a skill as learned in a character's Enemy Skill materia in the savemap.
         *
         * Calling this will make the materia to be an Enemy Skill materia and set the AP to 0. To
         * delete it, call {@see SetCharacterMateria} with the same character ID, weapon/armor and
         * slot and id -1.
         *
         * @param[in] id Character ID.
         * @param[in] weapon True to set the materia in the weapon, false to set it in the armor.
         * @param[in] slot Weapon or armor slot.
         * @param[in] skill Enemy Skill ID (sequential, from 0).
         * @param[in] learned TRue to mark as learned, false to mark as not learned.
         */
        void SetCharacterESkillMateria(
          const unsigned int id, const bool weapon, const unsigned int slot,
          const unsigned int skill, const bool learned
        );

        /**
         * Adds or removes a status to a character in the savemap.
         *
         * @param[in] id Character ID.
         * @param[in] status ID of the status.
         * @param[in] inflicted True to add the status, false to remove it.
         */
        void SetCharacterStatus(
          const unsigned int id, const unsigned int status, const bool inflicted
        );

        /**
         * Sets data in a data bank.
         *
         * @param[in] bank Data bank index.
         * @param[in] address Address in the data bank.
         * @param[in] value The value to store.
         */
        void SetData(const unsigned int bank, const unsigned int address, const int value);

        /**
         * Checks if the savemap is empty.
         *
         * @return True if the savemap is empty, false if it contains a saved game.
         */
        bool IsEmpty();

        /**
         * Retrieves the control key .
         *
         * @return The control key of the slot.
         */
        std::string GetControlKey();

        /**
         * Retrieves a colour component from a window corner.
         *
         * @param[in] corner The window corner. See {@see Corner}.
         * @param[in] comp The color component to get. See {@see Colour}.
         * @return The color component of the specified corner of windows. If an invalid slot is
         * specified, or if the slot is empty, or if an invalid corner or color are requested, 0.
         */
        unsigned int GetWindowCornerColourComponent(
          const unsigned int corner, const unsigned int comp
        );

        /**
         * Retrieves the money.
         *
         * @return The money of the slot.
         */
        unsigned int GetMoney();

        /**
         * Retrieves the total playtime.
         *
         * @param[in] slot The slot to read from.
         * @return The total playtime, in seconds.
         */
        unsigned int GetGameTime();

        /**
         * Retrieves the time in the timer.
         *
         * @return The timer time, in seconds. If there is no timer saved, 0.
         */
        unsigned int GetCountdownTime();

        /**
         * Retrieves the ID of a party member.
         *
         * @param[in] pos Position in the party.
         * @return The ID of the character at the requested position. If there is no character at
         * the requested position, -1.
         */
        int GetPartyMember(const unsigned int pos);

        /**
         * Retrieves the ID of an item in the inventory.
         *
         * @param[in] pos Position in the inventory.
         * @return The ID of the item at the requested inventory position. If there is no item at
         * the requested position, 0.
         */
        unsigned int GetItemAtPosId(const unsigned int pos);

        /**
         * Retrieves the quantity of an item in the inventory.
         *
         * @param[in] pos Position in the inventory.
         * @return The quantity of the item at the requested inventory position. If there is no
         * item at the requested position, 0.
         */
        unsigned int GetItemAtPosQty(const unsigned int pos);

        /**
         * Checks if a key item is owned.
         *
         * @param[in] id Key item ID.
         * @return True if the key item is owned, false if not. If an invalid id is passed, false.
         */
        bool GetKeyItem(const unsigned int id);

        /**
         * Retrieves the ID of a materia in the inventory.
         *
         * @param[in] pos Position in the materia inventory.
         * @return The ID of the materia at the requested inventory position. If an invalid
         * position is passed, or if there is no materia at the requested position, -1.
         */
        int GetMateriaAtPosId(const unsigned int pos);

        /**
         * Retrieves the AP of a materia in the inventory.
         *
         * @param[in] pos Position in the materia inventory.
         * @return The AP of the materia at the requested inventory position. If an invalid
         * position is passed, or if there is no materia at the requested position, 0.
         */
        unsigned int GetMateriaAtPosAp(const unsigned int pos);

        /**
         * Checks if there is an Enemy Skill materia at a inventory position.
         *
         * @param[in] pos Position in the materia inventory.
         * @return True if the materia in the specified position is an Enemy Skill materia, false
         * if not. If an invalid position is passed, or if there is no materia at that position,
         * false.
         */
        bool IsMateriaAtPosESkill(const unsigned int pos);

        /**
         * Checks if a a enemy skill is learned by a materia in the inventory.
         *
         * @param[in] pos Position in the materia inventory.
         * @param[in] skill Skill ID, starting from 0.
         * @return True if the materia in the specified position exists, is an Enemy Skill materia
         * and has learned the specified skill. False in any other case.
         */
        bool IsMateriaAtPosESkillLearned(const unsigned int pos, const unsigned int skill);

        /**
         * Retrieves the ID of a materia in the stash.
         *
         * @param[in] pos Position in the materia stash.
         * @return The ID of the materia at the requested stahs position. If an invalid
         * position is passed, or if there is no materia at the requested position, -1.
         */
        int GetStashAtPosId(const unsigned int pos);

        /**
         * Retrieves the AP of a materia in the stash.
         *
         * @param[in] pos Position in the materia stash.
         * @return The AP of the materia at the requested inventory position. If an invalid
         * position is passed, or if there is no materia at the requested position, 0.
         */
        unsigned int GetStashAtPosAp(const unsigned int pos);

        /**
         * Checks if there is an Enemy Skill materia at a stash position.
         *
         * @param[in] pos Position in the materia stash.
         * @return True if the materia in the specified position is an Enemy Skill materia, false
         * if not. If an invalid position is passed, or if there is no materia at that position,
         * false.
         */
        bool IsStashAtPosESkill(const unsigned int pos);

        /**
         * Checks if a a enemy skill is learned by a materia in the stash.
         *
         * @param[in] pos Position in the materia stash.
         * @param[in] skill Skill ID, starting from 0.
         * @return True if the materia in the specified position exists, is an Enemy Skill materia
         * and has learned the specified skill. False in any other case.
         */
        bool IsStashAtPosESkillLearned(const unsigned int pos, const unsigned int skill);

        /**
         * Retrieves the X coordinate of the player.
         *
         * @return The X coordinate.
         */
        float GetLocationX();

        /**
         * Retrieves the Y coordinate of the player.
         *
         * @return The Y coordinate.
         */
        float GetLocationY();

        /**
         * Retrieves the Z coordinate of the player.
         *
         * @return The Z coordinate. If it was not saved, it will be lower than 0.
         */
        float GetLocationZ();

        /**
         * Retrieves the walkmesh triangle of the player.
         *
         * @return The triangle ID.
         */
        unsigned int GetLocationTriangle();

        /**
         * Retrieves the facing angle of the player.
         *
         * @param[in] slot The slot to read from.
         * @return The angle.
         */
        int GetLocationAngle();

        /**
         * Retrieves the field ID.
         *
         * @return The field ID. If the savemap is saved in the worldmap, an empty string.
         */
        std::string GetLocationField();

        /**
         * Retrieves the location name.
         *
         * @return The location name to be displayed in the save slot. If the location name has not
         * been saved, an empty string.
         */
        std::string GetLocationName();

        /**
         * Retrieves a setting.
         *
         * @param[in] key Setting key.
         * @return The setting value.
         * @todo Implement and document properly.
         */
        int GetSetting(const unsigned int key);

        /**
         * Retrieves the char ID of a character.
         *
         * @param[in] id The character ID.
         * @return The char ID of the character. If an invalid id is passed, or if the character
         * doesn't have a char ID, -1.
         */
        int GetCharacterCharId(const unsigned int id);

        /**
         * Retrieves the name of a character.
         *
         * @param[in] id The character ID.
         * @return The name of the character. If an invalid id is passed, an empty string.
         */
        std::string GetCharacterName(const unsigned int id);

        /**
         * Retrieves the level of a character.
         *
         * @param[in] id The character ID.
         * @return The level of the character. If an invalid id is passed, 1.
         */
        unsigned int GetCharacterLevel(const unsigned int id);

        /**
         * Retrieves the total kills of a character.
         *
         * @param[in] id The character ID.
         * @return The kills of the character. If an invalid id is passed, 0.
         */
        unsigned int GetCharacterKills(const unsigned int id);

        /**
         * Checks if a character is enabled.
         *
         * @param[in] id The character ID.
         * @return True if the character is enabled, false if not. If an invalid id is passed,
         * false.
         */
        bool IsCharacterEnabled(const unsigned int id);

        /**
         * Checks if a character is locked in or out of the party
         *
         * @param[in] id The character ID.
         * @return True if the character is enabled, false if not. If an invalid id is passed,
         * false.
         */
        bool IsCharacterLocked(const unsigned int id);

        /**
         * Checks the row of a character.
         *
         * @param[in] id The character ID.
         * @return True if the character is in the back row, false if not. If an invalid id is
         * passed, false.
         */
        bool IsCharacterBackRow(const unsigned int id);

        /**
         * Retrieves the total experience of a character.
         *
         * @param[in] id The character ID.
         * @return The experience of the character. If an invalid id is passed, 0.
         */
        unsigned int GetCharacterExp(const unsigned int id);

        /**
         * Retrieves the experience for next level of a character.
         *
         * @param[in] id The character ID.
         * @return The experience of the character. If an invalid id is passed, 0.
         */
        unsigned int GetCharacterExpToNext(const unsigned int id);

        /**
         * Retrieves the current limit level of a character from a saved savemap.
         *
         * @param[in] id The character ID.
         * @return The limit level of the character. If an invalid id is passed, 0.
         */
        unsigned int GetCharacterLimitLevel(const unsigned int id);

        /**
         * Retrieves the current limit bar status level of a character.
         *
         * @param[in] id The character ID.
         * @return The limit bar status of the character. If an invalid id is passed, 0.
         */
        unsigned int GetCharacterLimitBar(const unsigned int id);

        /**
         * Retrieves the ID of the weapon of a character.
         *
         * @param[in] id The character ID.
         * @return The character's weapon ID. If an invalid id is passed, 0.
         */
        unsigned int GetCharacterWeaponId(const unsigned int id);

        /**
         * Retrieves the ID of the armor of a character.
         *
         * @param[in] id The character ID.
         * @return The character's armor ID. If an invalid id is passed, 0.
         */
        unsigned int GetCharacterArmorId(const unsigned int id);

        /**
         * Retrieves the ID of the weapon of a character.
         *
         * @param[in] id The character ID.
         * @return The character's weapon ID. If an invalid id is passed, or if the characetr has
         * no accessory, -1.
         */
        int GetCharacterAccessoryId(const unsigned int id);

        /**
         * Retrieves the base value of a stat of a character.
         *
         * @param[in] id The character ID.
         * @param[in] stat The stat ID (see {@see STAT}).
         * @return The base value of the specified stat. If an invalid id or stat is passed, 0.
         */
        unsigned int GetCharacterStatBase(const unsigned int id, const unsigned int stat);

        /**
         * Retrieves the extra value of a stat of a character.
         *
         * For HP and MP, the extra value means the current value. For any other stat, the bonus
         * gained by using sources.
         *
         * @param[in] id The character ID.
         * @param[in] stat The stat ID (see {@see Savemap::STAT}).
         * @return The extra value of the specified stat. If an invalid id or stat is passed, 0.
         */
        unsigned int GetCharacterStatExtra(const unsigned int id, const unsigned int stat);

        /**
         * Retrieves the uses of a character's limit level.
         *
         * @param[in] id The character ID.
         * @param[in] level The limit level.
         * @return Number of uses of the techniques in the specified limit level. If an invalid id
         * or limit level is passed, 0.
         */
        unsigned int GetCharacterLimitUses(const unsigned int id, const unsigned int level);

        /**
         * Checks if a limit technique is learned by a character.
         *
         * @param[in] id The character ID.
         * @param[in] level The limit level.
         * @param[in] tech The technique position in the level.
         * @return True if the technique has been learned, false if not. If an invalid id, limit
         * level or technique is passed, 0.
         */
        bool IsCharacterLimitLearned(
          const unsigned int id, const unsigned int level, const unsigned int tech
        );

        /**
         * Retrieves the ID of an equipped materia.
         *
         * @param[in] id The character ID.
         * @param[in] weapon If false, check materia equipped in the weapon. If false, check
         * materia equiped in the armor.
         * @param[in] pos Position in the equipment slots.
         * @return The ID of the equipped materia. Id or position is passed, or if there is no
         * materia at the requested position, -1.
         */
        int GetCharacterMateriaId(const unsigned int id, const bool weapon, const unsigned int pos);

        /**
         * Retrieves the AP of an equipped materia.
         *
         * @param[in] id The character ID.
         * @param[in] weapon If false, check materia equipped in the weapon. If false, check
         * materia equiped in the armor.
         * @param[in] pos Position in the equipment slots.
         * @return The AP of the equipped materia. If an invalid id or position is passed, or if
         * there is no materia at the requested position, 0.
         */
        unsigned int GetCharacterMateriaAp(
          const unsigned int id, const bool weapon, const unsigned int pos
        );

        /**
         * Checks if an equipped materia is Enemy Skill.
         *
         * @param[in] id The character ID.
         * @param[in] weapon If false, check materia equipped in the weapon. If false, check
         * materia equiped in the armor.
         * @param[in] pos Position in the equipment slots.
         * @return True if the materia in the specified position is an Enemy Skill materia, false
         * if not. If an invalid position is passed, or if there is no materia at that position,
         * false.
         */
        bool IsCharacterMateriaESkill(const unsigned int id, const bool weapon, const unsigned int pos);

        /**
         * Checks if a a enemy skill is learned by a equipped materia.
         *
         * @param[in] id The character ID.
         * @param[in] weapon If false, check materia equipped in the weapon. If false, check
         * materia equiped in the armor.
         * @param[in] pos Position in the equipment slots.
         * @param[in] skill Skill ID, starting from 0.
         * @return True if the materia in the specified position exists, is an Enemy Skill materia
         * and has learned the specified skill. False in any other case.
         */
        bool IsCharacterMateriaESkillLearned(
          const unsigned int id, const bool weapon, const unsigned int pos, const unsigned int skill
        );

        /**
         * Retrieves the value of a bank address.
         *
         * @param[in] bank The bank ID.
         * @param[in] address The address in the bank.
         * @return The value. If an invalid bank or address level is passed, 0.
         */
        int GetData(const unsigned int bank, const unsigned int address);

        /**
         * Character basic stats.
         */
        class STAT{

            public:

                /**
                 * Strength stat.
                 */
                static const unsigned int STR = 0;

                /**
                 * Vtatlity stat.
                 */
                static const unsigned int VIT = 1;

                /**
                 * Magic stat.
                 */
                static const unsigned int MAG = 2;

                /**
                 * Spirit stat.
                 */
                static const unsigned int SPR = 3;

                /**
                 * Dexterity stat.
                 */
                static const unsigned int DEX = 4;

                /**
                 * Luck stat.
                 */
                static const unsigned int LCK = 5;

                /**
                 * HP stat.
                 */
                static const unsigned int HP = 6;

                /**
                 * MP stat.
                 */
                static const unsigned int MP = 7;
        };

        /**
         * Window corners.
         */
        class CORNER{

            public:

                /**
                 * Top left window corner.
                 */
                static const unsigned int T_L = 0;

                /**
                 * Top right window corner.
                 */
                static const unsigned int T_R = 1;

                /**
                 * Bottom right window corner.
                 */
                static const unsigned int B_R = 2;

                /**
                 * Bottom left window corner.
                 */
                static const unsigned int B_L = 3;
        };

        /**
         * Colour codes
         */
        class COLOUR{

            public:

                /**
                 * Red colour.
                 */
                static const unsigned int R = 0;

                /**
                 * Green colour.
                 */
                static const unsigned int G = 1;

                /**
                 * Blue colour.
                 */
                static const unsigned int B = 2;
        };

    private:

        /**
         * Maximum number of characters.
         */
        static const unsigned int MAX_CHARACTERS = 11;

        /**
         * Maximum number of inventory slots.
         */
        static const unsigned int MAX_ITEM_SLOTS = 500;

        /**
         * Maximum number of inventory slots for key items.
         */
        static const unsigned int MAX_KEY_ITEM_SLOTS = 100;

        /**
         * Maximum number of materia slots.
         */
        static const unsigned int MAX_MATERIA_SLOTS = 500;

        /**
         * Maximum number of materia slots in the stash.
         */
        static const unsigned int MAX_STASH_SLOTS = 500;

        /**
         * Maximum number of materia slots in a weapon or armor.
         */
        static const unsigned int MAX_EQUIP_SLOTS = 10;

        /**
         * Maximum number of skills in an Enemy Skill materia.
         */
        static const unsigned int MAX_ENEMY_SKILLS = 32;

        /**
         * Maximum limit level.
         */
        static const unsigned int MAX_LIMIT_LEVELS = 4;

        /**
         * Maximum limit techniques per level.
         */
        static const unsigned int MAX_LIMIT_TECHNIQUES = 4;

        /**
         * Level at which the limit level is full.
         */
        static const unsigned int MAX_LIMIT_BAR = 254;

        /**
         * Maximum number of party member.
         */
        static const unsigned int MAX_PARTY_MEMBERS = 3;

        /**
         * Number of data banks.
         */
        static const unsigned int BANK_COUNT = 16;

        /**
         * Number of addresses in each data bank.
         */
        static const unsigned int BANK_ADDRESS_COUNT = 256;

        /**
         * MAx colour component value.
         */
        static const unsigned int MAX_COLOUR = 254;


        /**
         * Game settings.
         */
        struct Settings{

            /**
             * Magic sorting methods.
             */
            enum class MagicOrder{

                /**
                 * Restore-Attack-Indirect.
                 */
                RAI,

                /**
                 * Restore-Indirect-Attack.
                 */
                RIA,

                /**
                 * Attack-Indirect-Restore.
                 */
                AIR,

                /**
                 * Attack-Restore-Indirect.
                 */
                ARI,

                /**
                 * Indirect-Restore-Attack.
                 */
                IRA,

                /**
                 * Indirect-Attack-Restore.
                 */
                IAR,
            };

            /**
             * ATB modes.
             */
            enum class AtbMode{

                /**
                 * Active battle.
                 */
                ACTIVE,

                /**
                 * Mixed mode.
                 */
                RECOMMENDED,

                /**
                 * Paused mode.
                 */
                WAIT
            };

            /**
             * Battle speed.
             */
            unsigned int battle_speed;

            /**
             * Battle message speed.
             */
            unsigned int battle_msg_speed;

            /**
             * Field message speed
             */
            unsigned int msg_speed;

            /**
             * Magic order.
             */
            MagicOrder order;

            /**
             * ATB mode.
             */
            AtbMode atb_mode;

            /**
             * Indicates if the battle help window is to be shown.
             */
            bool batle_help;

            // TODO: Key mappings, when implemented.
        };

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
         * Indicates if the savemap is empty or contains actual data.
         */
        bool empty_;

        /**
         * The slot the savemap is saved at, or -1 if not saved
         */
        int slot_;

        /**
         * Control string.
         *
         * Not a checksum, it does not depend on the actual data. It never mutates, is used to
         * differenciate savemaps when asking to override.
         */
        std::string control_;

        /**
         * Window colours.
         */
        u8 window_colours_[4][3];

        /**
         * List of characters.
         */
        Character characters_[MAX_CHARACTERS];

        /**
         * IDs of characters in the party.
         */
        int party_[MAX_PARTY_MEMBERS];

        /**
         * Owned items.
         */
        Item items_[MAX_ITEM_SLOTS];

        /**
         * Owned key items.
         */
        bool key_items_[MAX_KEY_ITEM_SLOTS];

        /**
         * Owned materia.
         */
        Materia materia_[MAX_MATERIA_SLOTS];

        /**
         * Alternative materia stash.
         *
         * Used for the Yuffie materia event.
         */
        Materia materia_stash_[MAX_STASH_SLOTS];

        /**
         * Current party money.
         */
        unsigned int money_;

        /**
         * Seconds played
         */
        unsigned int seconds_;

        /**
         * Current countdown remaining seconds.
         */
        unsigned int countdown_;

        /**
         * Current location.
         */
        Location location_;

        /**
         * Game settings.
         */
        Settings settings_;

        /**
         * Memory banks.
         */
        int data_[BANK_COUNT][BANK_ADDRESS_COUNT];
};




