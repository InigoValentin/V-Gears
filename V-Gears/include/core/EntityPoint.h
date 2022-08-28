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
#include <OgreRoot.h>

/**
 * An entity point.
 */
class EntityPoint{

    public:

        /**
         * Constructor.
         *
         * @param name[in] Name of the entity point.
         */
        EntityPoint(const Ogre::String& name);

        /**
         * Destructor.
         */
        virtual ~EntityPoint();

        /**
         * Updates the point state with debug information.
         */
        void UpdateDebug();

        /**
         * Retrieves the entity point name.
         *
         * @return The name of the entity point.
         */
        const Ogre::String& GetName() const;

        /**
         * Sets the point position.
         *
         * @param point[in] The new point position
         */
        void SetPosition(const Ogre::Vector3& point);

        /**
         * Retrieves the point position.
         *
         * @return The point position.
         */
        const Ogre::Vector3& GetPosition() const;

        /**
         * Informs the script manager of the point position.
         */
        void ScriptGetPosition() const;

        /**
         * Sets the point orientation.
         *
         * @param rotation[in] The new orientation (0-360).
         */
        void SetRotation(const float rotation);

        /**
         * Retrieves the point orientation.
         *
         * @return The point orientation.
         */
        float GetRotation() const;

        /**
         * Retrieves the point orientation.
         *
         * @return The point orientation.
         */
        float ScriptGetRotation() const;

    protected:

        /**
         * The entity point name.
         */
        Ogre::String name_;

        /**
         * The entity point's position.
         */
        Ogre::Vector3 position_;

        /**
         * The entity point's orientation.
         */
        float rotation_;
};
