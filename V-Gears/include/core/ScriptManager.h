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



class ScriptManager : public Ogre::Singleton<ScriptManager>{

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
             * A field entity script.
             *
             * The script belongs to any of the enities on a map.
             */
            ENTITY,

            /**
             * A UI element script.
             *
             * The script is triggered by a UI element (menu, cursor...)
             */
            UI,

            /**
             * A field script.
             *
             * The script ios triggered by the map itself
             */
            FIELD
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
         * @param event[in] The event to handle.
         */
        void Input(const QGears::Event& event);

        /**
         * Updates the state of all scripts of a given type.
         *
         * @param type[in] Type of the scripts to update.
         */
        void Update(const Type type);

        /**
         * Runs a lua command string.
         *
         * No errors are handled, and nothing is returned
         *
         * @param lua[in] Lua string to run.
         */
        void RunString(const Ogre::String& lua);

        /**
         * Runs a lua file.
         *
         * No errors are handled, and nothing is returned
         *
         * @param file[in] Path to the lua file to run (relative to the
         * data directory).
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
         * @param type[in] Type of entity to add.
         * @param entity_name[in] The entity name.
         * @param entity[in] The entity to add.
         */
        void AddEntity(
          const Type type, const Ogre::String& entity_name, Entity* entity
        );

        /**
         * Deletes an entity from the manager.
         *
         * If there is no entity with name ENTITY_NAME and type TYPE, nothing
         * will be done.
         *
         * @param type[in] Type of the entity to remove.
         * @param entity_name[in] NAme of the entity to remove.
         */
        void RemoveEntity(const Type type, const Ogre::String& entity_name);

        /**
         * Adds an script to an entity.
         *
         * @param entity_name[in] Name of the entity to add a script to.
         * @param function_name[in] Name of the script to add.
         * @param priority[in] Script priority. Lower numbers have higher
         * priority.
         */
        void AddEntityScript(
          const Ogre::String& entity_name, const Ogre::String& function_name,
          int priority
        );

        /**
         * Removes the top script of an entity.
         *
         * @param entiry[in] Enthity whose first script to remove.
         */
        void RemoveEntityTopScript(ScriptEntity& entity);

        /**
         * @todo Understand and document.
         *
         * @param type[in] Type of script.
         * @param name[in] Script name.
         * @param state[in] Initial script state.
         * @return @todo.
         */
        luabind::object GetTableByEntityName(
          const ScriptManager::Type type, const Ogre::String& name,
          lua_State* state
        ) const;

        /**
         * Retrieves a script from it's ID.
         *
         * @param script[in] Script ID.
         * @return The script with the corresponding ID, or nullptr if there is
         * none
         */
        QueueScript* GetScriptByScriptId(const ScriptId& script) const;

        /**
         * Retrieves a script entity by it's name and type.
         *
         * @param type[in] Entity type.
         * @param entity_name[in] The entity name.
         * @return The entity by that type and name, or nullptr if there is
         * no one that matches.
         */
        ScriptEntity* GetScriptEntityByName(
          const Type type, const Ogre::String& entity_name
        ) const;

        /**
         * Retrieves the current script ID.
         *
         * @return The currently executing script ID.
         */
        const ScriptId GetCurrentScriptId() const;

        /**
         * continues the execution of a script.
         *
         * @param script[in] The ID of the script to resume.
         */
        void ContinueScriptExecution(const ScriptId& script);

        /**
         * Makes an script wait.
         *
         * @param seconds[in] Number of seconds to wait.
         * @todo Does it refer to the current script?
         */
        int ScriptWait(const float seconds);

        /**
         * Request an script execution.
         *
         * @param type[in] Script type.
         * @param entity[in] Entity the scripts belong to.
         * @param function[in] Name of the function of the selected entity to
         * execute.
         * @param priority[in] Execution priority.
         */
        void ScriptRequest(
          const Type type, const char* entity,
          const char* function, const int priority
        );

        /**
         * Request a synchronous script execution to start.
         *
         * @param type[in] Script type.
         * @param entity[in] Entity the scripts belong to.
         * @param function[in] Name of the function of the selected entity to
         * execute.
         * @param priority[in] Execution priority.
         * @return -1 on success, 1 if the entity or the script don't exist.
         */
        int ScriptRequestStartSync(
          const Type type, const char* entity,
          const char* function, const int priority);

        /**
         * Request a synchronous script execution to end.
         *
         * @param type[in] Script type.
         * @param entity[in] Entity the scripts belong to.
         * @param function[in] Name of the function of the selected entity to
         * execute.
         * @param priority[in] Execution priority.
         * @return -1 if the execution stops or if the script was not running,
         * 1 if the entity or the script don't exist.
         */
        int ScriptRequestEndSync(
          const Type type, const char* entity,
          const char* function, const int priority
        );

        /**
         * Request a script execution.
         *
         * @param script_entity[in] Entity the scripts belong to.
         * @param function[in] Name of the function of the selected entity to
         * execute.
         * @param priority[in] Execution priority.
         * @param argument1[in] First argument for the script.
         * @param argument2[in] Second argument for the script.
         * @param start_sync[in] If true, the script will be started
         * synchronously.
         * @param end_sync[in] @todo Understand and document.
         * @return True on success, false on error (i.e. if the entity or the
         * script don't exist)
         */
        bool ScriptRequest(
          ScriptEntity* script_entity, const Ogre::String& function,
          const int priority, const Ogre::String& argument1,
          const Ogre::String& argument2, bool start_sync, bool end_sync
        );

        /**
         * Adds a script to the stack.
         *
         * @param value[in] ID of the script to add.
         * @todo Verify this documentation.
         */
        void AddValueToStack(const float value);

    private:

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
         * The UI script table name.
         */
        Ogre::String ui_table_name_;

        /**
         * The list of script entities.
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
    ScriptEntity():
      name(""),
      type(ScriptManager::SYSTEM),
      resort(false)
    {}

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

