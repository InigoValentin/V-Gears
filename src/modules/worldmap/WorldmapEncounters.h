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
#include "common/TypeDefine.h"

namespace VGears {

    struct TerrainTriangle;

    /**
     * The random battles of the world map.
     */
    class WorldmapEncounters{

        public:

            /**
             * Loads the encounters of the world map. Without them there are no random battles.
             */
            void Load();

            /**
             * Forgets the loaded encounters.
             */
            void Clear();

            /**
             * Counts the time walked, and starts a battle when the chance says so.
             *
             * @param[in] delta_time Seconds walked since the last call.
             * @param[in] triangle Triangle where the player is.
             */
            void Update(const float delta_time, const TerrainTriangle& triangle);

            /**
             * Prevents or allows random encounters.
             *
             * @param[in] locked True to prevent them.
             */
            void SetLocked(const bool locked);

            /**
             * Starts counting the time to the next check again.
             */
            void ResetTimer();

        private:

            /**
             * Random encounters of a region and terrain group, from ENC_W.BIN.
             */
            struct Record{
                bool enabled;
                /// Lower values mean more frequent encounters.
                int rate;
                /// Walkability type that selects this record.
                int terrain;
                /// Words 1-15 of the record: battle ID (10 bits) and probability (6 bits).
                uint16 words[15];
            };

            /**
             * Chooses the battle of an encounter.
             *
             * @param[in] record Encounters of the terrain.
             * @return The battle ID, or -1 if none.
             */
            int RollBattle(const Record& record) const;

            std::vector<Record> encounter_records_;
            float encounter_frames_ = 0;
            int encounter_accumulator_ = 0;
            int encounter_last_battle_ = -1;
            bool encounters_locked_ = false;
    };
}
