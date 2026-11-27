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

#include <functional>
#include <vector>
#include <Ogre.h>

class Entity;
class Manager;
class Walkmesh;

/**
 * Moves the entities of a field on its walkmesh: walking, climbing ladders and jumping.
 */
class WalkmeshMovement{

    public:

        /**
         * Function called when the player moves, with the position it moves to, to check the lines.
         */
        typedef std::function<void(Entity*, const Ogre::Vector3&)> LineCheck;

        /**
         * Constructor.
         *
         * The references are kept, and have to be valid while the object is used.
         *
         * @param[in] manager The manager of the entities, to know if a battle is taking place.
         * @param[in] walkmesh The walkmesh of the field.
         * @param[in] entities The entities of the field.
         * @param[in] player The player entity, nullptr if there is none.
         * @param[in] player_locked True if the player can't be controlled.
         * @param[in] player_slip True if the player slides along the walls.
         * @param[in] player_move The direction the player is pressing.
         * @param[in] check_lines Function that checks the lines of the field.
         */
        WalkmeshMovement(
          const Manager& manager, const Walkmesh& walkmesh, const std::vector<Entity*>& entities,
          Entity* const& player, const bool& player_locked, const bool& player_slip,
          const Ogre::Vector3& player_move, const LineCheck& check_lines
        );

        /**
         * Limits a scripted movement step to its remaining distance.
         *
         * @param[in] distance Distance to the target.
         * @param[in] stop_distance Desired distance from the target.
         * @param[in] max_step Maximum distance to move this frame.
         * @return Distance to move this frame.
         */
        static float ScriptedStep(float distance, float stop_distance, float max_step);

        /**
         * Checks if the edge of the walkmesh stops an entity.
         *
         * It stops the player and the characters that wander. The characters that a script walks
         * to a position are not stopped: the original game lets them walk off the walkmesh.
         *
         * @param[in] is_player Whether the entity is the player.
         * @param[in] scripted_move Whether a script asked the entity to walk to a position.
         * @return True if the edge of the walkmesh blocks the entity.
         */
        static bool EdgeBlocks(bool is_player, bool scripted_move);

        /**
         * Calculates the point elevation over a plane.
         *
         * @param[in] point Point to calculate the elevation of.
         * @param[in] a First point of the plane.
         * @param[in] b Second point of the plane.
         * @param[in] c Third  point of the plane.
         * @return The elevation of the point.
         */
        static float PointElevation(
          const Ogre::Vector2& point,
          const Ogre::Vector3& a, const Ogre::Vector3& b, const Ogre::Vector3& c
        );

        /**
         * Determines at which side of a line a point is.
         *
         * @param[in] point The point to evaluate.
         * @param[in] p1 The first point of the line.
         * @param[in] p2 The second point of the line.
         * @return A negative value if the point is on the "left" of the line vector, a positive
         * value if the point is on the "right" of the vector line, or 0 if the point is part of
         * the line. "Left" and "right" are determined by going from p1 to p2.
         */
        static float SideOfVector(
          const Ogre::Vector2& point, const Ogre::Vector2& p1, const Ogre::Vector2& p2
        );

        /**
         * Retrieves the height of a point of a walkmesh triangle.
         *
         * @param[in] triangle The triangle.
         * @param[in] x X coordinate of the point.
         * @param[in] y Y coordinate of the point.
         * @param[in] fallback Value returned if there is no such triangle.
         * @return The height of the triangle at the point.
         */
        float GetTriangleHeight(int triangle, float x, float y, float fallback) const;

        /**
         * Checks if an entity is over the walkmesh triangle that it is attached to.
         *
         * @param[in] entity The entity.
         * @return False if it's not over it, or if it's not attached to any triangle.
         */
        bool IsInCurrentTriangle(const Entity& entity) const;

        /**
         * Attaches an entity to the walkmesh.
         *
         * It sets the triangle from the entity position coordinates. To account for multiple
         * triangles on different levels, it uses only the X and Y coordinates, and automatically
         * sets the Z one to the closest triangle. If the manager is in battle mode, it will do
         * nothing.
         *
         * @param[in] entity Entity to attach.
         * @return True if the entity was assigned to a walkmesh triangle, false if the entity is
         * not in a triangle or if the manager is in battle mode.
         */
        bool PlaceOnWalkmesh(Entity* entity);

        /**
         * Moves an entity in the walkmesh.
         *
         * If the manager is in battle mode, it will do nothing.
         *
         * @param[in] entity Entity to move.
         * @param[in] speed Movement speed.
         * @param[in] delta_time Time elapsed for this movement step.
         * @return True if the movement was possible and the entity was moved, false otherwise.
         * If the manager is in battle mode, it will always return false.
         */
        bool Walk(Entity* entity, const float speed, const float delta_time);

        /**
         * Calculates and sets the next position during a linear movement.
         *
         * If the manager is in battle mode, it will do nothing
         *
         * @param[in] entity The moving entity.
         */
        void LinearStep(Entity* entity);

