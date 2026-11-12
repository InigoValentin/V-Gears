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
 * A thing that is missing in an installation.
 */
struct VerifyProblem{

    /**
     * Part of the installation it belongs to ("Field maps", "World map"...).
     */
    std::string group;

    /**
     * What is missing.
     */
    std::string message;
};

/**
 * Result of verifying an installation.
 */
struct VerifyResult{

    /**
     * What is missing.
     */
    std::vector<VerifyProblem> problems;

    /**
     * Number of checks done.
     */
    int checked = 0;

    /**
     * True if nothing is missing.
     */
    bool Ok() const{return problems.empty();}
};

/**
 * Checks that an installation has everything the game needs.
 *
 * It only looks at the existence of files, it doesn't open them. Parts of the installation can be skipped on purpose
 * (for example the music), so a missing item is reported under the part it belongs to.
 *
 * @param[in] root Installation directory.
 * @param[in] reference_dir V-Gears "data" folder of the game (the one copied to the installation). If not empty,
 * every file in it must exist in the installation too. It can be empty.
 */
VerifyResult VerifyInstallation(const std::string& root, const std::string& reference_dir = "");
