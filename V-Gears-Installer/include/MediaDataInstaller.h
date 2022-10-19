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
         * Extracts all images contained in the menu LGP file and installs them.
         *
         * For composed  textures, it extracts individual images. All images are saved under the
         * "images" directory. This also includes font maps.
         */
        void InstallSprites();

    private:

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

};
