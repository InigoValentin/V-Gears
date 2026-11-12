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
 * "<dir>/data/..." and its intermediate world map files from "<dir>/temp/world_models".
 *
 * Every path is relative to the installation directory unless stated otherwise.
 */
namespace InstallLayout{

    /**
     * Gets the directory with the game data.
     *
     * @param[in] root Installation directory, ending with '/'.
     */
    std::string DataDir(const std::string& root);

    /**
     * Gets the directory with the intermediate files.
     *
     * @param[in] root Installation directory, ending with '/'.
     */
    std::string TempDir(const std::string& root);

    /**
     * Directories created inside the data directory.
     */
    const std::vector<std::string>& DataSubdirs();

    /**
     * Directories created inside the temp directory.
     */
    const std::vector<std::string>& TempSubdirs();

    /**
     * Temp directories that the game itself needs, so they are not removed when cleaning up.
     */
    const std::vector<std::string>& KeptTempSubdirs();

    /**
     * Temp directories that are removed when cleaning up.
     */
    std::vector<std::string> RemovableTempSubdirs();

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
