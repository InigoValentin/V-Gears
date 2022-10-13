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

#include "core/ConfigVar.h"
#include "core/DebugDraw.h"
#include "core/Logger.h"
#include "core/ScriptManager.h"
#include "core/ScriptManagerBinds.h"
#include "core/ScriptManagerCommands.h"
#include "core/Timer.h"
#include "core/Utilites.h"
#include "core/XmlScriptsFile.h"
#include "LuaIncludes.h"

ConfigVar cv_debug_script("debug_script", "Debug script flags. 0x01 - System, 0x02 - Entity, 0x04 - Ui.", "0");

template<>ScriptManager *Ogre::Singleton<ScriptManager>::msSingleton = nullptr;

Ogre::String script_entity_type[] = {"SYSTEM", "ENTITY", "UI"};

/**
 * Compares the priority of two scripts.
 *
 * @param[in] a The first script to compare.
 * @param[in] b The second script to compare.
 * @return True if the script a has a lower priority number than b, false if a
 * has a higher or equal priority than b.
 */
bool priority_queue_compare(QueueScript a, QueueScript b){
    return a.priority < b.priority;
}

ScriptManager::ScriptManager():
  system_table_name_("System"),
  entity_table_name_("EntityContainer"),
  ui_table_name_("UiContainer")
{
    LOG_TRIVIAL("ScriptManager starting.");
    lua_state_ = lua_open();
    luabind::open(lua_state_);
    luaopen_base(lua_state_);
    luaopen_string(lua_state_);
    luaopen_table(lua_state_);
    luaopen_math(lua_state_);
    luaopen_io(lua_state_);
    LOG_TRIVIAL("ScriptManager starting ... 1");
    InitBinds();
    LOG_TRIVIAL("ScriptManager starting ... 2");
    InitCmd();
    LOG_TRIVIAL("ScriptManager starting ... 3");
    RunFile("system/system.lua");
    XmlScriptsFile scripts("./data/scripts.xml");
    scripts.LoadScripts();
    LOG_TRIVIAL("ScriptManager started.");
}

ScriptManager::~ScriptManager(){
    lua_close(lua_state_);
    LOG_TRIVIAL("ScriptManager closed.");
}

void ScriptManager::Input(const VGears::Event& event){
    if (
      (
        event.type == VGears::ET_KEY_PRESS
        || event.type == VGears::ET_KEY_REPEAT_WAIT
      )
      && (
        event.param1 == OIS::KC_RETURN
        || event.param1 == OIS::KC_ESCAPE
        || event.param1 == OIS::KC_SPACE
        || event.param1 == OIS::KC_LCONTROL
        || event.param1 == OIS::KC_LEFT
        || event.param1 == OIS::KC_RIGHT
        || event.param1 == OIS::KC_DOWN
        || event.param1 == OIS::KC_UP
      )
    ){
        for (unsigned int i = 0; i < script_entity_.size(); ++ i){
            Ogre::String argument2 = "";
            if (event.type == VGears::ET_KEY_PRESS) argument2 = "Press";
            else if (event.type == VGears::ET_KEY_REPEAT_WAIT)
                argument2 = "Repeat";
            ScriptRequest(
              &script_entity_[i], "on_button", 100,
              KeyToString((OIS::KeyCode)((int)event.param1)), argument2,
              false, false
            );
        }
    }
}

