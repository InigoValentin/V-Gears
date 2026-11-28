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
#include "DiskReleases.h"

/**
 * Finds out which disk of the game an image is.
 */
namespace DiskIdentifier{

    /**
     * What was found in an image.
     */
    struct Identification{

        /**
         * The disk, or null if it's not a known one.
         */
        const DiskReleases::Release* release = nullptr;

        /**
         * The game ID of a PlayStation image. Empty for the others.
         */
        std::string game_id;

        /**
         * Why the image could not be opened. Empty if it was.
         */
        std::string open_error;
    };

    /**
     * Identifies an image.
     *
     * @param[in] path Path of the image.
     * @return What was found.
     */
    Identification Identify(const std::string& path);

    /**
     * Normalizes a path of a disk image: slashes, no surrounding spaces, no version suffix, no
     * trailing dots, upper case.
     *
     * @param[in] path The path.
     * @return The normalized path.
     */
    std::string NormalizePath(std::string path);

    /**
     * Removes the version suffix of an ISO9660 name (";1") and the trailing dots of the short
     * names.
     *
     * @param[in] name The name.
     * @return The name without them.
     */
    std::string RemoveVersion(std::string name);

    /**
     * Finds the game ID of a PlayStation disk.
     *
     * @param[in] system_cnf Contents of the SYSTEM.CNF file of the disk.
     * @return The ID, like "SCUS-94163", or an empty string if there is none.
     */
    std::string FindGameId(const std::string& system_cnf);
}
