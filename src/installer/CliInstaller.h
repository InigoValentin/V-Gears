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
#include "InstallCommandLine.h"

/**
 * The installer without its window: it installs, or verifies an installation, from the command line.
 *
 * It does the same steps as the installer window (see InstallSteps), and writes the same installer.log. Where the
 * window asks, it stops, unless the user said to continue with --yes.
 */
namespace CliInstaller{

    /**
     * Exit status when everything went well.
     */
    const int EXIT_OK = 0;

    /**
     * Exit status when the installation or the verification failed.
     */
    const int EXIT_FAILED = 1;

    /**
     * Exit status when the command line is wrong.
     */
    const int EXIT_USAGE = 2;

    /**
     * Does what the command line asks.
     *
     * A QCoreApplication must exist. The mode of the arguments must not be GUI.
     *
     * @param[in] arguments The parsed command line.
     * @param[in] program Name of the program, for the usage text.
     * @return The exit status.
     */
    int Run(const InstallCommandLine::Arguments& arguments, const std::string& program);

}
