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
#include "data/VGearsLGPArchiveSerializer.h"

namespace VGears{

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
        ReadFolders(stream, files);
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

    void LGPArchiveSerializer::ReadFolders(Ogre::DataStreamPtr &stream, FileList& files){
        // After the file table there's a lookup table of 30 * 30 entries, then the conflict tables.
        // Each conflict table lists the folders where files of the same name are, with the index
        // of the file in the file table.
        static const size_t LOOKUP_TABLE_SIZE = 30 * 30 * 4;
        static const size_t FOLDER_NAME_LENGTH = 128;
        if (stream->size() < stream->tell() + LOOKUP_TABLE_SIZE + 2) return;
        stream->skip(LOOKUP_TABLE_SIZE);
        uint16 table_count(0);
        ReadUInt16(stream, table_count);
        for (uint16 t = 0; t < table_count; ++ t){
            if (stream->eof()) return;
            uint16 entry_count(0);
            ReadUInt16(stream, entry_count);
            for (uint16 e = 0; e < entry_count; ++ e){
                if (stream->size() < stream->tell() + FOLDER_NAME_LENGTH + 2) return;
                String folder(readString(stream, FOLDER_NAME_LENGTH));
                uint16 index(0);
                ReadUInt16(stream, index);
                if (index < files.size()) files[index].folder = folder;
            }
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
