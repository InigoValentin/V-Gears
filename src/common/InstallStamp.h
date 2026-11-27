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

#include <map>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

namespace VGears{

    /**
     * The record of the installations of the game data in a directory (install.info).
     *
     * The installer writes it when an installation starts, as unfinished, and again when it finishes. It says what
     * was installed and with which version of the converted data, so the game and the verifier can tell when an
     * installation didn't finish, or when a part was converted by an older installer and must be installed again.
     */
    namespace InstallStamp{

        /**
         * Version of the converted data.
         *
         * Increase it when the installer converts something differently, so the installations made before have to
         * be done again. The parts installed with an older version are reported as outdated.
         *
         * - 1: First version of the record.
         * - 2: The converted data moved from "data" to "game".
         * - 3: The converted data is grouped by game area, and the terrain textures of the world map are in "game".
         */
        const int DATA_VERSION = 3;

        /**
         * Name of the file, in the installation directory.
         */
        extern const char* const FILE_NAME;

        /**
         * What the record says.
         */
        struct Stamp{

            /**
             * True if the last installation finished.
             */
            bool complete = false;

            /**
             * Version of the installer that made the last installation.
             */
            std::string installer;

            /**
             * When the last installation started, as "YYYY-MM-DD HH:MM:SS".
             */
            std::string date;

            /**
             * The disk of the last installation.
             */
            std::string disk;

            /**
             * The installed parts (as in the command line of the installer, like "fields"), with the version of
             * the data they were installed with. 0 if their installation didn't finish.
             */
            std::map<std::string, int> parts;

            /**
             * The parts of the installation that is in progress, or that didn't finish.
             */
            std::vector<std::string> installing;
        };

        /**
         * Reads the record of a directory.
         *
         * @param[in] directory The installation directory.
         * @param[out] stamp What it says.
         * @return False if there is no record.
         */
        bool Read(const boost::filesystem::path& directory, Stamp& stamp);

        /**
         * Writes the record of a directory.
         *
         * @param[in] directory The installation directory.
         * @param[in] stamp What to write.
         * @return False if it can't be written.
         */
        bool Write(const boost::filesystem::path& directory, const Stamp& stamp);

        /**
         * Starts an installation: the record says it is in progress.
         *
         * The parts installed before are kept. The ones of an installation that didn't finish, and are not
         * installed again now, are marked as not installed (version 0).
         *
         * @param[in,out] stamp The record of the directory. Empty if there was none.
         * @param[in] parts The parts that will be installed.
         * @param[in] installer Version of the installer.
         * @param[in] date When the installation starts.
         * @param[in] disk The disk.
         */
        void Start(
          Stamp& stamp, const std::vector<std::string>& parts, const std::string& installer,
          const std::string& date, const std::string& disk
        );

        /**
         * Finishes an installation: its parts are installed with the current version of the data.
         *
         * @param[in,out] stamp The record of the directory.
         */
        void Finish(Stamp& stamp);

        /**
         * Gets the parts installed with an older version of the data, or whose installation didn't finish.
         *
         * @param[in] stamp The record.
         * @return The names of the parts, sorted.
         */
        std::vector<std::string> Outdated(const Stamp& stamp);

        /**
         * Describes what is wrong with the installation of a directory, according to its record.
         *
         * @param[in] directory The installation directory.
         * @param[out] found True if there is a record. It can be null.
         * @return One sentence for each problem. Empty if there is none, or if there is no record.
         */
        std::vector<std::string> Problems(const boost::filesystem::path& directory, bool* found = nullptr);

        /**
         * Gets the current date and time, as written in the record.
         */
        std::string Now();
    }
}
