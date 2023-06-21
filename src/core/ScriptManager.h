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
#include <OgreString.h>
#include "Event.h"
#include "LuaIncludes.h"
#include "Manager.h"

class Entity;

/**
 * Script identifier.
 */
struct ScriptId{

    /**
     * Constructor.
     */
    ScriptId(): entity(""), function(""){}

    /**
     * Entity name.
     */
    Ogre::String entity;

    /**
     * Function (script) name.
     */
    Ogre::String function;
};

/**
 * A script entity.
 */
struct ScriptEntity;

/**
 * A script queue.
 */
struct QueueScript{

    /**
     * Constructor.
     */
    QueueScript():
      function(""),
      argument1(""),
      argument2(""),
      priority(0),
      state(NULL),
      seconds_to_wait(0),
      wait(false),
      yield(false)
    {}

    /**
     * Function name.
     */
    Ogre::String function;

    /**
     * First function argument.
     */
    Ogre::String argument1;

    /**
     * Second function argument.
     */
    Ogre::String argument2;

    /**
     * Function priority.
     */
    int priority;

    /**
     * Current state.
     */
    lua_State* state;

    /**
     * State thread identifier.
     *
     * Used to store and delete thread.
     */
    int state_id;

    /**
     * Seconds to wait for completion.
     */
    float seconds_to_wait;

    /**
     * Indicates if the script completion should be waited for,
     */
    bool wait;

    /**
     * @todo Understand and document.
     */
    bool yield;

    /**
     * The script paused by call of this script
     */
    ScriptId paused_script_start;

    /**
     * The script paused by call of this script
     */
    ScriptId paused_script_end;
};



class ScriptManager : public Manager, public Ogre::Singleton<ScriptManager>{

    public:

        /**
         * Script types.
         */
        enum Type{

            /**
             * System script.
             *
             * The script is triggered by a system event.
             */
            SYSTEM,

            /**
             * A field or world map entity script.
             *
             * The script belongs to any of the entities on a map.
             */
            ENTITY,

            /**
             * A UI element script.
             *
             * The script is triggered by a UI element (menu, cursor...)
             */
            UI,

            /**
             * A battle entity script.
             *
             * The script belongs to any of the entities on the battle.
             */
            BATTLE
        };

        /**
         * Constructor.
         */
        ScriptManager();

        /**
         * Destructor.
         */
        virtual ~ScriptManager();

        /**
         * Handles an input event.
         *
         * @param[in] event The event to handle.
         */
        void Input(const VGears::Event& event) override;

        /**
         * Updates the state of all scripts of a given type.
         *
         * @param[in] type Type of the scripts to update.
         */
        void Update(const Type type);

        /**
         * Updates the script in the manager with debug information.
         */
        void UpdateDebug() override;

        /**
         * Handles resizing events
         */
        void OnResize() override;

        /**
         * Clears all field information in the script manager.
         */
        void ClearField() override;

        /**
         * Clears all battle information in the script manager.
         */
        void ClearBattle() override;

        /**
         * Clears all world map information in the script manager.
         */
        void ClearWorld() override;

        /**
         * Runs a lua command string.
         *
         * No errors are handled, and nothing is returned
         *
         * @param[in] lua Lua string to run.
         */
        void RunString(const Ogre::String& lua);

        /**
         * Runs a lua file.
         *
         * No errors are handled, and nothing is returned
         *
         * @param[in] file Path to the lua file to run (relative to the data directory).
         */
        void RunFile(const Ogre::String& file);

        /**
         * Initializes Lua binds.
         *
         * It relates the command available in the field maps to C++ functions.
         */
        void InitBinds();

        /**
         * Initializes command bindings
         */
        void InitCmd();

        /**
         * Adds an entity to the manager.
         *
         * Usage depends on the currently active module. The entity will be added to the list of
         * entities of the module.
         *
         * @param[in] type Type of entity to add.
         * @param[in] entity_name The entity name.
         * @param[in] entity The entity to add.
         */
        void AddEntity(const Type type, const Ogre::String& entity_name, Entity* entity);

        /**
         * Deletes an entity from the manager.
         *
         * Usage depends on the currently active module. The entity will be removed from the list of
         * entities of the module. If there is no entity with name ENTITY_NAME and type TYPE,
         * nothing will be done.
         *
         * @param[in] type Type of the entity to remove.
         * @param[in] entity_name Name of the entity to remove.
         */
        void RemoveEntity(const Type type, const Ogre::String& entity_name);

        /**
         * Adds an script to an entity.
         *
         * Usage depends on the currently active module. The script will be added to the list of
         * scripts of the module.
         *
         * @param[in] entity_name Name of the entity to add a script to.
         * @param[in] function_name Name of the script to add.
         * @param[in] priority Script priority. Lower numbers have higher priority.
         */
        void AddEntityScript(
          const Ogre::String& entity_name, const Ogre::String& function_name, int priority
        );

        /**
         * Removes the top script of an entity.
         *
         * The entity must be in one of the currently active modules for it's script to be removed.
         *
         * @param[in] entity Entity whose first script to remove.
         */
        void RemoveEntityTopScript(ScriptEntity& entity);

