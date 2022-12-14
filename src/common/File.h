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

/**
 * Represents a file.
 */
class File{

    public:

        /**
         * Opens a file.
         *
         * @param[in] file Path to the file.
         */
        File(const Ogre::String& file);

        /**
         * Opens a file.
         *
         * @param[in] file Pointer to the file.
         */
        File(const File* file);

        /**
         * Loads a file fragment.
         *
         * @param[in] file Pointer to the file.
         * @param[in] offset Offset to the data to load.
         * @param[in] length Length of the data to load.
         */
        File(const File* file, u32 offset, u32 length);

        /**
         * Loads a file fragment from a buffer.
         *
         * @param[in] buffer Pointer to the buffer to load from.
         * @param[in] offset Offset to the data to load.
         * @param[in] length Length of the data to load.
         */
        File(const u8* buffer, u32 offset, u32 length);

        /**
         * Destructor.
         */
        virtual ~File();

        /**
         * Writes the contents of the buffer to a file.
         *
         * @param[in] file Path of the file to save
         */
        void WriteFile(const Ogre::String& file) const;

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
        void GetFileBuffer(
          u8* buffer, const u32 &start, const u32 &length
        ) const;

        /**
         * Retrieves a pointer to a byte address in the file.
         *
         * @param[in] offset The offset to the requested address.
         */
        u8 GetU8(u32 offset) const;

        /**
         * Retrieves a pointer to a two-byte address in the file.
         *
         * The data must b considered to be in little endian.
         *
         * @param[in] offset The offset to the requested address.
         */
        u16 GetU16LE(u32 offset) const;

        /**
         * Retrieves a pointer to a four-byte address in the file.
         *
         * The data must b considered to be in little endian.
         *
         * @param[in] offset The offset to the requested address.
         */
        u32 GetU32LE(u32 offset) const;

        /**
         * Reads a byte from the file.
         *
         * Advances the current offset by one byte.
         *
         * @return The data in the byte in the current offset of the file.
         */
        u8 readU8();

        /**
         * Reads two bytes from the file (little endian).
         *
         * Advances the current offset by two bytes.
         *
         * @return Two bytes of data from the current offset of the file.
         */
        u16 readU16LE();

        /**
         * Reads four bytes from the file (little endian).
         *
         * Advances the current offset by four byte.
         *
         * @return Four bytes of data from the current offset of the file.
         */
        u32 readU32LE();

        /**
         * Checks the current offset of the file.
         *
         * Default is 0. Can be set on instantiation with {@see
         * File(const File* file, u32 offset, u32 length)} or {@see
         * File(const u8* buffer, u32 offset, u32 length)} and advanced with
         * {@see readU8}, {@see readU16LE} or {@see readU32LE}
         */
        u32 GetCurrentOffset();

        /**
         * Sets the file offset for reading.
         *
         * If the offset is larger than the file size, the offset will be set to the end of the
         * file
         *
         * @param[in] offset The new offset.
         */
        void SetOffset(u32 offset);

    protected:

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
        u32 offset_ = 0;

        /**
         * The file buffer.
         *
         * It contains the file data.
         */
        u8* buffer_;

        /**
         * The allocated size of {@see buffer_}.
         */
        u32 buffer_size_;
};
