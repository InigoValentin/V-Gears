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

#include <iostream>
#include <bitset>
#include <cstdint>
#include "data/DaFile.h"

// Root offsets are in the same units as the model geometry (the root height of an idle Cloud is 466,
// against leg bones of 277 and 253), so they are not scaled.
const float DaFile::OFFSET_SCALE = 1.0f;

DaFile::DaFile(File file): da_file_(file){
    animations_.clear();
    // Save file contents as a byte array.
    file.SetOffset(0);
    da_bytes_ = new u8[file.GetFileSize()];
    file.GetFileBuffer(da_bytes_, 0, file.GetFileSize());
    bit_offset_ = 0;
    file.SetOffset(0);
    Read();
}

std::vector<std::string> DaFile::GenerateAFiles(std::string model_id, std::string path){
    std::vector<std::string> file_list;
    u32 zero = 0x00000000;
    u32 version = 0x00000001;
    u32 rotation_order = 0x00020001;
    for (int anim = 0; anim < animations_.size(); anim ++){
        /*
         * .a files have a 36 byte header header, and after that, each animation frame. Each
         * animation frame has three floats (of 4 bytes) for root rotation, three for root offset,
         * and after them, the bone rotation data.
         */
        std::string file_index_name = std::to_string(animations_[anim].index);
        while (file_index_name.size() < 2) file_index_name = "0" + file_index_name;
        std::string file_name = path + model_id + "_" + file_index_name + ".a";
        std::ofstream a(file_name, std::ios::out | std::ios::binary);
        int bone_count = animations_[anim].bone_count - 1;
        a.write(reinterpret_cast<const char*>(&version), 4);
        a.write(reinterpret_cast<const char*>(&animations_[anim].frame_count), 4);
        a.write(reinterpret_cast<const char*>(&bone_count), 4);
        a.write(reinterpret_cast<const char*>(&rotation_order), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4); // Runtime data, skip 5 bytes.
        a.write(reinterpret_cast<const char*>(&zero), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        for (int f = 0; f < animations_[anim].frames.size(); f ++){
            // Root rotation (always 0).
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            // Root offset.
            a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].offset.f_x), 4);
            a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].offset.f_y), 4);
            a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].offset.f_z), 4);
            // Bones.
            for (int b = 1; b < animations_[anim].frames[f].bones.size(); b ++){
                a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].bones[b].f_x), 4);
                a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].bones[b].f_y), 4);
                a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].bones[b].f_z), 4);
            }
        }
        a.close();
        file_list.push_back(file_name);
    }
    return file_list;
}

int DaFile::ExtendSignInteger(int val, int len){
    // Two's complement value of "len" bits to a signed integer.
    if (len > 0 && (val & (1 << (len - 1))) != 0) return val - (1 << len);
    return val;
}

int DaFile::GetSignExtendedShort(int val, int len){
    return ExtendSignInteger(val, len);
}

void DaFile::Read(){
    bit_offset_ = 0;
    da_file_.SetOffset(0);
    int num_animations = da_file_.readU32LE();
    bit_offset_ += 32;
    for (int a = 0; a < num_animations; a ++){
    //for (int a = 0; a < 1; a ++){ // TODO DEBUG
        int start_offset = bit_offset_ / 8; // In bytes.
        Animation anim = ReadAnimation();
        anim.index = a;
        if (anim.frame_count > 0) animations_.push_back(anim);
        // Manually go to the next animation
        bit_offset_ = (start_offset + anim.size + 12) * 8; // + 12 is for skipping the main header.
        da_file_.SetOffset(start_offset + anim.size + 12);
    }
}

DaFile::Animation DaFile::ReadAnimation(){
    Animation anim;
    // Read the animation headers.
    da_file_.SetOffset(bit_offset_ / 8);
    anim.bone_count = da_file_.readU32LE();
    bit_offset_ += 32;
    anim.frame_count = da_file_.readU32LE();
    bit_offset_ += 32;
    anim.size = da_file_.readU32LE();
    // Skip small (< 11 bytes) animations.
    if (anim.size < 11){
        anim.frame_count = 0;
        return anim;
    }
    bit_offset_ += 32;
    da_file_.readU32LE(); // Duplicated Frames + Size
    bit_offset_ += 32;
    anim.key = da_file_.readU8();
    bit_offset_ += 8;
    Frame frame = ReadFirstFrame(anim.bone_count, anim.key);
    anim.frames.push_back(frame);
    for (int i = 1; i < anim.frame_count; i ++){
        frame = ReadFrame(anim.bone_count, anim.key, frame);
        anim.frames.push_back(frame);
    }
    return anim;
}

