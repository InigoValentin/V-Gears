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
#include <functional>
#include <string>

struct archive;

/**
 * Copies the videos of a game disk.
 */
class DiskVideoExtractor{

    public:

        /**
         * Constructor.
         *
         * @param[in] image_path Path of the disk image.
         * @param[in] progress Called with the percentage of the videos that has been copied.
         * @param[in] log Function to write a line to the installation log.
         */
        DiskVideoExtractor(
          const std::string& image_path, const std::function<void(float)>& progress,
          const std::function<void(const std::string&)>& log
        );

        /**
         * Copies the videos.
         *
         * The videos that are already there, with the same size, are not copied again.
         *
         * @param[in] video_dir Folder to copy the videos to, ending in a slash.
         * @return True if all the videos were copied or were already there.
         */
        bool Extract(const std::string& video_dir);

        /**
         * Retrieves the number of videos copied by {@see Extract}.
         */
        int Extracted() const{return extracted_;}

        /**
         * Retrieves the number of videos that {@see Extract} found already there.
         */
        int Skipped() const{return skipped_;}

    private:

        /**
         * Adds up the size of the videos of the disk.
         *
         * @return The size, or 0 if the disk has no videos or can't be read.
         */
        uint64_t TotalSize();

        /**
         * Copies the current file of the disk.
         *
         * It is written under a temporary name, so an interrupted copy is never taken for a video.
         *
         * @param[in] image The disk image.
         * @param[in] name Name of the video.
         * @param[in] destination Path of the copy.
         * @return True if the video was copied.
         */
        bool Copy(struct archive* image, const std::string& name, const std::string& destination);

        /**
         * Counts bytes of the videos as done, and reports the progress.
         */
        void Advance(const uint64_t bytes);

        /**
         * Path of the disk image.
         */
        std::string image_path_;

        /**
         * Function to report the progress.
         */
        std::function<void(float)> progress_;

        /**
         * Function to write a line to the installation log.
         */
        std::function<void(const std::string&)> log_;

        /**
         * Size of all the videos.
         */
        uint64_t total_bytes_;

        /**
         * Size of the videos done.
         */
        uint64_t done_bytes_;

        /**
         * Number of videos copied.
         */
        int extracted_;

        /**
         * Number of videos that were already there.
         */
        int skipped_;
};
