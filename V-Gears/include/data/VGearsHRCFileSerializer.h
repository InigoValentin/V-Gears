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

#include "VGearsHRCFile.h"
#include "VGearsSerializer.h"

namespace VGears{

    /**
     * Handles the serialization of HRC files.
     */
    class HRCFileSerializer : public Serializer{

        public:

            /**
             * Constructor.
             */
            HRCFileSerializer();

            /**
             * Destructor.
             */
            virtual ~HRCFileSerializer();

            /**
             * Imports a HRC file.
             *
             * @param stream[in] The contents of the HRC file.
             * @param dest[out] The formed HRC file.
             */
            virtual void ImportHRCFile(
              Ogre::DataStreamPtr &stream, HRCFile* dest
            );

        protected:

            /**
             * A comment tag.
             */
            static const String TAG_COMMENT;

            /**
             * File version tag.
             */
            static const String TAG_VERSION;

            /**
             * The name tag.
             */
            static const String TAG_NAME;

            /**
             * The bone count tag.
             */
            static const String TAG_BONE_COUNT;

            typedef std::vector<String> Block;

            typedef HRCFile::Bone Bone;

            /**
             * Reads a file header and sets the instance data.
             *
             * @param stream[in] The contents of the HRC file.
             */
            virtual void ReadFileHeader(Ogre::DataStreamPtr &stream);

            /**
             * Reads an object as a block.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed block data.
             */
            virtual void ReadBlock(Ogre::DataStreamPtr &stream, Block& dest);

            /**
             * Reads an object as a bone.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed bone data.
             */
            virtual void readObject(Ogre::DataStreamPtr &stream, Bone &dest);

            template<typename ValueType> void ReadVector(
              Ogre::DataStreamPtr &stream, std::vector<ValueType> &dest,
              size_t count
            );

            /**
             * An HRC file header.
             */
            struct Header{

                /**
                 * File format version.
                 */
                long version;

                /**
                 * Number of bones in the file.
                 */
                long bone_count;

                /**
                 * Name of the skeleton in the file.
                 */
                String name;
            };

        private:

            /**
             * The file header.
             */
            Header  header_;
    };
}
