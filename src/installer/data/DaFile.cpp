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

DaFile::DaFile(File file): da_file_(file){

    //std::cout << "[DAFILE] " << file.GetFileName() << std::endl;
    animations_.clear();

    // Save file contents as a byte array.
    file.SetOffset(0);
    da_bytes_ = new u8[file.GetFileSize()];
    file.GetFileBuffer(da_bytes_, 0, file.GetFileSize());
    bit_offset_ = 0;
    file.SetOffset(0);
    Read();
}

int DaFile::GetBitsU(int bits){
    int value = 0;
    int base_byte, bi, res, num_bytes, unaligned_by_bits;
    int first_aligned_byte, last_aligned_byte, end_bits;
    bool is_aligned, clean_end;
    if (bits < 1) return value;
    base_byte = bit_offset_ / 8;
    unaligned_by_bits = bit_offset_ % 8;

    if (unaligned_by_bits + bits > 8){
        is_aligned = (unaligned_by_bits == 0);
        end_bits = (bit_offset_ + bits) % 8;
        clean_end = (end_bits == 0);
        num_bytes =
          (bits - (is_aligned ? 0 : 8 - unaligned_by_bits) - (clean_end ? 0 : end_bits)) / 8
          + (is_aligned ? 0 : 1) + (clean_end ? 0 : 1);
        last_aligned_byte = num_bytes - (clean_end ? 0 : 1) - 1;
        first_aligned_byte = 0;

        res = 0;
        //  Unaligned prefix
        //  Stored at the begining of the byte
        if (!is_aligned){
            res = da_bytes_[base_byte];
            res &= (int) (pow(2, (8 - unaligned_by_bits)) - 1);
            first_aligned_byte = 1;
        }

        //  Aligned bytes
        for (bi = first_aligned_byte; bi <= last_aligned_byte; bi ++){
            res *= 256;
            res |= da_bytes_[base_byte + bi];
        }

        //  Sufix
        //  Stored at the end of the byte
        if (!clean_end){
            res *= (int) pow(2, end_bits);
            res |= (
              (da_bytes_[base_byte + last_aligned_byte + 1]) / (int) (pow(2, 8 - end_bits))
              & (int) (pow(2, end_bits) - 1)
            );
        }
    }
    else{
        res = da_bytes_[base_byte];
        res /= (int) pow(2, 8 - (unaligned_by_bits + bits));
        res &= (int) (pow(2, bits) - 1);
    }

    value = (short)res;

    bit_offset_ += bits;

    return value;
}

int DaFile::ExtendSignInteger(int val, int len){
    int result, aux;
    if ((val & (int) pow(2, (len - 1))) != 0){
        aux = (int) pow(2, 16) - 1;
        aux ^= (int) (pow(2, len) - 1);
        aux |= val;
        result = aux;
    }
    else result = val;
    return result;
}

int DaFile::GetSignExtendedShort(int val, int len){
    int result = 0;
    if (len > 0){
        if (len < 16) result = ExtendSignInteger(val, len);
        else result = val;
    }
    return result;
}

int DaFile::GetBitsS(int bits){
    int tmp = GetBitsU(bits);
    return GetSignExtendedShort(tmp, bits);
}



void DaFile::Read(){
    int num_animations = da_file_.readU32LE();
    bit_offset_ += 32;
    for (int a = 0; a < num_animations; a ++){
        int start_offset = bit_offset_ / 8; // In bytes.
        //std::cout << "    ANIMATION " << a << "/" << num_animations
        //  << " START: " << start_offset;
        Animation anim = ReadAnimation();
        if (anim.frame_count > 0) animations_.push_back(anim);
        // Manually go to the next animation
        bit_offset_ = (start_offset + anim.size + 12) * 8; // + 12 is for skipping the naim header.
        da_file_.SetOffset(start_offset + anim.size + 12);

    }
}

