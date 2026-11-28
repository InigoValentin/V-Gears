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

#include <set>
#include <vector>
#include <Ogre.h>

class Entity;
class EntityTrigger;

/**
 * Runs the scripts that the player starts by being somewhere: crossing lines, touching entities,
 * and pressing the OK button in front of them.
 */
class EntityTriggers{

    public:

        /**
         * Retrieves how close to a line an entity has to be to trigger it.
         *
         * It's the solid radius of the entity, unless a script has made it too small, as scripts
         * do while a scene is playing. Lines couldn't be triggered at all then.
         *
         * @param[in] solid_radius Solid radius of the entity.
         * @return The distance to the line from which it triggers.
         */
        static float Reach(float solid_radius);

        /**
         * Decides whether an entity is a better target to interact with than the best one so far.
         *
         * The one the player faces more directly wins. If the angles are the same, the nearest
         * one does.
         *
         * @param[in] angle Angle in degrees between where the player faces and the candidate.
         * @param[in] distance Squared distance to the candidate.
         * @param[in] best_angle Angle of the best candidate so far, or the largest one allowed.
         * @param[in] best_distance Squared distance to the best candidate so far.
         * @return True if the candidate is better.
         */
        static bool IsBetterInteractionTarget(
          float angle, float distance, float best_angle, float best_distance
        );

        /**
         * Forgets which entities the player is touching.
         */
        void ClearContacts();

        /**
         * Runs the scripts of the lines that the player is close to.
         *
         * If there are lines, and the conditions are met, the appropriate trigger function is added
         * to the queue. This must be tested every time the player moves.
         *
         * @param[in] player The player entity. It must be solid and not locked.
         * @param[in] position The position the player is moving to.
         * @param[in] lines The lines of the map.
         */
        void CheckLines(
          Entity* player, const Ogre::Vector3& position, const std::vector<EntityTrigger*>& lines
        );

        /**
         * Runs the contact script (script 2) of the entities that the player touches.
         *
         * The script of an entity that the original game runs when the player collides with it
         * (the old man of condor1 starts his conversation that way, and only after it can be
         * talked to). It runs once for each touch.
         *
         * @param[in] player The player entity, or nullptr if there is none.
         * @param[in] player_locked True if the player can't be controlled.
         * @param[in] entities The entities of the field.
         */
        void CheckContacts(
          Entity* player, const bool player_locked, const std::vector<Entity*>& entities
        );

        /**
         * Runs the on_interact script of the entity in front of the player.
         *
         * It looks for the entities that can be interacted with from the player's position and
         * orientation, and selects the most appropriate one.
         *
         * @param[in] player The player entity. It must be solid and not locked.
         * @param[in] entities The entities of the map.
         * @param[in] world_map True in the world map, where the height is in the Y axis and the
         * direction the player faces doesn't count.
         */
        void Interact(Entity* player, const std::vector<Entity*>& entities, const bool world_map);

        /**
         * Runs the on_interact script of the lines that the player is next to.
         *
         * @param[in] lines The lines of the map.
         */
        void InteractWithLines(const std::vector<EntityTrigger*>& lines);

    private:

        /**
         * Priority of the script that runs when the player presses OK in front of an entity.
         *
         * It has to run before the scripts that fields leave running on an entity (the loops that
         * play an animation, requested with priority 5 or 6), or the entity could not be talked to.
         * But it can't be the highest: a script of the same entity that is requested with a higher
         * priority (3 or smaller) has to be able to run while the talk script waits for it, as in
         * the scenes where a talk script asks its own entity to do something and waits for it to
         * start.
         */
        static const int INTERACT_SCRIPT_PRIORITY;

        /**
         * Calculates the square distance between a point and a line.
         *
         * The calculated distance is the vector orthogonal to the line that passes by the point,
         * i.e. the shortest distance.
         *
         * @param[in] point The point.
         * @param[in] point_a First point of the line.
         * @param[in] point_b Second point of the line.
         * @param[out] proj Vector representing the shortest distance.
         * @return The distance from the point to the line.
         */
        static float SquareDistanceToLine(
          const Ogre::Vector3& point, const Ogre::Vector3& point_a, const Ogre::Vector3& point_b,
          Ogre::Vector3& proj
        );

        /**
         * Calculates the direction degree between to points.
         *
         * @param[in] current_point Origin point.
         * @param[in] direction_point Next point in the direction.
         * @return Degrees between the two points
         */
        static Ogre::Degree GetDirectionToPoint(
          const Ogre::Vector3& current_point, const Ogre::Vector3& direction_point
        );

        /**
         * Finds the entity of the world map that the player can talk to.
         *
         * @param[in] player The player entity.
         * @param[in] entities The entities of the map.
         * @return The nearest talkable entity in range, or nullptr if there is none.
         */
        static Entity* FindWorldMapTarget(Entity* player, const std::vector<Entity*>& entities);

        /**
         * Finds the entity of a field that the player can talk to.
         *
         * @param[in] player The player entity.
         * @param[in] entities The entities of the field.
         * @return The talkable entity in range that the player faces most, or nullptr if there is
         * none.
         */
        static Entity* FindFieldTarget(Entity* player, const std::vector<Entity*>& entities);

        /**
         * Entities that the player is touching.
         */
        std::set<const Entity*> contacts_;
};
