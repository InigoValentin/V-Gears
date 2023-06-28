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

#include "common/TypeDefine.h"
#include "common/File.h"

class TexFile{

    public:

        /**
         * Constructor.
         *
         * @param[in,out] file File with the tex data. The file data will not be modified, but it's
         * offset will.
         */
        TexFile(File file);

        /**
         * Constructor.
         *
         * @param[in] path Path to the tex file.
         */
        TexFile(std::string path);

        /**
         * Destructor.
         */
        ~TexFile();

        /**
         * Saves a fragment of the TEX file as a PNG file.
         *
         * @param[in] file_name Full path of the destination file.
         * @param[in] x X coordinate of the image to extract from the TEX file.
         * @param[in] y Y coordinate of the image to extract from the TEX file.
         * @param[in] w Width of the image to extract from the TEX file.
         * @param[in] h Height of the image to extract from the TEX file.
         * @param[in] palette Index of the color palette to use. For non paletted files it's
         * ignored.
         */
        void SavePng(
          std::string file_name, unsigned int x, unsigned int y, unsigned int w, unsigned int h,
          unsigned int palette = 0
        );

        /**
         * Saves the TEX file as a PNG file.
         *
         * @param[in] file_name Full path of the destination file.
         * @param[in] palette Index of the color palette to use. For non paletted files it's
         * ignored.
         */
        void SavePng(std::string file_name, unsigned int palette = 0);

        /**
         * Saves two fragments of the TEX file as a single PNG image.
         *
         * The second fragment will be at the right of the first one. They must have the same
         * height.
         *
         * @param[in] file_name Full path of the destination file.
         * @param[in] x1 X coordinate of the first image to extract from the TEX file.
         * @param[in] x2 X coordinate of the second image to extract from the TEX file.
         * @param[in] y1 Y coordinate of the first image to extract from the TEX file.
         * @param[in] y2 Y coordinate of the first image to extract from the TEX file.
         * @param[in] w1 Width of the first image to extract from the TEX file.
         * @param[in] w2 Width of the second image to extract from the TEX file.
         * @param[in] h Height of the images to extract from the TEX file.
         * @param[in] palette Index of the color palette to use. For non paletted files it's
         * ignored.
         */
        void SavePng(
          std::string file_name, unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2,
          unsigned int w1, unsigned int w2, unsigned int h, unsigned int palette = 0
        );

    private:

        /**
         * Reads data from a file.
         *
         * Called from constructors.
         *
         * @param[in] file The file to read from.
         */
        void Read(File file);

        /**
         * File format version. Always 1. 4 bytes.
         */
        u32 version_;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown_0_;

        /**
         * Color key flag. 4 bytes.
         */
        u32 colour_key_;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown_1_;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown_2_;

        /**
         * Minimum bits per color. 4 bytes.
         *
         * Ignored by V-Geas.
         */
        u32 min_bpc_;

        /**
         * Maximum bits per color. 4 bytes.
         *
         * Ignored by V-Geas.
         */
        u32 max_bpc_;

        /**
         * Minimum alpha bits. 4 bytes.
         *
         * Ignored by V-Gears.
         */
        u32 min_alpha_bits_;

        /**
         * Minimum alpha bits. 4 bytes.
         *
         * Ignored by V-Gears.
         */
        u32 max_alpha_bits_;

        /**
         * Minimum bits per pixel. 4 bytes.
         *
         * Ignored by V-Gears.
         */
        u32 min_bpp_;

        /**
         * Maximum bits per pixel. 4 bytes.
         *
         * Ignored by V-Gears.
         */
        u32 max_bpp_;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown_3_;

        /**
         * Number of colour palettes. 4 bytes.
         */
        u32 palette_count_;

        /**
         * Number of colours per palette. 4 bytes.
         */
        u32 palette_colour_count_;

        /**
         * Bit depth. 4 Bytes.
         *
         * Can be ignores, always follow {@see bpp}.
         */
        u32 bit_depth_;

        /**
         * Image width. 4 bytes
         */
        u32 width_;

        /**
         * Image height. 4 bytes
         */
        u32 height_;

        /**
         * Pitch or bytes per row. 4 bytes.
         *
         * Ignored, use {@see bytes_per_pixel} * {@see width}.
         */
        u32 pitch_;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown_4_;

        /**
         * Indicates a paletted image. 4 bytes.
         */
        u32 has_palette_;

        /**
         * Bits per index. 4 bytes.
         *
         * For paletted images only. 0 for non-paletted.
         */
        u32 bits_per_index_;

        /**
         * Indexed to 8 bit flag.
         *
         * Ignored by OG and V-Gears.
         */
        u32 indexed_to_8_bit_;

