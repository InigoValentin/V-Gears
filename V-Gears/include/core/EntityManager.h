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
#include "Walkmesh.h"

/**
 * The entity manager.
 */
class EntityManager : public Ogre::Singleton<EntityManager>{

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
         * @param event[in] Event to handle.
         */
        void Input(const VGears::Event& event);

        /**
         * Updates the entities in the manager.
         */
        void Update();

        /**
         * Updates the entities in the manager with debug information.
         *
         * It's automatically called from {@see Update}.
         */
        void UpdateDebug();

        /**
         * Handles resizing events
         */
        void OnResize();

        /**
         * Clears the entity manager.
         */
        void Clear();

        /**
         * Pauses or resumes an entity scripts.
         *
         * @param paused[in] True to pause, false to resume.
         * @todo Verify the description.
         */
        void ScriptSetPaused(const bool paused);

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
         * @param name[in] Entity name.
         * @param file_name[in] Path to the entity model file.
         * @param position[in] Entity position in the map.
         * @param direction[in] Entity face direction.
         * @param index[in] Index of the entity on the map.
         */
        void AddEntity(
          const Ogre::String& name, const Ogre::String& file_name,
          const Ogre::Vector3& position, const Ogre::Degree& direction, int index
        );

        /**
         * Adds an entity to the manager.
         *
         * @param name[in] Entity name.
         * @param file_name[in] Path to the entity model file.
         * @param position[in] Entity position in the map.
         * @param rotation[in] Entity face direction.
         * @param scale[in] Entity scale.
         * @param root_orientation[in] Map orientation.
         * @param index[in] Index of the entity on the map.
         */
        void AddEntity(
          const Ogre::String& name, const Ogre::String& file_name,
          const Ogre::Vector3& position, const Ogre::Degree& rotation,
          const Ogre::Vector3& scale, const Ogre::Quaternion& root_orientation, int index
        );

        /**
         * Adds an entity to the manager.
         *
         * @param name[in] Entity name.
         * @param file_name[in] Path to the entity model file.
         * @param x[in] X coordinate of the entity position in the map.
         * @param y[in] Y coordinate of the entity position in the map.
         * @param z[in] Z coordinate of the entity position in the map.
         * @param direction[in] Entity face direction.
         * @param index[in] Index of the entity on the map.
         */
        void ScriptAddEntity(
          const char* name, const char* file_name,
          const float x, const float y, const float z, const float direction, int index
        );

        /**
         * Adds an entity trigger to the manager.
         *
         * A trigger is a line that does something when approached or crossed.
         *
         * @param name[in] Entity trigger name.
         * @param point1[in] One point of the trigger line.
         * @param point2[in] One point of the trigger line.
         * @param enabled[in] True to enable the trigger, false to leave it
         * disabled.
         */
        void AddEntityTrigger(
          const Ogre::String& name, const Ogre::Vector3& point1,
          const Ogre::Vector3& point2, const bool enabled
        );

        /**
         * Adds an entity point to the manager.
         *
         * @param name[in] Entity point name.
         * @param position[in] Entity point position.
         * @param rotation[in] The point orientation.
         */
        void AddEntityPoint(
          const Ogre::String& name, const Ogre::Vector3& position,
          const float rotation
        );

        /**
         * Adds an entity script to the manager.
         *
         * @param name[in] Entity script name.
         */
        void AddEntityScript(const Ogre::String& name);

        /**
         * Adds an entity script to the manager.
         *
         * @param name[in] Entity script name.
         */
        void ScriptAddEntityScript(const char* name);

        /**
         * Retrieves an entity by name.
         *
         * @param name[in] Name of the entity to retrieve.
         * @return The entity by the specified name, or nullptr if there is no
         * one.
         */
        Entity* GetEntity(const Ogre::String& name) const;

        /**
         * Retrieves an entity by it's index in the field.
         *
         * @param index[in] Index of the entity to retrieve.
         * @return The entity with the ID, or nullptr if there is no one.
         */
        Entity* GetEntityFromIndex(const int id) const;

        /**
         * Retrieves an entity by it's assigned character ID.
         *
         * @param id[in] Character ID of the entity to retrieve.
         * @return The entity assigned to the character, or nullptr if there
         * is no one.
         */
        Entity* GetEntityFromCharacterId(const char* id) const;

        /**
         * Retrieves an entity by name.
         *
         * @param name[in] Name of the entity to retrieve.
         * @return The entity by the specified name, or nullptr if there is no
         * one.
         */
        Entity* ScriptGetEntity(const char* name) const;

        /**
         * Retrieves an entity point by name.
         *
         * @param name[in] Name of the entity point to retrieve.
         * @return The entity point by the specified name, or nullptr if there
         * is no one.
         */
        EntityPoint* ScriptGetEntityPoint(const char* name) const;

        /**
         * Sets the playable entity.
         *
         * If no entities are found by name, no one will be assigned, the
         * previous playable entity will remain so, and no warning will be
         * issued.
         *
         * @param name[in] Name of the entity to make playable.
         */
        void ScriptSetPlayerEntity(const char* name);

