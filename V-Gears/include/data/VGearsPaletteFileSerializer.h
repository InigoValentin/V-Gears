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

#include <OgrePixelFormat.h>
#include "common/TypeDefine.h"
#include "VGearsPaletteFile.h"
#include "VGearsSerializer.h"

namespace VGears{

    /**
     * Handles the serialization of colour palette files.
     */
    class PaletteFileSerializer : public Serializer{

        public:

            /**
             * Constructor.
             */
            PaletteFileSerializer();

            /**
             * Destructor.
             */
            virtual ~PaletteFileSerializer();

            /**
             * Imports a colour palette file.
             *
             * @param stream[in] The contents of the palette file.
             * @param dest[out] The formed palette file.
             */
            virtual void ImportPaletteFile(
              Ogre::DataStreamPtr &stream, PaletteFile* dest
            );

            enum {

                /**
                 * Bitmask for red colour.
                 */
                BIT_MASK_RED = 0x001F,

                /**
                 * Bitmask for green colour.
                 */
                BIT_MASK_GREEN = 0x03E0,

                /**
                 * Bitmask for blie colour.
                 */
                BIT_MASK_BLUE = 0x7C00,

                /**
                 * Bitmask for palette size.
                 */
                BIT_SIZE = 0x001F,

                /**
                 * Bitmask for RGB colour.
                 */
                BIT_MASK_RGB = BIT_MASK_BLUE | BIT_MASK_GREEN | BIT_MASK_RED,

                /**
                 * Bitmask for alpha component.
                 */
                BIT_MASK_ALPHA  = 0x8000
            };

            /**
             * A colour palette file header.
             */
            struct Header{

                /**
                 * The size of the file.
                 */
                uint32 file_size;

                /**
                 * The X coordinate of the palette.
                 *
                 * @todo Units?
                 */
                uint16 pal_x;

                /**
                 * The Y coordinate of the palette.
                 *
                 * @todo Units?
                 */
                uint16 pal_y;

                /**
                 * The number of colurs per palette page.
                 */
                uint16 colors_per_page;

                /**
                 * The number of palette pages.
                 */
                uint16 page_count;
            };

            typedef PaletteFile::Color Color;

            typedef PaletteFile::Page Page;

        protected:

            /**
             * Reads a file header and sets the instance data.
             *
             * @param stream[in] The contents of the HRC file.
             */
            virtual void ReadFileHeader(Ogre::DataStreamPtr &stream);

            /**
             * Reads an object as a colour.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed colour data.
             */
            virtual void readObject(Ogre::DataStreamPtr &stream, Color &dest);

            /**
             * Reads an object as a palette page.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed page data.
             */
            virtual void readObject(Ogre::DataStreamPtr &stream, Page &dest);

            using Serializer::readObject;

            /**
             * Converts a colour.
             *
             * @param [in|out] Colour to convert.
             * @todo What kind of conversion is it doing?
             */
            virtual void ConvertColour(uint16 &colour) const;

            /**
             * A pixel format.
             */
            static const Ogre::PixelFormat PIXEL_FORMAT;

            /**
             * Reads a stream as a vector.
             *
             * @param stream[in] The input stream.
             * @param dest[out] The vector data will be loaded here.
             * @param count[in] Data units to copy.
             */
            template<typename ValueType> void ReadVector(
              Ogre::DataStreamPtr &stream, std::vector<ValueType> &dest,
              size_t count
            ){
                dest.clear();
                dest.reserve(count);
                for (size_t i(count); i --;){
                    ValueType in_tmp;
                    readObject(stream, in_tmp);
                    dest.push_back(in_tmp);
                }
            }

        private:

            /**
             * The file header.
             */
            Header header_;
    };
}