void ScriptManager::Update(const ScriptManager::Type type){
    // Resort all queue. This will give correct info for debug draw.
    for (unsigned int i = 0; i < script_entity_.size(); ++ i){
        if (script_entity_[i].type == type){
            if (script_entity_[i].queue.size() > 0){
                if (script_entity_[i].resort == true){
                    std::stable_sort(
                      script_entity_[i].queue.begin(),
                      script_entity_[i].queue.end(), priority_queue_compare
                    );
                    script_entity_[i].resort = false;
                }
            }
        }
    }

    // Draw debug before update. This way it will be possible to see
    // scripts that run once
    int debug = cv_debug_script.GetI();
    if (debug != 0){
        DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.LEFT);
        DEBUG_DRAW.SetScreenSpace(true);
        int y = 34;

        for (unsigned int i = 0; i < 3; ++ i){
            if ((debug & (1 << i)) != 0){
                DEBUG_DRAW.SetColour(Ogre::ColourValue(0.8f, 0.8f, 0.0f, 1.0f));
                DEBUG_DRAW.Text(10.0f, static_cast<float>(y), "Script \"" + script_entity_type[i] + "\" entity:");
                y += 16;

                for (unsigned int j = 0; j < script_entity_.size(); ++ j){
                    if (script_entity_[j].type == i){
                        Ogre::String text = script_entity_[j].name;
                        unsigned int queue_size = script_entity_[j].queue.size();
                        if (queue_size > 0){
                            text += ": ";
                            DEBUG_DRAW.SetColour(
                              Ogre::ColourValue(0.8f, 0.8f, 0.0f, 1.0f)
                            );
                        }
                        else{
                            DEBUG_DRAW.SetColour(
                              Ogre::ColourValue(0.5, 0.5, 0.5, 1)
                            );
                        }
                        for (unsigned int k = 0; k < queue_size; ++ k){
                            if (k > 0) text += ", ";
                            text +=
                              "("
                              + Ogre::StringConverter::toString(
                                script_entity_[j].queue[k].priority
                              )
                              + ")" + script_entity_[j].queue[k].function;

                            if (script_entity_[j].queue[k].wait == true){
                                if (
                                  script_entity_[j].queue[k].seconds_to_wait
                                  != 0
                                ){
                                    text +=
                                      ":wait("
                                      + Ogre::StringConverter::toString(
                                        script_entity_[j].queue[k]
                                          .seconds_to_wait
                                      )
                                      + ")";
                                }
                            }
                        }
                        DEBUG_DRAW.Text(20.0f, static_cast<float>(y), text);
                        y += 16;
                    }
                }
            }
        }
    }

    for (unsigned int i = 0; i < script_entity_.size(); ++ i){
        if (script_entity_[i].type == type){
            if (script_entity_[i].queue.size() > 0){
                current_script_id_.entity = script_entity_[i].name;
                current_script_id_.function
                  = script_entity_[i].queue[0].function;

                if (script_entity_[i].queue[0].wait == false){
                    if (script_entity_[i].queue[0].yield == false){
                        LOG_TRIVIAL(
                          "[SCRIPT] Start script \""
                          + current_script_id_.function + "\" for entity \""
                          + current_script_id_.entity + "\"."
                        );

                        if (
                          script_entity_[i].queue[0].paused_script_start.entity
                          != ""
                        ){
                            ContinueScriptExecution(
                              script_entity_[i].queue[0].paused_script_start
                            );
                            script_entity_[i].queue[0]
                              .paused_script_start.entity = "";
                        }

                        luabind::object table = GetTableByEntityName(
                          type, current_script_id_.entity,
                          script_entity_[i].queue[0].state
                        );

                        if (
                          table.is_valid()
                          && luabind::type(table) == LUA_TTABLE
                          && luabind::type(table[current_script_id_.function])
                            == LUA_TFUNCTION
                        ){
                            int ret = 0;
                            try{
                                ret = luabind::resume_function<int>(
                                  table[current_script_id_.function], table,
                                  script_entity_[i].queue[0].argument1.c_str(),
                                  script_entity_[i].queue[0].argument2.c_str()
                                );
                            }
                            catch(luabind::error& e){
                                LOG_ERROR(
                                  "LUA error in entity "
                                  + script_entity_[i].name + " ("
                                  + current_script_id_.entity + ") "
                                  + " in function "
                                  + script_entity_[i].queue[0].function + " ("
                                  + current_script_id_.function + ") "
                                  + ". Details: "
                                  + Ogre::String(
                                    lua_tostring(
                                      script_entity_[i].queue[0].state, -1
                                    )
                                  ) + " : " + e.what()
                                );
                            }

                            if (ret == 0){
                                LOG_TRIVIAL(
                                  "[SCRIPT] Script \""
                                  + current_script_id_.function
                                  + "\" for entity \""
                                  + current_script_id_.entity + "\" finished."
                                );
                                if (
                                  script_entity_[i].queue[0].function
                                  != "on_update"
                                ){
                                    RemoveEntityTopScript(script_entity_[i]);
                                }
                            }
                            else if (ret == 1){
                                LOG_TRIVIAL(
                                  "[SCRIPT] Script \""
                                  + current_script_id_.function
                                  + "\" for entity \""
                                  + current_script_id_.entity
                                  + "\" not paused and will be continued "
                                  + "next cycle."
                                );
                                script_entity_[i].queue[0].yield = true;
                            }
                            else{
                                LOG_TRIVIAL(
                                  "[SCRIPT] Script \""
                                  + current_script_id_.function
                                  + "\" for entity \""
                                  + current_script_id_.entity
                                  + "\" not finished yet."
                                );
                                script_entity_[i].queue[0].yield = true;
                                script_entity_[i].queue[0].wait = true;
                            }
                        }
                        else{
                            LOG_WARNING(
                              "Script \"" + current_script_id_.function
                              + "\" for entity \"" + current_script_id_.entity
                              + "\" doesn't exist."
                            );
                            RemoveEntityTopScript(script_entity_[i]);
                        }
                    }
                    else{
                        LOG_TRIVIAL(
                          "[SCRIPT] Continue function \""
                          + current_script_id_.function + "\" for entity \""
                          + current_script_id_.entity + "\"."
                        );

                        int ret = 0;
                        try{
                            ret = luabind::resume<int>(
                              script_entity_[i].queue[0].state
                            );
                        }
                        catch(luabind::error& e){
                            luabind::object error_msg(
                              luabind::from_stack(e.state(), -1)
                            );
                            LOG_WARNING(
                              Ogre::String(
                                luabind::object_cast<std::string >(error_msg)
                              )
                            );
                        }

                        if (ret == 0){ // Finished.
                            LOG_TRIVIAL(
                              "[SCRIPT] Script \"" + current_script_id_.function
                              + "\" for entity \"" + current_script_id_.entity
                              + "\" finished."
                            );

                            // Stop yield for on_update.
                            script_entity_[i].queue[0].yield = false;

                            if (
                              script_entity_[i].queue[0].function != "on_update"
                            ){
                                RemoveEntityTopScript(script_entity_[i]);
                            }
                        }
                        else if (ret == 1){
                            LOG_TRIVIAL(
                              "[SCRIPT] Script \"" + current_script_id_.function
                              + "\" for entity \"" + current_script_id_.entity
                              + "\" not paused and will be continued "
                              + "next cycle."
                            );
                        }
                        else{
                            LOG_TRIVIAL(
                              "[SCRIPT] Script \"" + current_script_id_.function
                              + "\" for entity \"" + current_script_id_.entity
                              + "\" not finished yet."
                            );
                            script_entity_[i].queue[0].wait = true;
                        }
                    }
                }
                else if (script_entity_[i].queue[0].seconds_to_wait > 0){
                    script_entity_[i].queue[0].seconds_to_wait
                      -= Timer::getSingleton().GetGameTimeDelta();
                    script_entity_[i].queue[0].seconds_to_wait
                      = (script_entity_[i].queue[0].seconds_to_wait < 0)
                        ? 0 : script_entity_[i].queue[0].seconds_to_wait;

                    if (script_entity_[i].queue[0].seconds_to_wait == 0)
                        script_entity_[i].queue[0].wait = false;
                }
            }
        }
    }
}

