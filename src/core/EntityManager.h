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
#include "Background2D.h"
#include "Entity.h"
#include "EntityPoint.h"
#include "EntityTrigger.h"
#include "Event.h"
#include "Manager.h"
#include "Walkmesh.h"

/**
 * The entity manager.
 */
class EntityManager : public Manager, public Ogre::Singleton<EntityManager>{

    public:

        /**
         * Constructor.
         */
        EntityManager();

        /**
         * Destructor.
         */
        virtual ~EntityManager();

        /**
         * Handles an input event.
         *
         * @param[in] event Event to handle.
         */
        void Input(const VGears::Event& event) override;

        /**
         * Updates the entities in the manager with debug information.
         *
         * It's automatically called from {@see Update}.
         */
        void UpdateDebug() override;

        /**
         * Handles resizing events
         */
        void OnResize() override;

        /**
         * Clears all field information in the entity manager.
         *
         * It clears the background, the walkmesh, any pending actions and all the field entities.
         */
        void ClearField() override;

        /**
         * Clears all battle information in the entity manager.
         *
         * It clears any pending actions and all the battle entities.
         */
        void ClearBattle() override;

        /**
         * Clears all world map information in the entity manager.
         *
         * It clears the background, the walkmesh, any pending actions and all the world entities.
         */
        void ClearWorld() override;

        /**
         * Retrieves the walkmesh.
         *
         * @return The walkmesh.
         */
        Walkmesh* GetWalkmesh();

        /**
         * Retrieves the 2D background.
         *
         * @return The 2D background.
         */
        Background2D* GetBackground2D();

        /**
         * Adds an entity to the manager.
         *
         * The entity will be added to the EntityManager current module entity list.
         *
         * @param[in] name Entity name.
         * @param[in] file_name Path to the entity model file.
         * @param[in] position Entity position.
         * @param[in] direction Entity face direction.
         * @param[in] index Index of the entity on the manager.
         */
        void AddEntity(
          const Ogre::String& name, const Ogre::String& file_name,
          const Ogre::Vector3& position, const Ogre::Degree& direction, int index
        );

        /**
         * Adds a field entity to the manager.
         *
         * The entity will be added to the EntityManager current module entity list.
         *
         * @param[in] name Entity name.
         * @param[in] file_name Path to the entity model file.
         * @param[in] position Entity position.
         * @param[in] rotation Entity face direction.
         * @param[in] scale Entity scale.
         * @param[in] root_orientation Entity orientation. Unused for battle and world map entities.
         * @param[in] index Index of the entity on the manager.
         */
        void AddEntity(
          const Ogre::String& name, const Ogre::String& file_name,
          const Ogre::Vector3& position, const Ogre::Degree& rotation,
          const Ogre::Vector3& scale, const Ogre::Quaternion& root_orientation, const int index
        );

        /**
         * Adds an entity to the manager.
         *
         * Simplified version of {@see AddEntity}, to be called from Lua scripts.
         *
         * @param[in] name Entity name.
         * @param[in] file_name Path to the entity model file.
         * @param[in] x X coordinate of the entity position in the map.
         * @param[in] y Y coordinate of the entity position in the map.
         * @param[in] z Z coordinate of the entity position in the map.
         * @param[in] direction Entity face direction.
         * @param[in] index Index of the entity on the manager.
         */
        void ScriptAddEntity(
          const char* name, const char* file_name,
          const float x, const float y, const float z, const float direction, int index
        );

        /**
         * Adds an entity trigger to the manager.
         *
         * A trigger is a line that does something when approached or crossed. If the Battle manager
         * is in battle mode, nothing will be done.
         *
         * @param[in] name Entity trigger name.
         * @param[in] point1 One point of the trigger line.
         * @param[in] point2 One point of the trigger line.
         * @param[in] enabled True to enable the trigger, false to leave it disabled.
         */
        void AddEntityTrigger(
          const Ogre::String& name, const Ogre::Vector3& point1,
          const Ogre::Vector3& point2, const bool enabled
        );

