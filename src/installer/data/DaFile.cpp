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
#include "data/DaFile.h"

DaFile::DaFile(File file){
    animations_.clear();
    file.SetOffset(0);
    Read(file);
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
        a.write(reinterpret_cast<const char*>(&version), 4);
        a.write(reinterpret_cast<const char*>(&animations_[anim].frame_count), 4);
        a.write(reinterpret_cast<const char*>(&animations_[anim].bone_count), 4);
        a.write(reinterpret_cast<const char*>(&rotation_order), 4);
        a.write(reinterpret_cast<const char*>(&zero), 4);
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
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            a.write(reinterpret_cast<const char*>(&zero), 4);
            /*
            a.write(
              reinterpret_cast<const char*>(&animations_[anim].frames[f].position_offset.x), 4
            );
            a.write(
              reinterpret_cast<const char*>(&animations_[anim].frames[f].position_offset.y), 4
            );
            a.write(
              reinterpret_cast<const char*>(&animations_[anim].frames[f].position_offset.z), 4
            );
            */
            for (int b = 0; b < animations_[anim].frames[f].rotations.size(); b ++){
                /*if (anim == 0 && b == 0)
                    std::cout << "    FRAME " << f << " BONE 0: "
                      << animations_[anim].frames[f].rotations[b].x << ", "
                      << animations_[anim].frames[f].rotations[b].y << ", "
                      << animations_[anim].frames[f].rotations[b].z << std::endl;*/
                a.write(
                  reinterpret_cast<const char*>(&animations_[anim].frames[f].rotations[b].x), 4
                );
                a.write(reinterpret_cast<const char*>(
                  &animations_[anim].frames[f].rotations[b].y), 4
                );
                a.write(reinterpret_cast<const char*>(
                  &animations_[anim].frames[f].rotations[b].z), 4
                );
            }
        }
        a.close();
        file_list.push_back(file_name);
    }
    return file_list;
}

void DaFile::Read(File file){
    int num_animations = file.readU32LE();
    for (int a = 0; a < num_animations; a ++){
        Animation animation;
        AnimationHeader header;
        header.bone_count = file.readU32LE();
        header.frame_count = file.readU32LE();
        animation.frame_count = header.frame_count;
        animation.bone_count = header.bone_count - 1;
        header.data_size = file.readU32LE();
        int next_offset = file.GetCurrentOffset() + header.data_size;
        if (header.data_size < 11){
            file.SetOffset(next_offset);
            continue;
        }
        u8* animation_data = new u8[header.data_size];
        for (int b = 0; b < header.data_size; b ++) animation_data[b] = file.readU8();

        // This will be the buffer to hold one frame. Only one frame will be stored at a time.
        Frame frame;
        Frame prev_frame;
        frame.SetBones(header.bone_count);
        int bits = 0;
        for (int f = 0; f < header.frame_count; f ++){
            FrameData frm;
            bits = LoadFrames(&frame, header.bone_count, bits, animation_data);
            // Set relative rotations to the previous frame

            // Reverse the Y offset (required).
            frame.position_offset.f_y = 0.0f - frame.position_offset.f_y;
            frm.position_offset.x = frame.position_offset.f_x;
            frm.position_offset.y = frame.position_offset.f_y;
            frm.position_offset.z = frame.position_offset.f_z;

            // Calculate final rotations
            for (int b = 0; b < header.bone_count - 1; b ++){
                frame.rotations[b].f_x
                  = static_cast<float>(frame.rotations[b].i_x) / 4096.0f * 360.0f;
                frame.rotations[b].f_y
                  = static_cast<float>(frame.rotations[b].i_y) / 4096.0f * 360.0f;
                frame.rotations[b].f_z
                  = static_cast<float>(frame.rotations[b].i_z) / 4096.0f * 360.0f;
                Rotation rotation;
                rotation.x = frame.rotations[b].f_x;
                rotation.y = frame.rotations[b].f_y;
                rotation.z = frame.rotations[b].f_z;
                /*std::cout << "PASS " << f << ", " << b << ". prev is null? " <<
                   (prev_frame.rotations == nullptr ? "YES" : "NO") << std::endl;
                if (f > 0){
                    std::cout << "    COMPENSATING START" << std::endl;
                    rotation.x += prev_frame.rotations[b].f_x;
                    rotation.y += prev_frame.rotations[b].f_y;
                    rotation.z += prev_frame.rotations[b].f_z;
                    std::cout << "    COMPENSATING END" << std::endl;
                }*/
                frm.rotations.push_back(rotation);
            }
            //prev_frame = frame;
            animation.frames.push_back(frm);

        }
        animations_.push_back(animation);
        delete [] animation_data;
        file.SetOffset(next_offset);
    }
}

