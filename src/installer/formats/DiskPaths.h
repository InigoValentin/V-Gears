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

#include <memory>
#include <string>

/**
 * Finds files of an extracted disk, whose names depend on how the image stored them.
 */
namespace DiskPaths{

    /**
     * Finds a file.
     *
     * The case, the ISO9660 version suffixes, the trailing dots and the short names are ignored.
     * The path may be found in a subfolder of the base folder.
     *
     * @param[in] base_dir Folder to search in.
     * @param[in] path Path of the file, relative to the folder.
     * @return The absolute path of the file, or null if there is none.
     */
    std::unique_ptr<std::string> Find(const std::string& base_dir, const std::string& path);
}
