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

#include <functional>
#include <string>
#include <vector>
#include <Overlay/OgreUTFString.h>
#include <tinyxml.h>
#include "UiTextChar.h"

/**
 * Reads the markup of a text into the list of its characters.
 *
 * The elements it knows are: colour (value), pause_ok, pause (time, in frames of the game),
 * next_page, timer, variable (name), character (id), party (pos), include (name) and image
 * (sprite). The contents of the elements are read too.
 */
class UiTextParser{

    public:

        /**
         * Name of the variable with the time of the timer.
         */
        static const char* const TIMER_VARIABLE;

        /**
         * What the parser needs to know to expand the elements of a text. All must be set.
         */
        struct Sources{

            /**
             * Retrieves the value of a variable.
             */
            std::function<Ogre::UTFString(const Ogre::String&)> variable;

            /**
             * Retrieves the name of a character, by ID.
             */
            std::function<std::string(int)> character_name;

            /**
             * Retrieves the name of a party member, by position.
             */
            std::function<std::string(int)> party_name;

            /**
             * Retrieves a text to include, by name. Null if there is none.
             */
            std::function<TiXmlNode*(const Ogre::String&)> text;

            /**
             * Creates the sprite of an image, and sets it and its position in the character.
             * Returns false if there is no such image.
             */
            std::function<bool(const Ogre::String&, TextChar&)> image;
        };

        /**
         * Constructor.
         *
         * @param[in] sources Where the contents of the elements come from.
         * @param[out] text The characters are added at the end of this list.
         */
        UiTextParser(const Sources& sources, std::vector<TextChar>& text);

        /**
         * Reads a node, its contents and the nodes that follow it.
         *
         * @param[in] node The first node. Nothing is read if it's null.
         * @param[in] colour Colour of the characters.
         */
        void Parse(TiXmlNode* node, const Ogre::ColourValue& colour);

        /**
         * Checks if any of the parsed text shows the timer.
         */
        bool UsesTimer() const{return uses_timer_;}

    private:

        /**
         * Adds the characters of an element, not including its contents.
         *
         * @param[in] element The element.
         * @param[in] colour Colour of the characters.
         * @return The colour of the contents of the element.
         */
        Ogre::ColourValue ParseElement(const TiXmlElement& element, const Ogre::ColourValue& colour);

        /**
         * Adds a variable: a character that marks where it is, followed by its value.
         */
        void AddVariable(const Ogre::String& name, const Ogre::ColourValue& colour);

        /**
         * Adds the characters of a name.
         */
        void AddName(const std::string& name, const Ogre::ColourValue& colour);

        /**
         * Adds the characters of a string.
         */
        void AddString(const Ogre::UTFString& string, const Ogre::ColourValue& colour);

        /**
         * Where the contents of the elements come from.
         */
        const Sources& sources_;

        /**
         * The list of characters.
         */
        std::vector<TextChar>& text_;

        /**
         * True if any text shows the timer.
         */
        bool uses_timer_;
};