void ScriptManager::RunString(const Ogre::String& lua){
    int pre_eval_index = lua_gettop(lua_state_);
    if (luaL_dostring(lua_state_, lua.c_str()) == 1)
        LOG_ERROR(
          "ScriptManager::RunString error: "
          + Ogre::String(lua_tostring(lua_state_, -1))
        );
    else{
        int index = lua_gettop(lua_state_);
        if (index > 0 && index != pre_eval_index){
            auto str = lua_tostring(lua_state_, index);
            LOG_CONSOLE(
              str != nullptr
                ? str
                : "ScriptManager::RunString: (nullptr; try tostring(...)?)");
            lua_pop(lua_state_, 1);
        }
    }
}

void ScriptManager::RunFile(const Ogre::String& file){
    if (luaL_dofile(lua_state_, ("./data/" + file).c_str()) == 1)
        LOG_ERROR(Ogre::String(lua_tostring(lua_state_, -1)));
}

void ScriptManager::AddEntity(
  const ScriptManager::Type type, const Ogre::String& entity_name,
  Entity* entity
){
    for (unsigned int i = 0; i < script_entity_.size(); ++ i){
        if (
          script_entity_[i].type == type && script_entity_[i].name
          == entity_name
        ){
            LOG_ERROR(
              "Script \"" + script_entity_type[type] + "\" entity \""
              + entity_name + "\" already exist in script manager."
            );
            return;
        }
    }

    luabind::object table = GetTableByEntityName(type, entity_name, lua_state_);

    if (table.is_valid() && luabind::type(table) == LUA_TTABLE){
        ScriptEntity script_entity;
        script_entity.name = entity_name;
        script_entity.type = type;

        // Initialize entity field for model entity.
        if (entity != nullptr) table["entity"] = boost::ref(*entity);

        // Check the "on_start" script
        if (luabind::type(table["on_start"]) == LUA_TFUNCTION){
            QueueScript script;
            script.function = "on_start";
            script.priority = 1;
            script.state = lua_newthread(lua_state_);
            // The thread must not be garbage collected,
            // so it need to be stored.
            script.state_id = luaL_ref(lua_state_, LUA_REGISTRYINDEX);
            script.seconds_to_wait = 0;
            script.wait = false;
            script.yield = false;
            script_entity.queue.push_back(script);
        }

        // Check the "on_update" script.
        if (luabind::type(table["on_update"]) == LUA_TFUNCTION)
        {
            QueueScript script;
            script.function = "on_update";
            script.priority = 999;
            script.state = lua_newthread(lua_state_);
            // The thread must not be garbage collected,
            // so it need to be stored.
            script.state_id = luaL_ref(lua_state_, LUA_REGISTRYINDEX);
            script.seconds_to_wait = 0;
            script.wait = false;
            script.yield = false;
            script_entity.queue.push_back(script);
        }

        script_entity_.push_back(script_entity);
    }
}

