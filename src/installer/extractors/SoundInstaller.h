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
#include <vector>
#include "installer/extractors/MediaConverter.h"
#include "installer/formats/DiskImage.h"

/**
 * Installs the sound effects.
 */
class SoundInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] data_dir Directory of the installed game data.
         * @param[in] keep_originals True to keep original data after conversion, false to remove.
         * @param[in] converter Runs the audio conversions.
         */
        SoundInstaller(
          DiskImage& disk_image, const std::string& data_dir, const bool keep_originals,
          MediaConverter& converter
        );

        /**
         * Prepares the installer for the sounds extraction.
         *
         * @return The total number of sounds to process.
         */
        int InstallSoundsInit();

        /**
         * Extracts the next sound file contained in the audio.dat file and installs it.
         *
         * @return True if there are no more sounds to extract, false otherwise.
         */
        bool InstallSounds();

        /**
         * Writes the XML file with all audio entries.
         */
        void WriteSoundIndex();

    private:

        /**
         * Number of sound files to extract.
         */
        static const int TOTAL_SOUNDS;

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
         * Next sound to extract.
         */
        int processed_sounds_ = 0;

        /**
         * Sound data for each entry to write to the XML file
         */
        std::vector<std::string> sounds_;
};
