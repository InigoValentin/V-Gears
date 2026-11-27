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

#include <functional>
#include <string>
#include <vector>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include "DataInstaller.h"
#include "installer/formats/DiskImage.h"
#include "InstallOptions.h"
#include "InstallVerifier.h"

/**
 * Steps of an installation that don't depend on the user interface.
 *
 * They are shared by the installer window and the command line installer, so both install in the same way.
 */
namespace InstallSteps{

    /**
     * Result of the checks done before starting an installation.
     */
    struct PreflightResult{

        /**
         * Problems that prevent the installation.
         */
        QStringList errors;

        /**
         * Problems that the user can choose to ignore.
         */
        QStringList warnings;
    };

    /**
     * Gets the default installation directory, the one the game reads its data from.
     */
    QString DefaultInstallDir();

    /**
     * Normalizes an installation directory: absolute, with '/' separators, and ending with '/'.
     *
     * The working directory changes during installation, so the path must be absolute.
     */
    QString NormalizeOutputDir(const QString& dir);

    /**
     * Finds the V-Gears "data" folder shipped with the game.
     *
     * It contains the configuration files and plugins (copied to the installation directory) and a "data" subfolder
     * with the game data (copied to <installation directory>/data).
     *
     * @return Path of the folder, or an empty string if it can't be found.
     */
    QString FindGameDataDir();

    /**
     * Checks that an installation can succeed before anything is extracted or written.
     *
     * @param[in] disk_image The selected disk image.
     * @param[in] output Absolute installation directory, ending with '/'.
     * @param[in] copy_data True if the V-Gears data files will be copied.
     * @param[in] game_data_dir The V-Gears "data" folder to copy (only used if copy_data is true).
     * @param[in] clean True if the data of a previous installation will be deleted first.
     */
    PreflightResult RunPreflight(
      DiskImage& disk_image, const QString& output, bool copy_data, const QString& game_data_dir, bool clean
    );

    /**
     * Checks that the videos of a game disk can be extracted to a directory.
     *
     * Only the output directory is needed: no plugins, no configuration, no Ogre.
     *
     * @param[in] output Absolute installation directory, ending with '/'.
     * @return The problems found. Empty if the extraction can start.
     */
    QStringList CheckVideoOutput(const QString& output);

    /**
     * Gets where the verifier looks for the Ogre plugins: next to this program, with the Ogre version V-Gears was
     * built with.
     */
    VerifyPlugins PluginSearch();

    /**
     * Moves the files of an older version to their places, and creates the directories of the installation and the
     * configuration of the player if they are missing.
     *
     * @param[in] output Installation directory, ending with '/'.
     * @return The error, or an empty string.
     */
    QString PrepareInstallation(const QString& output);

    /**
     * Copies the V-Gears files to an installation, replacing the "engine" and "defaults" directories.
     *
     * @param[in] game_data The V-Gears data folder.
     * @param[in] output Installation directory, ending with '/'.
     * @param[in] progress Receives the progress, from 0 to 100.
     * @return False if something couldn't be copied.
     */
    bool CopyGameFiles(const QString& game_data, const QString& output, const std::function<void(float)>& progress);

    /**
     * Recursively copies a directory, overwriting existing files.
     *
     * The configuration of the game (data/config) is only copied if it isn't there: the user may have changed it.
     *
     * @param[in] src The directory to copy.
     * @param[in] dst Where to copy it.
     * @param[in] progress Receives the progress [0-100]. It can be empty.
     * @return True if every file was copied.
     */
    bool CopyDirectory(
      const QString& src, const QString& dst,
      const std::function<void(float)>& progress = std::function<void(float)>()
    );

    /**
     * Phases of an installation, for its progress.
     */
    enum class Phase{EXTRACT, COPY, CONVERT};

    /**
     * Gets the progress of a whole installation from the progress of one of its phases.
     *
     * The phases take a part of the progress that is about the part of the time they take: the extraction of the
     * disk the first 5%, the copy of the V-Gears data files 1%, and the conversion the rest.
     *
     * @param[in] phase The phase.
     * @param[in] progress Progress of the phase [0-100].
     * @return Progress of the installation [0-100].
     */
    float OverallProgress(Phase phase, float progress);

    /**
     * Gets the files the installer needs that are not in an extracted install disk.
     *
     * @param[in] disk_image The disk image, already extracted.
     * @return Paths of the missing files, relative to the disk. Empty if everything is there.
     */
    std::vector<std::string> MissingRequiredFiles(DiskImage& disk_image);

    /**
     * Converts the movies in a folder to what the game plays: frames and sound. The game can't
     * decode the original videos. The movies that already have a converted folder are skipped,
     * and the originals are kept so they are not extracted again.
     *
     * @param[in] video_dir Folder with the extracted movies.
     */
    void ConvertMovies(const std::string& video_dir);

    /**
     * Sets where FFmpeg and TiMidity are looked for.
     *
     * The order is: the paths given, a "tools" folder in the installation directory, a "tools" folder next to the
     * installer, the installer folder, and the system PATH.
     *
     * @param[in] install_dir The installation directory. It can be empty.
     * @param[in] ffmpeg Path to FFmpeg chosen by the user. It can be empty.
     * @param[in] timidity Path to TiMidity chosen by the user. It can be empty.
     */
    void SetToolLocations(const QString& install_dir, const std::string& ffmpeg, const std::string& timidity);

    /**
     * Gets the names of the parts an installation installs, as in the command line ("fields"...).
     *
     * The videos are not installed from the install disk, so they are not included.
     */
    std::vector<std::string> PartNames(const InstallOptions& options);

    /**
     * Records in the installation directory that an installation has started (see VGears::InstallStamp).
     *
     * Until it is marked as finished, the game and the verifier say that the installation didn't finish.
     *
     * @param[in] output The installation directory, ending with '/'.
     * @param[in] options What is installed.
     * @param[in] disk The disk.
     */
    void MarkInstallStarted(const QString& output, const InstallOptions& options, const std::string& disk);

    /**
     * Records in the installation directory that the installation has finished.
     *
     * @param[in] output The installation directory, ending with '/'.
     */
    void MarkInstallFinished(const QString& output);

    /**
     * Records in the installation directory that the videos of a game disk are installed.
     *
     * @param[in] output The installation directory, ending with '/'.
     */
    void MarkVideosInstalled(const QString& output);

    /**
     * Converts the options of an installation to the options of the data installer.
     *
     * Videos are not installed by the data installer: they are extracted from the game disks.
     *
     * @param[in] options The options of the installation.
     * @param[in] keep_originals True to keep the original data after converting it.
     */
    DataInstaller::AdvancedOptions ToAdvancedOptions(const InstallOptions& options, bool keep_originals);

}
