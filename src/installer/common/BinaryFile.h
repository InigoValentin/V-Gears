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

#include <OgreString.h>
#include "common/TypeDefine.h"
#include "common/File.h"

/**
 * Represents a file.
 *
 * This class is for files that need a lot of bit-reading for parsing. It's slower than it's
 * counterpart {@see File}, which is better used in the rest of cases.
 */
class BinaryFile{

    public:

        /**
         * Opens a file.
         *
         * @param[in] file Pointer to the file.
         */
        BinaryFile(const File* file);

        /**
         * Destructor.
         */
        virtual ~BinaryFile();

        /**
         * Retrieves the file name.
         *
         * @return The file name, without path.
         */
        const Ogre::String& GetFileName() const;

        /**
         * Retrieves the file size.
         *
         * @return File size, in bytes.
         */
        u32 GetFileSize() const;

        /**
         * Loads a buffer with the file data.
         *
         * @param[out] buffer Pointer to the buffer to load.
         * @param[in] start Offset to the data to load.
         * @param[in] length Length of the data to load.
         */
        void GetFileBuffer(u8* buffer, const u32 &start, const u32 &length) const;

        /**
         * Retrieves bits from the file.
         *
         * @param[in] offset_bits The offset, in bytes, to the bits to read.
         * @param[in] offset_bits The offset, in bits, of the byte at {@see offset_byte} to read
         * from.
         * @param[in] bits Number of bits to read.
         * @return Decimal valud of the read bits.
         */
        int GetBits(u32 offset_bytes, u8 offset_bits, u8 bits) const;

        /**
         * Retrieves bits from the file at the current offset.
         *
         * @param[in] bits Number of bits to read.
         * @return Decimal valud of the read bits.
         */
        int ReadBits(u8 bits);

        /**
         * Retrieves a byte.
         *
         * @param[in] offset The offset of the byte.
         * @return The byte at the specified offset.
         */
        u8 GetU8(u32 offset) const;

        /**
         * Retrieves two bytes (little endian) from the file.
         *
         * @param[in] offset The offset to the bytes.
         * @return The bytes at the specified offset.
         */
        u16 GetU16LE(u32 offset) const;

        /**
         * Retrieves four bytes (little endian) from the file.
         *
         * @return The bytes at the specified offset.
         */
        u32 GetU32LE(u32 offset) const;

        /**
         * Reads a byte from the file at the current offset.
         *
         * Before reading, it aligns the current offset to the next byte.Advances the current
         * offset by one byte when done.
         *
         * @return The data in the byte in the current offset of the file.
         */
        u8 ReadU8();

        /**
         * Reads two bytes from the file (little endian) at the current offset.
         *
         * Before reading, it aligns the current offset to the next byte.Advances the current
         * offset by two bytes when done.
         *
         * @return Two bytes of data from the current offset of the file.
         */
        u16 ReadU16LE();

        /**
         * Reads four bytes from the file (little endian) at the current offset.
         *
         * Before reading, it aligns the current offset to the next byte.Advances the current
         * offset by four bytes when done.
         *
         * @return Four bytes of data from the current offset of the file.
         */
        u32 ReadU32LE();


        /**
         * Checks the current offset, in bits, of the file.
         *
         * Default is 0. Can be set on instantiation with {@see
         * BinaryFile(const File* file, u32 offset, u32 length)} or {@see
         * BinaryFile(const u8* buffer, u32 offset, u32 length)} and advanced with
         * {@see GeadBitsLE}, or {@see ReadBitsLE}
         */
        unsigned int GetCurrentOffset();

        /**
         * Sets the file offset for reading.
         *
         * If the offset is larger than the file size, the offset will be set to the end of the
         * file
         *
         * @param[in] offset The new offset, in bits.
         */
        void SetOffset(unsigned int offset);

        /**
         * Aligns the current offset to the next byte.
         */
        void Align();

    private:

        /**
         * Offset, to the bit.
         */
        struct Offset{

            /**
             * File offset, in bytes.
             */
            u32 bytes;

            /**
             * Ofset of the current byte, in bits
             */
            u8 bits;

            /**
             * Constructor, initializes to 0.
             */
            Offset(): bytes(0), bits(0){};
        };

        /**
         * The file name.
         */
        Ogre::String file_name_;

        /**
         * Current offset of the file.
         *
         * Default is 0. Can be set on instantiation with {@see
         * File(const File* file, u32 offset, u32 length)} or {@see
         * File(const u8* buffer, u32 offset, u32 length)} and advanced with
         * {@see readU8}, {@see readU16LE} or {@see readU32LE}
         */
        Offset offset_;

        /**
         * The file buffer, in bytes.
         *
         * It contains the file data, in bytes.
         */
        u8* buffer_bytes_;

        /**
         * The allocated size of {@see buffer_bytes_}, in bytes.
         */
        u32 buffer_size_bytes_;

        /**
         * The file buffer, bit by bit.
         *
         * It contains the file data, in bits.
         */
        u8* buffer_bits_;

        /**
         * The allocated size of {@see buffer_bits_}, in bits.
         */
        u32 buffer_size_bits_;
};
