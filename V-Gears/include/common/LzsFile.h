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
 * Represents an LZS file.
 *
 * More info about lzs fromat: {@link
 * https://wiki.ffrtt.ru/index.php?title=FF7/LZSS_format}
 */
class LzsFile : public File{

    public:

        /**
         * Opens a lzs file and extracts it's contents
         *
         * @param file[in] Path to the file.
         */
        LzsFile(const Ogre::String& file);

        /**
         * Opens a lzs file and extracts it's contents
         *
         * @param file[in] Pointer to the file.
         */
        LzsFile(File* file);

        /**
         * Opens a lzs file fragment and extracts it's contents
         *
         * @param file[in] Path to the file.
         * @param offset[in] Offset to the file contents a which to start
         * reading.
         * @param ength[in] Length of the data to read from the file, in bytes.
         */
        LzsFile(File* file, u32 offset, u32 length);

        /**
         * Opens a lzs file fragment from a buffer and extracts it's contents
         *
         * @param file[in] Pointer to the buffer which holds the data.
         * @param offset[in] Offset to the buffer contents a which to start
         * reading.
         * @param ength[in] Length of the data to read, in bytes.
         */
        LzsFile(const u8* buffer, u32 offset, u32 length);

        /**
         * Destructor.
         */
        virtual ~LzsFile();

    private:

        /**
         * Extracts the file contents to the buffer.
         */
        void ExtractLzs();

        friend class LzsBuffer;
};

/**
 * A buffer forr the contents of a lzs file.
 */
class LzsBuffer{

    public:

        /**
         * No costructor.
         */
        LzsBuffer() = delete;

        /**
         * Decompresses lzs data in a buffer.
         *
         * @param buffer[in] Buffer with the data of the lzs file.
         * @return Decompressed data.
         */
        static std::vector<VGears::uint8> Decompress(
          const std::vector<VGears::uint8>& buffer
        );
};
