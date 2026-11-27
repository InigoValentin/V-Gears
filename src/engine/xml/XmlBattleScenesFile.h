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

#include "XmlFile.h"

/**
 * Handles battle scene list xml files.
 *
 * These files relate battle scene IDs with their models.
 */
class XmlBattleScenesFile : public XmlFile{

    public:

        /**
         * Information about a battle scene.
         */
        struct BattleScene{

            /**
             * Scene id.
             */
            int id;

            /**
             * Scene model.
             */
            std::string model;

            /**
             * Scene description;
             */
            std::string description;
        };

        /**
         * Constructor.
         *
         * Parses the information from the file.
         *
         * @param[in] file Path to the scenes XML file.
         */
        explicit XmlBattleScenesFile(const Ogre::String& file);

        /**
         * Destructor.
         */
        virtual ~XmlBattleScenesFile();

        /**
         * Retrieves the battle scene list.
         *
         * @return The map of battle scene models.
         */
        std::vector<BattleScene> GetScenes();

    private:

        std::vector<BattleScene> list_;

};
