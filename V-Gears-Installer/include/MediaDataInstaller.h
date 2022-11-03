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
         * @param[out] input_dir Path to the directory of the installation data.
         */
        MediaDataInstaller(const std::string input_dir, const std::string output_dir);

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
         * Extracts all sound files contained in the audio.dat file and installs them.
         *
         */
        void InstallSounds();

        /**
         * Writes the XML file with all audio entries.
         */
        void WriteSoundIndex();

    private:

        /**
         * Number of sound files to extract.
         */
        static int TOTAL_SOUNDS;

        /**
         * The standard WAV header.
         *
         * 78 bytes to be written to every wav file before anything else.
         */
        static u8 WAV_HEADER[78];

        /**
         * The structure of each audio file pointer in a sound FMT file. 74 bytes.
         */
        struct FmtFile{

            /**
             * Size of the wav file in the dat. 4 bytes.
             */
            u32 size;

            /**
             * Offset of the wav file in the dat. 4 bytes.
             */
            u32 offset;

            /**
             * Information for sound looping. 12 bytes.
             */
            u8 loop_metadata[16];

            /**
             * Microsoft WAVFORMATEX header for the wav file. 44 bytes.
             */
            u8 wav_header[18];

            /**
             * Samples per block. 2 bytes.
             */
            u16 samples_per_block;

            /**
             * Number of ADPCM coefficients (used for compression, should always be 7). 2 bytes.
             */
            u16 adpcm;

            /**
             * Standard Microsoft ADPCMCoefSets. 28 bytes.
             */
            u8 adpcm_sets[28];
        };

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
         * Map for sound files with descriptive names.
         */
        std::unordered_map<int, std::string> sound_map_;

        /**
         * Sound data for each entry to write to the XML file
         */
        std::vector<std::string> sounds_;



};
