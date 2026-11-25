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

#include <string>
#include <vector>
#include "common/TypeDefine.h"

/**
 * Builds the font images used by the game from the textures in WINDOW.BIN.
 *
 * WINDOW.BIN (data/kernel) holds three gzipped files. The first one is a 4 bpp TIM with the window
 * graphics, including the menu font, the small digits and the timer digits. The second one is a 4 bpp
 * TIM with the dialog font. The third one is the width table of the dialog font.
 *
 * The images built here have the layout described by the font XML files in data/fonts, so the XML
 * files don't change. Only the palette indices of the TIM files are used, and they are mapped to fixed
 * colours, because the game tints the text itself.
 */
namespace WindowFonts{

    /**
     * Palette indices of a 4 bpp TIM image, one byte per pixel.
     */
    struct IndexImage{

        /**
         * Image width, in pixels.
         */
        int width = 0;

        /**
         * Image height, in pixels.
         */
        int height = 0;

        /**
         * Palette index of every pixel, row by row.
         */
        std::vector<u8> pixels;

        /**
         * Gets the palette index of a pixel.
         *
         * @param[in] x X coordinate.
         * @param[in] y Y coordinate.
         * @return The palette index, or 0 if the coordinates are outside the image.
         */
        u8 At(int x, int y) const;
    };

    /**
     * An RGBA image, four bytes per pixel.
     */
    struct RgbaImage{

        /**
         * Image width, in pixels.
         */
        int width = 0;

        /**
         * Image height, in pixels.
         */
        int height = 0;

        /**
         * Red, green, blue and alpha of every pixel, row by row.
         */
        std::vector<u8> pixels;
    };

    /**
     * Reads the pixels of a 4 bpp TIM file.
     *
     * @param[in] data The TIM file.
     * @param[in] size Size of the TIM file, in bytes.
     * @param[out] image The palette indices of the image.
     * @return True if the data is a valid 4 bpp TIM file, false otherwise.
     */
    bool ReadTim4(const u8* data, size_t size, IndexImage& image);

    /**
     * Builds the dialog font image (ffvii_en.png) from the second TIM of WINDOW.BIN.
     *
     * @param[in] font The dialog font TIM.
     * @return The 256x256 font image.
     */
    RgbaImage BuildTextFont(const IndexImage& font);

    /**
     * Builds the menu font image (ffvii_menu.png) from the first TIM of WINDOW.BIN.
     *
     * The menu font is made of rows of 8 pixels. Each row is placed in a slot of 16 pixels, with the
     * glyphs in the lower half. The four last rows of the image are empty, the PC version doesn't have
     * the voiced kana that the XML expects there.
     *
     * @param[in] window The window graphics TIM.
     * @return The 128x256 font image.
     */
    RgbaImage BuildMenuFont(const IndexImage& window);

    /**
     * Builds the small digits font image (ffvii_digits.png) from the first TIM of WINDOW.BIN.
     *
     * @param[in] window The window graphics TIM.
     * @return The 128x8 font image.
     */
    RgbaImage BuildDigitsFont(const IndexImage& window);

    /**
     * Builds the timer font image (ffvii_timer.png) from the first TIM of WINDOW.BIN.
     *
     * @param[in] window The window graphics TIM.
     * @return The 128x48 font image.
     */
    RgbaImage BuildTimerFont(const IndexImage& window);

    /**
     * Saves an image as a PNG file.
     *
     * @param[in] image The image to save.
     * @param[in] file_name Path of the PNG file.
     */
    void SavePng(const RgbaImage& image, const std::string& file_name);
}
