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

#pragma once

#include <vector>
#include "common/File.h"
#include "common/TypeDefine.h"

/**
 * Represents a compiled animations file.
 *
 * Da files are bundled into battle.lgp. They contain information about a skeleton animation. They
 * are heavily compressed, and involve a lot of bit reading to decompile.
 */
class DaFile{

    public:

        /**
         * Constructor.
         *
         * Reads the file.
         *
         * @param[in, out] file The file to read from. The file contents will not be altered, but
         * it's offset will be changed while reading it.
         */
        DaFile(File file);

        /**
         * Generates .a files from the da file.
         *
         * A file will be generated for each animation in the da file. See {@see VGearsAFile.h} for
         * more info about .a files. Files will be created in the specified path, and will be named
         * like the model id, followed by an underscore, followed a two-digit animation index
         * (starting from 0) followed by the ".a extension". For example, if path is
         * "/home/user/.v-gears", the model id is "ac", and the da file contains three animations,
         * the following files will be generated: "/home/user/.v-gears/ac_00.a",
         * "/home/user/.v-gears/ac_01.a" and "/home/user/.v-gears/ac_02.a".
         *
         * @param[in] model_id ID of the model the animation belongs to, usually a two letter
         * character. Used to generate the file name.
         * @param[in] path Path to the directory where the files will be saved.
         * @return A list of the generated files. They include the path and the extension.
         */
        std::vector<std::string> GenerateAFiles(std::string model_id, std::string path);

    private:

        /**
         * A bone rotation, with all it's values calculated.
         */
        struct Rotation{

            /**
             * Rotation in the X axis, in degrees.
             */
            float x;

            /**
             * Rotation in the Y axis, in degrees.
             */
            float y;

            /**
             * Rotation in the Z axis, in degrees.
             */
            float z;
        };

        /**
         * Processed frame data, to be written to .a files.
         */
        struct FrameData{

            /**
             * Position offset for the frame.
             */
            Rotation position_offset;

            /**
             * List of bone rotations, one per bone.
             */
            std::vector<Rotation> rotations;
        };

        /**
         * Processed animation data, to be written to.a files.
         */
        struct Animation{

            /**
             * Number of frames in the animation.
             */
            unsigned int frame_count;

            /**
             * Number of bones involved in the animation.
             */
            unsigned int bone_count;

            /**
             * Each frame in the animation.
             */
            std::vector<FrameData> frames;
        };

        /**
         * Header for animations. 12 bytes, at the start of each animation.
         */
        struct AnimationHeader {

            /**
             * Number of bones + 1 in the animation.
             *
             * It's not always reliable, the number of bones in the skeleton is the real one. If
             * dealing with a weapon animation, the value will always be 1.
             */
            u32 bone_count;

            /**
             * Number of frames in the animation.
             */
            u32 frame_count;

            /**
             * Size, in bytes, of the animation data.
             */
            u32 data_size;
        };

        /**
         * Mini header for each animation. 5 bytes, immediately after each main header.
         */
        struct AnimationMiniHeader {

            /**
             * Number of frames in the animation.
             *
             * It may not match {@see AnimationHeader.frame_count}.
             */
            u16 frame_count;

            /**
             * Size of the animation data.
             *
             * It should be 6 bytes less than {@see AnimationHeader.data_size}.
             */
            u16 data_size;

            /**
             * Key used for decoding bone data.
             *
             * It indicates how many bits are to be read to get a bone rotation. Not to be used in
             * the first frame, which is not compressed.
             *
             * 0: 12 bits per bone rotation, 36 bits per bone.
             * 2: 10 bits per bone rotation, 30 bits per bone.
             * 4: 8 bits per bone rotation, 30 bits per bone.
             */
            u8 key;
        };

        /**
         * Bone rotation.
         */
        struct BoneRotation {

            /**
             * X axis rotation, as read from the da file.
             */
            u16 s_x;

            /**
             * Y axis rotation, as read from the da file.
             */
            u16 s_y;

