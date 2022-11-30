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

#include <vector>
#include <OgreSingleton.h>
#include <OgreColourValue.h>
#include "core/Savemap.h"


class SavemapManager : public Ogre::Singleton<SavemapManager>{

    public:

        /**
         * The maximum number of save slots.
         */
        static int MAX_SAVE_SLOTS;

        /**
         * Constructor.
         */
        SavemapManager();

        /**
         * Destructor.
         */
        virtual ~SavemapManager();

        /**
         * Retrieves the current savemap.
         *
         * @return The current Savemap.
         */
        Savemap GetCurrentSavemap();

        /**
         * Retrieves a savemap saved in a slot.
         *
         * @param[in] slot Slot of the savemap to retrieve.
         * @return The savemap in the slot. NULL if there is no such savemap or an invalid slot is
         * supplied.
         */
        Savemap* GetSavemap(unsigned int slot);

        /**
         * Retrieves a list of the saved maps.
         */
        std::vector<Savemap*> GetSavemaps();

        /**
         * Copies the current savemap to a slot and writes the file.
         *
         * @param[in] slot Slot at which to save.
         * @param[in] force If false, it will check if the control data is the save (i.e. if its
         * trying to overwrite a savemap of the same playthrought), and if they do not match, it
         * will do nothing and return false. If true, it will not be checked.
         * @return True if the data was saved, false ifforce was false and control data strings did
         * not match, or if there was an I/O error (which will be logged).
         */
        bool Save(unsigned int slot, bool force);

        /**
         * Saves a savemap to a slot and writes the file.
         *
         * @param[in] savemap The savemap to save.
         * @param[in] slot Slot at which to save.
         * @param[in] force If false, it will check if the control data is the save (i.e. if its
         * trying to overwrite a savemap of the same playthrought), and if they do not match, it
         * will do nothing and return false. If true, it will not be checked.
         * @return True if the data was saved, false ifforce was false and control data strings did
         * not match, or if there was an I/O error (which will be logged).
         */
        bool Save(Savemap savemap, unsigned int slot, bool force);

        /**
         * Releases savemaps from memory.
         *
         * The content of the current savemap which are not yet saved to a slot will be lost.
         */
        void Release();

        /**
         * Pushes data to the current savemap memory banks.
         *
         * @param[in] bank The memory bank.
         * @param[in] address The address in the bank.
         * @param[in] value The value to save.
         */
        void SetData(const unsigned int bank, const unsigned int address, const int value);

        /**
         * Sets the control string of the current savemap.
         *
         * @param[in] control The control string.
         */
        void SetControlKey(const char* control);

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
         * Sets the money of the current savemap.
         *
         * @param[in] money The current money.
         */
        void SetMoney(const unsigned int money);

        /**
         * Sets the total playtime of the current savemap.
         *
         * @param[in] seconds The total playtime, in seconds.
         */
        void SetGameTime(const unsigned int seconds);

        /**
         * Sets the time of the curent timer in the default savemap.
         *
         * @param[in] seconds The current countdown timer time, in seconds.
         */
        void SetCountdownTime(const unsigned int seconds);

        /**
         * Marks a key item as owned or non-owned in the current savemap.
         *
         * @param[in] item Key item id.
         * @param[in] owned True to mark it as owned, false to mark is as non-owned.
         */
        void SetKeyItem(const unsigned int item, const bool owned);

        /**
         * Sets the current party in the current savemap.
         *
         * @param[in] member_1 The first party member ID. -1 to leave empty.
         * @param[in] member_2 The second party member ID. -1 to leave empty.
         * @param[in] member_3 The third party member ID. -1 to leave empty.
         */
        void SetParty(const int member_1, const int member_2, const int member_3);

        /**
         * Sets an item in a inventory slot in the current savemap.
         *
         * @param[in] slot Inventory slot.
         * @param[in] id Item ID.
         * @param[in] quantity Item quantity. Set to 0 to mark as no item in the specified slot.
         */
        void SetItem(const unsigned int slot, const unsigned int id, const unsigned int quantity);

        /**
         * Sets a materia in a materia inventory slot in the current savemap.
         *
         * For Enemy Skill materia, don't use this, use {@see SetESkillMateria}.
         *
         * @param[in] slot Materia inventory slot.
         * @param[in] id Materia ID. Set to -1 to mark it as an empty slot.
         * @param[in] ap Total AP of the materia.
         */
        void SetMateria(const unsigned int slot, const int id, const unsigned int ap);

