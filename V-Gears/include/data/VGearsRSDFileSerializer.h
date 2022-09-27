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

#include "VGearsRSDFile.h"
#include "VGearsSerializer.h"
#include "common/TypeDefine.h"

namespace VGears{

    /**
     * Handles the serialization of RDS files.
     */
    class RSDFileSerializer : public Serializer{

        public:

            /**
             * Constructor.
             */
            RSDFileSerializer();

            /**
             * Destructor.
             */
            virtual ~RSDFileSerializer();

            /**
             * Imports a RSD file.
             *
             * @param stream[in] The contents of the RSD file.
             * @param dest[out] The formed RSD file.
             */
            virtual void ImportRSDFile(
              Ogre::DataStreamPtr &stream, RSDFile* dest
            );

        protected:

            /**
             * Reads a file header and sets the instance data.
             *
             * @param stream[in] The contents of the RSD file.
             */
            virtual void ReadFileHeader(Ogre::DataStreamPtr &stream);

            /**
             * Parses a line of the RSD file.
             *
             * @param line[in] The line to parse.
             */
            virtual void ParseLine(const String &line, RSDFile* dest);

            /**
             * The file header tag.
             */
            static const String TAG_HEADER;

            /**
             * The polygon tag.
             */
            static const String TAG_POLYGON;

            /**
             * The material tag.
             */
            static const String TAG_MATERIAL;

            /**
             * The group tag.
             */
            static const String TAG_GROUP;

            /**
             * The texture count tag.
             */
            static const String TAG_TEXTURE_COUNT;

            /**
             * The texture name tag.
             */
            static const String TAG_TEXTURE_NAME;

            /**
             * Delimiter for line parsing.
             */
            static const String PARSE_DELIMITER;

        private:

            /**
             * Indicates if the file has a texture count.
             */
            bool has_texture_count_;

            /**
             * The texture count (if any).
             */
            size_t  texture_count_;
    };
}
