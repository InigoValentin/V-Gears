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

/**
 * Where the installer puts things.
 *
 * The installation directory is also the working directory of the game, which reads its data from
 * "<dir>/engine" and "<dir>/game". "<dir>/temp" is only used while installing.
 *
 * Every path is relative to the installation directory unless stated otherwise.
 */
namespace InstallLayout{

    /**
     * Gets the directory with the data converted from the original game.
     *
     * @param[in] root Installation directory, ending with '/'.
     */
    std::string GameDir(const std::string& root);

    /**
     * Gets the directory with the intermediate files.
     *
     * @param[in] root Installation directory, ending with '/'.
     */
    std::string TempDir(const std::string& root);

    /**
     * Directories created inside the game directory.
     */
    const std::vector<std::string>& GameSubdirs();

    /**
     * Directories created inside the temp directory.
     */
    const std::vector<std::string>& TempSubdirs();

    /**
     * Gets the directory where the installation disk is extracted.
     *
     * Its files are only read while installing.
     *
     * @param[in] root Installation directory, ending with '/'.
     */
    std::string OriginalDataDir(const std::string& root);

    /**
     * Directories of the game data (in <installation directory>/game) that a clean installation keeps: the videos,
     * which come from the game disks, not from the install disk.
     */
    const std::vector<std::string>& CleanInstallKeptGameDirs();

    /**
     * Tells if a directory is a V-Gears installation: it has the configuration files the installer copies.
     *
     * @param[in] root The directory, ending with '/'.
     */
    bool IsInstallation(const std::string& root);

    /**
     * Result of cleaning an installation.
     */
    struct CleanResult{

        /**
         * Number of files and directories removed.
         */
        unsigned long long removed = 0;

        /**
         * What could not be removed.
         */
        std::vector<std::string> errors;
    };

    /**
     * Deletes what a previous installation wrote, so a new one starts from nothing.
     *
     * It removes the extracted disk, the temporary files, the record of the installations (install.info), the
     * files shipped with V-Gears ("engine" and "defaults"), the "data" directory of older versions, and the game
     * data, except the directories in {@see CleanInstallKeptGameDirs}. The files of the player ("user"), the Ogre
     * configuration and the logs are kept: the ones that older versions had elsewhere are moved first (see
     * VGears::DataDirectory::MigrateLayout). A directory that is not an installation ({@see IsInstallation}) is
     * not touched.
     *
     * @param[in] root Installation directory, ending with '/'.
     */
    CleanResult CleanInstallation(const std::string& root);

    /**
     * Directories the game registers as resource locations, relative to the installation directory.
     */
    const std::vector<std::string>& GameResourceDirs();

    /**
     * Calculates the installation directory from the settings of older versions, which stored the configuration
     * directory and the data directory (<installation directory>/data) separately.
     *
     * @param[in] legacy_data_dir Old data directory setting, can be empty.
     * @param[in] legacy_config_dir Old configuration directory setting, can be empty.
     * @param[in] default_dir Directory to use when there is nothing to migrate.
     */
    std::string MigrateInstallDir(
      std::string legacy_data_dir, const std::string& legacy_config_dir, const std::string& default_dir
    );

}
