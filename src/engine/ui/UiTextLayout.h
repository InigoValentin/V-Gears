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
#include <vector>
#include <OgreVector2.h>
#include "engine/graphics/QuadWriter.h"
#include "UiFont.h"
#include "UiTextChar.h"

/**
 * Decides where the characters of a text area go.
 *
 * Positions are relative to the position of the text area, in pixels of the screen. Sizes of the
 * fonts, paddings and images are for a screen 720 pixels high, and are scaled to the real one.
 */
class UiTextLayout{

    public:

        /**
         * Where a text area is, and its sizes.
         */
        struct Area{

            /** Origin of the area. */
            Ogre::Vector2 origin;

            /** Size of the area, in pixels. */
            Ogre::Vector2 size;

            /** Scale of the area. */
            Ogre::Vector2 scale;

            /** Height of the screen. */
            float screen_height;

            /** Padding at the top. */
            float padding_top;

            /** Padding at the right. */
            float padding_right;

            /** Padding at the bottom. */
            float padding_bottom;

            /** Padding at the left. */
            float padding_left;
        };

        /**
         * The rectangle of a character.
         */
        typedef QuadWriter::Rectangle Box;

        /**
         * Constructor. The first character goes at the beginning of the first line.
         *
         * @param[in] area The text area.
         * @param[in] align_width Width of the text to the left of the starting position: 0 for text
         * aligned to the left, the width of the text for the right, half of it to center it.
         * @param[in] scroll Vertical offset of the text.
         * @param[in] line_height Height of the lines of the font.
         */
        UiTextLayout(
          const Area& area, const float align_width, const float scroll, const int line_height
        );

        /**
         * Moves to the beginning of the next line.
         */
        void NewLine();

        /**
         * Places a character, in the next line if it doesn't fit in the current one.
         *
         * @param[in] character Sizes of the character.
         * @param[out] box Where the character goes.
         * @return False if the character goes below the area. It must not be drawn.
         */
        bool Place(const UiCharData& character, Box& box);

        /**
         * Places an image.
         *
         * @param[in] width Width of the image.
         * @param[out] x Horizontal position of the image, in the units of the sizes.
         * @param[out] y Vertical position of the line, in the units of the sizes.
         */
        void PlaceImage(const float width, float& x, float& y);

        /**
         * Calculates the width of a text: the width of its longest line.
         *
         * @param[in] text The text.
         * @param[in] character_data Retrieves the sizes of a character.
         * @return The width.
         */
        static float TextWidth(
          const std::vector<TextChar>& text,
          const std::function<UiCharData(int)>& character_data
        );

        /**
         * Calculates the corners of a rectangle on the screen, in the -1 to 1 range.
         *
         * @param[in] box The rectangle.
         * @param[in] translate Position of the text area.
         * @param[in] rotation Rotation of the text area, in degrees.
         * @param[in] screen Size of the screen.
         * @return The top left, top right, bottom right and bottom left corners.
         */
        static QuadWriter::Corners ScreenCorners(
          const Box& box, const Ogre::Vector2& translate, const float rotation,
          const Ogre::Vector2& screen
        );

    private:

        /**
         * Converts a horizontal size to pixels of the screen.
         */
        float ScaleX(const float size) const;

        /**
         * Converts a vertical size to pixels of the screen.
         */
        float ScaleY(const float size) const;

        /**
         * The text area.
         */
        Area area_;

        /**
         * Height of the lines.
         */
        int line_height_;

        /**
         * Where the lines begin.
         */
        float line_start_;

        /**
         * Horizontal position of the next character.
         */
        float x_;

        /**
         * Vertical position of the current line.
         */
        float y_;
};