        /**
         * Calculates and sets the next position during a jump.
         *
         * If the manager is in battle mode, it will do nothing
         *
         * @param[in] entity The jump entity.
         */
        void JumpStep(Entity* entity);

        /**
         * What is next to the end of a step: walls (the edge of the walkmesh) and other entities,
         * ahead of the entity, and 45 degrees to each side.
         */
        struct Obstacles{
            bool left_wall = false;
            bool right_wall = false;
            bool ahead_wall = false;
            bool left_entity = false;
            bool right_entity = false;
            bool ahead_entity = false;

            /**
             * Checks if there is any obstacle.
             *
             * @return True if there is one.
             */
            bool Any() const;
        };

        /**
         * Changes the direction of a step that finds obstacles, to go around them.
         *
         * @param[in] is_player Whether the entity that moves is the player.
         * @param[in] slip Whether the player slides along the walls.
         * @param[in] obstacles What the step finds. At least one of them is expected.
         * @param[in,out] direction The step.
         * @return True to try again with the new direction, false if the entity is blocked.
         */
        static bool Steer(
          const bool is_player, const bool slip, const Obstacles& obstacles,
          Ogre::Vector2& direction
        );

    private:

        /**
         * Rotates a step around the vertical axis.
         *
         * @param[in,out] direction The step.
         * @param[in] degrees Angle to rotate it.
         */
        static void Turn(Ogre::Vector2& direction, const float degrees);

        /**
         * Turns an entity to face the direction it walks to.
         *
         * @param[in] entity The entity.
         * @param[in] direction The direction.
         */
        static void FaceDirection(Entity* entity, const Ogre::Vector2& direction);

        /**
         * Finds the triangle of a position an entity moves to, going through the borders of the
         * triangles from the one the entity is in. The triangle is stored in the entity.
         *
         * @param[in] entity Entity to check.
         * @param[in,out] position The position. Its height is set to the one of the walkmesh.
         * @return True if the edge of the walkmesh blocks the move, false otherwise. In battle
         * mode, always false.
         */
        bool CrossBorder(Entity* entity, Ogre::Vector3& position);

        /**
         * Checks for collisions of the entity with other entities.
         *
         * @param[in] entity Entity to check for collisions.
         * @param[in] position Position of the entity.
         * @return True if the entity is colliding with another, false otherwise. If the entity is
         * not solid, always false. If the manager is in battle mode, it will always return false.
         */
        bool CheckSolidCollisions(Entity* entity, Ogre::Vector3& position);

        /**
         * Walks an entity that is not on the walkmesh in a straight line to its destination.
         *
         * @param[in] entity The entity.
         * @param[in] direction The step of this frame.
         * @return True if the entity moved.
         */
        bool WalkOffWalkmesh(Entity* entity, const Ogre::Vector2& direction);

        /**
         * Shortens a step according to the slope of the triangle it starts in.
         *
         * @param[in] triangle The triangle.
         * @param[in] start_point Start of the step.
         * @param[in,out] direction The step.
         */
        void FollowSlope(
          const int triangle, const Ogre::Vector3& start_point, Ogre::Vector2& direction
        ) const;

        /**
         * Finds where a step ends, turning it to go around obstacles if needed.
         *
         * @param[in] entity The entity that moves.
         * @param[in] triangle The triangle the entity is in.
         * @param[in] direction The step.
         * @param[out] obstacles The obstacles that block the entity. None if it can move.
         * @return The end point of the step, with the height of the start point.
         */
        Ogre::Vector3 FindWay(
          Entity* entity, const int triangle, Ogre::Vector2 direction, Obstacles& obstacles
        );

        /**
         * Looks for obstacles around the end of a step.
         *
         * @param[in] entity The entity that moves.
         * @param[in] triangle The triangle the entity is in.
         * @param[in] end_point End of the step.
         * @param[in] direction The step.
         * @param[in] radius Distance from the end point to look for obstacles.
         * @return The obstacles found.
         */
        Obstacles ProbeAround(
          Entity* entity, const int triangle, const Ogre::Vector3& end_point,
          const Ogre::Vector2& direction, const float radius
        );

        /**
         * The manager of the entities.
         */
        const Manager& manager_;

        /**
         * The walkmesh of the field.
         */
        const Walkmesh& walkmesh_;

        /**
         * The entities of the field.
         */
        const std::vector<Entity*>& entities_;

        /**
         * The player entity.
         */
        Entity* const& player_;

        /**
         * Indicates if player control is locked.
         */
        const bool& player_locked_;

        /**
         * Indicates if the player slides along the walls.
         */
        const bool& player_slip_;

        /**
         * The direction the player is pressing.
         */
        const Ogre::Vector3& player_move_;

        /**
         * Function that checks the lines of the field.
         */
        LineCheck check_lines_;
};
