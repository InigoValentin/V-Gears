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
     * Locates the directory the game runs from.
     *
     * The data directory holds the configuration (plugins.cfg, resources.cfg...), the game data in "data/", and
     * everything the game writes (saves, logs, temporary files). The game reads all of it relative to the working
     * directory, so it changes into the data directory on start.
     *
     * A read-only template of the data directory (the "data" folder of the repository, or "share/v-gears" in an
     * installed package) is shipped with the program. When the data directory has no V-Gears files yet, the files of
     * the template are copied there. The game data extracted from the original game is added later by the data
     * installer.
     */
    namespace DataDirectory{

        /**
         * Name of the environment variable that selects the data directory.
         */
        extern const char* ENV_DATA_DIR;

        /**
         * Name of the environment variable that selects the template directory.
         */
        extern const char* ENV_TEMPLATE_DIR;

        /**
         * Result of {@see Resolve}.
         */
        struct Resolution{

            /**
             * Absolute path of the data directory. Empty on error.
             */
            boost::filesystem::path directory;

            /**
             * True if files were copied from a template.
             */
            bool seeded = false;

            /**
             * The template the files were copied from, if any.
             */
            boost::filesystem::path template_directory;

            /**
             * Number of files copied from the template.
             */
            unsigned int copied_files = 0;

            /**
             * Description of the error. Empty on success.
             */
            std::string error;
        };

        /**
         * Checks if a directory has the V-Gears files: a plugins.cfg file and a "data" subdirectory.
         *
         * @param[in] directory The directory to check.
         * @return True if the directory has the V-Gears files.
         */
        bool IsDataDirectory(const boost::filesystem::path& directory);

        /**
         * Gets the default data directory of the user.
         *
         * It is "~/.v-gears" in Linux and macOS, and "%USERPROFILE%\v-gears" in Windows, the same defaults the data
         * installer uses.
         *
         * @return The default data directory, or an empty path if the home directory can't be found.
         */
        boost::filesystem::path DefaultDirectory();

        /**
         * Gets the directory of the running executable.
         *
         * @param[in] argv0 The first command line argument, used if the system can't tell the executable path.
         * @return The absolute directory of the executable.
         */
        boost::filesystem::path ExecutableDirectory(const char* argv0);

        /**
         * Gets the places where the template can be, in order of preference.
         *
         * They cover a build tree (<repository>/build/bin next to <repository>/data), an installed package
         * (<prefix>/bin or <prefix>/games next to <prefix>/share/v-gears) and a portable folder (a "data" folder
         * next to the executable).
         *
         * @param[in] executable_directory The directory of the running executable.
         * @return The candidate directories. They may not exist.
         */
        std::vector<boost::filesystem::path> TemplateCandidates(
          const boost::filesystem::path& executable_directory
        );

        /**
         * Finds the first candidate that has the V-Gears files.
         *
         * @param[in] candidates The candidates to check, in order of preference.
         * @param[in] exclude A directory that must not be returned, even if it qualifies.
         * @return The template, or an empty path if no candidate qualifies.
         */
        boost::filesystem::path FindTemplate(
          const std::vector<boost::filesystem::path>& candidates, const boost::filesystem::path& exclude
        );

        /**
         * Copies the files of a directory tree that are missing in another one.
         *
         * Existing files are never overwritten, so the changes of the user and the data extracted by the installer
         * are kept.
         *
         * @param[in] from The source directory.
         * @param[in] to The destination directory. It's created if it doesn't exist.
         * @return The number of files copied.
         * @throws boost::filesystem::filesystem_error If a file or directory can't be read or written.
         */
        unsigned int CopyMissing(const boost::filesystem::path& from, const boost::filesystem::path& to);

        /**
         * Decides the data directory and fills it from a template if needed.
         *
         * The data directory is, in order:
         * - The requested directory, if any (from the command line or the environment).
         * - The current directory, if it has the V-Gears files.
         * - The default directory.
         *
         * If the chosen directory doesn't have the V-Gears files, the missing files are copied from the first
         * template candidate that has them. A directory that is not empty and has no "data" subdirectory is not
         * filled, to avoid writing the files into an unrelated directory chosen by mistake.
         *
         * @param[in] requested The requested directory. Empty if none. If relative, it's relative to current.
         * @param[in] current The current directory.
         * @param[in] default_directory The default data directory.
         * @param[in] template_candidates The template candidates, in order of preference.
         * @return The resolution. Check its error field.
         */
        Resolution Resolve(
          const std::string& requested, const boost::filesystem::path& current,
          const boost::filesystem::path& default_directory,
          const std::vector<boost::filesystem::path>& template_candidates
        );
    }
}
