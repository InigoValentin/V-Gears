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
#include <string.h>
#include "common/File.h"
#include "common/FileSystem.h"
#include <algorithm>
#include <cstring>
#include "core/Logger.h"

File::File(){}

File::File(const Ogre::String& file): file_name_(file){
    LOG_TRIVIAL("Loading file: " + file_name_ + "\n");
    // Zeroed, so a file that can't be read is empty data, not whatever was in memory.
    buffer_.assign(FileSystem::GetFileSize(file_name_), 0);
    if (!buffer_.empty() && !FileSystem::ReadFile(file_name_, buffer_.data(), 0, buffer_.size()))
        LOG_TRIVIAL("Warning: " + file_name_ + " not found!\n");
}

File::File(const File* file, u32 offset, u32 length){
    assert(file != nullptr);
    file_name_ = file->GetFileName();
    // The fragment is a file of its own: it is read from its start.
    buffer_.assign(length, 0);
    if (length > 0) file->GetFileBuffer(buffer_.data(), offset, length);
}

File::File(const u8* buffer, u32 offset, u32 length): file_name_("BUFFER"){
    assert(buffer != nullptr);
    buffer_.assign(buffer + offset, buffer + offset + length);
}

File::File(const File* file){
    assert(file != nullptr);
    file_name_ = file->GetFileName();
    buffer_ = file->buffer_;
}

File::~File(){}

void File::WriteFile(const Ogre::String& file) const{
    FileSystem::WriteNewFile(file, buffer_.data(), static_cast<u32>(buffer_.size()));
}

const Ogre::String& File::GetFileName() const{return file_name_;}

u32 File::GetFileSize() const{return static_cast<u32>(buffer_.size());}

void File::GetFileBuffer(u8* buffer, const u32 &start, const u32 &length) const{
    // What is past the end of the file is read as zeros.
    const u32 size = GetFileSize();
    const u32 available = start < size ? std::min(length, size - start) : 0;
    if (available > 0) memcpy(buffer, buffer_.data() + start, available);
    if (available < length) memset(buffer + available, 0, length - available);
}

// Many of the original files are read past their end by the parsers (truncated animations, headers of the
// last entry of an archive...). Those bytes are read as zeros, instead of whatever is in memory after the file.

u8 File::GetU8(u32 offset) const{return offset < buffer_.size() ? buffer_[offset] : 0;}

u16 File::GetU16LE(u32 offset) const{
    return GetU8(offset) | (GetU8(offset + 1) << 8);
}

u32 File::GetU32LE(u32 offset) const{
    return
      static_cast<u32>(GetU8(offset))
      | (static_cast<u32>(GetU8(offset + 1)) << 8)
      | (static_cast<u32>(GetU8(offset + 2)) << 16)
      | (static_cast<u32>(GetU8(offset + 3)) << 24);
}

u8 File::readU8(){
    u8 data(GetU8(offset_));
    offset_ += 1;
    return data;
}

u16 File::readU16LE(){
    u16 data(GetU16LE(offset_));
    offset_ += 2;
    return data;
}

u32 File::readU32LE(){
    u32 data(GetU32LE(offset_));
    offset_ += 4;
    return data;
}

u32 File::GetCurrentOffset(){return offset_;}

void File::SetOffset(u32 offset){
    if (offset < buffer_.size()) offset_ = offset;
    else offset_ = buffer_.empty() ? 0 : GetFileSize() - 1;
}
