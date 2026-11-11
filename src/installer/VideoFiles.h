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

/**
 * Helpers to find the videos in the game disks.
 *
 * The PC game disks (1, 2 and 3) have the videos in "ff7/movies". Different disks repeat some of them.
 */
namespace VideoFiles{

    /**
     * Normalizes a path inside a disk image: forward slashes, no leading slash, no ISO9660 version suffix (";1"),
     * no trailing dots, lower case.
     */
    std::string NormalizePath(std::string path);

    /**
     * Checks if a path inside a disk image is a file of the movies folder, and gets its name.
     *
     * @param[in] path Path inside the image, with any case and separators.
     * @param[out] name Name of the file in lower case, without directories. Only set if it returns true.
     * @return True if the path is a file inside "ff7/movies" (not in a subdirectory of it).
     */
    bool IsMovie(const std::string& path, std::string* name = nullptr);

}
