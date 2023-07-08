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
         * A three-coordinate value.
         *
         * Can be used to store bone rotations, offsets, translations... Each coordinate has short,
         * integer and float formats.
         */
        struct TriValue{

            /**
             * X value, short format.
             */
            char16_t s_x;

            /**
             * Y value, short format.
             */
            char16_t s_y;

            /**
             * Z value, short format
             */
            char16_t s_z;

            /**
             * X value, integer format.
             */
            int i_x;

            /**
             * Y value, integer format.
             */
            int i_y;

            /**
             * Z value, integer format.
             */
            int i_z;

            /**
             * X value, float_format.
             */
            float f_x;

            /**
             * Y value, float format.
             */
            float f_y;

            /**
             * Z value, float format.
             */
            float f_z;
        };

        /**
         * Information for a frame.
         */
        struct Frame{

            /**
             * Root offset.
             */
            TriValue offset;

            /**
             * Root rotation.
             */
            TriValue rotation;

            /**
             * Each of the bone rotations.
             */
            std::vector<TriValue> bones;
        };

        struct Animation{

            /**
             * Number of bones in the animation.
             */
            u32 bone_count;

            /**
             * Number of frames in the animation.
             */
            u32 frame_count;

            /**
             * Size of the animation data.
             */
            u32 size;

            /**
             * Key for decoding values in an animation.
             */
            u8 key;

            /**
             * List of frames in the animation.
             */
            std::vector<Frame> frames;
        };

        /**
         * Extends the sign for an integer value for a specified bit length.
         *
         * @param[in] val The value to extend.
         * @param[in] len Number of bytes  to extend the value to.
         */
        static int ExtendSignInteger(int val, int len);

        /**
         * Extends the sign for a short value for a specified bit length.
         *
         * @param[in] val The value to extend.
         * @param[in] len Number of bytes  to extend the value to.
         */
        static int GetSignExtendedShort(int val, int len);


        /**
         * Reads the Da file and extracts all the data.
         *
         */
        void Read();

        Animation ReadAnimation();

        Frame ReadFirstFrame(u32 bones, u8 key);

        Frame ReadFrame(u32 bones, u8 key, Frame prev);

        /**
         * Retrieves a value of dynamic size from the byte stream.
         *
         * The first read bit will determine the length of the read data. If it's 1, 7 more bits of
         * data will be read and returned. It it isn't, 16 more bits of data will be read and
         * returned. In total, either 17 or 8 bits of data will be read.
         *
         * @return The read value.
         */
        u16 GetDynamicOffsetFromStream();

        /**
         * Reads 2 bytes from the stream as a little endian value.
         *
         * @param[in] bytes The byte stream to read from.
         * @param[in,out] stream_bit_offset The bit at which to start reading. It will be advanced
         * by 16.
         * @return The read value.
         */
        u16 ReadU16LE();

        /**
         * Read an arbitrary number of bits from the da contents as unsigned.
         *
         * @param[in] bits Number of bits.
         * @return Unsigned value.
         */
        int GetBitsU(int bits);

        /**
         * Read an arbitrary number of bits from the da contents as signed.
         *
         * @param[in] bits Number of bits.
         * @return Signed value.
         */
        int GetBitsS(int bits);

        /**
         * Reads an arbitrary length of bits from the stream.
         *
         * @param[in] bits Number of bits to read.
         * @return The read value.
         */
        int GetBitsFromStream(int bits);

        /**
         * Decodes a delta rotation read from the stream.
         *
         * @param lowered_precision_bits How many bits to sift the read value. The animation key.
         * @return The decoded value.
         */
        int GetCompressedDeltaFromStream(int lowered_precision_bits);

        /**
         * Loads the frames in an animation.
         *
         * @param[out] frame Read frame data will be set here.
         * @param[in] bones The number of bones in the animation.
         * @param[in] bit_start Bit to start reading from animation_buffer at.
         * @param[in] animation_buffer The animation data.
         */
        u32 LoadFrames(Frame* frame, int bones, int bit_start, u8* animation_buffer);

        /**
         * The da file.
         */
        File da_file_;

        /**
         * Contents of the da file as a byte array.
         */
        u8* da_bytes_;

        /**
         * List of animations.
         */
        std::vector<Animation> animations_;

        /**
         * Number of bits already read from the da file.
         */
        u32 bit_offset_;

};
