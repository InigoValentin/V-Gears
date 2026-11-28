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
#include "installer/extractors/MediaConverter.h"
#include "installer/extractors/MediaImageInstaller.h"
#include "installer/extractors/MusicInstaller.h"
#include "installer/extractors/SoundInstaller.h"
#include "installer/formats/DiskImage.h"

/**
 * The media data installer.
 *
 * It has an installer for the images, the sounds and the music, and passes the calls on to them.
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
         * Extracts all images contained in the menu LGP file and installs them.
         *
         * For composed  textures, it extracts individual images. All images are saved under the
         * "images" directory. This also includes font maps.
         */
        void InstallSprites();

        /**
         * Builds the font images from the textures in WINDOW.BIN and saves them in the "fonts" directory.
         *
         * The font XML files that describe them are part of the V-Gears data files. The fonts are always
         * needed, so this doesn't depend on the images option.
         *
         * @return True if all the fonts were saved, false if WINDOW.BIN couldn't be read.
         */
        bool InstallFonts();

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

    private:

        /**
         * The disk image to read the original data from.
         */
        DiskImage disk_image_;

        /**
         * Runs the audio conversions. Declared before the installers that use it.
         */
        MediaConverter converter_;

        /**
         * Images and fonts.
         */
        MediaImageInstaller images_;

        /**
         * Sound effects.
         */
        SoundInstaller sounds_;

        /**
         * Music.
         */
        MusicInstaller music_;
};
