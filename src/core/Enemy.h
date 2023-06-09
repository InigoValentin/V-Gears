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
#include "ScriptManager.h"

/**
 * Any enemy in a battle.
 */
class Enemy{

    public:

        /**
         * Enemy animation states.
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
         * Constructor.
         *
         * @param[in] name Enemy ID.
         * @param[in] node Scene node to which the enemy should be attached.
         */
        Enemy(const int enemy_id, Ogre::SceneNode* node);

        /**
         * Destructor.
         */
        virtual ~Enemy();

        /**
         * Updates the enemy status.
         */
        virtual void Update();

        /**
         * Updates the enemy status with debug information.
         */
        virtual void UpdateDebug();

        /**
         * Retrieves the enemy ID.
         *
         * @return The enemy ID, or -1 if it's not loaded.
         */
        const int GetEnemyId() const;

        /**
         * Sets the enemy ID.
         *
         * @param[in] id The enemy ID.
         */
        void SetEnemyId(const int id);

        /**
         * Retrieves the enemy name.
         *
         * @return The enemy name.
         */
        const Ogre::String& GetName() const;

        /**
         * Sets the enemy name.
         *
         * @param[in] name The enemy name.
         */
        void SetName(const Ogre::String& name);

        /**
         * Retrieves the enemy level.
         *
         * @return The enemy level.
         */
        unsigned int GetLevel() const;

        /**
         * Sets the enemy level.
         *
         * @param[in] level The enemy level.
         */
        void SetLevel(const unsigned int level);

        /**
         * Retrieves the AP gain upon defeating the enemy.
         *
         * @return The AP gain.
         */
        unsigned int GetAp() const;

        /**
         * Sets the AP gain upon defeating the enemy.
         *
         * @param[in] ap The AP gain.
         */
        void SetAp(unsigned int ap);

        /**
         * Retrieves the EXP gain upon defeating the enemy.
         *
         * @return The EXP gain.
         */
        unsigned int GetExp() const;

        /**
         * Sets the EXP gain upon defeating the enemy.
         *
         * @param[in] exp The EXP gain.
         */
        void SetExp(const unsigned int exp);

        /**
         * Retrieves the money gain upon defeating the enemy.
         *
         * @return The money gain.
         */
        unsigned int GetMoney() const;

        /**
         * Sets the money gain upon defeating the enemy.
         *
         * @param[in] money The money gain.
         */
        void SetMoney(const unsigned int money);

        /**
         * Retrieves the ID of the item the monster can be morphed into.
         *
         * @return ID of the item the enemy can be morphed into, -1 if none.
         */
        unsigned int GetMorph() const;

        /**
         * Sets the ID of the item the monster can be morphed into.
         *
         * @param[in] morph ID of the item the enemy can be morphed into, -1 if none.
         */
        void SetMorph(const int morph);

        /**
         * Retrieves the multiplier for the damage the enemy receives when attacked from the back.
         *
         * @return Back attack damage multiplier.
         */
        float GetBackDamage() const;

        /**
         * Sets the multiplier for the damage the enemy receives when attacked from the back.
         *
         * @param[in] back_damage Back attack damage multiplier, 0 or positive.
         */
        void SetBackDamage(const float back_damage);

        /**
         * Retrieves the enemy's strength stat.
         *
         * @return The strength stat.
         */
        unsigned int GetStr() const;

        /**
         * Sets the enemy's strength stat.
         *
         * @param[in] str The strength stat.
         */
        void SetStr(const unsigned int str);

        /**
         * Retrieves the enemy's defense stat.
         *
         * @return The defense stat.
         */
        unsigned int GetDef() const;

        /**
         * Sets the enemy's defense stat.
         *
         * @param[in] def The defense stat.
         */
        void SetDef(const unsigned int def);

        /**
         * Retrieves the enemy's magic stat.
         *
         * @return The magic stat.
         */
        unsigned int GetMag() const;

        /**
         * Sets the enemy's magic stat.
         *
         * @param[in] mag The magic stat.
         */
        void SetMag(const unsigned int mag);

        /**
         * Retrieves the enemy's spirit stat.
         *
         * @return The spirit stat.
         */
        unsigned int GetSpr() const;

        /**
         * Sets the enemy's spirit stat.
         *
         * @param[in] spr The spirit stat.
         */
        void SetSpr(const unsigned int spr);

