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
 * Handles a screen file
 */
class XmlScreenFile : public XmlFile{

    public:

        /**
         * Constructor.
         *
         * @param file[in] Path to the XMl screen file.
         */
        explicit XmlScreenFile(const Ogre::String& file);

        /**
         * Destructor.
         */
        virtual ~XmlScreenFile();

        /**
         * Parses the file and loads the screen data.
         */
        void LoadScreen();

        /**
         * Parses a XML node and recursively loads all of it's components.
         *
         * @param node[in] Node to parse.
         * @param base_name[in] Name for the top widget in the XML node.
         * @param widget[in] Top parent widget.
         */
        void LoadScreenRecursive(
          TiXmlNode* node, const Ogre::String& base_name, UiWidget* widget
        );
};
