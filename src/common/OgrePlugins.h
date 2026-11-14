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
#include <boost/filesystem.hpp>

namespace VGears{

    /**
     * Locates the Ogre plugins listed in plugins.cfg.
     *
     * Ogre only looks for the plugins in the single PluginFolder of plugins.cfg, so the same file can't work in a
     * build tree, an installed package and a portable folder. V-Gears loads the plugins itself instead, looking for
     * each one in several directories (see {@see SearchDirectories}).
     *
     * It doesn't depend on Ogre, so the installer and the tests can use it.
     */
    namespace OgrePlugins{

        /**
         * Contents of a plugins.cfg file.
         */
        struct Config{

            /**
             * Value of PluginFolder. Empty if it's not set.
             */
            std::string folder;

            /**
             * Names of the plugins, in order (the values of the Plugin lines).
             */
            std::vector<std::string> plugins;
        };

        /**
         * Reads a plugins.cfg file.
         *
         * @param[in] file Path to the file.
         * @param[out] config The contents of the file.
         * @return False if the file can't be read.
         */
        bool ReadConfig(const boost::filesystem::path& file, Config& config);

        /**
         * Gets the directories where the plugins are looked for, in order of preference.
         *
         * They are:
         * - The PluginFolder of plugins.cfg, if set (relative to the directory of plugins.cfg).
         * - The directory of plugins.cfg (the data directory).
         * - The directory of the executable, and "../lib/OGRE" and "../lib" from it (bundles and portable folders).
         * - The Ogre plugin directory found when V-Gears was built.
         * - The usual system directories of the Ogre plugins.
         *
         * @param[in] config_directory The directory of plugins.cfg.
         * @param[in] config_folder The PluginFolder of plugins.cfg. Can be empty.
         * @param[in] executable_directory The directory of the executable. Can be empty.
         * @param[in] build_directory The Ogre plugin directory found when V-Gears was built. Can be empty.
         * @return The directories, without duplicates. They may not exist.
         */
        std::vector<boost::filesystem::path> SearchDirectories(
          const boost::filesystem::path& config_directory, const std::string& config_folder,
          const boost::filesystem::path& executable_directory, const std::string& build_directory
        );

        /**
         * Gets the file names a plugin can have in this system.
         *
         * In Linux, Ogre loads "<name>.so.<version>". In Windows, "<name>.dll". In macOS, "<name>.dylib" or
         * "<name>.framework".
         *
         * @param[in] name Name of the plugin, as in plugins.cfg.
         * @param[in] ogre_version The Ogre version, as "major.minor.patch".
         * @return The file names, in order of preference.
         */
        std::vector<std::string> FileNames(const std::string& name, const std::string& ogre_version);

        /**
         * Finds a plugin.
         *
         * @param[in] name Name of the plugin, as in plugins.cfg. If it's an absolute path, it's used as is.
         * @param[in] directories The directories to look in, in order of preference.
         * @param[in] ogre_version The Ogre version, as "major.minor.patch".
         * @return The path of the plugin file, or an empty path if it's not found.
         */
        boost::filesystem::path Find(
          const std::string& name, const std::vector<boost::filesystem::path>& directories,
          const std::string& ogre_version
        );
    }
}