        /**
         * Adds an entity point to the manager.
         *
         * If the Battle manager is in battle mode, nothing will be done.
         *
         * @param[in] name Entity point name.
         * @param[in] position Entity point position.
         * @param[in] rotation The point orientation.
         */
        void AddEntityPoint(
          const Ogre::String& name, const Ogre::Vector3& position, const float rotation
        );

        /**
         * Adds an entity script to the manager.
         *
         * @param[in] name Entity script name.
         */
        void AddEntityScript(const Ogre::String& name);

        /**
         * Adds an entity script to the manager.
         *
         * @param[in] name Entity script name.
         */
        void ScriptAddEntityScript(const char* name);

        /**
         * Retrieves an entity by name.
         *
         * Only entities of the current active module will be searched.
         *
         * @param[in] name Name of the entity to retrieve.
         * @return The entity by the specified name, or nullptr if there is no one.
         */
        Entity* GetEntity(const Ogre::String& name) const;

        /**
         * Retrieves an entity by it's index in the field.
         *
         * Only entities of the current active module will be searched.
         *
         * @param[in] id Index of the entity to retrieve.
         * @return The entity with the ID, or nullptr if there is no one.
         */
        Entity* GetEntityFromIndex(const int id) const;

        /**
         * Retrieves an entity by it's assigned character ID.
         *
         * Only entities of the current active module will be searched.
         *
         * @param[in] id Character ID of the entity to retrieve.
         * @return The entity assigned to the character, or nullptr if there is no one.
         */
        Entity* GetEntityFromCharacterId(const int id) const;

        /**
         * Retrieves an entity by name.
         *
         * Only entities of the current active module will be searched.
         *
         * @param[in] name Name of the entity to retrieve.
         * @return The entity by the specified name, or nullptr if there is no one.
         */
        Entity* ScriptGetEntity(const char* name) const;

        /**
         * Retrieves an entity point by name.
         *
         * @param[in] name Name of the entity point to retrieve.
         * @return The entity point by the specified name, or nullptr if there is no one or the
         * manager is in battle mode.
         */
        EntityPoint* ScriptGetEntityPoint(const char* name) const;

        /**
         * Sets the playable entity.
         *
         * If no entities are found by name, no one will be assigned, the previous playable entity
         * will remain so, and no warning will be issued. Also, if the EntityManager is in battle
         * mode, nothing will be done.
         *
         * @param[in] name Name of the entity to make playable.
         */
        void ScriptSetPlayerEntity(const char* name);

        /**
         * Retrieves the playable entity.
         *
         * @return The playable entity, or nullptr if it's not defined or the EntityManager is in
         * battle mode.
         */
        Entity* ScriptGetPlayerEntity() const;

        /**
         * Unsets any playable entities.
         *
         * If the EntityManager is in battle mode, nothing will be done.
         */
        void ScriptUnsetPlayerEntity();

        /**
         * Locks or unlocks player control of the playable entity.
         *
         * If the EntityManager is in battle mode, nothing will be done.
         *
         * @param[in] lock True to lock, false to unlock.
         */
        void ScriptPlayerLock(const bool lock);

        /**
         * Sets the baseline rotation for the player controlled entity.
         *
         * If the EntityManager is in battle mode, nothing will be done.
         *
         * @param[in] rotation Baseline rotation.
         * @todo Verify this description.
         */
        void SetPlayerMoveRotation(const Ogre::Radian rotation);

        /**
         * Checks if random battle encounters are active in the field.
         *
         * @return True if random battles can occur, false otherwise.
         */
        bool GetRandomEncounters();

        /**
         * Enables or disables random encounters in the field.
         *
         * @param[in] active True to enable encounters, false to deactivate them.
         */
        void SetRandomEncounters(bool active);

        /**
         * Get the encounter rate for the field.
         *
         * @return The encounter rate, between 0 and 1 (both included).
         */
        float GetEncounterRate();