DaFile::Frame DaFile::ReadFirstFrame(u32 bones, u8 key){
    Frame frame;
    // Read root offset. In the first frame, always 16 bytes each, as short.
    frame.offset.s_x = GetBitsS(16);
    frame.offset.s_y = GetBitsS(16);
    frame.offset.s_z = GetBitsS(16);
    // Turn offset to float. Any scaling that needs to be done would be done here.
    frame.offset.f_x = static_cast<float>(frame.offset.s_x) * OFFSET_SCALE;
    // Y points down in the original data, up in the converted models.
    frame.offset.f_y = static_cast<float>(frame.offset.s_y) * -OFFSET_SCALE;
    frame.offset.f_z = static_cast<float>(frame.offset.s_z) * OFFSET_SCALE;
    // Read bones.
    for (int i = 0; i < bones; i ++){
        TriValue bone;
        // Now get each bone rotation (the first bone is actually the root, not part of the
        // skeleton). During the first frame, the rotations are always (12 - key) bits. The values
        // are shifted by key to align it to 12 bits.
        bone.s_x = GetBitsS(12 - key) * (1 << key);
        bone.s_y = GetBitsS(12 - key) * (1 << key);
        bone.s_z = GetBitsS(12 - key) * (1 << key);
        // Store the INT version as the angle in the 0-4095 range.
        bone.i_x = bone.s_x & 0xFFF;
        bone.i_y = bone.s_y & 0xFFF;
        bone.i_z = bone.s_z & 0xFFF;
        // Convert to float.
        bone.f_x = static_cast<float>(bone.i_x) / 4096.0f * 360.0f;
        bone.f_y = static_cast<float>(bone.i_y) / 4096.0f * 360.0f;
        bone.f_z = static_cast<float>(bone.i_z) / 4096.0f * 360.0f;
        frame.bones.push_back(bone);
    }
    return frame;
}

DaFile::Frame DaFile::ReadFrame(u32 bones, u8 key, Frame prev){
    Frame frame;

    // Get the root offset delta and add it to the previous frame values. Deltas are in the same
    // units as the first frame offset, so they get the same scale.
    frame.offset.s_x = 0;
    frame.offset.s_y = 0;
    frame.offset.s_z = 0;
    frame.offset.i_x = GetDynamicOffsetFromStream();
    frame.offset.i_y = GetDynamicOffsetFromStream();
    frame.offset.i_z = GetDynamicOffsetFromStream();
    frame.offset.f_x = static_cast<float>(frame.offset.i_x) * OFFSET_SCALE + prev.offset.f_x;
    frame.offset.f_y = static_cast<float>(frame.offset.i_y) * -OFFSET_SCALE + prev.offset.f_y;
    frame.offset.f_z = static_cast<float>(frame.offset.i_z) * OFFSET_SCALE + prev.offset.f_z;

    for (int i = 0; i < bones; i ++){
        TriValue bone;
        // Rotations are 12 bit angles (4096 is a full turn). Deltas are added to the previous
        // value, already shifted to 12 bits, and the result wraps around a full turn.
        bone.s_x = GetCompressedDeltaFromStream(key) + prev.bones[i].s_x;
        bone.s_y = GetCompressedDeltaFromStream(key) + prev.bones[i].s_y;
        bone.s_z = GetCompressedDeltaFromStream(key) + prev.bones[i].s_z;
        bone.i_x = bone.s_x & 0xFFF;
        bone.i_y = bone.s_y & 0xFFF;
        bone.i_z = bone.s_z & 0xFFF;
        bone.f_x = static_cast<float>(bone.i_x) / 4096.0f * 360.0f;
        bone.f_y = static_cast<float>(bone.i_y) / 4096.0f * 360.0f;
        bone.f_z = static_cast<float>(bone.i_z) / 4096.0f * 360.0f;
        frame.bones.push_back(bone);
    }
    return frame;
}

int DaFile::GetDynamicOffsetFromStream(){
    // The control bit is an unsigned flag: 1 for a 16 bit value, 0 for a 7 bit one.
    if (GetBitsU(1) == 1) return GetBitsS(16);
    return GetBitsS(7);
}

int DaFile::GetBitsU(int bits){
    // Bits are read most significant first.
    int value = 0;
    for (int i = 0; i < bits; i ++){
        const int byte = da_bytes_[bit_offset_ / 8];
        const int bit = (byte >> (7 - (bit_offset_ % 8))) & 1;
        value = (value << 1) | bit;
        bit_offset_ ++;
    }
    return value;
}

int DaFile::GetBitsS(int bits){
    return GetSignExtendedShort(GetBitsU(bits), bits);
}

int DaFile::GetCompressedDeltaFromStream(int lowered_precision_bits){
    unsigned int bits;
    int first_bit = GetBitsU(1);
    if (first_bit == 1){
        unsigned int key = GetBitsU(3);
        int temp, sign, orig_temp;
        unsigned int msb_mask;
        switch (key){
            case 0: // Return the smallest possible decrement delta (at given precision).
                return (-1 << lowered_precision_bits);
            case 1: case 2: case 3: case 4: case 5: case 6:
                bits = key;
                // Read a corresponding number of bits from the stream.
                temp = GetBitsS(bits);
                //  Invert the value of the last bit
                sign = (int) pow(2, bits - 1);
                // Set sign.
                if (temp < 0) temp -= sign;
                else temp += sign;
                // Convert to 12-bits value
                temp *= (int) pow(2, lowered_precision_bits);
                return temp;
            case 7:
                // Read an uncompressed value from the stream (at requested precision),
                // and return.
                temp = GetBitsS(12 - lowered_precision_bits);
                return (temp << lowered_precision_bits);
        }
    }
    // Default/error: return zero.
    return 0;
}