DaFile::Animation DaFile::ReadAnimation(){
    Animation anim;
    // Read the animation headers.
    anim.bone_count = da_file_.readU32LE();
    bit_offset_ += 32;
    anim.frame_count = da_file_.readU32LE();
    bit_offset_ += 32;
    anim.size = da_file_.readU32LE();
    //std::cout << " Bones: " << anim.bone_count << " Frames: " << anim.bone_count << " Size: "
    //  << anim.size;
    // Skip small (< 11 bytes) animations.
    if (anim.size < 11){
        //std::cout << " SKIP" << std::endl;
        //da_file_.SetOffset(da_file_.GetCurrentOffset() + anim.size);
        //bit_offset_ += anim.size;
        anim.frame_count = 0;
        return anim;
    }
    //std::cout << " READ" << std::endl;
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

    // Read root offset. In the first frame, always 16 bytes each.
    frame.offset.s_x = GetBitsS(16);
    frame.offset.s_y = GetBitsS(16);
    frame.offset.s_z = GetBitsS(16);

    // Turn offset to float. Any scaling that needs to be done would be done here.
    frame.offset.f_x = 0;//static_cast<float>(frame.offset.s_x);
    frame.offset.f_y = 0;//static_cast<float>(frame.offset.s_y) * -1;
    frame.offset.f_z = 0;//static_cast<float>(frame.offset.s_z);

    // Read root rotation
    frame.rotation.s_x = 0;//GetBitsFromStream(12 - key) << key;
    frame.rotation.s_y = 0;//GetBitsFromStream(12 - key) << key;
    frame.rotation.s_z = 0;//GetBitsFromStream(12 - key) << key;

    // convert rotation to integer.
    frame.rotation.i_x = frame.rotation.s_x < 0 ? frame.rotation.s_x + 0x1000 : frame.rotation.s_x;
    frame.rotation.i_y = frame.rotation.s_y < 0 ? frame.rotation.s_y + 0x1000 : frame.rotation.s_y;
    frame.rotation.i_z = frame.rotation.s_z < 0 ? frame.rotation.s_z + 0x1000 : frame.rotation.s_z;
    frame.rotation.f_x = static_cast<float>(frame.rotation.i_x) / (12 - key) * 360.0f;
    frame.rotation.f_y = static_cast<float>(frame.rotation.i_y) / (12 - key) * 360.0f;
    frame.rotation.f_z = static_cast<float>(frame.rotation.i_z) / (12 - key) * 360.0f;

    for (int i = 0; i < bones; i ++){
        TriValue bone;
        // Now get each bone rotation (the first bone is actually the root, not part of the
        // skeleton). During the first frame, the rotations are always (12 - key) bits. The values
        // are shifted by key to align it to 12 bits.
        bone.s_x = GetBitsFromStream(12 - key) << key;
        bone.s_y = GetBitsFromStream(12 - key) << key;
        bone.s_z = GetBitsFromStream(12 - key) << key;

        // Store the INT version as the absolute value of the SHORT version.
        bone.i_x = (bone.s_x < 0) ? bone.s_x + 0x1000 : bone.s_x;
        bone.i_y = (bone.s_y < 0) ? bone.s_y + 0x1000 : bone.s_y;
        bone.i_z = (bone.s_z < 0) ? bone.s_z + 0x1000 : bone.s_z;
        bone.f_x = static_cast<float>(bone.i_x) / 4096.0f * 360.0f;
        bone.f_y = static_cast<float>(bone.i_y) / 4096.0f * 360.0f;
        bone.f_z = static_cast<float>(bone.i_z) / 4096.0f * 360.0f;

        frame.bones.push_back(bone);
    }
    return frame;
}