void ScriptManager::RemoveEntity(
  const ScriptManager::Type type, const Ogre::String& entity_name
){
    for (unsigned int i = 0; i < script_entity_.size(); ++ i){
        if (
          script_entity_[i].type == type && script_entity_[i].name
          == entity_name
        ){
            while(script_entity_[i].queue.size() > 0)
                ScriptManager::RemoveEntityTopScript(script_entity_[i]);
            script_entity_.erase(script_entity_.begin() + i);
            return;
        }
    }
}

void ScriptManager::RemoveEntityTopScript(ScriptEntity& entity){
    if (entity.queue.size() > 0){
        // Delete the thread.
        luaL_unref(lua_state_, LUA_REGISTRYINDEX, entity.queue[0].state_id);

        if (entity.queue[0].paused_script_end.entity != ""){
            ContinueScriptExecution(entity.queue[0].paused_script_end);
            entity.queue[0].paused_script_end.entity = "";
        }
        entity.queue.erase(entity.queue.begin());
    }
}

luabind::object ScriptManager::GetTableByEntityName(
  const ScriptManager::Type type, const Ogre::String& name, lua_State* state
) const{
    // Get the real table by name
    Ogre::StringVector table_path = StringTokenise(name, ".");
    luabind::object table = luabind::globals(state);

    switch(type){
        case ScriptManager::SYSTEM: table = table[system_table_name_]; break;
        case ScriptManager::ENTITY: table = table[entity_table_name_]; break;
        case ScriptManager::UI: table = table[ui_table_name_]; break;
    }

    if (luabind::type(table) != LUA_TTABLE) return luabind::object();

    for (unsigned int i = 0; i < table_path.size(); ++ i){
        table = table[table_path[i]];
        if (luabind::type(table) != LUA_TTABLE) return luabind::object();
    }
    return table;
}

QueueScript* ScriptManager::GetScriptByScriptId(const ScriptId& script) const{
    for (unsigned int i = 0; i < script_entity_.size(); ++ i){
        if (script.entity == script_entity_[i].name){
            for (unsigned int j = 0; j < script_entity_[i].queue.size(); ++ j){
                if (script.function == script_entity_[i].queue[j].function)
                    return (QueueScript*) &(script_entity_[i].queue[j]);
            }
            return nullptr;
        }
    }
    return nullptr;
}

ScriptEntity* ScriptManager::GetScriptEntityByName(
  const Type type, const Ogre::String& entity_name
) const{
    for (unsigned int i = 0; i < script_entity_.size(); ++ i){
        if (
          script_entity_[i].type == type && script_entity_[i].name
          == entity_name
        ){
            return (ScriptEntity*) &(script_entity_[i]);
        }
    }
    return nullptr;
}

const ScriptId ScriptManager::GetCurrentScriptId() const{
    return current_script_id_;
}

void ScriptManager::ContinueScriptExecution(const ScriptId& script){
    QueueScript* script_pointer = GetScriptByScriptId(script);
    if (script_pointer == nullptr){
        LOG_ERROR(
          "There is no script \"" + script.function + "\" for entity \""
          + script.entity + "\". Can't continue script execution."
        );
        return;
    }
    script_pointer->wait = false;
}

int ScriptManager::ScriptWait(const float seconds){
    LOG_TRIVIAL(
      "script:wait: We set script wait for "
      + Ogre::StringConverter::toString(seconds) + " seconds."
    );
    QueueScript* script = GetScriptByScriptId(current_script_id_);
    if (script == nullptr){
        LOG_ERROR("script:wait: Currently no any script running.");
        return 1;
    }
    if (seconds == 0) return 1;
    script->seconds_to_wait = seconds;
    return -1;
}

