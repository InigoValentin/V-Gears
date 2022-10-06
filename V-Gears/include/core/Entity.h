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

#include <OgreString.h>
#include "EntityCollision.h"
#include "EntityDirection.h"
#include "ScriptManager.h"

/**
 * Action types.
 */
enum ActionType{

    /**
     * No action.
     */
    AT_NONE,

    /**
     * Linear action.
     *
     * It starts and ends at full speed.
     */
    AT_LINEAR,

    /**
     * Smooth action.
     *
     * The action speed steadily increases when started, and it steadily
     * decreases before the end.
     */
    AT_SMOOTH
};

/**
 * The direction for an entity turn.
 */
enum TurnDirection{

    /**
     * Turn clockwise.
     */
    TD_CLOCKWISE,

    /**
     * Turn anticlockwise.
     */
    TD_ANTICLOCKWISE,

    /**
     * Choose direction automatically.
     *
     * The direction in which the turn is shorter will be selected.
     */
    TD_CLOSEST
};

/**
 * Linear movement modes.
 */
enum LinearMovement{

    /**
     * Move down.
     */
    LM_UP_TO_DOWN,

    /**
     * Move up.
     */
    LM_DOWN_TO_UP,

    /**
     * Move right.
     */
    LM_LEFT_TO_RIGHT,

    /**
     * Move left.
     */
    LM_RIGHT_TO_LEFT
};

/**
 * Any entity in a field.
 */
class Entity{

    public:

        /**
         * Entity animation states.
         */
        enum AnimationState{

            /**
             * An animation has been requested.
             */
            REQUESTED_ANIMATION,

            /**
             * An animation is set to play automatically.
             */
            AUTO_ANIMATION
        };

        /**
         * Types of animations.
         */
        enum AnimationPlayType{

            /**
             * Default animation mode.
             *
             * @todo Same as PLAY_ONCE?
             */
            PLAY_DEFAULT,

            /**
             * Play the animation once, then stop.
             */
            PLAY_ONCE,

            /**
             * Play an animation in a continous loop.
             */
            PLAY_LOOPED
        };

        /**
         * Entity state.
         */
        enum State{

            /**
             * No state.
             *
             * The entity has not been placed or it has been removed.
             */
            NONE,

            /**
             * The entity is attached to the walkmesh.
             */
            WALKMESH,

            /**
             * The entity is in the middle of a linear movement.
             */
            LINEAR,

            /**
             * The entity is in the middle of a jump.
             */
            JUMP,

            /**
             * The entity has completed an action and needs to be reattached
             * to the walkmesh before continuing execution.
             */
            NEEDS_TO_REATTACH
        };

        /**
         * Constructor.
         *
         * @param[in] name Entity name.
         * @param[in] node Scene node to which the entity should be attached.
         */
        Entity(const Ogre::String& name, Ogre::SceneNode* node);

        /**
         * Destructor.
         */
        virtual ~Entity();

        /**
         * Updates the entity status.
         */
        virtual void Update();

        /**
         * Updates the entity status with debug information.
         */
        virtual void UpdateDebug();

        /**
         * Retrieves the entity name.
         *
         * @return The entity name.
         */
        const Ogre::String& GetName() const;

        /**
         * Sets the entity position.
         *
         * @param[in] position Entity's new position.
         */
        void SetPosition(const Ogre::Vector3& position);

        /**
         * Sets the entity position.
         *
         * It also resets the walkmesh triangle to reattach entity to walkmesh
         * again if needed.
         *
         * @param[in] x Entity's new position X coordinate.
         * @param[in] y Entity's new position Y coordinate.
         * @param[in] z Entity's new position Z coordinate.
         */
        void ScriptSetPosition(const float x, const float y, const float z);

        /**
         * Retrieves the entity position.
         *
         * @return The entity position.
         */
        const Ogre::Vector3 GetPosition() const;

        /**
         * Informs the script manager of the entity position.
         *
         * @todo Im not really sure what this actually does.
         */
        void ScriptGetPosition() const;

        /**
         * Sets the entity position.
         *
         * @param[in] position The entity new position.
         * @todo Offset to what?
         */
        void SetOffset(const Ogre::Vector3& position);

        /**
         * Retrieves the entity position.
         *
         * @return The entity position.
         * @todo Offset to what?
         */
        const Ogre::Vector3 GetOffset() const;

