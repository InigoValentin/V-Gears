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

#include <cstdint>
#include <vector>
#include <OgreImage.h>
#include <OgreString.h>
#include <OgreTexture.h>

/**
 * Palette animations of a 2D background.
 *
 * The backgrounds of the original game use 8 bit tiles with palettes, and the scripts change the
 * palettes while the field runs (waterfalls, lights, fades). The tiles of the image of a field are
 * already painted with their palette, so for the palettes a script changes the installer saves the
 * palettes and the indexes of the tiles (palette.bin), and this class paints them again when the
 * script changes a palette.
 *
 * The scripts work like in the original game: an array of palettes in memory (the "slots", 16
 * colors each) and the palettes of the background (the "pages"). Colors are the 16 bit values of
 * the original game: 5 bits per channel (red in the lowest bits) and the flag 0x8000.
 */
class BackgroundPalette{
    public:

        /**
         * Loads the palette data of a field.
         *
         * @param[in] name Resource name of the palette file.
         * @return True if it was loaded.
         */
        bool Load(const Ogre::String& name);

        /**
         * Removes all data.
         */
        void Clear();

        /**
         * Checks if there is palette data.
         */
        bool IsEmpty() const;

        /**
         * Stores colors of a page in the array of palettes (STPAL).
         */
        void Store(int page, int slot, int count);

        /**
         * Loads colors of a page from the array of palettes (LDPAL).
         */
        void Load(int slot, int page, int count);

        /**
         * Copies colors inside the array of palettes (CPPAL).
         */
        void Copy(int source, int destination, int count);

        /**
         * Multiplies the channels of colors in the array of palettes (MPPAL, MPPAL2). 64 is 1.0.
         */
        void Multiply(
          int source, int destination, int start, int count, int red, int green, int blue
        );

        /**
         * Adds signed values to the channels of colors in the array of palettes (ADPAL).
         */
        void Add(int source, int destination, int red, int green, int blue, int count);

        /**
         * Paints the tiles of the pages that changed in the image.
         *
         * @param[in,out] image The image of the whole background, which is changed.
         * @return True if the image changed.
         */
        bool Paint(Ogre::Image& image);

        /**
         * The color of a palette entry as it is shown, like the installer does it.
         *
         * @param[in] color Color of the original game (5 bits per channel and the flag 0x8000).
         * @param[in] first Color 0 of the same page, used for the empty colors.
         * @param[in] index Index of the color in the page.
         * @param[in] first_hidden True if the first color of the page is transparent.
         * @param[out] rgba The 4 bytes to write: red, green, blue, alpha.
         */
        static void ToRgba(
          uint16_t color, uint16_t first, int index, bool first_hidden, uint8_t* rgba
        );

        /**
         * Checks if the first color of a page is a bright marker color, whose empty entries are
         * hidden (see ToRgba).
         */
        static bool IsBrightFirstColor(uint16_t first);

        /**
         * Multiplies a color (see Multiply).
         */
        static uint16_t MultiplyColor(uint16_t color, int red, int green, int blue);

        /**
         * Adds a value to the channels of a color (see Add).
         */
        static uint16_t AddToColor(uint16_t color, int red, int green, int blue);

    private:

        /**
         * A tile that has to be painted again when its page changes.
         */
        struct Tile{
            uint16_t x;
            uint16_t y;
            uint16_t width;
            uint16_t height;
            uint16_t page;
            bool first_hidden;
            std::vector<uint8_t> indexes;
        };

        /**
         * Number of colors that a slot of the array has.
         */
        static const int SLOT_COLORS = 16;

        /**
         * Size of the array of palettes, in colors (256 slots).
         */
        static const int ARRAY_SIZE = 256 * SLOT_COLORS;

        /**
         * The palette pages.
         */
        std::vector<std::vector<uint16_t>> pages_;

        /**
         * The array of palettes.
         */
        std::vector<uint16_t> array_ = std::vector<uint16_t>(ARRAY_SIZE, 0);

        /**
         * The tiles.
         */
        std::vector<Tile> tiles_;

        /**
         * The pages that changed since the last call to Paint.
         */
        std::vector<bool> dirty_;
};