        /**
         * Sets the battle encounter rate.
         *
         * @param[in] rate The encounter rate (0-1).
         */
        void SetEncounterRate(float rate);

        /**
         * Checks if a key is being pressed.
         *
         * @param[in] key_code The code of the key to test.
         * @return True if the key is being pressed, false otherwise.
         */
        bool IsKeyOn(unsigned int key_code);

        /**
         * Checks if a key is not being pressed.
         *
         * @param[in] key_code The code of the key to test.
         * @return False if the key is being pressed, true otherwise.
         */
        bool IsKeyOff(unsigned int key_code);

        /**
         * Assigns a character to an entity.
         *
         * Only entities of the currently active module can be assigned
         *
         * @param[in] entity_name The entity name.
         * @param[in] char_id The character ID.
         */
        void SetEntityToCharacter(const char* entity_name, unsigned int char_id);

        /**
         * Retrieves the entity associated to the 3D background.
         *
         * @return The 3D background entity. nullptr if not set, of if the manager is in field mode.
         */
        Entity* GetBackground3D() const;

        /**
         * Sets the 3D model for the background.
         *
         * @param[in] name name for the background entity, for debugging purposes only.
         * @param[in] file_name Path to the 3D model file.
         */
        void SetBackground3D(const Ogre::String& name, const Ogre::String& file_name);

    private:

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
         * Scale factor for battle background models.
         */
        static const float SCENE_SCALE;

        /**
         * Entity ID for battle background entity.
         */
        static const unsigned int BATTLE_BACKGROUND_ID;

        /**
         * Entity ID for world map background entity.
         */
        static const unsigned int  WORLD_MAP_BACKGROUND_ID;

        /**
         * Updates the field entities in the manager.
         */
        void UpdateField() override;

        /**
         * Updates the battle entities in the manager.
         */
        void UpdateBattle() override;

        /**
         * Updates the world map entities in the manager.
         */
        void UpdateWorld() override;

        /**
         * Adds a field entity to the manager.
         *
         * @param[in] name Entity name.
         * @param[in] file_name Path to the entity model file.
         * @param[in] position Entity position in the map.
         * @param[in] rotation Entity face direction.
         * @param[in] scale Entity scale.
         * @param[in] root_orientation Map orientation. Unused for world map entities.
         * @param[in] index Index of the entity on the map.
         */
        void AddFieldOrWorldMapEntity(
          const Ogre::String& name, const Ogre::String& file_name,
          const Ogre::Vector3& position, const Ogre::Degree& rotation,
          const Ogre::Vector3& scale, const Ogre::Quaternion& root_orientation, const int index
        );

        /**
         * Adds a battle entity to the manager.
         *
         * If the manager is not in the battle module, it will do nothing.
         *
         * @param[in] name Entity name.
         * @param[in] file_name Path to the entity model file.
         * @param[in] position Entity position.
         * @param[in] orientation Entity face direction.
         * @param[in] scale Entity scale.
         * @param[in] root_orientation The node orientation.
         * @param[in] index Index of the entity.
         */
        void AddBattleEntity(
          const Ogre::String& name, const Ogre::String& file_name,
          const Ogre::Vector3& position, const Ogre::Degree& orientation,
          const Ogre::Vector3& scale, const Ogre::Quaternion& root_orientation, const int index
        );

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
        bool SetEntityOnWalkmesh(Entity* entity);

        /**
         * Moves an entity in the walkmesh.
         *
         * If the manager is in battle mode, it will do nothing.
         *
         * @param[in] entity Entity to move.
         * @param[in] speed Movement speed.
         * @return True if the movement was possible and the entity was moved, false otherwise.
         * If the manager is in battle mode, it will always return false.
         */
        bool PerformWalkmeshMove(Entity* entity, const float speed);