        /**
         * Sets a skill as learned in an Enemy Skill materia inventory slot in the current savemap.
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
         * Sets a materia in a materia inventory slot in the current savemap.
         *
         * For Enemy Skill materia, don't use this, use {@see SetEnemySkillMateriaStash}.
         *
         * @param[in] slot Materia inventory slot.
         * @param[in] id Materia ID. Set to -1 to mark it as an empty slot.
         * @param[in] ap Total AP of the materia.
         */
        void SetMateriaStash(const unsigned int slot, const int id, const unsigned int ap);

        /**
         * Sets a skill as learned in an Enemy Skill materia stash slot in the current savemap.
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
         * Sets the current location in the current savemap.
         *
         * @param[in] x X coordinate.
         * @param[in] y Y coordinate.
         * @param[in] z Z coordinate. It's optional, set it to lower than 0 to ignore it.
         * @param[in] triangle Walkmesh triangle ID.
         * @param[in] angle Facing direction.
         * @param[in] field Field map ID, or empty for the world map.
         * @param[in] name Name of the location to show in the save slot.
         */
        void SetLocation(
          const float x, const float y, const float z,
          const unsigned int triangle, const int angle, const char* field, const char* name
        );

        /**
         * Sets a setting value in the current savemap.
         *
         * @param[in] key Setting key.
         * @param[in] value Setting value.
         * @todo Does nothing, implement when settings are working.
         */
        void SetSetting(const unsigned int key, const unsigned int value);

        /**
         * Sets a character basic information in the current savemap.
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
          const unsigned int id, const int char_id, const char* name,
          const bool enabled, const bool locked,
          const unsigned int level, const unsigned int kills,
          const bool back_row, const unsigned int exp, const unsigned int exp_to_next,
          const unsigned int limit_level, const unsigned int limit_bar,
          const unsigned int weapon, const unsigned int armor, const int accessory
        );

        /**
         * Sets a character stat values in the current savemap.
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
         * Sets a character limit learned status in the current savemap.
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
         * Sets a materia in a character weapon or armor slot in the current savemap.
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
         * Sets a skill as learned in a character's Enemy Skill materia in the current savemap.
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
         * Adds or removes a status to a character in the current savemap.
         *
         * @param[in] id Character ID.
         * @param[in] status ID of the status.
         * @param[in] inflicted True to add the status, false to remove it.
         */
        void SetCharacterStatus(
          const unsigned int id, const unsigned int status, const bool inflicted
        );

        /**
         * Checks if a slot is empty.
         *
         * @return True if the slot is empty, or false if a game is saved there. If an invalid slot
         * is queried, it will always return true.
         */
        bool IsSlotEmpty(const unsigned int slot);

        /**
         * Retrieves the control key from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The control key of the slot. If an invalid slot is specified, or if the slot is
         * empty, an empty string.
         */
        std::string GetSlotControlKey(const unsigned int slot);

        /**
         * Retrieves a colour component from a window corner from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] corner The window corner. See {@see Savemap::Corner}.
         * @param[in] comp The color component to get. See {@see Savemap::Colour}.
         * @return The color component of the specified corner of windows. If an invalid slot is
         * specified, or if the slot is empty, or if an invalid corner or color are requested, 0.
         */
        unsigned int GetSlotWindowCornerColourComponent(
          const unsigned int slot, const unsigned int corner, const unsigned int comp
        );

        /**
         * Retrieves the money from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The money of the slot. If an invalid slot is specified, or if the slot is
         * empty, 0.
         */
        unsigned int GetSlotMoney(const unsigned int slot);

        /**
         * Retrieves the total playtime from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The total playtime, in seconds. If an invalid slot is specified, or if the slot
         * is empty, 0.
         */
        unsigned int GetSlotGameTime(const unsigned int slot);

        /**
         * Retrieves the time in the timer from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The timer time, in seconds. If an invalid slot is specified, or if the slot
         * is empty, or if there is no timer saved, 0.
         */
        unsigned int GetSlotCountdownTime(const unsigned int slot);

        /**
         * Retrieves the ID of a party member from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the party.
         * @return The ID of the character at the requested position. If an invalid slot is
         * specified, or if the slot is empty, or if an invalid position is passed, or if there is
         * no character at the requested position, -1.
         */
        int GetSlotPartyMember(const unsigned int slot, const unsigned int pos);

        /**
         * Retrieves the ID of an item in the inventory from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the inventory.
         * @return The ID of the item at the requested inventory position. If an invalid slot is
         * specified, or if the slot is empty, or if an invalid position is passed, or if there is
         * no item at the requested position, 0.
         */
        unsigned int GetSlotItemAtPosId(const unsigned int slot, const unsigned int pos);