        /**
         * Retrieves the enemy's dexterity stat.
         *
         * @return The dexterity stat.
         */
        unsigned int GetDex() const;

        /**
         * Sets the enemy's dexterity stat.
         *
         * @param[in] dex The dexterity stat.
         */
        void SetDex(const unsigned int dex);

        /**
         * Retrieves the enemy's luck stat.
         *
         * @return The luck stat.
         */
        unsigned int GetLck() const;

        /**
         * Sets the enemy's luck stat.
         *
         * @param[in] lck The luck stat.
         */
        void SetLck(const unsigned int lck);

        /**
         * Retrieves the enemy's current HP.
         *
         * @return The current HP.
         */
        unsigned int GetHp() const;

        /**
         * Sets the enemy's current HP.
         *
         * If set to higher then max HP reported by {@see GetHpMax()}, it will be capped to that
         * value.
         *
         * @param[in] hp The current HP.
         */
        void SetHp(const unsigned int hp);

        /**
         * Retrieves the enemy's max HP.
         *
         * @return The max HP.
         */
        unsigned int GetHpMax() const;

        /**
         * Sets the enemy's max HP.
         *
         * If the new max HP is higher than the current HP, the current HP will be set to the new
         * max HP.
         *
         * @param[in] str The strength stat.
         */
        void SetHpMax(const unsigned int hp_max);

        /**
         * Retrieves the enemy's current MP.
         *
         * @return The current MP.
         */
        unsigned int GetMp() const;

        /**
         * Sets the enemy's current MP.
         *
         * If set to higher then max MP reported by {@see GetMpMax()}, it will be capped to that
         * value.
         *
         * @param[in] mp The current MP.
         */
        void SetMp(const unsigned int mp);

        /**
         * Retrieves the enemy's max MP.
         *
         * @return The max MP.
         */
        unsigned int GetMpMax() const;

        /**
         * Sets the enemy's max MP.
         *
         * If the new max MP is higher than the current MP, the current MP will be set to the new
         * max MP.
         *
         * @param[in] str The strength stat.
         */
        void SetMpMax(const unsigned int mp_max);

        /**
         * Sets the enemy position.
         *
         * @param[in] position Enemy's new position.
         */
        void SetPosition(const Ogre::Vector3& position);

        /**
         * Sets the enemy position.
         *
         * @param[in] x Enemy's new position X coordinate.
         * @param[in] y Enemy's new position Y coordinate.
         * @param[in] z Enemy's new position Z coordinate.
         */
        void ScriptSetPosition(const float x, const float y, const float z);

        /**
         * Retrieves the enemy position.
         *
         * @return The enemy position.
         */
        const Ogre::Vector3 GetPosition() const;

        /**
         * Informs the script manager of the enemy position.
         */
        void ScriptGetPosition() const;

        /**
         * Sets the enemy offset.
         *
         * The offset is relative to it's defined position.
         *
         * @param[in] position The enemy's offset.
         */
        void SetOffset(const Ogre::Vector3& position);

        /**
         * Retrieves the enemy offset.
         *
         * The offset is relative to it's defined position.
         *
         * @return The enemy's offset.
         */
        const Ogre::Vector3 GetOffset() const;

        /**
         * Sets the enemy rotation.
         *
         * @param[in] rotation The enemy rotation.
         */
        void SetRotation(const Ogre::Degree& rotation);

        /**
         * Sets the enemy rotation.
         *
         * @param[in] rotation The enemy rotation, in degrees (0-360).
         */
        void ScriptSetRotation(const float rotation);

        /**
         * Retrieves the enemy rotation.
         *
         * @return The enemy rotation, in degrees.
         */
        Ogre::Degree GetRotation() const;

        /**
         * Retrieves the enemy rotation.
         *
         * @return The enemy rotation (0-360).
         */
        float ScriptGetRotation() const;

        /**
         * Sets the enemy scale.
         *
         * @param[in] scale Three dimensional scale.
         */
        virtual void setScale(const Ogre::Vector3 &scale);

        /**
         * Sets the enemy index in the field.
         *
         * @param[in] index Index of the enemy.
         */
        void SetIndex(const int index);

        /**
         * Retrieves the enemy index in the field.
         *
         * @return Index of the enemy.
         */
        int GetIndex();