        /**
         * Sets the entity rotation.
         *
         * @param[in] rotation The entity rotation, in degrees.
         */
        void SetRotation(const Ogre::Degree& rotation);

        /**
         * Sets the entity rotation.
         *
         * @param[in] rotation The entity rotation (0-360).
         */
        void ScriptSetRotation(const float rotation);

        /**
         * Retrieves the entity rotation.
         *
         * @return The entity rotation, in degrees.
         */
        Ogre::Degree GetRotation() const;

        /**
         * Retrieves the entity rotation.
         *
         * @return The entity rotation (0-360).
         */
        float ScriptGetRotation() const;

        /**
         * Sets the entity scale.
         *
         * @param[in] scale Three dimensional scale.
         */
        virtual void setScale(const Ogre::Vector3 &scale);

        /**
         * Sets the entity index in the field.
         *
         * @param[in] index Index of the entity.
         */
        void SetIndex(const int index);

        /**
         * Retrieves the entity index in the field.
         *
         * @return Index of the entity.
         */
        int GetIndex();

        /**
         * Sets the entity's absolute orientation.
         *
         * @param[in] root_orientation The entity's new orientation.
         */
        virtual void setRootOrientation(
          const Ogre::Quaternion &root_orientation
        );

        /**
         * Retrieves the entity's height.
         *
         * @return The entity's height.
         */
        float GetHeight() const;

        /**
         * Sets the entity's solid radius.
         *
         * The solid radius is used to detect collisions.
         *
         * @param[in] radius The solid radius.
         * @todo Is it in pixels?
         */
        void SetSolidRadius(const float radius);

        /**
         * Retrieves the entity's solid radius.
         *
         * The solid radius is used to detect collisions.
         *
         * @return The solid radius.
         * @todo Is it in pixels?
         */
        float GetSolidRadius() const;

        /**
         * Makes the entity solid or non-solid.
         *
         * Solid entities can produce collisions.
         *
         * @param[in] solid True to make the entity solid, false otherwise.
         */
        void SetSolid(const bool solid);

        /**
         * Checks if the entity is solid.
         *
         * Solid entities can produce collisions.
         *
         * @return True if the entity is solid, false otherwise.
         */
        bool IsSolid() const;

        /**
         * Sets the entity talk radius.
         *
         * The talk radius is the maximum distance at which an entity can be
         * talked to or interacted with.
         *
         * @param[in] radius The talk radius.
         * @todo Is it in pixels?
         */
        void SetTalkRadius(const float radius);

        /**
         * Retrieves the entity talk radius.
         *
         * The talk radius is the maximum distance at which an entity can be
         * talked to or interacted with.
         *
         * @return The talk radius.
         * @todo Is it in pixels?
         */
        float GetTalkRadius() const;

        /**
         * Sets an entity as talkable or non-talkable.
         *
         * Talkable units can be talked to or interacted with.
         *
         * @param[in] talkable True to make the entity talkable, false to make
         * it non-talkable.
         */
        void SetTalkable(const bool talkable);

        /**
         * Checks if an entity is talkable or non-talkable.
         *
         * Talkable units can be talked to or interacted with.
         *
         * @return True if the entity is talkable, false if it's not.
         */
        bool IsTalkable() const;

        /**
         * Makes the entity visible or invisible.
         *
         * Invisible entities can't be interacted with.
         *
         * @param[in] visible True to make the unit visible, false to make it
         * invisible.
         */
        virtual void SetVisible(const bool visible) = 0;

        /**
         * Checks if the entity is visible or invisible.
         *
         * Invisible entities can't be interacted with.
         *
         * @return True if the unit is visible, false if it's invisible.
         */
        virtual bool IsVisible() const = 0;

        /**
         * Sets the entity's state.
         *
         * @param[in] state The entity's state.
         */
        void SetState(const State state);

        /**
         * Retrieves the entity's state.
         *
         * @return The entity's state.
         */
        State GetState() const;

        /**
         * Sets the entity's automatic movement speed.
         *
         * @param[in] speed Automatic movement speed.
         * @todo Describe where this speed is used, and max and mins or
         * references.
         */
        void SetMoveAutoSpeed(const float speed);

        /**
         * Retrieves the entity's automatic movement speed.
         *
         * @return Automatic movement speed.
         * @todo Describe where this speed is used, and max and mins or
         * references.
         */
        float GetMoveAutoSpeed() const;