u16 DaFile::GetValueFromStream(u8* bytes, u32 &stream_bit_offset){
    /*
     * TODO #2 I think this is doing some weird things
     */
    //std::cout << "GetValueFromStream: ";
    u16 value; // The return value;
    // The number of whole bytes already consumed in the stream.
    u32 stream_byte_offset = stream_bit_offset / 8;
    // The number of bits already consumed in the current stream byte.
    u32 stream_current_byte_bits_eaten = stream_bit_offset % 8;
    // The distance from next_stream_bytes' LSB to the key bit.
    u32 type_bit_shift = 7 - stream_current_byte_bits_eaten;
    // A copy of the next two bytes in the stream (from big-endian).
    u32 next_stream_bytes = bytes[stream_byte_offset] << 8 | bytes[stream_byte_offset + 1];
    // Test the first bit (the 'type' bit) to determine the size of the value.
    if (next_stream_bytes & (1 << (type_bit_shift + 8))){ // Sixteen-bit value:
        // Collect one more byte from the stream.
        next_stream_bytes = next_stream_bytes << 8 | bytes[stream_byte_offset + 2];
        // Shift the delta value into place.
        value = (next_stream_bytes << (stream_current_byte_bits_eaten + 1)) >> 8;
        // Update the stream offset.
        stream_bit_offset += 17;
        //std::cout << " 17 bits: " << value << std::endl;
    }
    else{ // Seven-bit value
        // Shift the delta value into place (taking care to preserve the sign).
        value = ((short)(next_stream_bytes << (stream_current_byte_bits_eaten + 1))) >> 9;
        // Update the stream offset.
        stream_bit_offset += 8;
        //std::cout << " 8 bits: " << value << std::endl;
    }

    // Return the value.
    return value;
}

u16 DaFile::ReadU16LE(u8* bytes, u32 &stream_bit_offset){
    u32 byte_offset = stream_bit_offset / 8;
    u16 data = (bytes + byte_offset)[0] | ((bytes + byte_offset)[1] << 8);
    stream_bit_offset += 16;
    return data;
}

int DaFile::GetBitsFromStream(u8* bytes, u32 &stream_bit_offset, int bits){
    int value = 0;
    unsigned int byte_offset = stream_bit_offset / 8;
    unsigned int bit_offset = stream_bit_offset % 8;
    //std::cout << "GetBitsFromStream: " << bits << " bits from " << stream_bit_offset
    //  << ": (" << (int)bytes[byte_offset] << " " << (int)bytes[byte_offset + 1]
    //  << "):\t ";

    for (int b = 0; b < bits; b ++){
        u8 byte = bytes[byte_offset + (bit_offset + b) / 8];
        int bit = (byte >> ((bit_offset + 7 - b) % 8) & 0x01);
        //std::cout << " ["<< (int)byte <<":" << (int)((bit_offset + 7 - b) % 8) << "]" << bit;
        value += bit * (pow(2, bits - 1 - b));
        stream_bit_offset += 1;
    }
    return value;
}

u16 DaFile::GetCompressedDeltaFromStream(
  u8* bytes, u32& stream_bit_offset, int lowered_precision_bits
){
    unsigned int bits;
    int first_bit = GetBitsFromStream(bytes, stream_bit_offset, 1);
    if (first_bit){
        unsigned int key = GetBitsFromStream(bytes, stream_bit_offset, 3) & 7;
        int temp;
        switch (key){
            case 0: // Return the smallest possible decrement delta (at given precision).
                //std::cout << "  GCDFS 0: " << (-1 << lowered_precision_bits) << std::endl;
                return (-1 << lowered_precision_bits);
            case 1: case 2: case 3: case 4: case 5: case 6:
                bits = key;
                // Read a corresponding number of bits from the stream.
                temp = GetBitsFromStream(bytes, stream_bit_offset, bits);
                //std::cout << "  GCDFS " << key << ": ";
                // Transform the value into the full seven-bit value, using the bit length
                // as part of the encoding scheme (see notes).
                if (temp < 0) temp -= 1 << (bits - 1);
                else temp += 1 << (bits - 1);
                //std::cout << temp << " -> ";
                // Adapt to the requested precision and return.
                //std::cout << (temp << lowered_precision_bits) << std::endl;
                return (temp << lowered_precision_bits);
            case 7:
                // Read an uncompressed value from the stream (at requested precision),
                // and return.
                temp = GetBitsFromStream(
                  bytes, stream_bit_offset, 12 - lowered_precision_bits
                );
                //std::cout << "  GCDFS 7: " << temp << " -> " << (temp << lowered_precision_bits)
                //  << std::endl;
                return (temp << lowered_precision_bits);
            //default:
                //std::cout << "  GCDFS " << key << " UNKNOWN " << std::endl;
        }
    }
    // Default/error: return zero.
    //std::cout << "  GCDFS 0 --"<< std::endl;
    return 0;
}