DaFile::Frame DaFile::ReadFrame(u32 bones, u8 key, Frame prev){
    Frame frame;

    // Get dynamic root offset and add the previous frame values.
    frame.offset.s_x = GetDynamicOffsetFromStream() + prev.offset.s_x;
    frame.offset.s_y = GetDynamicOffsetFromStream() + prev.offset.s_y;
    frame.offset.s_z = GetDynamicOffsetFromStream() + prev.offset.s_z;
    frame.offset.f_x = 0.0f;//static_cast<float>(frame.offset.s_x);
    frame.offset.f_y = 0.0f;//static_cast<float>(frame.offset.s_y) * -1;
    frame.offset.f_z = 0.0f;//static_cast<float>(frame.offset.s_z);

    frame.rotation.s_x = 0;//GetCompressedDeltaFromStream(key) + prev.rotation.s_x;
    frame.rotation.s_y = 0;//GetCompressedDeltaFromStream(key) + prev.rotation.s_y;
    frame.rotation.s_z = 0;//GetCompressedDeltaFromStream(key) + prev.rotation.s_z;
    frame.rotation.i_x = 0;//frame.rotation.s_x < 0 ? frame.rotation.s_x + 0x1000 : frame.rotation.s_x;
    frame.rotation.i_y = 0;//frame.rotation.s_y < 0 ? frame.rotation.s_y + 0x1000 : frame.rotation.s_y;
    frame.rotation.i_z = 0;//frame.rotation.s_z < 0 ? frame.rotation.s_z + 0x1000 : frame.rotation.s_z;
    frame.rotation.f_x = static_cast<float>(frame.rotation.i_x) / pow(2, 12 - key) * 360.0f;
    frame.rotation.f_y = static_cast<float>(frame.rotation.i_y) / pow(2, 12 - key) * 360.0f;
    frame.rotation.f_z = static_cast<float>(frame.rotation.i_z) / pow(2, 12 - key) * 360.0f;

    for (int i = 0; i < bones; i ++){
        TriValue bone;
        // The same applies here.  Add the offsets and convert to INT form, adding
        // 0x1000 if it is less than 0.
        // When Final Fantasy VII loads these animations, it is possible for the value
        // to sneak up above the 4095 boundary through a series of positive offsets.
        bone.s_x = GetCompressedDeltaFromStream(key) + prev.bones[i].s_x;
        bone.s_y = GetCompressedDeltaFromStream(key) + prev.bones[i].s_y;
        bone.s_z = GetCompressedDeltaFromStream(key) + prev.bones[i].s_z;
        bone.i_x = (bone.s_x < 0) ? bone.s_x + 0x1000 : bone.s_x;
        bone.i_y = (bone.s_y < 0) ? bone.s_y + 0x1000 : bone.s_y;
        bone.i_z = (bone.s_z < 0) ? bone.s_z + 0x1000 : bone.s_z;
        bone.f_x = static_cast<float>(bone.i_x) / pow(2, 12 - key) * 360.0f;
        bone.f_y = static_cast<float>(bone.i_y) / pow(2, 12 - key) * 360.0f;
        bone.f_z = static_cast<float>(bone.i_z) / pow(2, 12 - key) * 360.0f;
        frame.bones.push_back(bone);
    }
    return frame;
}

