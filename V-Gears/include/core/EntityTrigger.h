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
         * @param[in] name The trigger name.
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
         * @param[in] enabled True to enable the trigger, false to disable it.
         */
        void SetEnabled(const bool enabled);

        /**
         * Checks if the trigger is enabled.
         *
         * @return True if it's enabled, false otherwise.
         */
        bool IsEnabled() const;

        /**
         * Sets the vertices of the line that acts as the trigger.
         *
         * @param[in] point1 One of the vertices of the line.
         * @param[in] point2 One of the vertices of the line.
         */
        void SetPoints(const Ogre::Vector3& point1, const Ogre::Vector3& point2);

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

        /**
         * Check if the line has been approached.
         *
         * @return True if the playable entity solid radius has collided with the line and has not
         * existed yet, false otherwise.
         */
        const bool CheckApproached();

        /**
         * Sets if the the playable entity has approached the line.
         *
         * Must be set to true when the playable entity's solid radius touches the line while
         * manually moving, and to false when the playable entity is not near the line.
         *
         * @param[in] entered If the line has been approached.
         */
        void SetApproached(const bool approached);

        /**
         * Check if the line has been crossed.
         *
         * @return True if the playable entity's center has collided with the line and has not
         * existed yet (exit as in the line being outside the entity solid radius), false otherwise.
         */
        const bool CheckCrossed();

        /**
         * Sets if the the playable entity has crossed the line.
         *
         * Must be set to true when the playable entity's center touches the line while
         * manually moving, and to false when the playable entity is not near the line.
         *
         * @param[in] crossed If the line has been approached.
         */
        void SetCrossed(const bool crossed);

        /**
         * Checks if the on_near_once event has been fired.
         *
         * @return True if the on_near_once event has been fired and the entity has not yet exited
         * the line (exit as in the line being outside the entity solid radius), and false if the
         * playable entity has approached the line but the event has not been triggered yet.
         */
        const bool CheckNearSingleEventTriggered();

        /**
         * Sets if the on_near_once have been triggered.
         *
         * Must be set to true when it's run, and to false when the playable entity is not near the
         * line.
         *
         * @param[in] triggered If the on_near_once has been triggered.
         */
        void SetNearSingleEventTriggered(const bool triggered);

        /**
         * Checks if the cooldown that prevents caling the on_near event multiple times has ended.
         *
         * After the on_near script is run, a cooltime of a number of frames is set before running
         * it again. This is because most of the time this trigger checks for a key, and if its
         * pressed, is run multiple times.
         *
         * @return The number of frames remaining in the cooldown. If it's 0, the on_near event can
         * be called again. If not, {@see DecreaseNearEventCooldown} must be called.
         */
        const int GetNearEventCooldown();

        /**
         * Decreases the cooldown time required between calls to on_near event by one.
         *
         * After the on_near script is run, a cooltime of a number of frames is set before running
         * it again. This is because most of the time this trigger checks for a key, and if its
         * pressed, is run multiple times.
         */
        void DecreaseNearEventCooldown();

        /**
         * Resets the cooldown time required between calls to on_near event.
         *
         * After the on_near script is run, a cooltime of a number of frames is set before running
         * it again. This is because most of the time this trigger checks for a key, and if its
         * pressed, is run multiple times.
         *
         * Must be called after firing the on_near event. After calling this, the cooldown will be
         * reset, and all frames must be waited before calling it again.
         */
        void ResetNearEventCooldown();

        /**
         * Sets the cooldown time required between calls to on_near event to 0.
         *
         * After the on_near script is run, a cooltime of a number of frames is set before running
         * it again. This is because most of the time this trigger checks for a key, and if its
         * pressed, is run multiple times.
         *
         * Must be called whent the playable entity gets away from the line
         */
        void ClearNearEventCooldown();

        /**
         * Clears the proximity statuses and event triggering statuses.
         *
         * Can be called once the playable entity leaves the line.
         */
        void Clear();

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
         * One of the vertices of the trigger line.
         */
        Ogre::Vector3 point_1_;

        /**
         * One of the vertices of the trigger line.
         */
        Ogre::Vector3 point_2_;

    private:

        /**
         * Frames to cooldown after triggering on_near event
         *
         * After the on_near script is run, a cooltime of a number of frames is set before running
         * it again. This is because most of the time this trigger checks for a key, and if its
         * pressed, is run multiple times.
         */
        static int NEAR_EVENT_COOLDOWN_FRAMES;

        /**
         * Indicates if the playable entity has approached the line.
         */
        bool approached_;

        /**
         * Indicates if the playable entity has crossed the line.
         */
        bool crossed_;

        /**
         * Indicates if the near_once event has been triggered.
         */
        bool near_single_event_triggered_;

        /**
         * Remaining frames to cooldown after triggering on_near event
         *
         * After the on_near script is run, a cooltime of a number of frames is set before running
         * it again. This is because most of the time this trigger checks for a key, and if its
         * pressed, is run multiple times.
         */
        int near_event_cooldown_;

};