        /**
         * Sets the entity's walking movement speed.
         *
         * @param[in] speed Walking speed.
         * @todo Describe where this speed is used, and max and mins or
         * references.
         */
        void SetMoveWalkSpeed(const float speed);

        /**
         * Retrieves the entity's walking speed.
         *
         * @return Walking speed.
         * @todo Describe where this speed is used, and max and mins or
         * references.
         */
        float GetMoveWalkSpeed() const;

        /**
         * Sets the entity's running movement speed.
         *
         * @param[in] speed Running speed.
         * @todo Describe where this speed is used, and max and mins or
         * references.
         */
        void SetMoveRunSpeed(const float speed);

        /**
         * Retrieves the entity's running speed.
         *
         * @return Running speed.
         * @todo Describe where this speed is used, and max and mins or
         * references.
         */
        float GetMoveRunSpeed() const;

        /**
         * Sets the entity's movement destination position.
         *
         * @param[in] target The destination position.
         */
        void SetMovePosition(const Ogre::Vector3& target);

        /**
         * Retrieves the entity's movement destination position.
         *
         * @return The destination position.
         */
        const Ogre::Vector3& GetMovePosition() const;

        /**
         * Retrieves the distance to destination.
         *
         * It's the distance between the entity's current position and it's
         * current movement destination point.
         *
         * @return The distance to destination.
         * @todo Verify this description. Also, indicate units (pixels?).
         */
        float GetMoveStopDistance() const;

        /**
         * Sets the destination triangle in the walkmesh.
         *
         * Sets the units destination to one of the walkmesh triangles. The
         * movement will stop once the unit enters the triangle.
         *
         * @param[in] triangle Destination triangle in the walkmesh.
         */
        void SetMoveTriangleId(const int triangle);

        /**
         * Sets the destination triangle in the walkmesh.
         *
         * @return Destination triangle in the walkmesh.
         */
        int GetMoveTriangleId() const;

        /**
         * Enables or disables autorotation while the entity is moving.
         *
         * @param[in] rotate If true, the entity will rotate automatically
         * while moving. If false, the entity will not rotate.
         */
        void SetMoveAutoRotation(const bool rotate);

        /**
         * Checks if the unit can autorotate while it's moving.
         *
         * @return If true, the entity will rotate automatically while moving.
         * If false, the entity will not rotate.
         */
        bool GetMoveAutoRotation() const;

        /**
         * Enables or disables autoanimation while the entity is moving.
         *
         * @param[in] animate If true, the entity will animate automatically
         * while moving. If false, the entity will not animate.
         */
        void SetMoveAutoAnimation(const bool animate);

        /**
         * Checks if the unit can autoanimate while it's moving.
         *
         * @return If true, the entity will animate automatically while moving.
         * If false, the entity will not animate.
         */
        bool GetMoveAutoAnimation() const;

        /**
         * Retrieves the entity's walk animation name.
         *
         * @return The walk animation name.
         */
        const Ogre::String& GetMoveAnimationWalkName() const;

        /**
         * Retrieves the entity's run animation name.
         *
         * @return The run animation name.
         */
        const Ogre::String& GetMoveAnimationRunName() const;

        /**
         * Makes the entity move to a point in the map.
         *
         * @param[in] x X coordinate of the destination point.
         * @param[in] y Y coordinate of the destination point.
         */
        void ScriptMoveToPosition(const float x, const float y);

        /**
         * Makes the unit move towards another in the map.
         *
         * @param[in] entity entity to move towards.
         */
        void ScriptMoveToEntity(Entity* entity);

        /**
         * Waits for entity's movement to end.
         *
         * @return Always -1.
         */
        int ScriptMoveSync();

        /**
         * Waits for the jump to finish.
         *
         * @return Always -1.
         */
        int ScriptJumpSync();

        /**
         * Cancels the entity's current movement.
         *
         * It also clears the movement sync queue.
         */
        void UnsetMove();

        /**
         * Linearly moves the entity.
         *
         * @param[in] x X coordinate of the destination point.
         * @param[in] y Y coordinate of the destination point.
         * @param[in] z Z coordinate of the destination point.
         * @param[in] movement Movement direction.
         * @param[in] animation Movement animation.
         * @param[in] orientation Orientation during the animation.
         * @param[in] dest_triangle Triangle to place the entity after the
         * linear movement.
         */
        void ScriptLinearToPosition(
          const float x, const float y, const float z, const LinearMovement movement,
          const char* animation, const float orientation, const int dest_triangle
        );

