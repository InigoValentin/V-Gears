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
 * Handles battle character model list xml files.
 *
 * Those files contain a correlation between playable characters and their models.
 */
class XmlBattleCharactersFile : public XmlFile{

    public:

        /**
         * Information about each character model.
         */
        struct BattleCharacterModel{

            /**
             * Model file.
             */
            std::string model;

            /**
             * Descriptive name of the model.
             */
            std::string name;
        };

        /**
         * Relation of character and it's models
         */
        struct BattleCharacter{

            /**
             * Character id.
             */
            int id;

            /**
             * Character models.
             */
            std::vector<BattleCharacterModel> models;
        };

        /**
         * Constructor.
         *
         * Parses the information from the file.
         *
         * @param[in] file Path to the scenes XML file.
         */
        explicit XmlBattleCharactersFile(const Ogre::String& file);

        /**
         * Destructor.
         */
        virtual ~XmlBattleCharactersFile();

        /**
         * Retrieves the battle character list.
         *
         * @return The map of battle character models.
         */
        std::vector<BattleCharacter> GetCharacters();

    private:

        std::vector<BattleCharacter> list_;

};
