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

#include "UiWidget.h"
#include "XmlFile.h"

/**
 * Handles an XML font file.
 */
class XmlFontFile : public XmlFile{

    public:

        /**
         * Constructor.
         *
         * @param file[in] Path to the XMl file with the font data.
         */
        explicit XmlFontFile(const Ogre::String& file);

        /**
         * Destructor.
         */
        virtual ~XmlFontFile();

        /**
         * Parses the XML file and loads the font data.
         */
        void LoadFont();
};
