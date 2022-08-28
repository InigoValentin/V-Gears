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

#include <OgreString.h>

/**
 * Character data for UI elements.
 */
struct UiCharData{

    /**
     * The character code.
     */
    int char_code;

    /**
     * The character X position in the font image.
     */
    int x;

    /**
     * The character Y position in the font image.
     */
    int y;

    /**
     * The character width.
     */
    int width;

    /**
     * The character height.
     */
    int height;

    /**
     * @todo Understand and document.
     */
    int pre;

    /**
     * @todo Understand and document.
     */
    int post;
};

/**
 * The UI widgets font.
 */
class UiFont{

    public:

        /**
         * Constructor.
         *
         * @param name[in] The font name.
         * @param language[in] The font language.
         */
        UiFont(const Ogre::String& name, const Ogre::String& language);

        /**
         * Destructor.
         */
        virtual ~UiFont();

        /**
         * Retrieves the font name.
         *
         * @return The font name.
         */
        const Ogre::String& GetName() const;

        /**
         * Retrieves the font language.
         *
         * @return The font language.
         */
        const Ogre::String& GetLanguage() const;

        /**
         * Sets an image for the font.
         *
         * The image is a character map.
         *
         * @param image[in] Path to the font image, relative to data/fonts.
         * @param width[in] Image width.
         * @param height[in] Image height.
         */
        void SetImage(
          const Ogre::String& image, const int width, const int height
        );

        /**
         * Retrieves the font image file name.
         *
         * @return The font image file name.
         */
        const Ogre::String& GetImageName() const;

        /**
         * Retrieves the font image width.
         *
         * @return The font image height.
         */
        int GetImageWidth() const;

        /**
         * Retrieves the font image height.
         *
         * @return The font image height.
         */
        int GetImageHeight() const;

        /**
         * Sets the character height for the font.
         *
         * @param height[in] Character height.
         */
        void SetHeight(const int height);

        /**
         * Retrieves the character height of the font.
         *
         * @return The character height.
         */
        int GetHeight() const;

        /**
         * Adds data for a character.
         *
         * @param data[in] The character data to add.
         */
        void AddCharData(const UiCharData& data);

        /**
         * Retrieves character data from a char code.
         *
         * @param char_code[in] The code of the character to retrieve data
         * about.
         * @return Character data associated to the char code. A default {@see
         * UiCharData} structure wil be returnd if there is no data for the
         * char code.
         */
        UiCharData GetCharData(const int char_code) const;

    private:

        /**
         * The font name.
         */
        Ogre::String name_;

        /**
         * The font language.
         */
        Ogre::String language_;

        /**
         * The font image file name.
         */
        Ogre::String image_name_;

        /**
         * The font image file width.
         */
        int image_width_;

        /**
         * The font image file height.
         */
        int image_height_;

        /**
         * The height of the font characters.
         */
        int height_;

        /**
         * List of character data for the font.
         */
        std::vector<UiCharData> char_data_;
};