        /**
         * Retrieves a table.
         *
         * The retrieved table depends on the currently active module.
         *
         * @param[in] type Type of script.
         * @param[in] name Script name.
         * @param[in] state Initial script state.
         * @return The table.
         * @todo Understand and document.
         */
        luabind::object GetTableByEntityName(
          const ScriptManager::Type type, const Ogre::String& name, lua_State* state
        ) const;

        /**
         * Retrieves a script from it's ID.
         *
         * @param[in] script Script ID.
         * @return The script with the corresponding ID, or nullptr if there is none.
         */
        QueueScript* GetScriptByScriptId(const ScriptId& script) const;

        /**
         * Retrieves a script entity by it's name and type.
         *
         * Usage depends on the currently active module. The entity will be searched among the
         * entities of the currently active module.
         *
         * @param[in] type Entity type.
         * @param[in] entity_name The entity name.
         * @return The entity by that type and name, or nullptr if there is no one that matches.
         */
        ScriptEntity* GetScriptEntityByName(const Type type, const Ogre::String& entity_name) const;

        /**
         * Retrieves the current script ID.
         *
         * @return The currently executing script ID.
         */
        const ScriptId GetCurrentScriptId() const;

        /**
         * continues the execution of a script.
         *
         * @param[in] script The ID of the script to resume.
         */
        void ContinueScriptExecution(const ScriptId& script);

        /**
         * Makes an script wait.
         *
         * @param[in] seconds Number of seconds to wait.
         * @todo Does it refer to the current script?
         */
        int ScriptWait(const float seconds);

        /**
         * Request an script execution.
         *
         * The script will execute only if it's not already being executed.
         *
         * @param[in] type Script type.
         * @param[in] entity Entity the scripts belong to.
         * @param[in] function Name of the function of the selected entity to execute.
         * @param[in] priority Execution priority.
         */
        void ScriptRequest(
          const Type type, const char* entity, const char* function, const int priority
        );

        /**
         * Request a synchronous script execution to start.
         *
         * @param[in] type Script type.
         * @param[in] entity Entity the scripts belong to.
         * @param[in] function Name of the function of the selected entity to execute.
         * @param[in] priority Execution priority.
         * @return -1 on success, 1 if the entity or the script don't exist.
         */
        int ScriptRequestStartSync(
          const Type type, const char* entity, const char* function, const int priority
        );

        /**
         * Request a synchronous script execution to end.
         *
         * @param[in] type Script type.
         * @param[in] entity Entity the scripts belong to.
         * @param[in] function Name of the function of the selected entity to execute.
         * @param[in] priority Execution priority.
         * @return -1 if the execution stops or if the script was not running,
         * 1 if the entity or the script don't exist.
         */
        int ScriptRequestEndSync(
          const Type type, const char* entity, const char* function, const int priority
        );

        /**
         * Request a script execution.
         *
         * @param[in] script_entity Entity the scripts belong to.
         * @param[in] function Name of the function of the selected entity to execute.
         * @param[in] priority Execution priority.
         * @param[in] argument1 First argument for the script.
         * @param[in] argument2 Second argument for the script.
         * @param[in] start_sync If true, the script will be started synchronously.
         * @param[in] end_sync @todo Understand and document.
         * @return True on success, false on error (i.e. if the entity or the script don't exist)
         */
        bool ScriptRequest(
          ScriptEntity* script_entity, const Ogre::String& function,
          const int priority, const Ogre::String& argument1,
          const Ogre::String& argument2, bool start_sync, bool end_sync
        );

        /**
         * Adds a script to the stack.
         *
         * @param[in] value ID of the script to add.
         * @todo Verify this documentation.
         */
        void AddValueToStack(const float value);

    private:

        /**
         * Updates the script while in a field.
         */
        void UpdateField() override;

        /**
         * Updates the scripts during a battle.
         */
        void UpdateBattle() override;

        /**
         * Updates the scripts while on the world map.
         */
        void UpdateWorld() override;

        /**
         * Lua state.
         */
        lua_State* lua_state_;

        /**
         * The system script table name.
         */
        Ogre::String system_table_name_;

        /**
         * The entity script table name.
         */
        Ogre::String entity_table_name_;

        /**
         * The battle entity script table name.
         */
        Ogre::String battle_table_name_;

        /**
         * The UI script table name.
         */
        Ogre::String ui_table_name_;

        /**
         * The list of map entity scripts.
         */
        std::vector<ScriptEntity> script_entity_;

        /**
         * The current script ID.
         */
        ScriptId current_script_id_;
};

struct ScriptEntity{

    /**
     * Constructor.
     *
     *By default, the type is {@see ScriptManager::SYSTEM}.
     */
    ScriptEntity(): name(""), type(ScriptManager::SYSTEM), resort(false){}

    /**
     * The script name.
     */
    Ogre::String name;

    /**
     * The script type.
     */
    ScriptManager::Type type;

    /**
     * The script queue.
     */
    std::vector<QueueScript> queue;

    /**
     * @todo Understand and document.
     */
    bool resort;
};

