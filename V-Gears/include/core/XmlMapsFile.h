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

#include <OgreStringVector.h>
#include "XmlFile.h"

/**
 * Handles the main maps file.
 */
class XmlMapsFile : public XmlFile{

    public:

        /**
         * Constructor.
         *
         * @param[in] file Path to the maps file.
         */
        XmlMapsFile(const Ogre::String& file);

        /**
         * Destructor.
         */
        virtual ~XmlMapsFile();

        /**
         * Retrieves a map file by map name.
         *
         * @param[in] name Name of the map to look for.
         * @return Path to the specified map file, or an empty string if there
         * is no file for such map.
         */
        const Ogre::String GetMapFileNameByName(const Ogre::String& name);

        /**
         * Retrieves a list of map names.
         *
         * @param[out] complete_params The list of map names will be loaded
         * here.
         */
        void GetMapNames(Ogre::StringVector& complete_params);
};
