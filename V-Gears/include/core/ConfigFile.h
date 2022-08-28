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

#include <OgreString.h>

/**
 * A configuration file.
 */
class ConfigFile{

    public:

        /**
         * Executes a configuration file.
         *
         * The configuration file must be populated with commands, as described
         * in {@see ConfigCmdManagerCommands.h}
         *
         * Executing a file will sequentially execute all the commands in it.
         *
         * @param name[in] Configuration file path.
         */
        void Execute(const Ogre::String& name);
};

