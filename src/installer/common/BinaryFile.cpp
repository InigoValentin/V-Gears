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

#include "BinaryFile.h"

#include <iostream>
#include <cassert>
#include <math.h>
#include <string.h>
#include "common/FileSystem.h"
#include "core/Logger.h"

BinaryFile::BinaryFile(const File* file){
    assert(file != nullptr);
    buffer_size_bytes_ = file->GetFileSize();
    buffer_size_bits_ = buffer_size_bytes_ * 8;
    file_name_   = file->GetFileName();
    buffer_bytes_ = static_cast<u8*>(malloc(sizeof(u8) * buffer_size_bytes_));
    buffer_bits_ = static_cast<u8*>(malloc(sizeof(u8) * buffer_size_bits_));
    file->GetFileBuffer(buffer_bytes_, 0, buffer_size_bytes_);
    for (int b = 0; b < file->GetFileSize(); b ++){
        u8 byte = file->GetU8(b);
        for (int i = 0; i < 8; i ++){
            buffer_bits_[8 * b + i] = (byte >> 7 - i) & 1;
            if (b < 9){
                std::cout << "BYTE " << b << ": " << (int) byte << " " << i << ": " << (int)buffer_bits_[8 * b + i] << "\n";
            }

        }
    }
}

BinaryFile::~BinaryFile(){}

const Ogre::String& BinaryFile::GetFileName() const{return file_name_;}

u32 BinaryFile::GetFileSize() const{return buffer_size_bytes_;}

int BinaryFile::GetBits(u32 offset_bytes, u8 offset_bits, u8 bits) const{
    int value = 0;
    for (int b = 0; b < bits; b ++){
        std::cout << (int)buffer_bits_[offset_bytes * 8 + offset_bits + b];
        if (buffer_bits_[offset_bytes * 8 + offset_bits + b] == 1){
            std::cout << "(" << (int)pow(2, (bits - 1 - b)) << ") ";
            value += pow(2, (bits - 1 - b));
        }

    }
    std::cout << "\n";
    return value;
}

int BinaryFile::ReadBits(u8 bits){
    int data(GetBits(offset_.bytes, offset_.bits, bits));
    //std::cout << "READING BITS: " << (int)bits << " FROM " << offset_.bytes << "." << (int)offset_.bits << ": " << data << "\n";
    offset_.bits += bits;
    while (offset_.bits > 6){
        offset_.bytes += 1;
        offset_.bits -= 7;
    }
    return data;
}

u8 BinaryFile::GetU8(u32 offset) const{return *(buffer_bytes_ + offset);}

u16 BinaryFile::GetU16LE(u32 offset) const{
    return (buffer_bytes_ + offset)[0] | ((buffer_bytes_ + offset)[1] << 8);
}

u32 BinaryFile::GetU32LE(u32 offset) const{
    return
      (buffer_bytes_ + offset)[0]
      | ((buffer_bytes_ + offset)[1] << 8)
      | ((buffer_bytes_ + offset)[2] << 16)
      | ((buffer_bytes_ + offset)[3] << 24);
}

u8 BinaryFile::ReadU8(){
    Align();
    u8 data(GetU8(offset_.bytes));
    offset_.bytes += 1;
    return data;
}

u16 BinaryFile::ReadU16LE(){
    Align();
    u16 data(GetU16LE(offset_.bytes));
    offset_.bytes += 2;
    return data;
}

u32 BinaryFile::ReadU32LE(){
    Align();
    u32 data(GetU32LE(offset_.bytes));
    offset_.bytes += 4;
    return data;
}

void BinaryFile::Align(){
    if (offset_.bits == 0) return;
    offset_.bits = 0;
    offset_.bytes += 1;
}

u32 BinaryFile::GetCurrentOffset(){return offset_.bytes * 8 + offset_.bits;}

void BinaryFile::SetOffset(u32 offset){
    if (offset < buffer_size_bits_){
        offset_.bytes = offset / 8;
        offset_.bits = offset % 8;
    }
    else{
        offset_.bytes = buffer_size_bits_ / 8 - 1;
        offset_.bits = 7;
    }
}
