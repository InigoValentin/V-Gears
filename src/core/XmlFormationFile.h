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
 * Handles formation files.
 *
 * Formation files contain data specific for a battle. They have information about battle
 * parameters, enemies, battle location, camera. They are also used for arena battles.
 */
class XmlFormationFile : public XmlFile{

    public:

        /**
         * Constructor.
         *
         * @param[in] file Path to the formation XML file.
         */
        explicit XmlFormationFile(const Ogre::String& file);

        /**
         * Destructor.
         */
        virtual ~XmlFormationFile();

        /**
         * Parses the file and loads the formation data.
         */
        void LoadFormation();

};