        /**
         * Retrieves the quantity of an item in the inventory from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the inventory.
         * @return The quantity of the item at the requested inventory position. If an invalid slot
         * is specified, or if the slot is empty, or if an invalid position is passed, or if there
         * is no item at the requested position, 0.
         */
        unsigned int GetSlotItemAtPosQty(const unsigned int slot, const unsigned int pos);

        /**
         * Checks the status of a key item from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id Key item ID.
         * @return True if the key item is owned, false if not. If an invalid slot is specified,
         * or if the slot is empty, or if an invalid id is passed, false.
         */
        bool GetSlotKeyItem(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the ID of a materia in the inventory from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the materia inventory.
         * @return The ID of the materia at the requested inventory position. If an invalid slot is
         * specified, or if the slot is empty, or if an invalid position is passed, or if there is
         * no materia at the requested position, -1.
         */
        int GetSlotMateriaAtPosId(const unsigned int slot, const unsigned int pos);

        /**
         * Retrieves the AP of a materia in the inventory from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the materia inventory.
         * @return The AP of the materia at the requested inventory position. If an invalid slot
         * is specified, or if the slot is empty, or if an invalid position is passed, or if there
         * is no materia at the requested position, 0.
         */
        unsigned int GetSlotMateriaAtPosAp(const unsigned int slot, const unsigned int pos);

        /**
         * Checks if there is an Enemy Skill materia at a inventory position from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the materia inventory.
         * @return True if the materia in the specified position is an Enemy Skill materia, false
         * if not. If an invalid slot is specified, or if the slot is empty, or if an invalid
         * position is passed, or if there is no materia at that position, false.
         */
        bool IsSlotMateriaAtPosESkill(const unsigned int slot, const unsigned int pos);

        /**
         * Checks if a a enemy skill is learned by a materia at a position from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the materia inventory.
         * @param[in] skill Skill ID, starting from 0.
         * @return True if the materia in the specified position exists, is an Enemy Skill materia
         * and has learned the specified skill. False in any other case.
         */
        bool IsSlotMateriaAtPosESkillLearned(
          const unsigned int slot, const unsigned int pos, const unsigned int skill
        );

        /**
         * Retrieves the ID of a materia in the stash from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the materia stash.
         * @return The ID of the materia at the requested stash position. If an invalid slot is
         * specified, or if the slot is empty, or if an invalid position is passed, or if there is
         * no materia at the requested position, -1.
         */
        int GetSlotStashAtPosId(const unsigned int slot, const unsigned int pos);

        /**
         * Retrieves the AP of a materia in the stash from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the materia stash.
         * @return The AP of the materia at the requested stash position. If an invalid slot
         * is specified, or if the slot is empty, or if an invalid position is passed, or if there
         * is no materia at the requested position, 0.
         */
        unsigned int GetSlotStashAtPosAp(const unsigned int slot, const unsigned int pos);

        /**
         * Checks if there is an Enemy Skill materia at a stash position from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the materia stash.
         * @return True if the materia in the specified position is an Enemy Skill materia, false
         * if not. If an invalid slot is specified, or if the slot is empty, or if an invalid
         * position is passed, or if there is no materia at that position, false.
         */
        bool IsSlotStashAtPosESkill(const unsigned int slot, const unsigned int pos);

        /**
         * Checks if a a enemy skill is learned by a mat. at a stash position from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] pos Position in the materia stash.
         * @param[in] skill Skill ID, starting from 0.
         * @return True if the materia in the specified position exists, is an Enemy Skill materia
         * and has learned the specified skill. False in any other case.
         */
        bool IsSlotStashAtPosESkillLearned(
          const unsigned int slot, const unsigned int pos, const unsigned int skill
        );

        /**
         * Retrieves the X coordinate of the player from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The X coordinate. If an invalid slot is specified, or if the slot is empty, 0.0.
         */
        float GetSlotLocationX(const unsigned int slot);

        /**
         * Retrieves the Y coordinate of the player from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The Y coordinate. If an invalid slot is specified, or if the slot is empty, 0.0.
         */
        float GetSlotLocationY(const unsigned int slot);

        /**
         * Retrieves the Z coordinate of the player from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The Z coordinate. If an invalid slot is specified, or if the slot is empty, or
         * if the coordinate can be ignored, lower than 0.
         */
        float GetSlotLocationZ(const unsigned int slot);

        /**
         * Retrieves the walkmesh triangle of the player from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The triangle ID. If an invalid slot is specified, or if the slot is empty, 0.
         */
        unsigned int GetSlotLocationTriangle(const unsigned int slot);

        /**
         * Retrieves the facing angle of the player from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The angle. If an invalid slot is specified, or if the slot is empty, 0.
         */
        int GetSlotLocationAngle(const unsigned int slot);

        /**
         * Retrieves the field ID from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The field ID. If an invalid slot is specified, or if the slot is empty, or if the
         * savemap is saved in the worldmap, an empty string.
         */
        std::string GetSlotLocationField(const unsigned int slot);

        /**
         * Retrieves the location name from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @return The location name to be displayed in the save slot. If an invalid slot is
         * specified, or if the slot is empty, or if the location name has not been saved, an empty
         * string.
         */
        std::string GetSlotLocationName(const unsigned int slot);

        /**
         * Retrieves a setting from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] key Setting key.
         * @return The setting value.
         * @todo Implement and document properly.
         */
        int GetSlotSetting(const unsigned int slot, const unsigned int key);

        /**
         * Retrieves the char ID of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The char ID of the character. If an invalid slot is specified, or if the slot is
         * empty, or if an invalid id is passed, or if the character doesn't have a char ID, -1.
         */
        int GetSlotCharacterCharId(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the name of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The name of the character. If an invalid slot is specified, or if the slot is
         * empty, or if an invalid id is passed, an empty string.
         */
        std::string GetSlotCharacterName(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the level of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The level of the character. If an invalid slot is specified, or if the slot is
         * empty, or if an invalid id is passed, 1.
         */
        unsigned int GetSlotCharacterLevel(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the total kills of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The kills of the character. If an invalid slot is specified, or if the slot is
         * empty, or if an invalid id is passed, 0.
         */
        unsigned int GetSlotCharacterKills(const unsigned int slot, const unsigned int id);

        /**
         * Checks the enabled status of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return True if the character is enabled, false if not. If an invalid slot is specified,
         * or if the slot is empty, or if an invalid id is passed, false.
         */
        bool IsSlotCharacterEnabled(const unsigned int slot, const unsigned int id);

        /**
         * Checks the lock status of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return True if the character is enabled, false if not. If an invalid slot is specified,
         * or if the slot is empty, or if an invalid id is passed, false.
         */
        bool IsSlotCharacterLocked(const unsigned int slot, const unsigned int id);

        /**
         * Checks the row of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return True if the character is in the back row, false if not. If an invalid slot is
         * specified, or if the slot is empty, or if an invalid id is passed, false.
         */
        bool IsSlotCharacterBackRow(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the total experience of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The experience of the character. If an invalid slot is specified, or if the slot
         * is empty, or if an invalid id is passed, 0.
         */
        unsigned int GetSlotCharacterExp(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the experience for next level of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The experience of the character. If an invalid slot is specified, or if the slot
         * is empty, or if an invalid id is passed, 0.
         */
        unsigned int GetSlotCharacterExpToNext(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the current limit level of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The limit level of the character. If an invalid slot is specified, or if the
         * slot is empty, or if an invalid id is passed, 0.
         */
        unsigned int GetSlotCharacterLimitLevel(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the current limit bar status level of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The limit bar status of the character. If an invalid slot is specified, or if
         * the slot is empty, or if an invalid id is passed, 0.
         */
        unsigned int GetSlotCharacterLimitBar(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the ID of the weapon of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The character's weapon ID. If an invalid slot is specified, or if the slot is
         * empty, or if an invalid id is passed, 0.
         */
        unsigned int GetSlotCharacterWeaponId(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the ID of the armor of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The character's armor ID. If an invalid slot is specified, or if the slot is
         * empty, or if an invalid id is passed, 0.
         */
        unsigned int GetSlotCharacterArmorId(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the ID of the accessory of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @return The character's accessory ID. If an invalid slot is specified, or if the slot is
         * empty, or if an invalid id is passed, or if the character has no accessory equipped, -1.
         */
        int GetSlotCharacterAccessoryId(const unsigned int slot, const unsigned int id);

        /**
         * Retrieves the base value of a stat of a character from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @param[in] stat The stat ID (see {@see Savemap::STAT}).
         * @return The base value of the specified stat. If an invalid slot is specified, or if the
         * slot is empty, or if an invalid id or stat is passed, 0.
         */
        unsigned int GetSlotCharacterStatBase(
          const unsigned int slot, const unsigned int id, const unsigned int stat
        );

        /**
         * Retrieves the extra value of a stat of a character from a saved savemap.
         *
         * For HP and MP, the extra value means the current value. For any other stat, the bonus
         * gained by using sources.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @param[in] stat The stat ID (see {@see Savemap::STAT}).
         * @return The extra value of the specified stat. If an invalid slot is specified, or if
         * the slot is empty, or if an invalid id or stat is passed, 0.
         */
        unsigned int GetSlotCharacterStatExtra(
          const unsigned int slot, const unsigned int id, const unsigned int stat
        );

        /**
         * Retrieves the uses of a character's limit level from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @param[in] level The limit level.
         * @return Number of uses of the techniques in the specified limit level. If an invalid
         * slot is specified, or if the slot is empty, or if an invalid id or limit level is
         * passed, 0.
         */
        unsigned int GetSlotCharacterLimitUses(
          const unsigned int slot, const unsigned int id, const unsigned int level
        );

        /**
         * Checks if a limit technique is learned by a character from a savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @param[in] level The limit level.
         * @param[in] tech The technique position in the level.
         * @return True if the technique has been learned, false if not. If an invalid slot is
         * specified, or if the slot is empty, or if an invalid id, limit level or technique is
         * passed, false.
         */
        bool IsSlotCharacterLimitLearned(
          const unsigned int slot, const unsigned int id,
          const unsigned int level, const unsigned int tech
        );

        /**
         * Retrieves the ID of an equipped materia from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @param[in] weapon If false, check materia equipped in the weapon. If false, check
         * materia equiped in the armor.
         * @param[in] pos Position in the equipment slots.
         * @return The ID of the equipped materia. If an invalid slot is specified, or if the slot
         * is empty, or if an invalid id or position is passed, or if there is no materia at the
         * requested position, -1.
         */
        int GetSlotCharacterMateriaId(
          const unsigned int slot, const unsigned int id, const bool weapon, const unsigned int pos
        );

        /**
         * Retrieves the AP of an equipped materia from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @param[in] weapon If false, check materia equipped in the weapon. If false, check
         * materia equiped in the armor.
         * @param[in] pos Position in the equipment slots.
         * @return The AP of the equipped materia. If an invalid slot is specified, or if the slot
         * is empty, or if an invalid id or position is passed, or if there is no materia at the
         * requested position, 0.
         */
        unsigned int GetSlotCharacterMateriaAp(
          const unsigned int slot, const unsigned int id, const bool weapon, const unsigned int pos
        );

        /**
         * Checks if an equipped materia is Enemy Skill from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @param[in] weapon If false, check materia equipped in the weapon. If false, check
         * materia equiped in the armor.
         * @param[in] pos Position in the equipment slots.
         * @return True if the materia in the specified position is an Enemy Skill materia, false
         * if not. If an invalid id or slot is specified, or if the slot is empty, or if an invalid
         * position is passed, or if there is no materia at that position, false.
         */
        bool IsSlotCharacterMateriaESkill(
          const unsigned int slot, const unsigned int id, const bool weapon, const unsigned int pos
        );

        /**
         * Checks if a a enemy skill is learned by a equipped materia from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] id The character ID.
         * @param[in] weapon If false, check materia equipped in the weapon. If false, check
         * materia equiped in the armor.
         * @param[in] pos Position in the equipment slots.
         * @param[in] skill Skill ID, starting from 0.
         * @return True if the materia in the specified position exists, is an Enemy Skill materia
         * and has learned the specified skill. False in any other case.
         */
        bool IsSlotCharacterMateriaESkillLearned(
          const unsigned int slot, const unsigned int id, const bool weapon,
          const unsigned int pos, const unsigned int skill
        );

        /**
         * Retrieves the value of a bank address from a saved savemap.
         *
         * @param[in] slot The slot to read from.
         * @param[in] bank The bank ID.
         * @param[in] address The address in the bank.
         * @return The value. If an invalid slot is specified, or if the slot is empty, or if an
         * invalid bank or address level is passed, 0.
         */
        int GetSlotData(
          const unsigned int slot, const unsigned int bank, const unsigned int address
        );

    private:

        /**
         * Path for the save files.
         */
        static std::string SAVE_PATH;

        /**
         * Reads every saved savemap from it's file.
         */
        void ReadSavemaps();

        /**
         * The current savemap.
         */
        Savemap* current_savemap_;

        /**
         * List of saved savemaps.
         */
        std::vector<Savemap*> saved_savemaps_;

        /**
         * Indicates if the saved savemaps have been read from files.
         */
        bool savemaps_read_;

};


