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
#include <iostream>
#include <zlib.h>
#include "installer/support/Logger.h"
#include "installer/formats/BinGZipFile.h"

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

bool BinGZipFile::Inflate(const u8* data, size_t size, std::vector<u8>& out, std::string& error){
    out.assign(256 * 1024, 0);
    z_stream strm{};
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = const_cast<u8*>(data);
    strm.avail_in = static_cast<uInt>(size);
    strm.next_out = out.data();
    strm.avail_out = static_cast<uInt>(out.size());
    int ret = inflateInit2(&strm, 15 + 32);
    if (ret != Z_OK){
        error = ret == Z_MEM_ERROR ? "inflateInit2: Z_MEM_ERROR"
          : (ret == Z_STREAM_ERROR ? "inflateInit2: Z_STREAM_ERROR" : "inflateInit2: unknown error");
        out.clear();
        return false;
    }
    do{
        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR || ret == Z_STREAM_ERROR){
            error = ret == Z_NEED_DICT ? "inflate: Z_NEED_DICT"
              : (ret == Z_DATA_ERROR ? "inflate: Z_DATA_ERROR"
              : (ret == Z_MEM_ERROR ? "inflate: Z_MEM_ERROR" : "inflate: Z_STREAM_ERROR"));
            inflateEnd(&strm);
            out.clear();
            return false;
        }
        if (ret == Z_STREAM_END) break;
        if (strm.avail_out == 0){
            // The output is full: make it twice as big.
            const size_t used = strm.total_out;
            out.resize(out.size() * 2);
            strm.next_out = out.data() + used;
            strm.avail_out = static_cast<uInt>(out.size() - used);
        }
        else if (strm.avail_in == 0){
            // Room to write but nothing left to read: the data is cut.
            error = "the compressed data is incomplete";
            inflateEnd(&strm);
            out.clear();
            return false;
        }
    } while (true);
    out.resize(strm.total_out);
    inflateEnd(&strm);
    return true;
}

File BinGZipFile::ExtractGZip(u32 file_index){
    if (file_index >= file_count_) return File();
    const u32 offset = InnerGetFileOffset(file_index);
    if (offset >= GetFileSize()) return File();
    std::vector<u8> out;
    std::string error;
    if (!Inflate(buffer_.data() + offset, GetFileSize() - offset, out, error)){
        LOGGER->Log("Warning: " + error + " in file " + file_name_);
        return File();
    }
    if (out.empty()) return File();
    return File(out.data(), 0, static_cast<u32>(out.size()));
}

void BinGZipFile::InnerGetNumberOfFiles(){
    file_count_ = 0;
    // Each file has a 6 byte header, then the GZip data, whose header is checked: stop when they don't fit.
    for (u32 pointer = 0; pointer + 6 + 8 <= GetFileSize();){
        u16 temp = GetU16LE(pointer);
        if (temp == 0xFFFF) temp = 0; // Hack to deal with scene.bin
        pointer += 6;
        // Condition for GZip header.
        if (GetU32LE(pointer) == 0x00088B1F && GetU32LE(pointer + 4) == 0x00000000) ++ file_count_;
        pointer += temp;
    }

    if (file_count_ == 0){
        std::cout << "Warning: " + file_name_ + " isn't archive. number_of_files == 0\n";
        /*LOGGER->Log(
          "Warning: " + file_name_ + " isn't archive. number_of_files == 0"
        );*/
    }
}

u32 BinGZipFile::InnerGetFileOffset(u32 file_index){
    if (file_index >= file_count_) return 0;
    u32 current_file = 0;
    for (u32 pointer = 0; pointer + 6 + 8 <= GetFileSize();){
        u16 temp = GetU16LE(pointer);
        pointer += 6;
        // Condition for GZip header
        if (GetU32LE(pointer) == 0x00088B1F && GetU32LE(pointer + 4) == 0x00000000){
            if (file_index == current_file) return pointer;
        }
        ++ current_file;
        pointer += temp;
    }
    return 0;
}
