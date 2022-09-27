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
#include "data/VGearsBackgroundFile.h"
#include "data/VGearsSerializer.h"

namespace VGears{

    /**
     * Handles the serialization of background files.
     */
    class BackgroundFileSerializer : public Serializer{

        public:

            /**
             * Constructor.
             */
            BackgroundFileSerializer();

            /**
             * Destructor.
             */
            virtual ~BackgroundFileSerializer();

            /**
             * Imports a background file.
             *
             * @param stream[in] The contents of the background file.
             * @param dest[out] The background file.
             */
            void ImportBackgroundFile(
              Ogre::DataStreamPtr &stream, BackgroundFile *dest
          );

            enum {

                /**
                 * Bitmask for red colour.
                 */
                BIT_MASK_RED = 0xF800,

                /**
                 * Bitmask for green colour.
                 */
                BIT_MASK_GREEN = 0x07C0,

                /**
                 * Bitmask for blue colour.
                 */
                BIT_MASK_BLUE = 0x001F,

                /**
                 * @todo Understand and document.
                 */
                BIT_SIZE = 0x001F,

                /**
                 * Bitmask for RGB colour.
                 */
                BIT_MASK_RGB = BIT_MASK_BLUE | BIT_MASK_GREEN | BIT_MASK_RED,

                /**
                 * @todo Understand and document.
                 */
                SPRITE_DST_MAX = 1024
            };

            /**
             * A background file header.
             */
            struct Header{

                /**
                 * Unused data.
                 */
                uint16 unused;

                /**
                 * @todo Understand and document.
                 */
                uint16 sort_sprites_by_palette;
            };

            typedef BackgroundFile::Layer Layer;

            typedef BackgroundFile::SpriteData SpriteData;

            typedef BackgroundFile::SpriteList SpriteList;

            typedef BackgroundFile::Page Page;

            typedef BackgroundFile::Color Color;

        protected:

            /**
             * Reads a background file header.
             *
             * @param stream[in] The contents of the header.
             */
            virtual void ReadFileHeader(Ogre::DataStreamPtr &stream);

            /**
             * Reads a section of a background file header.
             *
             * @param stream[in] The contents of the header.
             * @param section_name[in] The name of the section to read.
             */
            virtual void ReadSectionHeader(
              Ogre::DataStreamPtr &stream, const String &section_name
           );

            /**
             * Reads pallete data from a background file.
             *
             * @param stream[in] Input stream.
             * @param dest[out] The data will be set on this file.
             */
            virtual void ReadPallete(
              Ogre::DataStreamPtr &stream, BackgroundFile *dest
            );

            /**
             * Reads background data from a background file.
             *
             * @param stream[in] Input stream.
             * @param dest[out] The data will be set on this file.
             */
            virtual void ReadBackground(
              Ogre::DataStreamPtr &stream, BackgroundFile *dest
            );

            /**
             * Reads texture data from a background file.
             *
             * @param stream[in] Input stream.
             * @param dest[out] The data will be set on this file.
             */
            virtual void ReadTexture(
              Ogre::DataStreamPtr &stream, BackgroundFile *dest
            );

            /**
             * @todo Understand and document.
             *
             * @param stream[in] Input stream.
             */
            virtual void ReadEnd(Ogre::DataStreamPtr &stream);

            /**
             * Reads a layer from a background file.
             *
             * @param stream[in] Input stream.
             * @param dest[out] The layer info will be loaded here.
             * @param layer_index[in] Index of the layer to read.
             */
            virtual void ReadLayer(
              Ogre::DataStreamPtr &stream, Layer *dest, size_t layer_index
            );

            /**
             * Reads an object as a sprite.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed sprite.
             */
            virtual void readObject(
              Ogre::DataStreamPtr &stream, SpriteData &dest
            );

            /**
             * Reads an object as colour data.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed colour data.
             */
            virtual void readObject(Ogre::DataStreamPtr &stream, Color &dest);

            /**
             * Reads an object as a background page.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed page.
             */
            virtual void readObject(Ogre::DataStreamPtr &stream, Page &dest);

            using Serializer::readObject;

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
                for(size_t i(count); i --;){
                    ValueType in_tmp;
                    readObject(stream, in_tmp);
                    dest.push_back(in_tmp);
                }
            }

            /**
             * Name of the palette section in the file.
             */
            static const String SECTION_NAME_PALETTE;

            /**
             * Name of the back section in the file.
             */
            static const String SECTION_NAME_BACK;

            /**
             * Name of the texture section in the file.
             */
            static const String SECTION_NAME_TEXTURE;

            /**
             * End-of-file tag.
             */
            static const String TAG_FILE_END;

            /**
             * @todo Understand and document.
             */
            static const Ogre::Real SRC_BIG_SCALE;

        private:

            /**
             * Removes malformed sprites from the map.
             *
             * @param[in|out] The list from which to remove malformed sprites.
             * @todo Why are they malformed?
             */
            void RemoveBuggySprites(SpriteList &sprites);

            /**
             * The file header.
             */
            Header  header_;

            /**
             * @todo Understand and document.
             */
            size_t layer_index_;
    };
}
