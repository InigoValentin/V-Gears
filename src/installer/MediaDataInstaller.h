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

#include <unordered_map>
#include "data/VGearsLGPArchive.h"
#include "common/BinGZipFile.h"
#include "TexFile.h"

/**
 * The media data installer.
 */
class MediaDataInstaller{


    public:

        /**
         * Constructor
         *
         * @param[in] input_dir Path to the directory containing the original data to parse.
         * @param[in] output_dir Path to the directory of the installation data.
         * @param[in] keep_originals True to keep original data after conversion, false to remove.
         * @param[in] no_ffmpeg True to prevent system calls to ffmpeg command, false to allow.
         * @param[in] no_timidity True to prevent system calls to timidity command, false to allow.
         */
        MediaDataInstaller(
          const std::string input_dir, const std::string output_dir, const bool keep_originals,
          const bool no_ffmpeg, const bool no_timidity
        );

        /**
         * Destructor.
         */
        ~MediaDataInstaller();

        /**
         * Populates the audio name maps.
         */
        void PopulateMaps();

        /**
         * Extracts all images contained in the menu LGP file and installs them.
         *
         * For composed  textures, it extracts individual images. All images are saved under the
         * "images" directory. This also includes font maps.
         */
        void InstallSprites();

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
         * Number of sound files to extract.
         */
        static int TOTAL_SOUNDS;
		
		/*
		 * Retrieves the path to the currently running executable.
		 *
		 * @return Path to the folder containig the current executable, or an empty string in case
		 * of failure.
		 */
		std::string GetExecutablePath();

        /**
         * The path to the directory from which to read the PC game data.
         */
        std::string input_dir_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

        /**
         * The menu.lgp file.
         */
        VGears::LGPArchive menu_;

        /**
         * The WINDOW.BIN file.
         */
        BinGZipFile window_;

        /**
         * Number of sounds already processed.
         */
        int processed_sounds_;

        /**
         * Map for sound files with descriptive names.
         */
        std::unordered_map<int, std::string> sound_map_;

        /**
         * Map for sound files with human readable descriptions.
         */
        std::unordered_map<int, std::string> sound_description_;

        /**
         * Sound data for each entry to write to the XML file
         */
        std::vector<std::string> sounds_;

        /**
         * The midi.lgp file.
         */
        VGears::LGPArchive midi_;

        /**
         * Number of music tracks already processed.
         */
        int processed_musics_;

        /**
         * Map for music files with descriptive names.
         */
        std::unordered_map<int, std::string> musics_map_;

        /**
         * Sound data for each entry to write to the XML file
         */
        std::vector<std::string> musics_;

        /**
         * If true, orginal files will not be deleted after conversion.
         */
        bool keep_originals_;

        /**
         * Flag to prevent system calls to ffmpeg.
         */
        bool no_ffmpeg_;

        /**
         * Flag to prevent system calls to timidity.
         */
        bool no_timidity_;

};
