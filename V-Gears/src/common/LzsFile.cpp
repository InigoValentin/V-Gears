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

#include "common/LzsFile.h"
#include "core/Logger.h"

LzsFile::LzsFile(const Ogre::String& file): File(file){
    ExtractLzs();
}

LzsFile::LzsFile(File* pFile, u32 offset, u32 length):
  File(pFile, offset, length)
{
    ExtractLzs();
}

LzsFile::LzsFile(const u8* pBuffer, u32 offset, u32 length):
  File(pBuffer, offset, length)
{
    ExtractLzs();
}

LzsFile::LzsFile(File* pFile): File(pFile){
    ExtractLzs();
}

LzsFile::~LzsFile(){}

void LzsFile::ExtractLzs(){
    u32 input_length = GetU32LE(0) + 4;
    if (input_length != buffer_size_){
        LOG_TRIVIAL("Warning: extract failed, this is not lzs!\n");
        return;
    }
    u32 extract_size = (buffer_size_ + 255) & ~255;
    u8* extractbuffer_ = static_cast<u8*>(malloc(extract_size));
    unsigned int inputoffset_ = 4;
    int outputoffset_ = 0;
    u8 control_byte = 0;
    u8 control_bit = 0;
    while (inputoffset_ < buffer_size_){
        if (control_bit == 0){
            control_byte = (static_cast<u8*>(buffer_))[inputoffset_ ++];
            control_bit = 8;
        }

        if (control_byte & 1){
            (static_cast<u8*>(extractbuffer_))[outputoffset_ ++] =
              (static_cast<u8*>(buffer_))[inputoffset_ ++];
            if (outputoffset_ == extract_size){
                extract_size += 256;
                extractbuffer_ = static_cast<u8*>(realloc(extractbuffer_, extract_size));
            }
        }
        else{
            u8 reference1 = (static_cast<u8*>(buffer_))[inputoffset_++];
            u8 reference2 = (static_cast<u8*>(buffer_))[inputoffset_++];
            u16 referenceoffset_ = reference1 | ((reference2 & 0xF0) << 4);
            u8 reference_length = (reference2 & 0xF) + 3;
            int realoffset_
              = outputoffset_ - ((outputoffset_ - 18 - referenceoffset_) & 0xFFF);
            for (int j = 0; j < reference_length; ++ j){
                if (realoffset_ < 0){
                    (static_cast<u8*>(extractbuffer_))[outputoffset_++] = 0;
                }
                else{
                    (static_cast<u8*>(extractbuffer_))[outputoffset_++] =
                      (static_cast<u8*>(extractbuffer_))[realoffset_];
                }
                if (outputoffset_ == extract_size){
                    extract_size += 256;
                    extractbuffer_ =
                      static_cast<u8*>(realloc(extractbuffer_, extract_size));
                }
                ++ realoffset_;
            }
        }
        control_byte >>= 1;
        control_bit --;
    }
    free(buffer_);
    // the real buffer size and mBufferSize will be a bit different
    buffer_ = extractbuffer_;
    buffer_size_ = outputoffset_;
}

std::vector<VGears::uint8> LzsBuffer::Decompress(
  const std::vector<VGears::uint8>& buffer
){
    LzsFile tmp(static_cast<const u8*>(buffer.data()), 0, buffer.size());
    std::vector<VGears::uint8> ret(tmp.buffer_size_);
    ret.assign(tmp.buffer_, tmp.buffer_ + tmp.buffer_size_);
    return ret;
}
