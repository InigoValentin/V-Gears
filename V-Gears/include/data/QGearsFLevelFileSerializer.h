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
#include "QGearsFLevelFile.h"
#include "QGearsSerializer.h"

namespace QGears{

    /**
     * Handles the serialization of flevel files.
     */
    class FLevelFileSerializer : public Serializer{

        public:

            /**
             * Constructor.
             */
            FLevelFileSerializer();

            /**
             * Destructor.
             */
            virtual ~FLevelFileSerializer();

            virtual void ImportFLevelFile(
              Ogre::DataStreamPtr &stream, FLevelFile* dest
            );

            enum{

                /**
                 * Scripts section.
                 */
                SECTION_SCRIPT,

                /**
                 * Camera matrix section/
                 */
                SECTION_CAMERA_MATRIX,

                /**
                 * Model section.
                 */
                SECTION_MODEL_LOADER,

                /**
                 * Color palette section/
                 */
                SECTION_PALETTE,

                /**
                 * Walkmesh section.
                 */
                SECTION_WALKMESH,

                /**
                 * Tile map section.
                 */
                SECTION_TILE_MAP,

                /**
                 * Battle encounter section.
                 */
                SECTION_ENCOUNTER,

                /**
                 * Triggers section.
                 */
                SECTION_TRIGGER,

                /**
                 * Background section.
                 */
                SECTION_BACKGROUND,

                /**
                 * @todo Count of what?
                 */
                SECTION_COUNT
            };

            /**
             * A flevel file header.
             */
            struct Header{

                /**
                 * File format version.
                 */
                uint16 version;

                /**
                 * Number of sections in the file.
                 */
                uint32 section_count;
            };

        protected:

            /**
             * Reads the file header and sets instance data..
             *
             * @param stream[in] Contents of the file.
             */
            void ReadFileHeader(Ogre::DataStreamPtr &stream);

            /**
             * Reads a section data.
             *
             * @param stream[in] Contents of the file.
             * @param out_buffer[out] The contents of the section are loaded
             * here.
             * @param section_size[in] Size of the section to read, in bytes.
             */
            void ReadSectionData(
              Ogre::DataStreamPtr &stream, Ogre::DataStreamPtr &out_buffer,
              size_t section_size
            );

            /**
             * Reads a file section.
             *
             * @param stream[in] Contents of the file.
             * @param dest[out] The section data will be loaded here.
             * @param section_index[in] Index of the section to read.
             */
            virtual void ReadSection(
              Ogre::DataStreamPtr &stream, FLevelFile* dest,
              const size_t section_index
            );

            /**
             * Creates a resource.
             *
             * @param dest[out] The crteated resource.
             * @param extension[in] The resource file extension.
             */
            template<typename ResourceManagerType>
              Ogre::ResourcePtr CreateResource(
                FLevelFile *dest, const String &extension
              );

            /**
             * Reads camera matrix data from a flevel file.
             *
             * @param stream[in] Input stream.
             * @param dest[out] The data will be set on this file.
             */
            void ReadCameraMatrix(
              Ogre::DataStreamPtr &stream, FLevelFile* dest
            );

            /**
             * Reads model list data from a flevel file.
             *
             * @param stream[in] Input stream.
             * @param dest[out] The data will be set on this file.
             */
            void ReadModelList(Ogre::DataStreamPtr &stream, FLevelFile* dest);

            /**
             * Reads color palette data from a flevel file.
             *
             * @param stream[in] Input stream.
             * @param dest[out] The data will be set on this file.
             */
            void ReadPalette(Ogre::DataStreamPtr &stream, FLevelFile* dest);

            /**
             * Reads walkmesh data from a flevel file.
             *
             * @param stream[in] Input stream.
             * @param dest[out] The data will be set on this file.
             */
            void ReadWalkmesh(Ogre::DataStreamPtr &stream, FLevelFile* dest);

            /**
             * Reads background data from a flevel file.
             *
             * @param stream[in] Input stream.
             * @param dest[out] The data will be set on this file.
             */
            void ReadBackground(Ogre::DataStreamPtr &stream, FLevelFile* dest);

            /**
             * Reads trigger data from a flevel file.
             *
             * @param stream[in] Input stream.
             * @param dest[out] The data will be set on this file.
             */
            void ReadTriggers(Ogre::DataStreamPtr &stream, FLevelFile* dest);

            /**
             * @todo Understand and document.
             *
             * @param stream[in] Input stream.
             */
            void ReadEnd(Ogre::DataStreamPtr &stream);

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
            );

            /**
             * Retrieves the base name of a flevel file.
             *
             * @param dest[in] Flevel file.
             * @return The filename, without path or extension.
             */
            virtual String GetBaseName(const FLevelFile* dest) const;

            /**
             * End-of-file tag.
             */
            static const String TAG_FILE_END;

        private:

            /**
             * The file header.
             */
            Header header_;
    };
}
