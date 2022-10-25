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

#include <cassert>
#include <zlib.h>
#include "common/Logger.h"
#include "common/BinGZipFile.h"

BinGZipFile::BinGZipFile(const Ogre::String& file): File(file), file_count_(0){
    InnerGetNumberOfFiles();
}

BinGZipFile::BinGZipFile(File* file): File(file), file_count_(0){
    InnerGetNumberOfFiles();
}

BinGZipFile::BinGZipFile(File* file, u32 offset, u32 length):
  File(file, offset, length), file_count_(0)
{InnerGetNumberOfFiles();}

BinGZipFile::BinGZipFile(u8* buffer, u32 offset, u32 length):
    File(buffer, offset, length), file_count_(0)
{
    InnerGetNumberOfFiles();
}

BinGZipFile::~BinGZipFile(){}

File* BinGZipFile::ExtractGZip(u32 file_index){
    if (file_index >= file_count_) return NULL;
    u32 extract_size = 256 * 1024;
    u8* extract_buffer = static_cast<u8*>(malloc(extract_size));
    int ret;
    z_stream strm;
    u32 offset = InnerGetFileOffset(file_index);

    strm.zalloc = Z_NULL; // Used to allocate the internal state.
    strm.zfree = Z_NULL; // Used to free the internal state.
    strm.opaque = Z_NULL; // Private data object passed to zalloc and zfree.
    strm.next_in = buffer_ + offset; //Next input byte.
    strm.avail_in = buffer_size_; // Number of bytes available at next_in.
    strm.next_out = extract_buffer; // Next output byte should be put there.
    strm.avail_out = extract_size; // Remaining free space at next_out.
    ret = inflateInit2(&strm, 15 + 32);
    if (ret != Z_OK){
        switch (ret){
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                LOGGER->Log(
                  "Warning: inflateInit2 - Z_MEM_ERROR in file " + file_name_
                );
                break;
            case Z_STREAM_ERROR:
                inflateEnd(&strm);
                LOGGER->Log(
                  "Warning: inflateInit2 - Z_STREAM_ERROR in file " + file_name_
                );
                break;
            default:
                inflateEnd(&strm);
                LOGGER->Log(
                  "Warning: inflateInit2 - unknown error in file " + file_name_
                );
        }
        return NULL;
    }
    do{
        if (strm.next_out == NULL){
            inflateEnd(&strm);
            LOGGER->Log("Warning: strm.next_out == NULL in file " + file_name_);
            return NULL;
        }
        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);
        switch (ret){
            case Z_NEED_DICT:
                inflateEnd(&strm);
                LOGGER->Log(
                  "Warning: inflate - Z_NEED_DICT in file " + file_name_
                );
                return NULL;
            case Z_DATA_ERROR:
                inflateEnd(&strm);
                LOGGER->Log(
                  "Warning: inflate - Z_DATA_ERROR in file " + file_name_
                );
                return NULL;
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                LOGGER->Log(
                  "Warning: inflate - Z_MEM_ERROR in file " + file_name_
                );
                return NULL;
        }

        if (ret != Z_STREAM_END){
            extract_buffer = static_cast<u8*>(realloc(extract_buffer, extract_size * 2));
            if (extract_buffer == NULL){
                inflateEnd(&strm);
                LOGGER->Log(
                  "Warning: extract_buffer == NULL in file " + file_name_
                );
                return NULL;
            }
            strm.next_out = static_cast<u8*>(extract_buffer + extract_size);
            strm.avail_out = extract_size;
            extract_size *= 2;
        }
    } while (ret != Z_STREAM_END);
    extract_size = extract_size - strm.avail_out;
    (void)inflateEnd(&strm);
    if (ret != Z_STREAM_END){
        LOGGER->Log("Warning: ret != Z_STREAM_END in file " + file_name_);
        return NULL;
    }
    File* file = new File(extract_buffer, 0, extract_size);
    free(extract_buffer);
    return file;
}

u32 BinGZipFile::GetNumberOfFiles(){return file_count_;}

void BinGZipFile::InnerGetNumberOfFiles(){
    file_count_ = 0;
    for (u32 pointer = 0; pointer < buffer_size_;){
        u16 temp = GetU16LE(pointer);
        pointer += 6;
        // Condition for GZip header.
        if (
          GetU32LE(pointer) == 0x00088B1F && GetU32LE(pointer + 4) == 0x00000000
        ){
            ++ file_count_;
        }
        pointer += temp;
    }

    if (file_count_ == 0){
        LOGGER->Log(
          "Warning: " + file_name_ + " isn't archive. number_of_files == 0"
        );
    }
}

u32 BinGZipFile::InnerGetFileOffset(u32 file_index){
    if (file_index >= file_count_) return 0;
    u32 current_file = 0;
    for (u32 pointer = 0; pointer < buffer_size_;){
        u16 temp = GetU16LE(pointer);
        pointer += 6;
        // Condition for GZip header
        if (
          GetU32LE(pointer) == 0x00088B1F && GetU32LE(pointer + 4) == 0x00000000
        ){
            if (file_index == current_file) return pointer;
        }
        ++ current_file;
        pointer += temp;
    }
    return 0;
}
