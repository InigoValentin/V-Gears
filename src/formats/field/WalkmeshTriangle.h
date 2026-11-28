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

#include <OgreVector3.h>

/**
 * A triangle of a walkmesh.
 */
struct WalkmeshTriangle{

    /**
     * Constructor.
     */
    WalkmeshTriangle(): a(0, 0, 0), b(0, 0, 0), c(0, 0, 0), locked(false){
        access_side[0] = -1;
        access_side[1] = -1;
        access_side[2] = -1;
    }

    /**
     * A side of the triangle.
     */
    Ogre::Vector3 a;

    /**
     * A side of the triangle.
     */
    Ogre::Vector3 b;

    /**
     * A side of the triangle.
     */
    Ogre::Vector3 c;

    /**
     * @todo Understand and document.
     */
    int access_side[3];

    /**
     * Indicates if the triangle is locked (if it's walkable).
     */
    bool locked;
};