        /**
         * Adds the unit linear movement to the sync queue.
         *
         * @return Always -1.
         * @todo Properly describe this.
         */
        int ScriptLinearSync();

        /**
         * Linearly moves the entity.
         *
         * @param[in] end Destination point.
         * @param[in] movement Movement direction.
         * @param[in] animation Movement animation.
         * @param[in] orientation Orientation during the animation.
         * @param[in] dest_triangle Triangle to place the entity after the
         * linear movement.
         */
        void SetLinear(
          const Ogre::Vector3& end, const LinearMovement movement,
          const Ogre::String& animation, const float orientation, const int dest_triangle
        );

        /**
         * Cancels the entity's current linear movement.
         *
         * It also clears the movement sync queue.
         */
        void UnsetLinear();

        /**
         * Retrieves the entity's current linear movement.
         *
         * @return Current linear movement.
         */
        LinearMovement GetLinearMovement() const;

        /**
         * Retrieves the starting point of the current linear movement.
         *
         * @return Starting point.
         */
        const Ogre::Vector3& GetLinearStart() const;

        /**
         * Retrieves the ending point of the current linear movement.
         *
         * @return Ending point.
         */
        const Ogre::Vector3& GetLinearEnd() const;

        /**
         * Retrieves the ending triangle of the current linear movement.
         *
         * @return Ending triangle.
         */
        const int GetLinearDestTriangle() const;

        /**
         * Makes the unit jump to a point in the field.
         *
         * @param[in] x X coordinate of the jump destination point.
         * @param[in] y Y coordinate of the jump destination point.
         * @param[in] z Maximum height of the jump.
         * @param[in] seconds Jump duration.
         * @param[in] dest_triangle Triangle to place the entity after the
         * jump.
         */
        void ScriptJumpToPosition(
          const float x, const float y, const float z, const float seconds, const int dest_triangle
        );

        /**
         * Makes the unit jump to a point in the field.
         *
         * @param[in] jump_to The jump destination point.
         * @param[in] seconds Jump duration.
         * @param[in] dest_triangle Triangle to place the entity after the
         * jump.
         */
        void SetJump(const Ogre::Vector3& jump_to, const float seconds, const int dest_triangle);

        /**
         * Cancels the entity's current jump.
         *
         * It also clears the movement sync queue.
         */
        void UnsetJump();

        /**
         * Retrieves the starting point of the current jump.
         *
         * @return Starting point.
         */
        const Ogre::Vector3& GetJumpStart() const;

        /**
         * Retrieves the ending point of the current jump.
         *
         * @return Ending point.
         */
        const Ogre::Vector3& GetJumpEnd() const;

        /**
         * Gets the total duration of the jump.
         *
         * @return Jump total duration, in seconds.
         */
        float GetJumpSeconds() const;

        /**
         * Sets the current duration of the jump.
         *
         * @param[in] seconds Jump current duration, in seconds.
         */
        void SetJumpCurrentSeconds(const float seconds);

        /**
         * Gets the current duration of the jump.
         *
         * @return Jump current duration, in seconds.
         */
        float GetJumpCurrentSeconds() const;

        /**
         * Retrieves the ending triangle of the current jump.
         *
         * @return Ending triangle.
         */
        const int GetJumpDestTriangle() const;

        /**
         * @todo Understand and document.
         *
         * @param[in] x X coordinate of the destination point.
         * @param[in] y Y coordinate of the destination point.
         * @param[in] z Z coordinate of the destination point.
         * @param[in] type Type of action.
         * @param[in] seconds Duration of the action, in seconds.
         */
        void ScriptOffsetToPosition(
          const float x, const float y, const float z,
          const ActionType type, const float seconds
        );

        /**
         * @todo Understand and document.
         *
         * @return Always -1.
         */
        int ScriptOffsetSync();

        /**
         * @todo Understand and document.
         */
        void UnsetOffset();

        /**
         * @todo Understand and document.
         *
         * @return Starting position.
         */
        const Ogre::Vector3& GetOffsetPositionStart() const;