        /**
         * Checks if the entity is crossing a walkmesh triangle border.
         *
         * @param[in] entity Entity to check.
         * @param[in] position The position. @todo document more.
         * @param[in] move_vector The move vector. @todo document more.
         * @return True if the entity is crossing a triangle border, false otherwise. If the manager
         * is in battle mode, it will always return false.
         */
        bool WalkmeshBorderCross(
          Entity* entity, Ogre::Vector3& position, const Ogre::Vector2& move_vector
        );

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
         * Sets an entity direction
         *
         * @param[in] entity Entity whose direction is to be set.
         * @param[in] vector Direction vector.
         */
        void SetEntityDirectionByVector(Entity* entity, const Ogre::Vector2& vector);

        /**
         * Checks for entity triggers near a entity.
         *
         * If there are triggers, and the conditions are met, the appropriate trigger function will
         * be added to the queue. This must be tested every time an entity moves. If the entity is
         * not the playable character, is not solid or is locked, it will do nothing. If the manager
         * is in battle mode, it will do nothing.
         *
         * @param[in] entity Entity to check for nearby triggers.
         * @param[in] position The position of the entity.
         */
        void CheckTriggers(Entity* entity, const Ogre::Vector3& position);

        /**
         * Checks if an entity can be interacted.
         *
         * It checks if there are entities that can be interacted with from the players current
         * position and orientation. If there are, the most appropriate one is selected and, if it
         * has an on_interact script, it is run. If the manager is in battle mode, it will do
         * nothing.
         */
        void CheckEntityInteract();

        /**
         * Calculates the next offset step for an entity.
         *
         * @param[in] entity The entity.
         */
        void SetNextOffsetStep(Entity* entity);

        /**
         * Calculates the next turn step for an entity
         *
         * @param[in] entity The entity.
         */
        void SetNextTurnStep(Entity* entity);

        /**
         * Calculates and sets the next position during a linear movement.
         *
         * If the manager is in battle mode, it will do nothing
         *
         * @param[in] entity The moving entity.
         */
        void SetNextLinearStep(Entity* entity);

        /**
         * Calculates and sets the next position during a jump.
         *
         * If the manager is in battle mode, it will do nothing
         *
         * @param[in] entity The jump entity.
         */
        void SetNextJumpStep(Entity* entity);

        /**
         * @todo Understand and document.
         */
        void SetNextScrollStep();

        /**
         * The map walkmesh.
         */
        Walkmesh walkmesh_;

        /**
         * The map background
         */
        Background2D background_2d_;

        /**
         * The entity table name.
         */
        Ogre::String entity_table_name_;

        /**
         * The list of field or world entities.
         */
        std::vector<Entity*> entity_;

        /**
         * The list of battle entities.
         */
        std::vector<Entity*> battle_entity_;

        /**
         * A 3D background.
         */
        Entity* background_3d_;

        /**
         * The player controlled entity.
         */
        Entity* player_entity_;

        /**
         * The playable entity current movement indicator.
         *
         * Applies only for manual movement.
         */
        Ogre::Vector3 player_move_;

        /**
         * The playable entity current movement turn indicator.
         *
         * Applies only for manual movement.
         */
        Ogre::Radian player_move_rotation_;

        /**
         * Indicates if player control is locked.
         */
        bool player_lock_;

        /**
         * Indicates if the playable is being moved by running.
         *
         * Applies only for manual movement.
         */
        bool player_run_;

        /**
         * List of triggers.
         */
        std::vector<EntityTrigger*> entity_triggers_;

        /**
         * List of points.
         */
        std::vector<EntityPoint*> entity_points_;

        /**
         * List of scripts.
         */
        std::vector<Ogre::String> entity_scripts_;

        /**
         * The scene node.
         */
        Ogre::SceneNode* scene_node_;

        /**
         * @todo Understand and document.
         */
        Ogre::Entity* grid_;

        /**
         * @todo Understand and document.
         */
        Ogre::Entity* axis_;

        /**
         * Indicates if random encounters happen in the map.
         */
        bool random_encounters_;

        /**
         * The encounter rate of the map.
         */
        float encounter_rate_;
};
