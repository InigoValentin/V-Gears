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

#include <vector>
#include <boost/cstdint.hpp>
#include <OgreImageCodec.h>
#include <OgrePlatform.h>
#include "common/TypeDefine.h"

namespace VGears{

    /**
     * Handles TEX files.
     *
     * TEX files contain texture data for the PC version of Final Fantasy VII.
     */
    class TexFile{

        public:

            /**
             * Constructor.
             */
            TexFile();

            /**
             * Destructor.
             */
            virtual ~TexFile();

            /**
             * Reads texture data.
             *
             * @param[in] input The contents of the TEX file.
             */
            void Read(Ogre::DataStreamPtr& input);

            /**
             * Reads the TEX file header and sets the instance data.
             *
             * @param[in] input The contents of the TEX file.
             */
            void ReadHeader(Ogre::DataStreamPtr& input);

            /**
             * Reads texture data and matches it with the color palette.
             *
             * @param[in] input The contents of the TEX file.
             */
            void ReadPalleted(Ogre::DataStreamPtr& input);

            /**
             * Retrieves pixel information from the texture image.
             *
             * @return Pixel data.
             */
            Ogre::MemoryDataStream* GetPixelData() const;

            /**
             * Retrieves image data from the texture.
             *
             * @return The texture image.
             */
            Ogre::ImageCodec::ImageData* GetImageData() const;

            typedef Ogre::int32 TexEntry;

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma pack (push, 1)
#else
#pragma pack (1)
#endif
            /**
             * Bit data in a TEX file.
             */
            struct BitData{

                /**
                 * Minimum colour value.
                 */
                TexEntry color_min;

                /**
                 * Maximum colour value.
                 */
                TexEntry color_max;

                /**
                 * Minimum alpha value.
                 */
                TexEntry alpha_min;

                /**
                 * Maximum alpha value.
                 */
                TexEntry alpha_max;

                /**
                 * @todo Understand and document.
                 */
                TexEntry pixel_min;

                /**
                 * @todo Understand and document.
                 */
                TexEntry pixel_max;
            };

            /**
             * Texture image data.
             */
            struct ImageData{

                /**
                 * Image depth, in bits.
                 */
                TexEntry bit_depth;

                /**
                 * Image width.
                 */
                TexEntry width;

                /**
                 * Image height.
                 */
                TexEntry height;

                /**
                 * Image pitch.
                 */
                TexEntry pitch;
            };

            /**
             * Colour palette data.
             */
            struct PaletteData{

                /**
                 * @todo Understand and document.
                 */
                TexEntry flag;

                /**
                 * @todo Understand and document.
                 */
                TexEntry index_bits;

                /**
                 * @todo Understand and document.
                 */
                TexEntry index_8bit;

                /**
                 * @todo Understand and document.
                 */
                TexEntry total_color_count;

                /**
                 * @todo Understand and document.
                 */
                TexEntry colors_per_palette;
            };

            /**
             * RGBA colour data.
             */
            struct RGBAData{

                /**
                 * Red component.
                 */
                TexEntry red;

                /**
                 * Green component.
                 */
                TexEntry green;

                /**
                 * Blue component.
                 */
                TexEntry blue;

                /**
                 * Alpha component.
                 */
                TexEntry alpha;
            };

            /**
             * Pixel format structure.
             */
            struct PixelFormat{

                /**
                 * Number of bits per pixel definition.
                 */
                TexEntry bits_per_pixel;

                /**
                 * Number of bytes per pixel definition.
                 */
                TexEntry bytes_per_pixel;

                /**
                 * Number of bits.
                 */
                RGBAData bit_count;

                /**
                 * @todo Understand and document.
                 */
                RGBAData bit_mask;

                /**
                 * @todo Understand and document.
                 */
                RGBAData bit_shift;

                /**
                 * @todo Understand and document.
                 */
                RGBAData bit_count_unused;

                /**
                 * @todo Understand and document.
                 */
                RGBAData shades;
            };

            /**
             * A TEX file header.
             */
            struct Header{

                /**
                 * File format version.
                 */
                TexEntry version;

                /**
                 * Unknown data.
                 */
                TexEntry unknown_0x04;

                /**
                 * Color key.
                 */
                TexEntry color_key_flag;

                /**
                 * Unknown data.
                 */
                TexEntry unknown_0x0C;

                /**
                 * Unknown data.
                 */
                TexEntry unknown_0x10;

                /**
                 * Bit data.
                 */
                BitData bit_data;