void ScriptManager::ScriptRequest(
  const Type type, const char* entity, const char* function, const int priority
){
    LOG_TRIVIAL(
      "[SCRIPT] script:request: Request function \"" + Ogre::String(function)
      + "\" for entity \"" + Ogre::String(entity) + "\" with priority "
      + Ogre::StringConverter::toString(priority) + "."
    );
    ScriptEntity* script_entity
      = GetScriptEntityByName(type, Ogre::String(entity));
    if (script_entity == nullptr){
        LOG_WARNING(
          "[SCRIPT] script:request: Entity \"" + Ogre::String(entity)
          + "\" doesn't exist."
        );
        return;
    }

    bool added
      = ScriptRequest(script_entity, function, priority, "", "", false, false);

    if (added == false){
        LOG_WARNING(
          "Script '" + Ogre::String(function) + "' for entity '"
          +  Ogre::String(entity) + "' doesn't exist."
        );
    }
}


int ScriptManager::ScriptRequestStartSync(
  const Type type, const char* entity, const char* function, const int priority
){
    LOG_TRIVIAL(
      "[SCRIPT] script:request_start_sync: Request function \""
      + Ogre::String(function) + "\" for entity \"" + Ogre::String(entity)
      + "\" with priority " + Ogre::StringConverter::toString(priority) + "."
    );

    ScriptEntity* script_entity
      = GetScriptEntityByName(type, Ogre::String(entity));

    if (script_entity == nullptr){
        LOG_ERROR(
          "[SCRIPT] script:request_start_sync: Entity \"" + Ogre::String(entity)
          + "\" doesn't exist.");
        return 1;
    }

    bool added
      = ScriptRequest(script_entity, function, priority, "", "", true, false);

    if (added == false){
        LOG_WARNING(
          "Script '" + Ogre::String(function) + "' for entity '"
          + Ogre::String(entity) + "' doesn't exist."
        );
        return 1;
    }
    return -1;
}

int ScriptManager::ScriptRequestEndSync(
  const Type type, const char* entity, const char* function, const int priority
){
    LOG_TRIVIAL(
      "[SCRIPT] script:request_end_sync: Request function \""
      + Ogre::String(function) + "\" for entity \"" + Ogre::String(entity)
      + "\" with priority " + Ogre::StringConverter::toString(priority) + "."
    );

    ScriptEntity* script_entity
      = GetScriptEntityByName(type, Ogre::String(entity));

    if (script_entity == nullptr){
        LOG_ERROR(
          "[SCRIPT] script:request_end_sync: Entity \"" + Ogre::String(entity)
          + "\" doesn't exist."
        );
        return 1;
    }

    bool added
      = ScriptRequest(script_entity, function, priority, "", "", false, true);

    if (added == false){
        LOG_WARNING(
          "Script '" + Ogre::String(function) + "' for entity '"
          +  Ogre::String(entity) + "' doesn't exist."
        );
        return 1;
    }

    return -1;
}

bool ScriptManager::ScriptRequest(
  ScriptEntity* script_entity, const Ogre::String& function, const int priority,
  const Ogre::String& argument1, const Ogre::String& argument2,
  bool start_sync, bool end_sync
){
    luabind::object table
      = GetTableByEntityName(script_entity->type, script_entity->name, lua_state_);
    if (
      table.is_valid() && luabind::type(table) == LUA_TTABLE
      && luabind::type(table[function]) == LUA_TFUNCTION
    ){
        QueueScript script;
        script.function = function;
        script.argument1 = argument1;
        script.argument2 = argument2;
        script.priority = priority;
        script.state = lua_newthread(lua_state_);
        // The thread must not be garbage collected,
        // so it need to be stored.
        script.state_id = luaL_ref(lua_state_, LUA_REGISTRYINDEX);
        script.seconds_to_wait = 0;
        script.wait = false;
        script.yield = false;
        if (start_sync == true)
            script.paused_script_start = GetCurrentScriptId();
        if (end_sync == true) script.paused_script_end = GetCurrentScriptId();
        script_entity->queue.push_back(script);
        script_entity->resort = true;
        return true;
    }
    return false;
}

void ScriptManager::AddValueToStack(const float value){
    QueueScript* script = GetScriptByScriptId(current_script_id_);
    if (script != nullptr) lua_pushnumber(script->state, value);
}
