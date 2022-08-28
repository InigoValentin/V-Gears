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

class Entity;

/**
 * An entity trigger.
 */
class EntityTrigger{

    public:

        /**
         * Constructor.
         *
         * @param name[in] The trigger name.
         */
        EntityTrigger(const Ogre::String& name);

        /**
         * Destructor.
         */
        virtual ~EntityTrigger();

        /**
         * Updates the trigger state with debug information.
         */
        void UpdateDebug();

        /**
         * Retrieves the trigger name.
         *
         * @return The trigger name.
         */
        const Ogre::String& GetName() const;

        /**
         * Enables or disables the trigger.
         *
         * When disabled, all the activators are removed.
         *
         * @param enabled[in] True to enable the trigger, false to disable it.
         */
        void SetEnabled(const bool enabled);

        /**
         * Checks if the trigger is enabled.
         *
         * @return True if it's enabled, false otherwise.
         */
        bool IsEnabled() const;

        /**
         * Adds an activator to the trigger.
         *
         * @param activator[in] Trigger activator entity.
         */
        void AddActivator(Entity* activator);

        /**
         * Removes an activator from the trigger.
         *
         * @param activator[in] Entity to remove as activator.
         */
        void RemoveActivator(Entity* activator);

        /**
         * Checks if an entity is an activator of the trigger.
         *
         * @param activator[in] Entity to test as an activator.
         * @return True if the entity is an activator, false otherwise.
         */
        bool IsActivator(Entity* activator);

        /**
         * Sets the vertices of the line that acts as the trigger.
         *
         * @param point1[in] One of the vertices of the line.
         * @param point2[in] One of the vertices of the line.
         */
        void SetPoints(
          const Ogre::Vector3& point1, const Ogre::Vector3& point2
        );

        /**
         * Retrieves the first vertex of the line that acts as trigger.
         *
         * @return The first vertex.
         */
        const Ogre::Vector3& GetPoint1() const;

        /**
         * Retrieves the second vertex of the line that acts as trigger.
         *
         * @return The second vertex.
         */
        const Ogre::Vector3& GetPoint2() const;

    protected:

        /**
         * The name of the trigger.
         */
        Ogre::String name_;

        /**
         * Indicates if the trigger is enabled or disabled.
         */
        bool enabled_;

        /**
         * List of activators of the trigger.
         */
        std::vector<Entity*> activators_;

        /**
         * One of the vertices of the trigger line.
         */
        Ogre::Vector3 point_1_;

        /**
         * One of the vertices of the trigger line.
         */
        Ogre::Vector3 point_2_;
};

