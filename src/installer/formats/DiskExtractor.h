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

#include <functional>
#include <string>

/**
 * Copies all the files of a disk image to a folder.
 */
class DiskExtractor{

    public:

        /**
         * Constructor.
         *
         * @param[in] image_path Path of the image.
         * @param[in] progress Called with the percentage of the image that has been read, each
         * time it goes up by one. It can be empty.
         */
        DiskExtractor(const std::string& image_path, const std::function<void(float)>& progress);

        /**
         * Extracts the image.
         *
         * @param[in] output_dir Folder to extract to. It is created if needed.
         * @param[out] error_message Why the image could not be extracted.
         * @return True if all the files were extracted.
         */
        bool Extract(const std::string& output_dir, std::string& error_message);

    private:

        /**
         * Extracts the image with libarchive.
         *
         * @param[in] output_dir Folder to extract to.
         * @param[out] has_entries False if libarchive lists no files in the image.
         * @param[out] error_message Why the image could not be extracted.
         * @return True if all the files were extracted.
         */
        bool ExtractArchive(const std::string& output_dir, bool& has_entries, std::string& error_message);

        /**
         * Extracts the image reading the ISO9660 folders directly.
         *
         * @param[in] output_dir Folder to extract to.
         * @param[out] error_message Why the image could not be extracted.
         * @return True if all the files were extracted.
         */
        bool ExtractFolders(const std::string& output_dir, std::string& error_message);

        /**
         * Reports the progress, if it went up.
         *
         * The reader can go back to read some parts again, so the progress only goes up.
         *
         * @param[in] position How much of the image has been read.
         */
        void Report(const int64_t position);

        /**
         * Path of the image.
         */
        std::string image_path_;

        /**
         * Function to report the progress.
         */
        std::function<void(float)> progress_;

        /**
         * Size of the image.
         */
        int64_t image_size_;

        /**
         * Last percentage reported.
         */
        int last_percent_;
};
