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
 * Handles background XML files.
 */
class XmlBackground2DFile : public XmlFile{

    public:

        /**
         * Constructor.
         *
         * @param file[in] Path to the file with the background data.
         */
        XmlBackground2DFile(const Ogre::String& file);

        /**
         * Destructor.
         */
        virtual ~XmlBackground2DFile();

        /**
         * Reads the XML file set on construction and loads the data.
         */
        void Load();
};

