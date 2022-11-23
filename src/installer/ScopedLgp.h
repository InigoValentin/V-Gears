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

#include <Ogre.h>

/**
 * Handles a scoped LGP archive.
 */
class ScopedLgp{

    public:

        /**
         * Constructor.
         *
         * Don't use directly.
         */
        ScopedLgp(const ScopedLgp&) = delete;

        /**
         * Copy constructor.
         *
         * Don't use directly.
         */
        ScopedLgp& operator = (const ScopedLgp&) = delete;

        /**
         * Constructor.
         */
        ScopedLgp(Ogre::Root* root, std::string full_path, std::string type, std::string group);

        /**
         * Destructor.
         */
        ~ScopedLgp();

    private:

        /**
         * The Ogre root system.
         */
        Ogre::Root* root_;

        /**
         * Full path to the LGP archive.
         */
        std::string full_path_;

        /**
         * Group for the resource.
         */
        std::string group_;
};
