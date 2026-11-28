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
#include "formats/lgp/LGPArchive.h"
#include "installer/formats/BinGZipFile.h"
#include "installer/formats/DiskImage.h"
#include "installer/formats/TexFile.h"

/**
 * Installs the images of the interface and the fonts.
 */
class MediaImageInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] data_dir Directory of the installed game data.
         * @param[in] keep_originals True to keep original data after conversion, false to remove.
         */
        MediaImageInstaller(
          DiskImage& disk_image, const std::string& data_dir, const bool keep_originals
        );

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

    private:

        /**
         * Saves the images that are cut out of a texture.
         *
         * @param[in] tex The texture.
         * @param[in] texture Name of the texture.
         */
        void SaveImages(TexFile& tex, const std::string& texture) const;

        /**
         * Saves the fonts of the digits and the timer, in every colour.
         *
         * @param[in] tex The texture that has them.
         */
        void SaveNumberFonts(TexFile& tex) const;

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
         * The menu.lgp file.
         */
        VGears::LGPArchive menu_;

        /**
         * The WINDOW.BIN file.
         */
        BinGZipFile window_;
};
