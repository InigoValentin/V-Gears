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

#include <cstdint>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

/**
 * Reads the folders and files of an ISO9660 image.
 *
 * It's used for the images that libarchive lists no files of.
 */
class Iso9660Reader{

    public:

        /**
         * Size of a sector of the image.
         */
        static const size_t SECTOR_SIZE;

        /**
         * A file or folder of the image.
         */
        struct Entry{

            /**
             * Path from the root: the names of the folders, as returned by the name filter, and
             * the name, separated by slashes.
             */
            std::string path;

            /**
             * True for folders.
             */
            bool directory;

            /**
             * First sector of the contents.
             */
            uint32_t sector;

            /**
             * Size of the contents, in bytes.
             */
            uint32_t size;
        };

        /**
         * Result of a walk through the image.
         */
        enum class Result{
            COMPLETE, /**< All the entries were visited. */
            STOPPED, /**< A function asked to stop. */
            READ_ERROR /**< A folder could not be read. */
        };

        /**
         * Constructor.
         *
         * @param[in] path Path of the image.
         */
        explicit Iso9660Reader(const std::string& path);

        /**
         * Checks if the file is an ISO9660 image.
         *
         * @return True if it could be opened and has a primary volume descriptor.
         */
        bool IsValid() const;

        /**
         * Visits every file and folder of the image, each folder before its contents.
         *
         * Entries whose name could lead outside of their folder are skipped.
         *
         * @param[in] normalize Converts the names of the image to the ones used in the paths.
         * @param[in] visit Called for each entry. It returns false to stop.
         * @return How the walk ended.
         */
        Result Walk(
          const std::function<std::string(const std::string&)>& normalize,
          const std::function<bool(const Entry&)>& visit
        );

        /**
         * Reads the contents of a file in pieces.
         *
         * @param[in] entry The file.
         * @param[in] write Called with each piece. It returns false to stop.
         * @return False if the file could not be read or written completely.
         */
        bool Read(const Entry& entry, const std::function<bool(const char*, size_t)>& write);

        /**
         * Retrieves how far into the image the last read was.
         *
         * @return The position, in bytes.
         */
        int64_t Position();

    private:

        /**
         * Reads a part of the image.
         *
         * @param[in] offset Where to start.
         * @param[in] size How many bytes to read.
         * @return The bytes. Fewer than requested if the image ends before.
         */
        std::vector<uint8_t> ReadBytes(const uint64_t offset, const size_t size);

        /**
         * Visits the entries of a folder.
         */
        Result WalkFolder(
          const uint32_t sector, const uint32_t size, const std::string& prefix, const int depth,
          const std::function<std::string(const std::string&)>& normalize,
          const std::function<bool(const Entry&)>& visit
        );

        /**
         * The image.
         */
        std::ifstream file_;

        /**
         * Size of the image.
         */
        uint64_t file_size_;

        /**
         * First sector of the root folder.
         */
        uint32_t root_sector_;

        /**
         * Size of the root folder.
         */
        uint32_t root_size_;

        /**
         * True if the image has a primary volume descriptor.
         */
        bool valid_;
};