        /**
         * @todo Understand and document.
         *
         * @return Ending position.
         */
        const Ogre::Vector3& GetOffsetPositionEnd() const;

        /**
         * @todo Understand and document.
         *
         * @return The action type.
         */
        ActionType GetOffsetType() const;

        /**
         * @todo Understand and document.
         *
         * @return Action total duration in seconds.
         */
        float GetOffsetSeconds() const;

        /**
         * @todo Understand and document.
         *
         * @param[in] seconds Action current duration in seconds.
         */
        void SetOffsetCurrentSeconds(const float seconds);

        /**
         * @todo Understand and document.
         *
         * @return Action current duration in seconds.
         */
        float GetOffsetCurrentSeconds() const;

        /**
         * Makes the entity turn to a fixed direction.
         *
         * @param[in] direction Final direction to turn the entity's to.
         * @param[in] turn_direction Direction of the turn.
         * @param[in] turn_type Turn mode.
         * @param[in] seconds Total turn duration, in seconds.
         */
        void ScriptTurnToDirection(
          const float direction, const TurnDirection turn_direction,
          const ActionType turn_type, const float seconds
        );

        /**
         * Makes the entity turn towards another entity.
         *
         * @param[in] entity Entity to turn to.
         * @param[in] turn_direction Direction of the turn.
         * @param[in] seconds Total turn duration, in seconds.
         */
        void ScriptTurnToEntity(
          Entity* entity, const TurnDirection turn_direction,
          const float seconds
        );

        /**
         * Adds the entity's turn to the sync queue.
         *
         * @return Always -1.
         * @todo Properly describe this.
         */
        int ScriptTurnSync();

        /**
         * Makes the entity turn towards a point or another entity.
         *
         * @param[in] direction_to Final direction to turn the entity's to.
         * @param[in] entity Entity to turn to.
         * @param[in] turn_direction Direction of the turn.
         * @param[in] turn_type Turn mode.
         * @param[in] seconds Total turn duration, in seconds.
         * @todo What if the point
         */
        void SetTurn(
          const Ogre::Degree& direction_to, Entity* entity, const TurnDirection turn_direction,
          const ActionType turn_type, const float seconds
        );

        /**
         * Cancels the entity's current jump.
         *
         * It also clears the movement sync queue.
         */
        void UnsetTurn();

        /**
         * Calculates the turn angle.
         *
         * If the turn direction is {@see TD_CLOSEST}, the result is the
         * smallest angle between the two orientations. Otherwise, is the angle
         * in the specified turn direction.
         *
         * @param[in] start Starting angle.
         * @param[in] end Ending angle.
         * @return[in] Calculated turn angle.
         */
        Ogre::Degree CalculateTurnAngle(const Ogre::Degree& start, const Ogre::Degree& end) const;

        /**
         * Retrieves the turn staring orientation.
         *
         * @return The turn starting orientation.
         */
        Ogre::Degree GetTurnDirectionStart() const;

        /**
         * Retrieves the turn ending orientation.
         *
         * @return The turn ending orientation.
         */
        Ogre::Degree GetTurnDirectionEnd() const;

        /**
         * Retrieves the turn type.
         *
         * @return The turn type.
         */
        ActionType GetTurnType() const;

        /**
         * Retrieves the turn total duration.
         *
         * @return The turn total duration, in seconds.
         */
        float GetTurnSeconds() const;

        /**
         * Sets the turn current duration.
         *
         * @param[in] seconds The turn current duration, in seconds.
         */
        void SetTurnCurrentSeconds(const float seconds);

        /**
         * Retrieves the turn current duration.
         *
         * @return The turn current duration, in seconds.
         */
        float GetTurnCurrentSeconds() const;

        /**
         * Sets the animation speed.
         *
         * @param[in] speed The animation speed.
         * @todo Indicate units, max and mins, or references.
         */
        void ScriptSetAnimationSpeed(const float speed);

        /**
         * Retrieves the entity's default animation name.
         *
         * @return The default animation name.
         */
        const Ogre::String& GetDefaultAnimationName() const;

        /**
         * Retrieves the entity's current animation name.
         *
         * @return The current animation name.
         */
        const Ogre::String& GetCurrentAnimationName() const;

        /**
         * Retrieves the entity's current animation state.
         *
         * @return The current animation state.
         */
        AnimationState GetAnimationState() const;

