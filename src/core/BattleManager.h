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
#include "Enemy.h"
#include "Event.h"
#include "Manager.h"
#include "core/XmlBattleCharactersFile.h"
#include "core/XmlBattleScenesFile.h"

/**
 * The battle manager.
 */
class BattleManager : public Manager, public Ogre::Singleton<BattleManager>{

    public:

        /**
         * Possible battle layouts.
         */
        enum LAYOUT{

            /**
             * Normal battle.
             *
             * Party facing enemies, all ATBs filled at random, they start filling at the battle
             * start.
             */
            NORMAL = 0,

            /**
             * Preemptive attack.
             *
             * Enemies are facing backwards, their ATBs start empty and only start to fill after an
             * attack or after a set amount of time. PArty starts with full ATBs.
             */
            PREEMPTIVE = 1,

            /**
             * Party attacked from the back.
             *
             * Party members are facing backwards, with their rows reversed. Their ATBs are empty
             * and only start to fill after an attack or after a set amount of time. Enemies start
             * with full ATBs.
             */
            BACK_ATTACK = 2,

            /**
             * Party surrounds the enemy.
             *
             * Party members alternated on both sides of the battle. They are considered front row
             * for offense and back row for defense. Their ATBs start full. Party-targeted commands
             * only target one side. Enemies in the middle, facing in a random direction. Their
             * ATBs start with a random amount. When attacked, an enemy turns to the attacker.
             */
            SIDE_ATTACK = 3,

            /**
             * Enemies surrounds the party.
             *
             * Enemies alternated on both sides of the battle. They are considered front row for
             * offense and back row for defense. Their ATBs start full. Multiple-targeted commands
             * only target one side. Party members in the middle, facing in a random direction.
             * Their ATBs start with a random amount. When attacked, they turn to the attacker.
             */
            PINCER_ATTACK = 4,

            /**
             * Reserved for scripted battles with only one character.
             *
             * Works the same as {@see NORMAL}. In the original game, it's used for the Yuffie
             * pagoda battles and Cloud's last battle against sephiroth
             */
            SOLO = 5,

            /**
             * Unknown.
             *
             * Never used in the original game.
             */
            UNKNOWN_0 = 6,

            /**
             * Unknown.
             *
             * Never used in the original game.
             */
            UNKNOWN_1 = 7,

            /**
             * Locked formation.
             *
             * Same as {@see NORMAL}, but every party member and enemy is forced in the front row,
             * and the "Change" command is disabled.
             */
            LOCKED = 8,
        };

        /**
         * Item earned during the battle.
         */
        struct Spoil {

            /**
             * Item ID.
             */
            unsigned int id;

            /**
             * Item quantity.
             */
            unsigned int qty;
        };

        /**
         * A battle camera settings.
         */
        struct BattleCamera{

            /**
             * Camera ID.
             */
            unsigned int id;

            /**
             * Camera location.
             */
            Ogre::Vector3 location;

            /**
             * Camera orientation.
             *
             * It's not an angle, or a rotation. but the coordinates the camera points at.
             */
            Ogre::Vector3 orientation;
        };

        /**
         * Constructor.
         */
        BattleManager();

        /**
         * Destructor.
         */
        virtual ~BattleManager();

        /**
         * Handles an input event.
         *
         * @param[in] event Event to handle.
         */
        void Input(const VGears::Event& event) override;

        /**
         * Updates the battle in the manager with debug information.
         *
         * It's automatically called from {@see Update}.
         */
        void UpdateDebug() override;

        /**
         * Handles resizing events
         */
        void OnResize() override;

        /**
         * Clears all field information in the battle manager.
         *
         * Does nothing.
         */
        void ClearField() override;

        /**
         * Clears all battle information in the battle manager.
         */
        void ClearBattle() override;

        /**
         * Clears all world map information in the battle manager.
         */
        void ClearWorld() override;

        /**
         * Starts a battle.
         *
         * @param id Battle ID.
         */
        void StartBattle(const unsigned int id);

        /**
         * Ends the current battle.
         */
        void EndBattle();

        /**
         * Retrieves the list of enemies.
         *
         * @return The list of enemies.
         */
        std::vector<Enemy> GetEnemies() const;

        /**
         * Adds an enemy to the manager for the next battle.
         *
         * @param[in] id Enemy ID.
         * @param[in] pos Enemy position (x, y, z).
         * @param[in] front True to set the enemy in the front row, false for back row.
         * @param[in] visible Indicates enemy visibility.
         * @param[in] targeteable Indicates if the enemy can be targeted.
         * @param[in] active Indicates whether the enemy main script is active or not.
         * @param[in] cover Cover binary flags string.
         */
        void AddEnemy(
          const unsigned int id, const Ogre::Vector3 pos, const bool front, const bool visible,
          const bool targeteable, const bool active, const std::string cover
        );