std::vector<std::string> DaFile::GenerateAFiles(std::string model_id, std::string path){
    std::vector<std::string> file_list;
    u32 zero = 0x00000000;
    u32 version = 0x00000001;
    u32 rotation_order = 0x00020001;
    for (int anim = 0; anim < animations_.size(); anim ++){
        std::string file_index_name = std::to_string(anim);
        while (file_index_name.size() < 2) file_index_name = "0" + file_index_name;
        std::string file_name = path + model_id + "_" + file_index_name + ".a";
        std::ofstream a(file_name, std::ios::out | std::ios::binary);
        int bone_count = animations_[anim].bone_count;
        a.write(reinterpret_cast<const char*>(&version), 4);
        a.write(reinterpret_cast<const char*>(&animations_[anim].frame_count), 4);
        a.write(reinterpret_cast<const char*>(&bone_count), 4);
        a.write(reinterpret_cast<const char*>(&rotation_order), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4); // Runtime data, skip
        a.write(reinterpret_cast<const char*>(&zero), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
        for (int f = 0; f < animations_[anim].frames.size(); f ++){
            /*
             * TODO #1
             * .a files have a 36 byte header header, and after that, each animation frame. Each
             * animation frame has three floats (of 4 bytes) for root rotation, three for root
             * position, and after them, the bone rotation data. In most .a files (in char.lgp, for
             * example), root rotation is all zeros, but the root location isn't. I don't know how
             * to get those values, so I'm setting them all to zero. With this, the first frame
             * works, but the others are all meshed up. Setting root location values give very
             * strange results, like the model being way offset-ed from it's position. It's likely
             * that Frame.position_offset has something to do with this.
             */
            a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].rotation.i_x), 4);
            a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].rotation.i_y), 4);
            a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].rotation.i_z), 4);

            a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].offset.f_x), 4);
            a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].offset.f_y), 4);
            a.write(reinterpret_cast<const char*>(&animations_[anim].frames[f].offset.f_z), 4);

            for (int b = 1; b < animations_[anim].frames[f].bones.size() + 1; b ++){
                a.write(
                  reinterpret_cast<const char*>(&animations_[anim].frames[f].bones[b].f_x), 4
                );
                a.write(
                  reinterpret_cast<const char*>(&animations_[anim].frames[f].bones[b].f_y), 4
                );
                a.write(
                  reinterpret_cast<const char*>(&animations_[anim].frames[f].bones[b].f_z), 4
                );
            }
        }
        a.close();
        file_list.push_back(file_name);
    }
    return file_list;
}

u16 DaFile::GetDynamicOffsetFromStream(){
    u16 value;
    int control_bit = GetBitsS(1);
    if (control_bit == 1) value = GetBitsS(16);
    else value = GetBitsS(7);
    return value;
}

u16 DaFile::ReadU16LE(){
    u32 byte_offset = bit_offset_ / 8;
    u16 data = (da_bytes_ + byte_offset)[0] | ((da_bytes_ + byte_offset)[1] << 8);
    bit_offset_ += 16;
    return data;
}

int DaFile::GetBitsFromStream(int bits){
    int value = 0;
    unsigned int byte_offset = bit_offset_ / 8;
    unsigned int bit_offset = bit_offset_ % 8;
    for (int b = 0; b < bits; b ++){
        u8 byte = da_bytes_[byte_offset + (bit_offset + b) / 8];
        int bit = (byte >> ((bit_offset + 7 - b) % 8) & 0x01);
        value += bit * (pow(2, bits - 1 - b));
        bit_offset_ += 1;
    }
    return value;
}

int DaFile::GetCompressedDeltaFromStream(int lowered_precision_bits){
    unsigned int bits;
    int first_bit = GetBitsU(1);
    if (first_bit == 1){
        unsigned int key = GetBitsU(3);// & 7;
        int temp, sign, orig_temp;
        unsigned int msb_mask;
        switch (key){
            case 0: // Return the smallest possible decrement delta (at given precision).
                return (-1 << lowered_precision_bits);
            case 1: case 2: case 3: case 4: case 5: case 6:
                bits = key;
                // Read a corresponding number of bits from the stream.
                temp = (int16_t) GetBitsS(bits);
                //  Invert the value of the last bit
                sign = (int) pow(2, bits - 1);

                if (temp < 0) temp -= sign;
                else temp += sign;
                // Convert to 12-bits value
                temp *= (int) pow(2, lowered_precision_bits);
                return temp;
            case 7:
                // Read an uncompressed value from the stream (at requested precision),
                // and return.
                temp = (int16_t) GetBitsS(12 - lowered_precision_bits);
                return (temp << lowered_precision_bits);
        }
    }
    // Default/error: return zero.
    return 0;
}