        /**
         * Plays one of the entity's animations.
         *
         * @param[in] animation Name of the animation to play.
         * @param[in] state The animation initial state.
         * @param[in] play_type The animation play type, to play it once or in
         * a loop.
         * @param[in] start Animation starting point in time, in seconds.
         * @param[in] end Animation ending point in time, in seconds.
         */
        virtual void PlayAnimation(
          const Ogre::String& animation, AnimationState state,
          AnimationPlayType play_type, const float start, const float end
        ) = 0;

        /**
         * Resumes an animation.
         *
         * @param[in] animation Name of the animation to resume.
         */
        virtual void PlayAnimationContinue(const Ogre::String& animation) = 0;

        /**
         * Updates the animation state.
         *
         * @param[in] delta The animation delta.
         */
        virtual void UpdateAnimation(const float delta) = 0;

        /**
         * Plays one of the entity's animations.
         *
         * @param[in] name Name of the animation to play.
         */
        void ScriptPlayAnimation(const char* name);

        /**
         * Stops one of the entity's animations.
         *
         * @param[in] name Name of the animation to stop.
         */
        void ScriptPlayAnimationStop(const char* name);

        /**
         * Plays one of the entity's animations.
         *
         * @param[in] name Name of the animation to play.
         * @param[in] start Animation starting point in time, in seconds.
         * @param[in] end Animation ending point in time, in seconds.
         */
        void ScriptPlayAnimation(const char* name, const float start, const float end);

        /**
         * Stops one of the entity's animations.
         *
         * @param[in] name Name of the animation to stop.
         * @param[in] start Animation starting point in time, in seconds.
         * @param[in] end Animation ending point in time, in seconds.
         */
        void ScriptPlayAnimationStop(const char* name, const float start, const float end);

        /**
         * Sets the default animation of the entity.
         *
         * @param[in] animation Name of the default animation.
         */
        void ScriptSetDefaultAnimation(const char* animation);

        /**
         * Adds the entity's animation to the sync queue.
         *
         * @return Always -1.
         * @todo Properly describe this.
         */
        int ScriptAnimationSync();

        /**
         * Assigns the entity as a character.
         *
         * Marks the entity as a character, and assigns a character name and
         * ID.
         *
         * @param[in] character_name The character name.
         */
        void SetCharacter(const char* character_name);

        /**
         * Checks if the entity is a character.
         *
         * An entity is not a character until {@see SetCharacter} has been
         * called.
         *
         * @return True if the entity is a character, false otherwise.
         */
        bool IsCharacter();

        /**
         * Retrieves the entity's character ID.
         *
         * @return The character ID, or 0 if the entity is not a character.
         */
        uint GetCharacterId();

        /**
         * Retrieves the entity's character name.
         *
         * @return The character name, or an empty string if the entity is not
         * a character.
         */
        std::string GetCharacterName();

        /**
         * Checks if the entity is a line.
         *
         * @return True if the entity is a line, false otherwise.
         */
        bool IsLine();

    protected:

        /**
         * The name of the entity.
         */
        Ogre::String name_;

        /**
         * The scene node the entity is attached to.
         */
        Ogre::SceneNode* scene_node_;

        /**
         * The entity's model.
         */
        Ogre::SceneNode* model_node_;

        /**
         * The entity's root node.
         */
        Ogre::SceneNode* model_root_node_;

        /**
         * The entity's height.
         */
        float height_;

        /**
         * The entity's direction node.
         */
        Ogre::SceneNode* direction_node_;

        /**
         * The entity's direction.
         */
        EntityDirection* direction_;

        /**
         * The entity's collision node.
         */
        Ogre::SceneNode* solid_collision_node_;

        /**
         * The entity's collision.
         */
        EntityCollision* solid_collision_;

        /**
         * The entity's solid radius.
         */
        float solid_radius_;

        /**
         * Indicates it the entity is solid and can handle collisions.
         */
        bool solid_;

        /**
         * The entity's talk collision node.
         */
        Ogre::SceneNode* talk_collision_node_;

        /**
         * The entity's talk collision.
         */
        EntityCollision* talk_collision_;

        /**
         * The radius at which the entity can be interacted with.
         */
        float talk_radius_;

        /**
         * Indicates if the entity can be interacted with.
         */
        bool talkable_;

        /**
         * Entity's movement status.
         */
        State state_;

        /**
         * Entity's movement sync queue.
         */
        std::vector<ScriptId> sync_;