                /**
                 * The type of colour palette.
                 */
                TexEntry palette_type;

                /**
                 * The number of palettes.
                 */
                TexEntry palette_count;

                /**
                 * The number of colours between all the palettes.
                 */
                TexEntry palette_total_color_count;

                /**
                 * Texture image data.
                 */
                ImageData image_data;

                /**
                 * Unknown data.
                 */
                TexEntry unknown_0x48;

                /**
                 * Colour palette data.
                 */
                PaletteData palette_data;

                /**
                 * @todo Understand and document.
                 */
                TexEntry runtime_data_ptr_palette_data;

                /**
                 * Pixel format.
                 */
                PixelFormat pixel_format;

                /**
                 * @todo Understand and document.
                 */
                TexEntry color_key_array_flag;

                /**
                 * @todo Understand and document.
                 */
                TexEntry runtime_data_ptr_color_key_array;

                /**
                 * @todo Understand and document.
                 */
                TexEntry reference_alpha;

                /**
                 * @todo Understand and document.
                 */
                TexEntry runtime_data_02;

                /**
                 * @todo Understand and document.
                 */
                TexEntry unknown_0xCC;

                /**
                 * @todo Understand and document.
                 */
                TexEntry runtime_data_palette_index;

                /**
                 * @todo Understand and document.
                 */
                TexEntry runtime_data_ptr_image_data;

                /**
                 * @todo Understand and document.
                 */
                TexEntry runtime_data_04;

                /**
                 * Unknown data.
                 */
                TexEntry unknown_06;

                /**
                 * Unknown data.
                 */
                TexEntry unknown_07;

                /**
                 * Unknown data.
                 */
                TexEntry unknown_08;

                /**
                 * Unknown data.
                 */
                TexEntry unknown_09;
            } header_;

            /**
             * Texture format.
             */
            struct TextureFormat{

                // reversing ff7 code
                // structure @ 0x3c
                // size 0x80

                /**
                 * Texture width.
                 */
                TexEntry width;

                /**
                 * Texture height.
                 */
                TexEntry height;

                /**
                 * Texture pitch.
                 */
                TexEntry pitch;

                /**
                 * Unknown data.
                 */
                TexEntry unknown_0x48;

                /**
                 * @todo Understand and document.
                 */
                TexEntry palette_flag;

                /**
                 * @todo Understand and document.
                 */
                TexEntry bits_per_palette_index;

                /**
                 * @todo Understand and document.
                 */
                TexEntry palette_index_8bit;

                /**
                 * @todo Understand and document.
                 */
                TexEntry palette_size;

                /**
                 * The number of colours in the palette.
                 */
                TexEntry palette_color_count;

                /**
                 * @todo Understand and document.
                 */
                TexEntry runtime_data_ptr_palette_data;

                /**
                 * The format for the texture pixels.
                 */
                PixelFormat pixel_format;
            };

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma pack (pop)
#else
#pragma pack ()
#endif
            enum PaletteType{
                // as in ogre, eg msb -> lsb

                /**
                 * Palette type: ARGB, 8 bit per component.
                 */
                PF_A8R8G8B8     = 0x00

                /**
                 * Palette type: ARGB, floating values.
                 */
               ,PF_FLOAT32_ARGB = 0x01

               /**
                * Palette type: BGRA, 8 bit per component.
                */
               ,PF_B8G8R8A8     = 0x02
            };

            typedef ColorA8R8G8B8 Color;

            typedef std::vector<Color> Palette;

            typedef std::vector<Color> TexImageData;

            /**
             * The texture palette.
             */
            Palette palette;

            /**
             * The texture image data.
             */
            TexImageData image_data_;

        private:

            /**
             * Flips the endian mode of data.
             *
             * If the data was big endian, it will be conterted to little
             * endian. If it was little endian, it will be converted to big
             * endian.
             *
             * @param[in,out] data Data to flip.
             * @param[in] size Size of data.
             * @param[in] count How much data to flip.
             */
            void FlipEndian(void * data, size_t size, size_t count) const;

            /**
             * Flips the endian mode of data.
             *
             * If the data was big endian, it will be conterted to little
             * endian. If it was little endian, it will be converted to big
             * endian.
             *
             * @param[in,out] data Data to flip.
             * @param[in] size Size of data.
             */
            void FlipEndian(void * data, size_t size) const;

            /**
             * @todo Understand and document.
             */
            static const Ogre::uint8 USE_REFERENCE_ALPHA;
    };
}
