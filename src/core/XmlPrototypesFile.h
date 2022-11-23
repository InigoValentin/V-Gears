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
 * Handles the main XML prototypes file.
 */
class XmlPrototypesFile : public XmlFile{

    public:

        /**
         * Constructor.
         *
         * @param[in] file Path to the main prototypes file.
         */
        XmlPrototypesFile(const Ogre::String& file);

        /**
         * Destructor.
         */
        virtual ~XmlPrototypesFile();

        /**
         * Parses the file and loads the prototypes.
         */
        void LoadPrototypes();
};