        /**
         * The entity's automatic movement speed.
         */
        float move_auto_speed_;

        /**
         * The entity's walking speed.
         */
        float move_walk_speed_;

        /**
         * The entity's running speed.
         */
        float move_run_speed_;

        /**
         * The entity's movement destination point.
         */
        Ogre::Vector3 move_position_;

        /**
         * The entity's movement destination entity.
         */
        Entity* move_entity_;

        /**
         * Distance between the entity and it's movement destination point.
         */
        float move_stop_distance_;

        /**
         * Entity's movement destination triangle ID.
         */
        int move_triangle_id_;

        /**
         * Indicates if the entity can rotate while moving.
         */
        bool move_auto_rotation_;

        /**
         * Indicates if the entity can animate while moving.
         */
        bool move_auto_animation_;

        /**
         * The name of the entity's walk animation.
         */
        Ogre::String move_animation_walk_;

        /**
         * The name of the entity's run animation.
         */
        Ogre::String move_animation_run_;

        /**
         * The entity's linear movement direction.
         */
        LinearMovement linear_movement_;

        /**
         * The linear movement starting point.
         */
        Ogre::Vector3 linear_start_;

        /**
         * The linear movement ending point.
         */
        Ogre::Vector3 linear_end_;

        /**
         * Triangle to set the entity on after a linear movement.
         */
        int linear_dest_triangle_;

        /**
         * The jump starting point.
         */
        Ogre::Vector3 jump_start_;

        /**
         * The jump ending point.
         */
        Ogre::Vector3 jump_end_;

        /**
         * Total jump duration.
         */
        float jump_seconds_;

        /**
         * Current jump duration.
         */
        float jump_current_seconds_;

        /**
         * Used to store solidity status before a jump.
         */
        bool jump_was_solid_;

        /**
         * Triangle to set the entity on after a jump.
         */
        int jump_dest_triangle_;

        /**
         * @todo Understand and document.
         */
        Ogre::Vector3 offset_position_start_;

        /**
         * @todo Understand and document.
         */
        Ogre::Vector3 offset_position_end_;

        /**
         * @todo Understand and document.
         */
        ActionType offset_type_;

        /**
         * @todo Understand and document.
         */
        float offset_seconds_;

        /**
         * @todo Understand and document.
         */
        float offset_current_seconds_;

        /**
         * @todo Understand and document.
         */
        std::vector<ScriptId> offset_sync_;

        /**
         * Turn movement direction.
         */
        TurnDirection turn_direction_;

        /**
         * Turn initial orientation.
         */
        Ogre::Degree turn_direction_start_;

        /**
         * Turn final orientation.
         */
        Ogre::Degree turn_direction_end_;

        /**
         * The turn destination entity.
         */
        Entity* turn_entity_;

        /**
         * The turn type.
         */
        ActionType turn_type_;

        /**
         * Total turn duration.
         */
        float turn_seconds_;

        /**
         * Current turn duration.
         */
        float turn_current_seconds_;

        /**
         * Entity's turning sync queue.
         */
        std::vector<ScriptId> turn_sync_;

        /**
         * The animation speed.
         */
        float animation_speed_;

        /**
         * The entity's current animation name.
         */
        Ogre::String animation_current_name_;

        /**
         * Entity's animation sync queue.
         */
        std::vector<ScriptId> animation_sync_;

        /**
         * The entity's current animation state.
         */
        AnimationState animation_state_;

        /**
         * The entity's current animation type.
         */
        AnimationPlayType animation_play_type_;

        /**
         * The name of the entity's default name.
         */
        Ogre::String animation_default_;

        /**
         * @todo Understand and document.
         */
        float animation_end_time_;

        /**
         * Indicates if an automation must be played automatically.
         */
        bool animation_auto_play_;

    private:

        /**
         * Constructor.
         */
        Entity();

        /**
         * Calculates the angular distance to an entity.
         *
         * @param[in] entity Entity to calculate the angular distance to.
         * @return Angular distance to the specified entity.
         */
        Ogre::Degree GetDirectionToEntity(Entity* entity) const;

        bool is_character_;

        uint character_id_;

        std::string character_name_;

        /**
         * Index of the entity on the field.
         */
        int index_;

        /**
         * Indicates if the entity is a line.
         */
        bool is_line_;

};