            /**
             * Z axis rotation, as read from the da file.
             */
            u16 s_z;     // Signed short versions.   0x00

            /**
             * X axis rotation, converted to integer. Rotation between 0 and 4096.
             */
            int i_x;

            /**
             * Y axis rotation, converted to integer. Rotation between 0 and 4096.
             */
            int i_y;

            /**
             * Z axis rotation, converted to integer. Rotation between 0 and 4096.
             */
            int i_z;

            /**
             * X axis rotation, in degrees.
             */
            float f_x;

            /**
             * Y axis rotation, in degrees.
             */
            float f_y;

            /**
             * Z axis rotation, in degrees.
             */
            float f_z;
        };

        /**
         * A frame of an animation.
         *
         * Stores all bone rotations for a frame.
         */
        struct Frame {

            /**
             * Number of bones in the animation frame.
             */
            u32 bone_count;

            /**
             * Position offset.
             *
             * @todo Verify if it's the previuos frame?
             */
            BoneRotation position_offset;

            /**
             * Bone rotations for the frame.
             */
            BoneRotation* rotations;

            /**
             * Constructor. Initializes data.
             */
            Frame() {
                bone_count = 0;
                rotations = NULL;
            }

            /**
             * Destructor.
             */
            ~Frame() {
                bone_count = 0;
                delete [] rotations;
                rotations = NULL;
            }

            /**
             * Sets the number of bones and initializes data arrays.
             */
            void SetBones(u32 bones ) {
                // Delete the old.
                bone_count = 0;
                delete [] rotations;
                // Create the new.
                rotations = new BoneRotation[bones];
                if (rotations != NULL) bone_count = bones;
            }
        };

        /**
         * Reads the Da file and extracts all the data.
         *
         * @param[in, out] file The file to read from. The file contents will not be altered, but
         * it's offset will be changed while reading it.
         */
        void Read(File file);

        /**
         * Retrieves a value of dynamic size from the byte stream.
         *
         * The first read bit will determine the length of the read data. If it's 1, 7 more bits of
         * data will be read and returned. It it isn't, 16 more bits of data will be read and
         * returned. In total, either 17 or 8 bits of data will be read.
         *
         * @param[in] bytes The byte stream to read from.
         * @param[in,out] stream_bit_offset The bit at which to start reading. For each bit read,
         * it will be advanced by one.
         * @return The read value.
         */
        u16 GetValueFromStream(u8* bytes, u32 &stream_bit_offset);

        /**
         * Reads 2 bytes from the stream as a little endian value.
         *
         * @param[in] bytes The byte stream to read from.
         * @param[in,out] stream_bit_offset The bit at which to start reading. It will be advanced
         * by 16.
         * @return The read value.
         */
        u16 ReadU16LE(u8* bytes, u32 &stream_bit_offset);

        /**
         * Reads an arbitrary length of bits from the stream.
         *
         * @param[in] bytes The byte stream to read from.
         * @param[in,out] stream_bit_offset The bit at which to start reading. It will be advanced
         * by one per read bit.
         * @return The read value.
         */
        int GetBitsFromStream(u8* bytes, u32 &stream_bit_offset, int bits);

        /**
         * Decodes a delta rotation read from the stream.
         *
         * @param[in] bytes The byte stream to read from.
         * @param[in,out] stream_bit_offset The bit at which to start reading. It will be advanced
         * by one per read bit.
         * @param How many bits to sift the read value.
         * @return The decoded value.
         */
        u16 GetCompressedDeltaFromStream(
          u8* bytes, u32& stream_bit_offset, int lowered_precision_bits
        );

        /**
         * Loads the frames in an animation.
         *
         * @param[out] Read frame data will be set here.
         * @param[in] bones The number of bones in the animation.
         * @param[in] bit_start Bit to start reading from animation_buffer at.
         * @param[in] animation_buffer The animation data.
         */
        u32 LoadFrames(Frame* frame, int bones, int bit_start, u8* animation_buffer);

        /**
         * List of animations.
         */
        std::vector<Animation> animations_;

};
