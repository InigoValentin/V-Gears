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

#include <OgreSingleton.h>
#include <vector>
#include "ConfigVar.h"

/**
 * Configuration variable manager.
 */
class ConfigVarManager : public Ogre::Singleton<ConfigVarManager>{

    public:

        /**
         * Constructor.
         */
        ConfigVarManager();

        /**
         * Finds a variable by name.
         *
         * @param name[in] Name of the variable to retrieve.
         * @return The variable by the specified name, nullptr if there is no
         * one by that name.
         */
        ConfigVar* Find(const Ogre::String& name) const;

        /**
         * Counts the configuration variables.
         *
         * @return The number of defined configuration variables.
         */
        unsigned int GetConfigVarNumber() const;

        /**
         * Retrieves a configuration variable by index.
         *
         * @param i[in] Index of the configuration variable.
         * @return Variable at the specified index.
         */
        ConfigVar* GetConfigVar(const unsigned int i) const;

    private:

        /**
         * List of configuration variables.
         */
        std::vector<ConfigVar*> config_vars_;
};
