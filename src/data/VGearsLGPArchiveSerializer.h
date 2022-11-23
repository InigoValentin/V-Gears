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

#include "VGearsSerializer.h"
#include "VGearsLGPArchive.h"

namespace VGears{

    /**
     * Manages LGP archives serialization.
     */
    class LGPArchiveSerializer : public Serializer{

        public:

            /**
             * Constructor.
             */
            LGPArchiveSerializer();

            /**
             * Destructor.
             */
            virtual ~LGPArchiveSerializer();

            /**
             * Imports an A file.
             *
             * @param[in] stream The contents of the archive.
             * @param[out] dest The formed LGP archive file.
             */
            virtual void ImportLGPArchive(
              Ogre::DataStreamPtr &stream, LGPArchive* dest
            );

            enum {
                /**
                 * Magic MIME string length.
                 */
                MAGIC_STRING_LENGTH = 10,

                /**
                 * Max file name length.
                 */
                FILE_NAME_LENGTH = 20
            };

            typedef LGPArchive::FileEntry FileEntry;

            typedef LGPArchive::FileList  FileList;

        protected:

            /**
             * Reads an archive header and sets the instance data.
             *
             * @param[in] stream The contents of the LGP archive.
             */
            virtual void ReadFileHeader(Ogre::DataStreamPtr &stream);

            /**
             * Reads an object as a LGP archive.
             *
             * @param[in] stream Input data.
             * @param[out] file_entry The formed LGP archive.
             */
            virtual void readObject(
              Ogre::DataStreamPtr &stream, FileEntry& file_entry
            );

            template<typename ValueType> void ReadVector(
              Ogre::DataStreamPtr &stream, std::vector<ValueType> &dest,
              size_t count
            ){
                dest.clear();
                dest.reserve( count );
                for (size_t i(count); i --;){
                    ValueType in_tmp;
                    readObject(stream, in_tmp);
                    dest.push_back(in_tmp);
                }
            }

    };
}
