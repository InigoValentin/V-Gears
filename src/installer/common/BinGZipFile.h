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

#include "common/File.h"

/**
 * Handles GZip-compressed BIn files.
 */
class BinGZipFile : public File{

    public:

        /**
         * Constructor.
         *
         * @param[in] file Path to the BIN file.
         */
        BinGZipFile(const Ogre::String& file);

        /**
         * Constructor.
         *
         * @param[in] file The BIN file.
         */
        BinGZipFile(File* file);

        /**
         * Constructor.
         *
         * @param[in] file The BIN file.
         * @param[in] offset The offset at which to start reading the file.
         * @param[in] length Length of the data to read.
         */
        BinGZipFile(File* file, u32 offset, u32 length);

        /**
         * Constructor.
         *
         * @param[in] buffer Buffer with the contents of the BIN file.
         * @param[in] offset The offset at which to start reading the file.
         * @param[in] length Length of the data to read.
         */
        BinGZipFile(u8* buffer, u32 offset, u32 length);

        /**
         * Destructor.
         */
        virtual ~BinGZipFile();

        /**
         * Extracts a file from the compressed BIN file.
         *
         * @param[in] file_index The index of the file to extract.
         * @return The extracted file, or NULL if there is no file at the
         * specified index.
         */
        File* ExtractGZip(u32 file_index);

        /**
         * Counts the number of files packed in the compressed BIN file.
         *
         * @return The number of files.
         */
        u32 GetNumberOfFiles();

    private:

        /**
         * Counts the number of packed files and sets {@see file_count_}.
         */
        void InnerGetNumberOfFiles();

        /**
         * Retrieves a file offset.
         *
         * @param[in] file_index The file which offset to calculate.
         * @return The offset to the specified file, or 0 if there is no such
         * file.
         */
        u32 InnerGetFileOffset(u32 file_index);

        /**
         * The number of files in the archive.
         */
        u32 file_count_;
};
