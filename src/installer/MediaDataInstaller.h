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

#include <memory>
#include <unordered_map>
#include "DiskImage.h"
#include "data/VGearsLGPArchive.h"
#include "common/BinGZipFile.h"
#include "TexFile.h"
#include "ExternalTools.h"
#include "TaskPool.h"

/**
 * The media data installer.
 */
class MediaDataInstaller{


    public:

        /**
         * Constructor
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] output_dir Path to the directory of the installation data.
         * @param[in] keep_originals True to keep original data after conversion, false to remove.
         */
        MediaDataInstaller(const DiskImage& disk_image, const std::string output_dir, const bool keep_originals);

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
         * Waits until every audio conversion started by the install steps has finished.
         *
         * The conversions run in the background, so the other steps don't wait for them. Call it before using or
         * deleting their results.
         */
        void WaitForConversions();

        /**
         * Writes the XML file with all music tracks.
         */
        void WriteMusicsIndex();

        /**
         * Prepares the installer for the video extraction.
         *
         * @return The total number of videos to process.
         */
        int InstallVideosInit();

        /**
         * Copies the FMV videos.
         */
        void InstallVideos();

    private:

        /**
         * Runs a conversion in the background.
         *
         * A failed conversion is logged as a warning.
         *
         * @param[in] description What is being converted, for the log.
         * @param[in] job Does the conversion. Returns false, and sets the error, if it fails.
         * @param[in] on_success Called, in the background thread, if the conversion succeeds.
         */
        void RunConversion(
          const std::string& description, std::function<bool(std::string*)> job,
          std::function<void()> on_success = nullptr
        );

        /**
         * Finds the conversion programs the first time it is called, and reports what is missing.
         *
         * @param[in] need_timidity True if TiMidity is needed too (the music).
         * @return True if the programs are usable.
         */
        bool ToolsReady(bool need_timidity);

        /**
         * FFmpeg and TiMidity, found by {@see ToolsReady}.
         */
        ExternalTools::Tool ffmpeg_tool_;
        ExternalTools::Tool timidity_tool_;
        bool tools_resolved_ = false;

        /**
         * Threads that run the conversions. Created when the first conversion starts.
         */
        std::unique_ptr<TaskPool> conversions_;

        /**
         * Number of sound files to extract.
         */
        static int TOTAL_SOUNDS;
		
        /**
         * The disk image to read the original data from.
         */
        DiskImage disk_image_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

        /**
         * Directory where the game data is installed (<install dir>/data/).
         */
        std::string data_dir_;

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
         * Video files to copy
         */
        std::vector<std::string> videos_;

        /**
         * The midi.lgp file.
         */
        VGears::LGPArchive midi_;

        /**
         * Number of music tracks already processed.
         */
        int processed_musics_;

        /**
         * Number of video files already processed.
         */
        int processed_videos_;

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

};
