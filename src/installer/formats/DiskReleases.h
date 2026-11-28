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
#include "DiskImage.h"

/**
 * The releases of the game that can be identified.
 */
namespace DiskReleases{

    /**
     * A disk of a release of the game.
     */
    struct Release{

        /**
         * What identifies the disk: for PC disks, the path of a file only that disk has; for
         * PlayStation disks, the game ID.
         */
        const char* marker;

        /**
         * Human readable name of the disk.
         */
        const char* name;

        /**
         * Platform of the release.
         */
        DiskImage::Platform platform;

        /**
         * Region of the release.
         */
        DiskImage::Region region;

        /**
         * Language of the release.
         */
        DiskImage::Language language;

        /**
         * Disk number.
         */
        DiskImage::Disk disk;

        /**
         * If the release is supported.
         */
        bool supported;

        /**
         * Warning about the release. Empty if there is none.
         */
        const char* warning;
    };

    /**
     * Finds the PC disk that a file belongs to.
     *
     * @param[in] path Path of a file of the disk, normalized with
     * {@see DiskIdentifier::NormalizePath}.
     * @return The disk, or null if the file doesn't identify any.
     */
    const Release* FindPcRelease(const std::string& path);

    /**
     * Finds a PlayStation disk.
     *
     * @param[in] game_id The game ID, like "SCUS-94163".
     * @return The disk, or null if the ID is unknown.
     */
    const Release* FindPlayStationRelease(const std::string& game_id);
}
