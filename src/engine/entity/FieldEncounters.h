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

#include <map>
#include <utility>
#include <vector>
#include <OgreVector3.h>

class Entity;

/**
 * The random battles of a field: its encounter tables, and the count of the steps of the player
 * that starts them.
 */
class FieldEncounters{

    public:

        /**
         * Gets the multiplier of the random encounter rate (the encounter_multiplier cvar).
         *
         * @return The multiplier. 0 prevents random encounters.
         */
        static float GetMultiplier();

        /**
         * Checks if the random encounter checks are logged (the debug_encounters cvar).
         *
         * @return True if they are logged.
         */
        static bool IsDebug();

        /**
         * Enables or disables random encounters in the field.
         *
         * @param[in] active True to enable encounters, false to deactivate them.
         */
        void SetActive(bool active);

        /**
         * Removes the random encounter tables of the field, and forgets the walked distance.
         */
        void Clear();

        /**
         * Adds a random encounter table to the field.
         *
         * @param[in] id Table ID (1 or 2).
         * @param[in] rate Encounter rate, as in the original game (24 is very low, 240 is high).
         * @param[in] battles Standard encounters, pairs of battle ID and probability (out of 64).
         * @param[in] specials Special encounters, with the same format.
         */
        void AddTable(
          const int id, const int rate, const std::vector<std::pair<int, int>>& battles,
          const std::vector<std::pair<int, int>>& specials
        );

        /**
         * Selects the encounter table to use.
         *
         * @param[in] id Table ID (1 or 2). If it doesn't exist, no encounters happen.
         */
        void SetTable(const int id);

        /**
         * Picks the battle of a random encounter from the current encounter table.
         *
         * @return The battle ID, -1 if the field has no encounters.
         */
        int RollBattle() const;

        /**
         * Counts the steps of the player and starts random encounters.
         *
         * It has to be called once per frame.
         *
         * @param[in] player The player entity, or nullptr if there is none.
         * @param[in] player_locked True if the player can't be controlled.
         * @param[in] walked True if the player was moved by the player's input in this frame.
         */
        void Update(const Entity* player, const bool player_locked, const bool walked);

    private:

        /**
         * Random encounter table of a field.
         */
        struct Table{

            /**
             * Encounter rate, as in the original game.
             */
            int rate;

            /**
             * Standard encounters: battle ID and probability (out of 64).
             */
            std::vector<std::pair<int, int>> battles;

            /**
             * Special encounters: battle ID and probability (out of 64).
             */
            std::vector<std::pair<int, int>> specials;
        };

        /**
         * Indicates if random encounters happen in the map.
         */
        bool active_ = false;

        /**
         * Random encounter tables of the field, by ID.
         */
        std::map<int, Table> tables_;

        /**
         * ID of the encounter table in use.
         */
        int table_ = 1;

        /**
         * Frames walked by the player since the last encounter check.
         */
        float distance_ = 0.0f;

        /**
         * Position of the player in the last frame, to know how much it has walked.
         */
        Ogre::Vector3 last_position_ = Ogre::Vector3::ZERO;

        /**
         * Indicates if {@see last_position_} is valid.
         */
        bool last_position_valid_ = false;

        /**
         * Accumulated chance of a random encounter, out of 256 * 256. It grows while walking.
         */
        int accumulator_ = 0;

        /**
         * Last random battle, which is not repeated twice in a row. -1 if none.
         */
        int last_battle_ = -1;
};