        /**
         * Retrieves the playable entity.
         *
         * @return The playable entity.
         */
        Entity* ScriptGetPlayerEntity() const;

        /**
         * Unsets any playable entities.
         */
        void ScriptUnsetPlayerEntity();

        /**
         * Locks or unlocks player control of the playable entity.
         *
         * @param lock[in] True to lock, false to unlock.
         */
        void ScriptPlayerLock(const bool lock);

        /**
         * Sets the baseline rotation for the player controlled entity.
         *
         * @param rotation[in] Baseline rotation.
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
         * @param active[in] True to enable encounters, false to deactivate
         * them.
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
         * @param rate[in] The encounter rate (0-1).
         */
        void SetEncounterRate(float rate);

        /**
         * Starts a battle.
         *
         * @param formation[in] The enemy formation to fight.
         * @return True if the battle victory conditions are met, false
         * otherwise.
         * @todo Implement
         */
        bool StartBattleForResult(unsigned int formation);

        /**
         * Starts a battle.
         *
         * @param formation[in] The enemy formation to fight.
         * @todo Implement
         */
        void StartBattle(unsigned int formation);

        /**
         * Checks if a key is being pressed.
         *
         * @param key_code[in] The code of the key to test.
         * @return True if the key is being pressed, false otherwise.
         */
        bool IsKeyOn(unsigned int key_code);

        /**
         * Checks if a key is not being pressed.
         *
         * @param key_code[in] The code of the key to test.
         * @return False if the key is being pressed, true otherwise.
         */
        bool IsKeyOff(unsigned int key_code);

        /**
         * Assigns a character to an entity.
         *
         * @param entity_name[in] The entity name.
         * @param character name[in] The character name. Usually the same as
         * the entity.
         */
        void SetEntityToCharacter(const char* entity_name, const char* character_name);

    private:

        /**
         * Attaches an entity to the walkmesh.
         *
         * It sets the triangle from the entity position coordinates. To
         * account for multiple triangles on different levels, it uses only
         * the X and Y coordinates, and automatically sets the Z one to the
         * closest triangle.
         *
         * @param entity[in] Entity to attach.
         * @return True if the entity was assigned to a walkmesh triangle,
         * false if the entity is not in a triangle.
         */
        bool SetEntityOnWalkmesh(Entity* entity);

        /**
         * @todo Understand and document.
         *
         * @param entity[in] Entity to move.
         * @param speed[in] Movement speed.
         * @return @todo.
         */
        bool PerformWalkmeshMove(Entity* entity, const float speed);

        /**
         * Cheks if the entity is crossing a walkmesh triangle border.
         *
         * @param entity[in] Entity to check.
         * @param position[in] @todo.
         * @param move_vector[in] @todo.
         * @return True if the entity is crossing a triangle border, false
         * otherwise.
         */
        bool WalkmeshBorderCross(
          Entity* entity, Ogre::Vector3& position,
          const Ogre::Vector2& move_vector
        );

        /**
         * Checks for collisions of the entity with other entities.
         *
         * @param entity[in] Entity to check for collisions.
         * @param position[in] Position of the entity.
         * @return True if the entity is colliding with another, false
         * otherwise. If the entity is not solid, always false.
         */
        bool CheckSolidCollisions(Entity* entity, Ogre::Vector3& position);

        /**
         * Sets an entity direction
         *
         * @param entity[in] Entity whose direction is to be set.
         * @param vector[in] Direction vector.
         */
        void SetEntityDirectionByVector(
          Entity* entity, const Ogre::Vector2& vector
        );

        /**
         * Checks for entity triggers near a entity.
         *
         * If there are triggers, and the conditions are met, the appropriate
         * trigger function will be added to the queue. This must be tested
         * every time an entity moves. If the entity is not solid, it will do
         * nothing.
         *
         * @param entity[in] Entity to check for nearby triggers.
         * @param position[in] The position of the entity.
         */
        void CheckTriggers(Entity* entity, const Ogre::Vector3& position);

        /**
         * Checks if an entity can be interacted.
         *
         * It checks if there are entities that can be interacted with from the
         * players current position and orientation. If there are, the most
         * appropriate one is selected and, if it has an on_interact script, it
         * is run.
         */
        void CheckEntityInteract();

        /**
         * @todo Understand and document.
         *
         * @param entity[in] @todo.
         */
        void SetNextOffsetStep(Entity* entity);

        /**
         * @todo Understand and document.
         *
         * @param entity[in] @todo.
         */
        void SetNextTurnStep(Entity* entity);

        /**
         * Calculates and sets the next position during a linear movement.
         *
         * @param entity[in] The moving entity.
         */
        void SetNextLinearStep(Entity* entity);

        /**
         * Calculates and sets the next position during a jump.
         *
         * @param entity[in] The jump entity.
         */
        void SetNextJumpStep(Entity* entity);

        /**
         * @todo Understand and document.
         */
        void SetNextScrollStep();

        /**
         * Indicates if the script execution is paused.
         */
        bool paused_;

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
         * The list of entities.
         */
        std::vector<Entity*> entity_;

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
