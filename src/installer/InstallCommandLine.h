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
#include "InstallOptions.h"

/**
 * Command line of the installer.
 *
 * Without arguments the installer opens its window. With "--cli", or any of the installation options, it installs
 * from the command line instead.
 */
namespace InstallCommandLine{

    /**
     * What the installer must do.
     */
    enum class Mode{

        /**
         * Open the installer window.
         */
        GUI,

        /**
         * Install from the command line.
         */
        INSTALL,

        /**
         * Verify an installation from the command line.
         */
        VERIFY,

        /**
         * Print the list of installable parts.
         */
        LIST_PARTS,

        /**
         * Print the usage.
         */
        HELP,

        /**
         * Print the version.
         */
        VERSION,

        /**
         * The command line is wrong. The error is in Arguments::error.
         */
        INVALID
    };

    /**
     * The parsed command line.
     */
    struct Arguments{

        /**
         * What to do.
         */
        Mode mode = Mode::GUI;

        /**
         * Why the command line is wrong, when the mode is INVALID.
         */
        std::string error;

        /**
         * Path of the disk image.
         */
        std::string iso;

        /**
         * Installation directory. Empty to use the default one.
         */
        std::string output;

        /**
         * The preset the options start from.
         */
        InstallPreset preset = InstallPreset::EVERYTHING;

        /**
         * The parts to install: the preset, with the parts added and removed by the user.
         */
        InstallOptions options;

        /**
         * True to copy the V-Gears data files (configuration, scripts...) to the installation directory.
         */
        bool copy_data = true;

        /**
         * Path to the V-Gears data folder to copy. Empty to look for it.
         */
        std::string data_dir;

        /**
         * True to keep the original data after converting it.
         */
        bool keep_originals = true;

        /**
         * Path to FFmpeg. Empty to look for it.
         */
        std::string ffmpeg;

        /**
         * Path to TiMidity. Empty to look for it.
         */
        std::string timidity;

        /**
         * True to continue when there are warnings or the audio tools are missing, instead of stopping.
         */
        bool yes = false;
    };

    /**
     * Name and description of an installable part.
     */
    struct Part{

        /**
         * Name used in the command line.
         */
        const char* name;

        /**
         * Description shown to the user.
         */
        const char* description;

        /**
         * The option of the part.
         */
        bool InstallOptions::* option;
    };

    /**
     * Gets the installable parts.
     */
    const std::vector<Part>& Parts();

    /**
     * Parses the command line.
     *
     * Arguments that are not installer options are left for Qt when the window is opened (for example "-platform"),
     * and are an error from the command line.
     *
     * @param[in] args The arguments, without the program name.
     */
    Arguments Parse(const std::vector<std::string>& args);

    /**
     * Gets the usage text.
     *
     * @param[in] program Name of the program.
     */
    std::string Usage(const std::string& program);

}
