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

#include <OgreLogManager.h>
#include <OgreException.h>
#include "data/QGearsLGPArchiveSerializer.h"

namespace QGears{

    LGPArchiveSerializer::LGPArchiveSerializer() : Serializer(){}

    LGPArchiveSerializer::~LGPArchiveSerializer(){}

    void LGPArchiveSerializer::ImportLGPArchive(
      Ogre::DataStreamPtr &stream, LGPArchive* dest
    ){
        ReadFileHeader(stream);
        uint32 file_count(0);
        ReadUInt32(stream, file_count);
        FileList& files(dest->GetFiles());
        ReadVector(stream, files, file_count);
        FileList::iterator it(files.begin());
        FileList::const_iterator it_end(files.end());
        while (it != it_end){
            stream->seek(it->file_offset);
            it->datafile_name = readString(stream, FILE_NAME_LENGTH);
            ReadUInt32(stream, it->data_size);
            it->data_offset = stream->tell();
            ++ it;
        }
    }

    void LGPArchiveSerializer::ReadFileHeader(Ogre::DataStreamPtr &stream){
        stream->skip(2); // unused
        String magic_string(readString(stream, MAGIC_STRING_LENGTH));
    }

    void LGPArchiveSerializer::readObject(
      Ogre::DataStreamPtr &stream, FileEntry& file_entry
    ){
        file_entry.file_name = readString(stream, FILE_NAME_LENGTH);
        ReadUInt32(stream, file_entry.file_offset);
        stream->read(&file_entry.unknown1, sizeof(file_entry.unknown1));
        ReadShort(stream, file_entry.unknown2);

        if (
          (file_entry.unknown1 != 14 && file_entry.unknown1 != 11)
          || file_entry.unknown2 != 0
        ){
            Ogre::LogManager::getSingleton().stream()
              << "file_name: " << file_entry.file_name
              << " file_offset: " << file_entry.file_offset
              << " unknown1: " << (uint16)file_entry.unknown1
              << " unknown2: " << file_entry.unknown2;
        }
    }

}