u32 DaFile::LoadFrames(Frame* frame, int bones, int bit_start, u8* animation_buffer){
    // Get backups of the information we need.
    u32 orig_bit_start = bit_start;
    int orig_bones = bones;
    u8* orig_animation_buffer = animation_buffer;
    AnimationMiniHeader* mini_header = (AnimationMiniHeader*) animation_buffer;
    u16 data_size = mini_header->data_size;
    u8 key = mini_header->key;

    // Skip the first 5 bytes because they are part of the frame header.
    orig_animation_buffer += sizeof(AnimationMiniHeader);

    if (bit_start == 0){ // First frame?

        // Skip the next 6 bytes. Why? I don't know what they are.
        orig_bit_start += 8 * 8;

        // The first frame is uncompressed and each value is the actual rotation.
        //std::cout << "FIRSR TRAME READING AT " << orig_bit_start;
        frame->position_offset.s_x = GetBitsFromStream(orig_animation_buffer, orig_bit_start, 16);
          //= ReadU16LE(orig_animation_buffer, orig_bit_start);
        //std::cout << ", " << orig_bit_start;
        frame->position_offset.s_y = GetBitsFromStream(orig_animation_buffer, orig_bit_start, 16);
          //= ReadU16LE(orig_animation_buffer, orig_bit_start);
        //std::cout << ", " << orig_bit_start;
        frame->position_offset.s_z = GetBitsFromStream(orig_animation_buffer, orig_bit_start, 16);
          //= ReadU16LE(orig_animation_buffer, orig_bit_start);
        //std::cout << " END AT " << orig_bit_start << "\n";

        // This function will set the FLOAT values for the positions.
        // Any scaling that needs to be done would be done here.
        frame->position_offset.f_x = static_cast<float>(frame->position_offset.s_x);
        frame->position_offset.f_y = static_cast<float>(frame->position_offset.s_y);
        frame->position_offset.f_z = static_cast<float>(frame->position_offset.s_z);

        // TODO: For first bone, go back 2*3 bytes, and reread them, but this time as
        // little endian for the rotations.
        orig_bit_start -= (2 * 3 * 8);

        frame->rotations[0].s_x = ReadU16LE(orig_animation_buffer, orig_bit_start);
        frame->rotations[0].s_y = ReadU16LE(orig_animation_buffer, orig_bit_start);
        frame->rotations[0].s_z = ReadU16LE(orig_animation_buffer, orig_bit_start);
        frame->rotations[0].i_x = (frame->rotations[0].s_x < 0) ?
          frame->rotations[0].s_x + 0x1000 : frame->rotations[0].s_x;
        frame->rotations[0].i_y = (frame->rotations[0].s_y < 0) ?
          frame->rotations[0].s_y + 0x1000 : frame->rotations[0].s_y;
        frame->rotations[0].i_z = (frame->rotations[0].s_z < 0) ?
          frame->rotations[0].s_z + 0x1000 : frame->rotations[0].s_z;

        //std::cout << "FIRST FRAME FIRST BONE: "
        //  << frame->rotations[0].i_x << " (" <<  (int) frame->rotations[0].s_x << "), "
        //  << frame->rotations[0].i_y << " (" <<  (int) frame->rotations[0].s_y << "), "
        //  << frame->rotations[0].i_z << " (" <<  (int) frame->rotations[0].s_z << ")\n";

        for (int i = 1; i < orig_bones - 1; i ++){
            // Now get each bone rotation (the first bone is actually the root, not part of
            // the skeleton). During the first frame, the rotations are always (12 - key).
            // bits. The values are shifted by key to align it to 12 bits.
            frame->rotations[i].s_x
              = GetBitsFromStream(orig_animation_buffer, orig_bit_start, 12 - key) << key;
            frame->rotations[i].s_y
              = GetBitsFromStream(orig_animation_buffer, orig_bit_start, 12 - key) << key;
            frame->rotations[i].s_z
              = GetBitsFromStream(orig_animation_buffer, orig_bit_start, 12 - key) << key;

            // Store the INT version as the absolute value of the SHORT version.
            frame->rotations[i].i_x = (frame->rotations[i].s_x < 0) ?
              frame->rotations[i].s_x + 0x1000 : frame->rotations[i].s_x;
            frame->rotations[i].i_y = (frame->rotations[i].s_y < 0) ?
              frame->rotations[i].s_y + 0x1000 : frame->rotations[i].s_y;
            frame->rotations[i].i_z = (frame->rotations[i].s_z < 0) ?
              frame->rotations[i].s_z + 0x1000 : frame->rotations[i].s_z;

            /*if (i < 3){
                std::cout << "FIRST FRAME BONE " << i << ": "
                  << frame->rotations[i].i_x << " (" <<  (int) frame->rotations[i].s_x << "), "
                  << frame->rotations[i].i_y << " (" <<  (int) frame->rotations[i].s_y << "), "
                  << frame->rotations[i].i_z << " (" <<  (int) frame->rotations[i].s_z << ")\n";
            }*/
        }
    }
    else { // All other frames.
        u16 x, y, z; // Get the positional offsets.
        x = GetValueFromStream(orig_animation_buffer, orig_bit_start);
        y = GetValueFromStream(orig_animation_buffer, orig_bit_start);
        z = GetValueFromStream(orig_animation_buffer, orig_bit_start);

        // When reaching this area of the function, frame will have the previous frame
        // still stored in it.  Just add the offsets.
        frame->position_offset.s_x += x;
        frame->position_offset.s_y += y;
        frame->position_offset.s_z += z;
        frame->position_offset.f_x = static_cast<float>(frame->position_offset.s_x);
        frame->position_offset.f_y = static_cast<float>(frame->position_offset.s_y);
        frame->position_offset.f_z = static_cast<float>(frame->position_offset.s_z);
        for (int i = 0; i < orig_bones; i ++){

            // The same applies here.  Add the offsets and convert to INT form, adding
            // 0x1000 if it is less than 0.
            // When Final Fantasy VII loads these animations, it is possible for the value
            // to sneak up above the 4095 boundary through a series of positive offsets.

            // TODO: This is NOT a fix. third parameter should be key, not 12 - key.
            // The reason this looks better is because the rotations are now made so small they are
            // imperceptible
            x = GetCompressedDeltaFromStream(orig_animation_buffer, orig_bit_start, 12 - key);
            y = GetCompressedDeltaFromStream(orig_animation_buffer, orig_bit_start, 12 - key);
            z = GetCompressedDeltaFromStream(orig_animation_buffer, orig_bit_start, 12 - key);

            frame->rotations[i].s_x += x;
            frame->rotations[i].s_y += y;
            frame->rotations[i].s_z += z;
            //if (i == 0)
            //    std::cout << "    ADDED ROTATION: " << frame->rotations[i].s_x << ", "
            //      << frame->rotations[i].s_y << ", " << frame->rotations[i].s_z << std::endl;
            frame->rotations[i].i_x = (frame->rotations[i].s_x < 0) ?
              frame->rotations[i].s_x + 0x1000 : frame->rotations[i].s_x;
            frame->rotations[i].i_y = (frame->rotations[i].s_y < 0) ?
              frame->rotations[i].s_y + 0x1000 : frame->rotations[i].s_y;
            frame->rotations[i].i_z = (frame->rotations[i].s_z < 0) ?
              frame->rotations[i].s_z + 0x1000 : frame->rotations[i].s_z;
            //if (i == 0)
            //    std::cout << "    PROCESSED ROTATION: " << frame->rotations[i].i_x << ", "
            //      << frame->rotations[i].i_y << ", " << frame->rotations[i].i_z << std::endl;
        }
    }

    // If not as many bits as there are in the frame are read, return the location where
    // the bits should start for the next frame. Otherwise, return 0.
    if (orig_bit_start / 8 < data_size) return orig_bit_start;
    return 0;
}