        /**
         * Loads enemy info from the enemy XML enemy file.
         */
        void Load();

        /**
         * Sets the battle layout.
         *
         * @param[in] layout The battle layout. {@see LAYOUT}. When one of the unknowns is
         * specified, the normal will be applied.
         */
        void SetLayout(const LAYOUT layout);

        /**
         * Sets the battle formation ID.
         *
         * @param[in] id Battle formation ID. Negative values to clean the manager.
         */
        void SetFormationId(const int id);

        /**
         * Sets the battle formation ID for the next battle, if any.
         *
         * If set, a new battle will happen when this one is over, instead of returning to the
         * field.
         *
         * @param[in] id Next battle formation ID. Negative values if none.
         */
        void SetNextFormationId(const int id);

        /**
         * Sets escaping data for the battle.
         *
         * @param[in] difficulty Values between 0 and 1 (exclusive) set the escaping difficulty
         * from easy to hard. 0 Means the battle can be escaped instantly. Any other value means
         * the battle can't be escaped from.
         */
        void SetEscapeability(const float difficulty);

        /**
         * Sets the battle to show or skip the victory pose.
         */
        void SetSkipVictoryPose(const bool skip);

        /**
         * Sets the battle to show or skip the spoils screens.
         */
        void SetSkipSpoils(const bool skip);

        /**
         * Sets the battle location.
         *
         * @param[in] id Location ID.
         * @param[in] name Location name, for debug purposes only. Can be empty or null.
         */
        void SetLocation(const int id, const Ogre::String name);

        /**
         * Indicates if the battle is an arena battle.
         *
         * @param[in] arena True for arena battles, false for every other battles.
         */
        void SetArenaBattle(const bool arena);

        /**
         * Sets the initial camera.
         *
         * Can be set before configuring the camera, but if the battle starts and this index is
         * invalid, it's undefined behavior.
         *
         * @param[in] id ID of the initial camera.
         */
        void SetInitialCamera(const unsigned int id);

        /**
         * Adds a camera for the next battle.
         *
         * @param[in] id Camera ID.
         * @param[in] pos Camera position (x, y, z).
         * @param[in] dir Camera orientation (a point the camera will point to).
         */
        void AddCamera(
          const unsigned int id, const Ogre::Vector3 pos, const Ogre::Vector3 dir
        );

    private:

        /**
         * Loads the party members into the battle.
         */
        void LoadParty();

        /**
         * Updates the manager while in the field.
         *
         * It does nothing
         */
        void UpdateField() override;

        /**
         * Updates the manager during a battle.
         */
        void UpdateBattle() override;

        /**
         * Updates manager while in the world map.
         *
         * It does nothing
         */
        void UpdateWorld() override;

        /**
         * Scale factor for battle enemy models.
         */
        static const float ENEMY_SCALE;

        /**
         * Scale factor for battle character models.
         */
        static const float PARTY_SCALE;

        /**
         * Scale factor for battle background models.
         */
        static const float SCENE_SCALE;

        /**
         * The scene node.
         */
        Ogre::SceneNode* scene_node_;

        /**
         * The current battle formation ID.
         */
        int formation_id_;

        /**
         * The battle formation ID for the next battle, if any.
         */
        int next_formation_id_;

        // TODO: Location
        //BattleLocation location_;

        // TODO: Camera list
        std::vector<BattleCamera> camera_;

        /**
         * Default camera ID
         */
        unsigned int initial_camera_;

        /**
         * Battle layout.
         */
        LAYOUT layout_;

        /**
         * Difficulty to escape the battle.
         */
        float escape_difficulty_;

        /**
         * Indicates if the current battle is an arena one.
         */
        bool arena_battle_;

        /**
         * Indicates if the victory pose must be shown at the battle end.
         */
        bool show_victory_pose_;

        /**
         * Indicates if the spoils screens must be shown at the battle end.
         */
        bool show_spoils_;

        /**
         * Indicates if the battle can be preemptive (not if it will definitely be).
         */
        bool preemptive_;

        /**
         * Money earned during the current battle.
         */
        unsigned int money_;

        /**
         * List of items earned during the battle.
         */
        std::vector<Spoil> spoil_;

        /**
         * List of enemies for the battle.
         */
        std::vector<Enemy> enemies_;

        /**
         * Map of playable character models.
         */
        std::vector<XmlBattleCharactersFile::BattleCharacter> character_model_map_;

        /**
         * Map of battle scene models.
         */
        std::vector<XmlBattleScenesFile::BattleScene> scene_model_map_;

        /**
         * List of party members in the battle.
         */
        // TODO std::vector<Player> party_;

};
