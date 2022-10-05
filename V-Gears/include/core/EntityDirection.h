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

#include <OgreCamera.h>
#include <OgreSimpleRenderable.h>

/**
 * An entity direction.
 */
class EntityDirection : public Ogre::SimpleRenderable{

    public:

        /**
         * Constructor.
         */
        EntityDirection();

        /**
         * Destructor.
         */
        virtual ~EntityDirection();

        /**
         * Retrieves the view depth, relative to a camera.
         *
         * @param[in] cam The camera
         * @return The squared view depth (always 0).
         */
        Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;

        /**
         * Retrieves the bounding radius.
         *
         * @return The bounding radius (always 0).
         */
        Ogre::Real getBoundingRadius() const;
};
