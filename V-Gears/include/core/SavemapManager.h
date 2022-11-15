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
         * @param slot[in] Slot at which to save.
         * @param force[in] If false, it will check if the control data is the save (i.e. if its
         * trying to overwrite a savemap of the same playthrought), and if they do not match, it
         * will do nothing and return false. If true, it will not be checked.
         * @return True if the data was saved, false ifforce was false and control data strings did
         * not match, or if there was an I/O error (which will be logged).
         */
        bool Save(unsigned int slot, bool force);

        /**
         * Saves a savemap to a slot and writes the file.
         *
         * @param slot[in] Slot at which to save.
         * @param force[in] If false, it will check if the control data is the save (i.e. if its
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
         * @param[in] t_l_r Top-left corner, green component.
         * @param[in] t_l_r Top-left corner, blue component.
         * @param[in] t_r_r Top-right corner, red component.
         * @param[in] t_r_r Top-right corner, green component.
         * @param[in] t_r_r Top-right corner, blue component.
         * @param[in] b_r_r Bottom-right corner, red component.
         * @param[in] b_r_r Bottom-right corner, green component.
         * @param[in] b_r_r Bottom-right corner, blue component.
         * @param[in] b_l_r Bottom-left corner, red component.
         * @param[in] b_l_r Bottom-left corner, green component.
         * @param[in] b_l_r Bottom-left corner, blue component.
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
         * @param[in] z Z coordinate. It's optional, set it to -1 to ignore it.
         * @param[in] triangle Walkmesh triangle ID.
         * @param[in] angle Facing direction.
         * @param[in] field Field map ID, or empty for the world map.
         * @param[in] Name of the location to show in the save slot.
         */
        void SetLocation(
          const unsigned int x, const unsigned int y, const int z,
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
        bool SlotIsEmpty(const unsigned int slot);

        /**
         * Generates a string with all the required data to generate a preview of a savemap.
         *
         * The included information contains the following fields, separated by the character "#":
         * - Slot
         * - Control key
         * - Party money
         * - Game time (in seconds)
         * - Location text
         * - Party member 1 name
         * - Party member 1 level
         * - Party member 1 character ID.
         * - Party member 2 character ID.
         * - Party member 3 character ID.
         *
         * @param[in]
         * @return The generated string. An empty string if a wrong slot is specified, or the slot
         * is empty.
         */
        //char* GetPreviewData(const unsigned int slot);
        std::string GetPreviewData(const unsigned int slot);

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

        //char* temp_;

};


