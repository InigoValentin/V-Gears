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
#include "core/Logger.h"

File::File(const Ogre::String& file):
  file_name_(file),
  buffer_(nullptr),
  buffer_size_(0),
  offset_(0)
{
    LOG_TRIVIAL("Loading file: " + file_name_ + "\n");
    buffer_size_ = FileSystem::GetFileSize(file_name_);
    buffer_ = static_cast<u8*>(malloc(sizeof(u8) * buffer_size_));
    if (!FileSystem::ReadFile(file_name_, buffer_, 0, buffer_size_))
        LOG_TRIVIAL("Warning: " + file_name_ + " not found!\n");
}

File::File(const File* file, u32 offset, u32 length):
  buffer_(nullptr), buffer_size_(length), offset_(offset)
{
    assert(file != nullptr);
    file_name_ = file->GetFileName();
    buffer_ = static_cast<u8*>(malloc(sizeof(u8) * buffer_size_));
    file->GetFileBuffer(buffer_, offset_, buffer_size_);
}

File::File(const u8* buffer, u32 offset, u32 length):
  file_name_("BUFFER"), buffer_(nullptr), buffer_size_(length), offset_(offset)
{
    assert(buffer != nullptr);
    buffer_ = static_cast<u8*>(malloc(sizeof(u8) * buffer_size_));
    memcpy(buffer_, buffer + offset, buffer_size_);
}

File::File(const File* file){
    assert(file != nullptr);
    buffer_size_ = file->GetFileSize();
    file_name_   = file->GetFileName();
    buffer_ = static_cast<u8*>(malloc(sizeof(u8) * buffer_size_));
    file->GetFileBuffer(buffer_, 0, buffer_size_);
}

File::~File(){free(buffer_);}

void File::WriteFile(const Ogre::String& file) const{
    FileSystem::WriteNewFile(file, buffer_, buffer_size_);
}

const Ogre::String& File::GetFileName() const{return file_name_;}

u32 File::GetFileSize() const{return buffer_size_;}

void File::GetFileBuffer(u8* buffer, const u32 &start, const u32 &length) const{
    memcpy(buffer, buffer_ + start, length);
}

u8 File::GetU8(u32 offset) const{return *(buffer_ + offset);}

u16 File::GetU16LE(u32 offset) const{
    return (buffer_ + offset)[0] | ((buffer_ + offset)[1] << 8);
}

u32 File::GetU32LE(u32 offset) const{
    return
      (buffer_ + offset)[0]
      | ((buffer_ + offset)[1] << 8)
      | ((buffer_ + offset)[2] << 16)
      | ((buffer_ + offset)[3] << 24);
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
    if (offset < buffer_size_) offset_ = offset;
    else offset_ = buffer_size_ - 1;
}