        /**
         * Sets the enemy's absolute orientation.
         *
         * @param[in] root_orientation The enemy's new orientation.
         */
        virtual void setRootOrientation(const Ogre::Quaternion &root_orientation);

        /**
         * Retrieves the enemy's height.
         *
         * @return The enemy's height.
         */
        float GetHeight() const;

        /**
         * Makes the enemy visible or invisible.
         *
         * Invisible entities can't be interacted with.
         *
         * @param[in] visible True to make the unit visible, false to make it invisible.
         */
        virtual void SetVisible(const bool visible) = 0;

        /**
         * Checks if the enemy is visible or invisible.
         *
         * Invisible entities can't be interacted with.
         *
         * @return True if the unit is visible, false if it's invisible.
         */
        virtual bool IsVisible() const = 0;

        /**
         * Sets the enemy's movement destination position.
         *
         * @param[in] target The destination position.
         */
        void SetMovePosition(const Ogre::Vector3& target);

        /**
         * Sets the enemy's model movement speed.
         *
         * @param[in] speed Movement speed.
         */
        void SetMoveSpeed(const float speed);

        /**
         * Retrieves the enemy's automatic movement speed.
         *
         * @return The movement speed.
         */
        float GetMoveSpeed() const;

        /**
         * Retrieves the enemy's movement destination position.
         *
         * @return The destination position.
         */
        const Ogre::Vector3& GetMovePosition() const;

        /**
         * Retrieves the distance to destination.
         *
         * It's the distance between the enemy's current position and it's current movement
         * destination point.
         *
         * @return The distance to destination.
         */
        float GetMoveStopDistance() const;

        /**
         * Makes the enemy move to a point in the map.
         *
         * @param[in] x X coordinate of the destination point.
         * @param[in] y Y coordinate of the destination point.
         */
        void ScriptMoveToPosition(const float x, const float y);

        /**
         * Waits for enemy's movement to end.
         *
         * @return Always -1.
         */
        int ScriptMoveSync();

        /**
         * Cancels the enemy's current movement.
         *
         * It also clears the movement sync queue.
         */
        void UnsetMove();

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
         * Makes the enemy turn to a fixed direction.
         *
         * @param[in] direction Final direction to turn the enemy's to.
         * @param[in] turn_direction Direction of the turn.
         * @param[in] turn_type Turn mode.
         * @param[in] seconds Total turn duration, in seconds.
         */
        void ScriptTurnToDirection(
          const float direction, const TurnDirection turn_direction,
          const ActionType turn_type, const float seconds
        );

        /**
         * Makes the enemy turn to a fixed point.
         *
         * @param[in] x X coordinate of the point to turn to.
         * @param[in] y Y coordinate of the point to turn to.
         * @param[in] turn_direction Direction of the turn.
         * @param[in] turn_type Turn mode.
         * @param[in] seconds Total turn duration, in seconds.
         */
        void ScriptTurnToPosition(
          const int x, const int y, const TurnDirection turn_direction,
          const ActionType turn_type, const float seconds
        );

        /**
         * Adds the enemy's turn to the sync queue.
         *
         * @return Always -1.
         * @todo Properly describe this.
         */
        int ScriptTurnSync();

        /**
         * Makes the enemy turn towards a point or another enemy.
         *
         * @param[in] direction_to Final direction to turn the enemy's to.
         * @param[in] turn_direction Direction of the turn.
         * @param[in] turn_type Turn mode.
         * @param[in] seconds Total turn duration, in seconds.
         * @todo What if the point
         */
        void SetTurn(
          const Ogre::Degree& direction_to, const TurnDirection turn_direction,
          const ActionType turn_type, const float seconds
        );

        /**
         * Cancels the enemy's current turn.
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
         * Retrieves the enemy's default animation name.
         *
         * @return The default animation name.
         */
        const Ogre::String& GetDefaultAnimationName() const;

        /**
         * Retrieves the enemy's current animation name.
         *
         * @return The current animation name.
         */
        const Ogre::String& GetCurrentAnimationName() const;

        /**
         * Retrieves the enemy's current animation state.
         *
         * @return The current animation state.
         */
        AnimationState GetAnimationState() const;

        /**
         * Plays one of the enemy's animations.
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
         * Plays one of the enemy's animations.
         *
         * @param[in] name Name of the animation to play.
         */
        void ScriptPlayAnimation(const char* name);

