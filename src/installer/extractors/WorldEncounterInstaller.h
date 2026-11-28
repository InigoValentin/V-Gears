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

#include <memory>
#include <string>
#include <vector>
#include <tinyxml.h>
#include "common/TypeDefine.h"

/**
 * Writes the random encounters of the world map (world/world_encounters.xml).
 */
class WorldEncounterInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] data_dir The game data folder.
         */
        explicit WorldEncounterInstaller(const std::string& data_dir);

        /**
         * Writes the encounters.
         *
         * Nothing is written if the data doesn't have the expected size.
         *
         * @param[in] data The contents of ENC_W.BIN.
         */
        void Install(const std::vector<u8>& data);

        /**
         * Creates the element with the encounters.
         *
         * ENC_W.BIN has 160 bytes of chocobo encounters and battle data, then 64 records of 16
         * words. Record = region * 4 + terrain group. Word 0: enabled (low byte bit 0) and rate
         * (high byte, the lower the more often). Words 1-6: standard battles, 7-10: special
         * battles (back attack x2, side, pincer) and 11-15: others. A word is a battle ID (10
         * bits) and a probability out of 64 (6 bits).
         *
         * @param[in] data The contents of ENC_W.BIN.
         * @return The element, or null if the data is too short.
         */
        static std::unique_ptr<TiXmlElement> CreateEncounters(const std::vector<u8>& data);

    private:

        /**
         * The game data folder.
         */
        std::string data_dir_;
};
