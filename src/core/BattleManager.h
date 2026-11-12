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
#include "core/XmlAttackFile.h"

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
         * Retrieves the number of enemies in the battle.
         *
         * To be called from Lua scripts.
         *
         * @return The number of enemies in the battle formation.
         */
        unsigned int ScriptGetEnemyCount() const;

        /**
         * Retrieves an enemy.
         *
         * To be called from Lua scripts.
         *
         * @param[in] index The enemy index, 0 based. must be lower than reported by
         * {@see ScriptGetEnemyCount}.
         * @return The enemy at the selected indes, nullptr if it doesn't exists.
         */
        Enemy* ScriptGetEnemy(const unsigned int index);

        /**
         * Adds an attack model to the battle.
         *
         * @param[in] id Attack model ID.
         * @param[in] model Model name, without path or extension.
         * @param[in] pos Model position.
         * @param[in] orientation Model orientation.
         * @param[in] animation Initial animation name for the model. If empty or null, the first
         * available animation will be used.
         */
        void AddAttackModel(
          const unsigned int id, const std::string model, const Ogre::Vector3 pos,
          const Ogre::Degree orientation, const std::string animation
        );

        /**
         * Adds an attack model to the battle.
         *
         * Simplified version of {@see AddAttackModel}, to be used from Lua scripts.
         *
         * @param[in] id Attack model ID.
         * @param[in] model Model name, without path or extension.
         * @param[in] x Model position, X coordinate.
         * @param[in] y Model position, Y coordinate.
         * @param[in] z Model position, Z coordinate.
         * @param[in] orientation Model orientation.
         * @param[in] animation Initial animation name for the model. If empty or null, the first
         * available animation will be used.
         */
        void ScriptAddAttackModel(
          const unsigned int id, const char* model, const float x, const float y, const float z,
          const int orientation, const char* animation
        );


        /**
         * Adds a mesh of a visual effect to the battle scene.
         *
         * The meshes come from the installer, in models/battle/effects. The entity can be moved,
         * scaled and blended from the scripts, and should be removed with
         * {@see ScriptRemoveEffectModel} when the effect ends.
         *
         * @param[in] model Model name, without path or extension, like "magic_faira_1".
         * @param[in] x Position, X coordinate.
         * @param[in] y Position, Y coordinate.
         * @param[in] z Position, Z coordinate.
         * @param[in] scale Scale, relative to the size of attack models. 1 is the original size.
         * @return Name of the new entity, empty if it couldn't be created.
         */
        std::string ScriptAddEffectModel(
          const char* model, const float x, const float y, const float z, const float scale
        );

        /**
         * Removes an effect mesh added with {@see ScriptAddEffectModel}.
         *
         * @param[in] name Name of the entity.
         */
        void ScriptRemoveEffectModel(const char* name);


        /**
         * Sets the battle layout.
         *
         * @param[in] layout The battle layout. {@see LAYOUT}. When one of the unknowns is
         * specified, the normal will be applied.
         */
        void SetLayout(const LAYOUT layout);

        /**
         * Retrieves the battle layout.
         *
         * To be called from Lua scripts.
         *
         * @return The layout, as in {@see LAYOUT}, as an integer.
         */
        int ScriptGetLayout() const;

        /**
         * Retrieves the formation ID of the next battle.
         *
         * To be called from Lua scripts.
         *
         * @return The next battle formation ID, -1 if there is none.
         */
        int GetNextFormationId() const;

        /**
         * Sets the battle mode flags for the next battles (the BTLMD field opcode).
         *
         * To be called from Lua scripts. The flags are: 0x80 no results screens, 0x40 arena, 0x20 no
         * victory music, 0x08 can't escape, 0x04 preemptive attack, 0x02 timed battle and 0x100
         * disable game over. They are kept until they are set again or the field changes.
         *
         * @param[in] mode The flags.
         */
        void ScriptSetMode(const int mode);

        /**
         * Retrieves the battle mode flags.
         *
         * To be called from Lua scripts.
         *
         * @return The flags, as in {@see ScriptSetMode}.
         */
        int GetMode() const;

        /**
         * Retrieves the ID of the current battle formation.
         *
         * To be called from Lua scripts.
         *
         * @return The formation ID, -1 if there is none.
         */
        int GetFormationId() const;

        /**
         * Retrieves the escape difficulty of the battle.
         *
         * To be called from Lua scripts.
         *
         * @return Values between 0 and 1 (both included), from easy to hard. -1 if the battle
         * can't be escaped from.
         */
        float GetEscapeDifficulty() const;

        /**
         * Checks if the battle can start as preemptive.
         *
         * @return False if preemptive attacks are disabled for this battle.
         */
        bool IsPreemptiveAllowed() const;

        /**
         * Sets if preemptive attacks are disabled for the battle.
         *
         * @param[in] disabled True to disable preemptive attacks.
         */
        void SetPreemptiveDisabled(const bool disabled);

        /**
         * Checks if the victory pose must be shown.
         *
         * To be called from Lua scripts.
         */
        bool ShowVictoryPose() const;

        /**
         * Checks if the spoils screens must be shown.
         *
         * To be called from Lua scripts.
         */
        bool ShowSpoils() const;

        /**
         * Shows or hides an enemy model.
         *
         * To be called from Lua scripts.
         *
         * @param[in] index The enemy index, 0 based.
         * @param[in] visible True to show the enemy model, false to hide it.
         */
        void ScriptSetEnemyVisible(const unsigned int index, const bool visible);

        /**
         * Retrieves the name of the entity of an enemy.
         *
         * To be called from Lua scripts.
         *
         * @param[in] index The enemy index, 0 based.
         * @return The entity name, empty if the index is not valid.
         */
        std::string ScriptGetEnemyEntityName(const unsigned int index) const;

        /**
         * Retrieves the name of the entity of a party member.
         *
         * To be called from Lua scripts.
         *
         * @param[in] index The party position, 0 based (the same as in the party).
         * @return The entity name, empty if there is no one in that position.
         */
        std::string ScriptGetPartyEntityName(const unsigned int index) const;

        /**
         * Retrieves the prefix used in the animation names of a party member model.
         *
         * To be called from Lua scripts.
         *
         * @param[in] index The party position, 0 based (the same as in the party).
         * @return The animation prefix (the model name first two letters), empty if there is no
         * one in that position.
         */
        std::string ScriptGetPartyAnimationPrefix(const unsigned int index) const;

        /**
         * Retrieves the model name of a party member, without path or extension (as in
         * "rt_cloud").
         *
         * To be called from Lua scripts.
         *
         * @param[in] index The party position, 0 based (the same as in the party).
         * @return The model name, empty if there is no one in that position.
         */
        std::string ScriptGetPartyModelName(const unsigned int index) const;

        /**
         * Retrieves the data of an attack.
         *
         * Attack files are read the first time they are needed.
         *
         * @param[in] id Attack ID.
         * @return The attack data, nullptr if there is no such attack.
         */
        const AttackData* GetAttackData(const int id);

        /**
         * Checks if there is attack data for an attack, without logging an error if there isn't.
         *
         * To be called from Lua scripts.
         *
         * @param[in] id Attack ID.
         * @return True if there is an attack file for the attack.
         */
        bool HasAttackData(const int id);

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
         * Changes the battle camera to one of the cameras of the formation.
         *
         * To be called from Lua scripts.
         *
         * @param[in] index Camera index, 0 based.
         * @return The number of cameras of the formation. The camera isn't changed if the index is
         * not valid.
         */
        int ScriptSetCamera(const int index);

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
         * Counter to give names to effect entities.
         */
        unsigned int next_effect_id_ = 0;


        /**
         * Loads the party members into the battle.
         */
        void LoadParty();

        /**
         * Logs the state of the camera.
         *
         * @param[in] title Text to start the message with.
         */
        void LogCamera(const std::string& title) const;

        /**
         * Model rotations, in degrees, to face the party, the enemies, or the left or right side
         * of the battle camera.
         */
        static const int FACING_TO_PARTY;
        static const int FACING_TO_ENEMIES;
        static const int FACING_TO_LEFT;
        static const int FACING_TO_RIGHT;

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
         * Frames left until the initial battle camera is checked and set again.
         */
        int camera_check_frames_ = 0;

        /**
         * Battle mode flags, set by the field scripts. {@see ScriptSetMode}
         */
        int battle_mode_ = 0;

        /**
         * Updates manager while in the world map.
         *
         * It does nothing
         */
        void UpdateWorld() override;

        /**
         * Depth (Y coordinate) of the party in normal battles.
         */
        static const float PARTY_Y;

        /**
         * Distance between party members.
         */
        static constexpr float PARTY_SPACING = 6.0f;

        /**
         * Distance from the center to each side in side attacks.
         */
        static const float SIDE_DISTANCE;

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
         * Scale factor for battle attack and spell models.
         */
        static const float ATTACK_SCALE;

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
         * True if the current battle started in the world map.
         */
        bool battle_from_world_ = false;

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
         * Names of the entities of the enemies in the battle, in the same order as the enemies.
         */
        std::vector<std::string> enemy_entity_name_;

        /**
         * Names of the entities of the party members, by party position. Empty if no one is in
         * that position.
         */
        std::vector<std::string> party_entity_name_;

        /**
         * Animation prefixes of the party members models, by party position.
         */
        std::vector<std::string> party_animation_prefix_;

        /**
         * Model names of the party members, by party position, without path or extension.
         */
        std::vector<std::string> party_model_name_;

        /**
         * File names of the attacks by ID, loaded the first time an attack is requested.
         */
        std::map<int, std::string> attack_index_;

        /**
         * Indicates if {@see attack_index_} has been loaded.
         */
        bool attack_index_loaded_ = false;

        /**
         * Attacks already read.
         */
        std::map<int, AttackData> attack_cache_;

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
