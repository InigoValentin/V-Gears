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
#include "QGearsAFile.h"
#include "QGearsSerializer.h"

namespace QGears{

    /**
     * Handles the serialization of A files.
     */
    class AFileSerializer : public Serializer{

        public:

            /**
             * Constructor.
             */
            AFileSerializer();

            /**
             * Destructor.
             */
            virtual ~AFileSerializer();

            /**
             * Imports an A file.
             *
             * @param stream[in] The contents of the A file.
             * @param dest[out] The A file.
             */
            void ImportAFile(Ogre::DataStreamPtr &stream, AFile* dest);

            /**
             * An A file header.
             */
            struct Header{

                /**
                 * The file type version.
                 */
                uint32 version;

                /**
                 * The frame count.
                 */
                uint32 frame_count;

                /**
                 * The number of bones.
                 */
                uint32 bone_count;

                /**
                 * The axis rotation order.
                 */
                uint32 rotation_order;

                /**
                 * @todo Understand and document.
                 */
                uint32 runtime_data[5];
            };

        protected:

            /**
             * Reads a file header and sets the instance data.
             *
             * @param stream[in] The contents of the A file.
             */
            virtual void ReadFileHeader(Ogre::DataStreamPtr &stream) final;

            /**
             * Reads an object as an A file.
             *
             * @param stream[in] Input data.
             * @param dest[out] The formed A file.
             */
            void readObject(Ogre::DataStreamPtr &stream, AFile::Frame &dest);

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
            );

        private:

            /**
             * The file header.
             */
            Header header_;
        };
}