        /**
         * Palette size. 4 bytes.
         *
         * It must match {@see palette_count} * {@see colours_per_palette}.
         */
        u32 palette_size_;

        /**
         * Number of colours per palette. 4 bytes.
         *
         * Ignore, use {@see palette_colour_count}
         */
        u32 colours_per_palette_;

        /**
         * Runtime data.
         *
         * V-gears ignores it.
         */
        u32 run_0_;

        /**
         * Bits per pixel.
         */
        u32 bits_per_pixel_;

        /**
         * Bytes per pixel.
         */
        u32 bytes_per_pixel_;

        // Pixel format 0 for non-paletted images.

        /**
         * Number of red bits.
         */
        u32 bits_red_;

        /**
         * Number of green bits
         */
        u32 bits_green_;

        /**
         * Number of blue bits
         */
        u32 bits_blue_;

        /**
         * Number of alpha bits
         */
        u32 bits_alpha_;

        /**
         * Red shift. 4 bytes.
         */
        u32 bitmask_red_;

        /**
         * Green bitmask. 4 bytes.
         */
        u32 bitmask_green_;

        /**
         * Blue bitmask. 4 bytes.
         */
        u32 bitmask_blue_;

        /**
         * Alpha bitmask. 4 bytes.
         */
        u32 bitmask_alpha_;

        /**
         * Red shift. 4 bytes.
         */
        u32 shift_red_;

        /**
         * Green shift. 4 bytes.
         */
        u32 shift_green_;

        /**
         * Blue shift. 4 bytes.
         */
        u32 shift_blue_;

        /**
         * Alpha shift. 4 bytes.
         */
        u32 shift_alpha_;

        /**
         * Red bits. Always 8. 4 bytes.
         *
         * Unused, use {@see bits_red}.
         */
        u32 bits_red_2_;

        /**
         * Green bits. Always 8. 4 bytes.
         *
         * Unused, use {@see bits_green}.
         */
        u32 bits_green_2_;

        /**
         * Blue bits. Always 8. 4 bytes.
         *
         * Unused, use {@see bits_blue}.
         */
        u32 bits_blue_2_;

        /**
         * Alpha bits. Always 8. 4 bytes.
         *
         * Unused, use {@see bits_alpha}.
         */
        u32 bits_alpha_2_;

        /**
         * Max value for red colour. 4 bytes.
         */
        u32 max_red_;

        /**
         * Max value for green colour. 4 bytes.
         */
        u32 max_green_;

        /**
         * Max value for blue colour. 4 bytes.
         */
        u32 max_blue_;

        /**
         * Max alpha value. 4 bytes.
         */
        u32 max_alpha_;

        // Pixel format end.

        /**
         * Colour key array flag. 4 bytes.
         *
         * Indicates the presence of a color key array,
         */
        u32 colour_k_array_;

        /**
         * Runtime data. 4 bytes.
         *
         * V-Gears ignores it.
         */
        u32 run_1_;

        /**
         * Alpha reference value. 4 bytes.
         *
         * Only applies to paltted images, if the alpha value sampled from the palee is 0xFE, this
         * value should be replaced with the reference alpha.
         */
        u32 ref_alpha_;

        /**
         * Runtime data. 4 bytes.
         *
         * V-Gears ignores it.
         */
        u32 run_2_;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown_5_;

        /**
         * Palette index. 4 bytes.
         *
         * Runtime data.
         */
        u32 palette_index_;

        /**
         * Runtime data. 4 bytes.
         *
         * V-Gears ignores it.
         */
        u32 run_3_;

        /**
         * Runtime data. 4 bytes.
         *
         * V-Gears ignores it.
         */
        u32 run_4_;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown_6_;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown_7_;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown_8_;

        /**
         * Unknown data. 4 bytes.
         */
        u32 unknown_9_;

        // Palette data.

        /**
         * Palette data. 4 * {@see palette_size} bytes.
         *
         * Used only for paletted images. Blocks of 32-bit BGRA colour format.
         */
        std::vector<std::vector<Ogre::ColourValue>> palettes_;

        // Pixel data.

        /**
         * Pixel data for non paletted images. {@see width} * {@see height}.
         */
        std::vector<Ogre::ColourValue> pixel_colour_;

        /**
         * Pixel data for paletted images. {@see width} * {@see height} * {@see bytes_per pixels}.
         *
         * In paletted images, every byte is a color reference in the palette. For non-paletted
         * images, a pixel format section.
         */
        std::vector<u8> pixel_ref_;

        // Colour key array

        /**
         * Colour key array. {@see palette_count} bytes.
         */
        std::vector<u8> colour_key_array_;
};
