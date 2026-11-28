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

#include <string>
#include <unordered_map>
#include <vector>
#include "formats/lgp/LGPArchive.h"
#include "installer/extractors/MediaConverter.h"
#include "installer/formats/DiskImage.h"

/**
 * Installs the music.
 */
class MusicInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] data_dir Directory of the installed game data.
         * @param[in] keep_originals True to keep original data after conversion, false to remove.
         * @param[in] converter Runs the audio conversions.
         */
        MusicInstaller(
          DiskImage& disk_image, const std::string& data_dir, const bool keep_originals,
          MediaConverter& converter
        );

        /**
         * Prepares the installer for the sounds extraction.
         *
         * @return The total number of sounds to process.
         */
        int InstallMusicsInit();

        /**
         * Extracts the next music file contained in the midi.lgp, converts it and installs it.
         *
         * @return True if there are no more musics to extract, false otherwise.
         */
        bool InstallMusics();

        /**
         * Converts high quality musiscs to OGG.
         *
         * There are four of them.
         */
        void InstallHQMusics();

        /**
         * Writes the XML file with all music tracks.
         */
        void WriteMusicsIndex();

    private:

        /**
         * The disk image to read the original data from.
         */
        DiskImage& disk_image_;

        /**
         * Directory where the game data is installed (<install dir>/data/).
         */
        std::string data_dir_;

        /**
         * True to keep the original data once converted.
         */
        bool keep_originals_;

        /**
         * Runs the audio conversions.
         */
        MediaConverter& converter_;

        /**
         * The midi.lgp file.
         */
        VGears::LGPArchive midi_;

        /**
         * Next track to extract.
         */
        int processed_musics_ = 0;

        /**
         * Map for music files with descriptive names.
         */
        std::unordered_map<int, std::string> musics_map_;

        /**
         * Sound data for each entry to write to the XML file
         */
        std::vector<std::string> musics_;
};