        /**
         * Stops one of the enemy's animations.
         *
         * @param[in] name Name of the animation to stop.
         */
        void ScriptPlayAnimationStop(const char* name);

        /**
         * Plays one of the enemy's animations.
         *
         * @param[in] name Name of the animation to play.
         * @param[in] start Animation starting point in time, in seconds.
         * @param[in] end Animation ending point in time, in seconds.
         */
        void ScriptPlayAnimation(const char* name, const float start, const float end);

        /**
         * Stops one of the enemy's animations.
         *
         * @param[in] name Name of the animation to stop.
         * @param[in] start Animation starting point in time, in seconds.
         * @param[in] end Animation ending point in time, in seconds.
         */
        void ScriptPlayAnimationStop(const char* name, const float start, const float end);

        /**
         * Sets the default animation of the enemy.
         *
         * @param[in] animation Name of the default animation.
         */
        void ScriptSetDefaultAnimation(const char* animation);

        /**
         * Adds the enemy's animation to the sync queue.
         *
         * @return Always -1.
         * @todo Properly describe this.
         */
        int ScriptAnimationSync();

    protected:

        /**
         * The enemy ID.
         */
        int enemy_id_;

        /**
         * The name of the enemy.
         */
        Ogre::String name_;

        /**
         * The enemy level.
         */
        unsigned int level_;

        /**
         * EXP given upon defeating the enemy.
         */
        unsigned int exp_;

        /**
         * AP given upon defeating the enemy.
         */
        unsigned int ap_;

        /**
         * Money given upon defeating the enemy.
         */
        unsigned int money_;

        /**
         * ID of the item the enemy can be morphed into. -1 if none..
         */
        int morph_;

        /**
         * Multiplier for back damage.
         */
        float back_damage_;

        /**
         * Enemy strength stat.
         */
        unsigned int str_;

        /**
         * Enemy defense stat.
         */
        unsigned int def_;

        /**
         * Enemy magic stat.
         */
        unsigned int mag_;

        /**
         * Enemy spirit stat.
         */
        unsigned int spr_;

        /**
         * Enemy dexterity stat.
         */
        unsigned int dex_;

        /**
         * Enemy luck stat
         */
        unsigned int lck_;

        /**
         * Enemy's current HP.
         */
        unsigned int hp_;

        /**
         * Enemy's current MP.
         */
        unsigned int mp_;

        /**
         * Enemy's max HP.
         */
        unsigned int hp_max_;

        /**
         * Enemy's max MP.
         */
        unsigned int mp_max_;

        /**
         * The scene node the enemy is attached to.
         */
        Ogre::SceneNode* scene_node_;

        /**
         * The enemy's model.
         */
        Ogre::SceneNode* model_node_;

        /**
         * The enemy's root node.
         */
        Ogre::SceneNode* model_root_node_;

        /**
         * The enemy's height.
         */
        float height_;

        /**
         * Enemy's movement sync queue.
         */
        std::vector<ScriptId> sync_;

        /**
         * The speed of the models movements.
         */
        float move_speed_;

        /**
         * The enemy's movement destination point.
         */
        Ogre::Vector3 move_position_;

        /**
         * Distance between the enemy and it's movement destination point.
         */
        float move_stop_distance_;

        /**
         * Indicates if the enemy can rotate while moving.
         */
        bool move_auto_rotation_;

        /**
         * Indicates if the enemy can animate while moving.
         */
        bool move_auto_animation_;

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
         * Enemy's turning sync queue.
         */
        std::vector<ScriptId> turn_sync_;

        /**
         * The animation speed.
         */
        float animation_speed_;

        /**
         * The enemy's current animation name.
         */
        Ogre::String animation_current_name_;

        /**
         * Enemy's animation sync queue.
         */
        std::vector<ScriptId> animation_sync_;

        /**
         * The enemy's current animation state.
         */
        AnimationState animation_state_;

        /**
         * The enemy's current animation type.
         */
        AnimationPlayType animation_play_type_;

        /**
         * The name of the enemy's default name.
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
        Enemy();

        /**
         * Calculates the angular distance to a point.
         *
         * @param[in] point Point to calculate the angular distance to.
         * @return Angular distance to the specified point.
         */
        Ogre::Degree GetDirectionToPoint(Ogre::Vector2 point) const;

        /**
         * Index of the enemy on the battle.
         */
        int index_;

};

