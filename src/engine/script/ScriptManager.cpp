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

#include <algorithm>
#include "engine/config/ConfigVar.h"
#include "engine/graphics/DebugDraw.h"
#include "common/Logger.h"
#include "engine/script/ScriptManager.h"
#include "engine/script/ScriptManagerBinds.h"
#include "engine/script/ScriptManagerCommands.h"
#include "engine/Timer.h"
#include "common/Utilities.h"
#include "engine/xml/XmlScriptsFile.h"
#include "LuaIncludes.h"

ConfigVar cv_debug_script(
  "debug_script", "Debug script flags. 0x01 - System, 0x02 - Entity, 0x04 - Ui.", "0"
);

/**
 * Script manager singleton.
 */
template<>ScriptManager *Ogre::Singleton<ScriptManager>::msSingleton = nullptr;

Ogre::String script_entity_type[] = {"SYSTEM", "ENTITY", "UI", "BATTLE"};

/**
 * Compares the priority of two scripts.
 *
 * @param[in] a The first script to compare.
 * @param[in] b The second script to compare.
 * @return True if the script a has a lower priority number than b, false if a has a higher or
 * equal priority than b.
 */
bool priority_queue_compare(QueueScript a, QueueScript b){return a.priority < b.priority;}

ScriptManager::ScriptManager(const std::vector<ModuleBinder>& module_binders):
  system_table_name_("System"), entity_table_name_("EntityContainer"), ui_table_name_("UiContainer")
{
    lua_state_ = lua_open();
    sol::register_main_thread(lua_state_);
    luaopen_base(lua_state_);
    luaopen_string(lua_state_);
    luaopen_table(lua_state_);
    luaopen_math(lua_state_);
    luaopen_io(lua_state_);
    InitBinds();
    for (ModuleBinder bind : module_binders) bind(lua_state_);
    InitCmd();
    // TODO: Use the one in data/ ?
    RunFile("engine/system/system.lua");
    XmlScriptsFile scripts("./engine/scripts/_scripts.xml");
    scripts.LoadScripts();
}

ScriptManager::~ScriptManager(){lua_close(lua_state_);}

void ScriptManager::Input(const VGears::Event& event){
    if (
      event.event != ""
      && (event.type == VGears::ET_KEY_PRESS || event.type == VGears::ET_KEY_REPEAT_WAIT)
    ){
        Ogre::String argument2 = "";
        if (event.type == VGears::ET_KEY_PRESS) argument2 = "Press";
        else if (event.type == VGears::ET_KEY_REPEAT_WAIT) argument2 = "Repeat";
        const Ogre::String source_key = KeyToString(
          static_cast<OIS::KeyCode>(event.param1)
        );
        for (unsigned int i = 0; i < script_entity_.size(); ++ i){
            ScriptRequest(
              &script_entity_[i], "on_button", 100,
              event.event, argument2, false, false, source_key, true
            );
        }
        return;
    }
    if (
      (event.type == VGears::ET_KEY_PRESS || event.type == VGears::ET_KEY_REPEAT_WAIT)
      && (
        // Only the keys that scripts read by themselves reach them as raw keys: the letters and the
        // space bar, to type in the name menu and for the debug keys of the battle. Everything else
        // goes through the game events of config.cfg, so it can be remapped.
        // TODO: Do this the other way: block keys NOT TO pass to scripts.
        event.param1 == OIS::KC_SPACE
        || event.param1 == OIS::KC_A || event.param1 == OIS::KC_B || event.param1 == OIS::KC_C
        || event.param1 == OIS::KC_D || event.param1 == OIS::KC_E || event.param1 == OIS::KC_F
        || event.param1 == OIS::KC_G || event.param1 == OIS::KC_H || event.param1 == OIS::KC_I
        || event.param1 == OIS::KC_J || event.param1 == OIS::KC_K || event.param1 == OIS::KC_L
        || event.param1 == OIS::KC_M || event.param1 == OIS::KC_N || event.param1 == OIS::KC_O
        || event.param1 == OIS::KC_P || event.param1 == OIS::KC_Q || event.param1 == OIS::KC_R
        || event.param1 == OIS::KC_S || event.param1 == OIS::KC_T || event.param1 == OIS::KC_U
        || event.param1 == OIS::KC_V || event.param1 == OIS::KC_W || event.param1 == OIS::KC_X
        || event.param1 == OIS::KC_Y || event.param1 == OIS::KC_Z || event.param1 == OIS::KC_X
      )
    ){

        for (unsigned int i = 0; i < script_entity_.size(); ++ i){
            Ogre::String argument2 = "";
            if (event.type == VGears::ET_KEY_PRESS) argument2 = "Press";
            else if (event.type == VGears::ET_KEY_REPEAT_WAIT) argument2 = "Repeat";
            ScriptRequest(
              &script_entity_[i], "on_button", 100,
              KeyToString((OIS::KeyCode) ((int) event.param1)), argument2, false, false
            );
        }
    }
}

void ScriptManager::Update(const ScriptManager::Type type){
    RemoveReplacedScripts(type);
    SortQueues(type);
    // The debug information is drawn before the scripts run, so the ones that run once are seen.
    DrawDebugQueues();
    // Before updating entities, call on_update on the system timer.
    RunString("Timer.update()");
    for (size_t entity = 0; entity < script_entity_.size(); ++ entity){
        if (script_entity_[entity].type == type) RunEntityScripts(entity);
    }
}

void ScriptManager::RemoveReplacedScripts(const Type type){
    for (ScriptEntity& entity : script_entity_){
        if (entity.type != type) continue;
        for (size_t k = entity.queue.size(); k --;){
            if (entity.queue[k].replaced) RemoveEntityScript(entity, k);
        }
    }
}

void ScriptManager::SortQueues(const Type type){
    for (ScriptEntity& entity : script_entity_){
        if (entity.type != type || entity.queue.empty() || !entity.resort) continue;
        std::stable_sort(entity.queue.begin(), entity.queue.end(), priority_queue_compare);
        entity.resort = false;
    }
}

void ScriptManager::DrawDebugQueues() const{
    const int debug = cv_debug_script.GetI();
    if (debug == 0) return;
    DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.LEFT);
    DEBUG_DRAW.SetScreenSpace(true);
    int y = 34;
    // The bits of the variable are the types of entities to show.
    for (unsigned int type = 0; type < 3; ++ type){
        if ((debug & (1 << type)) == 0) continue;
        DEBUG_DRAW.SetColour(Ogre::ColourValue(0.8f, 0.8f, 0.0f, 1.0f));
        DEBUG_DRAW.Text(
          10.0f, static_cast<float>(y), "Script \"" + script_entity_type[type] + "\" entity:"
        );
        y += 16;
        for (const ScriptEntity& entity : script_entity_){
            if (entity.type != type) continue;
            if (entity.queue.empty()) DEBUG_DRAW.SetColour(Ogre::ColourValue(0.5, 0.5, 0.5, 1));
            else DEBUG_DRAW.SetColour(Ogre::ColourValue(0.8f, 0.8f, 0.0f, 1.0f));
            DEBUG_DRAW.Text(20.0f, static_cast<float>(y), DescribeQueue(entity));
            y += 16;
        }
    }
}

Ogre::String ScriptManager::DescribeQueue(const ScriptEntity& entity){
    Ogre::String text = entity.name;
    for (size_t k = 0; k < entity.queue.size(); ++ k){
        const QueueScript& script = entity.queue[k];
        text += (k == 0) ? ": " : ", ";
        text += "(" + Ogre::StringConverter::toString(script.priority) + ")" + script.function;
        if (script.wait && script.seconds_to_wait != 0)
            text += ":wait(" + Ogre::StringConverter::toString(script.seconds_to_wait) + ")";
    }
    return text;
}

void ScriptManager::RunEntityScripts(const size_t entity){
    // One script of an entity is run each cycle, the first in the queue. If it yields without
    // waiting for anything, as the ones that loop for ever, or until a flag changes, do, it
    // doesn't take the cycle of the rest: the next one is run too. In the original game the
    // scripts of an entity that have different priorities all run.
    // Scripts can add entities and scripts while they run: no references to them are kept.
    for (size_t index = 0; index < script_entity_[entity].queue.size(); ++ index){
        current_script_id_.entity = script_entity_[entity].name;
        current_script_id_.function = script_entity_[entity].queue[index].function;
        bool yielded = false;
        if (script_entity_[entity].queue[index].wait) CountDownWait(entity, index);
        else if (script_entity_[entity].queue[index].yield) yielded = ResumeScript(entity, index);
        else yielded = StartScript(entity, index);
        if (!yielded && !SkipToWaitedScript(script_entity_[entity], index)) break;
    }
}

bool ScriptManager::StartScript(const size_t entity, const size_t index){
    LOG_TRIVIAL("[SCRIPT] Start " + CurrentScriptDescription() + ".");
    for (const ScriptId& waiting : script_entity_[entity].queue[index].paused_scripts_start)
        ContinueScriptExecution(waiting);
    script_entity_[entity].queue[index].paused_scripts_start.clear();
    const sol::object table = GetTableByEntityName(
      script_entity_[entity].type, current_script_id_.entity,
      script_entity_[entity].queue[index].state
    );
    sol::object function;
    if (table.get_type() == sol::type::table)
        function = table.as<sol::table>()[current_script_id_.function];
    if (table.get_type() != sol::type::table || function.get_type() != sol::type::function){
        LOG_WARNING(
          "Script \"" + current_script_id_.function + "\" for entity \""
          + current_script_id_.entity + "\" doesn't exist."
        );
        RemoveEntityScript(script_entity_[entity], index);
        return false;
    }
    const QueueScript& script = script_entity_[entity].queue[index];
    lua_State* thread = script.state;
    sol::coroutine coroutine(thread, sol::function(function));
    auto result = coroutine(table, script.argument1, script.argument2, script.argument3);
    int returned = 0;
    if (result.valid()) returned = TakeReturnValue(thread);
    else{
        const sol::error error = result;
        std::string message = "LUA error in entity " + current_script_id_.entity
          + " in function " + current_script_id_.function;
        if (lua_tostring(thread, -1) != nullptr)
            message += ". Details: " + std::string(lua_tostring(thread, -1));
        message += std::string(". Exception: ") + error.what();
        LOG_ERROR(message);
        lua_settop(thread, 0);
    }
    return HandleReturnValue(entity, index, returned);
}

bool ScriptManager::ResumeScript(const size_t entity, const size_t index){
    LOG_TRIVIAL("[SCRIPT] Continue " + CurrentScriptDescription() + ".");
    lua_State* thread = script_entity_[entity].queue[index].state;
    // sol.hpp bundles a compat53 shim: on LuaJIT this maps to the native 2-argument lua_resume.
    const int status = lua_resume(thread, nullptr, 0);
    int returned = 0;
    if (status == LUA_YIELD || status == 0) returned = TakeReturnValue(thread);
    else{
        const char* error = lua_tostring(thread, -1);
        LOG_WARNING(Ogre::String(error != nullptr ? error : "unknown lua error"));
        lua_settop(thread, 0);
    }
    return HandleReturnValue(entity, index, returned);
}

int ScriptManager::TakeReturnValue(lua_State* thread){
    // The coroutine yields with the bound function return value on top of the thread stack; a
    // completed script leaves its return value there.
    int value = 0;
    if (lua_gettop(thread) > 0 && lua_isnumber(thread, -1))
        value = static_cast<int>(lua_tointeger(thread, -1));
    lua_settop(thread, 0);
    return value;
}

bool ScriptManager::HandleReturnValue(const size_t entity, const size_t index, const int value){
    QueueScript& script = script_entity_[entity].queue[index];
    if (value == 0){
        LOG_TRIVIAL("[SCRIPT] Finished " + CurrentScriptDescription() + ".");
        // The on_update scripts stay, and start again.
        script.yield = false;
        if (script.function != "on_update") RemoveEntityScript(script_entity_[entity], index);
        return false;
    }
    script.yield = true;
    if (value == 1){
        LOG_TRIVIAL(
          "[SCRIPT] Not paused, will be continued next cycle: " + CurrentScriptDescription() + "."
        );
        return true;
    }
    LOG_TRIVIAL("[SCRIPT] Not finished yet: " + CurrentScriptDescription() + ".");
    script.wait = true;
    return false;
}

void ScriptManager::CountDownWait(const size_t entity, const size_t index){
    QueueScript& script = script_entity_[entity].queue[index];
    // Scripts that wait for something other than time are woken by it.
    if (script.seconds_to_wait <= 0) return;
    script.seconds_to_wait = std::max(
      script.seconds_to_wait - Timer::getSingleton().GetGameTimeDelta(), 0.0f
    );
    if (script.seconds_to_wait == 0) script.wait = false;
}

bool ScriptManager::SkipToWaitedScript(const ScriptEntity& entity, size_t& index){
    // The scripts of an entity don't wait for each other in the original game. If the first one is
    // waiting (for another script to end, for example) and a script that someone waits to start
    // (request_start_sync) is queued behind it, that script has to start, or the two would wait
    // for each other forever (the lines of mrkt3 do this with Aerith).
    for (size_t next = index + 1; next < entity.queue.size(); ++ next){
        const QueueScript& queued = entity.queue[next];
        if (!queued.yield && !queued.wait && !queued.paused_scripts_start.empty()){
            index = next - 1;
            return true;
        }
    }
    return false;
}

Ogre::String ScriptManager::CurrentScriptDescription() const{
    return "script \"" + current_script_id_.function + "\" for entity \""
      + current_script_id_.entity + "\"";
}

void ScriptManager::UpdateDebug(){}

void ScriptManager::OnResize(){}

void ScriptManager::ClearField(){}

void ScriptManager::ClearBattle(){
    // Remove all battle script entities.
    for (unsigned int i = 0; i < script_entity_.size(); ++ i){
        if (script_entity_[i].type == BATTLE){
            while(script_entity_[i].queue.size() > 0)
                ScriptManager::RemoveEntityTopScript(script_entity_[i]);
            script_entity_.erase(script_entity_.begin() + i);
            i --;
        }
    }
}

void ScriptManager::ClearWorld(){}

void ScriptManager::RunString(const Ogre::String& lua){
    int pre_eval_index = lua_gettop(lua_state_);
    if (luaL_dostring(lua_state_, lua.c_str()) == 1)
        LOG_ERROR("ScriptManager::RunString error: " + Ogre::String(lua_tostring(lua_state_, -1)));
    else{
        int index = lua_gettop(lua_state_);
        if (index > 0 && index != pre_eval_index){
            auto str = lua_tostring(lua_state_, index);
            LOG_CONSOLE(
              str != nullptr ? str : "ScriptManager::RunString: (nullptr; try tostring(...)?)"
            );
            lua_pop(lua_state_, 1);
        }
    }
}

float ScriptManager::EvaluateNumber(const Ogre::String& expression, const float default_value){
    const int pre_eval_index = lua_gettop(lua_state_);
    float result = default_value;
    if (luaL_dostring(lua_state_, ("return " + expression).c_str()) != 0){
        LOG_ERROR("ScriptManager::EvaluateNumber error: " + Ogre::String(lua_tostring(lua_state_, -1)));
    }
    else if (lua_gettop(lua_state_) > pre_eval_index && lua_isnumber(lua_state_, -1)){
        result = static_cast<float>(lua_tonumber(lua_state_, -1));
    }
    lua_settop(lua_state_, pre_eval_index);
    return result;
}

void ScriptManager::RunFile(const Ogre::String& file){
    if (luaL_dofile(lua_state_, ("./" + file).c_str()) == 1)
        LOG_ERROR(Ogre::String(lua_tostring(lua_state_, -1)));
}

void ScriptManager::AddEntity(
  const ScriptManager::Type type, const Ogre::String& entity_name, Entity* entity
){
    for (unsigned int i = 0; i < script_entity_.size(); ++ i){
        if (script_entity_[i].type == type && script_entity_[i].name == entity_name){
            LOG_ERROR(
              "Script \"" + script_entity_type[type] + "\" entity \""
              + entity_name + "\" already exist in script manager."
            );
            return;
        }
    }

    sol::object table = GetTableByEntityName(type, entity_name, lua_state_);

    if (table.get_type() == sol::type::table){
        ScriptEntity script_entity;
        script_entity.name = entity_name;
        script_entity.type = type;

        // Initialize entity field for model entity.
        if (entity != nullptr) table.as<sol::table>()["entity"] = entity;

        // Check the "on_start" script
        sol::object on_start = table.as<sol::table>()["on_start"];
        if (on_start.get_type() == sol::type::function){
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
        sol::object on_update = table.as<sol::table>()["on_update"];
        if (on_update.get_type() == sol::type::function){
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

void ScriptManager::RemoveEntity(const ScriptManager::Type type, const Ogre::String& entity_name){
    for (unsigned int i = 0; i < script_entity_.size(); ++ i){
        if (script_entity_[i].type == type && script_entity_[i].name == entity_name){
            while(script_entity_[i].queue.size() > 0)
                ScriptManager::RemoveEntityTopScript(script_entity_[i]);
            script_entity_.erase(script_entity_.begin() + i);
            return;
        }
    }
}

void ScriptManager::RemoveEntityScript(ScriptEntity& entity, const size_t index){
    if (entity.queue.size() > index){
        // Delete the thread.
        luaL_unref(lua_state_, LUA_REGISTRYINDEX, entity.queue[index].state_id);

        for (const ScriptId& waiting : entity.queue[index].paused_scripts_end)
            ContinueScriptExecution(waiting);
        entity.queue[index].paused_scripts_end.clear();
        entity.queue.erase(entity.queue.begin() + index);
    }
}

void ScriptManager::RemoveEntityTopScript(ScriptEntity& entity){RemoveEntityScript(entity, 0);}

sol::object ScriptManager::GetTableByEntityName(
  const ScriptManager::Type type, const Ogre::String& name, lua_State* state
) const{
    // Get the real table by name
    Ogre::StringVector table_path = StringTokenise(name, ".");
    sol::state_view lua(state);
    sol::object table = lua.globals();

    switch(type){
        case ScriptManager::SYSTEM:
          table = table.as<sol::table>()[system_table_name_]; break;
        case ScriptManager::ENTITY:
          table = table.as<sol::table>()[entity_table_name_]; break;
        case ScriptManager::UI:
          table = table.as<sol::table>()[ui_table_name_]; break;
        case ScriptManager::BATTLE:
          table = table.as<sol::table>()[battle_table_name_]; break;
    }

    if (table.get_type() != sol::type::table) return sol::make_object(state, sol::nil);

    for (unsigned int i = 0; i < table_path.size(); ++ i){
        table = table.as<sol::table>()[table_path[i]];
        if (table.get_type() != sol::type::table) return sol::make_object(state, sol::nil);
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
        if (script_entity_[i].type == type && script_entity_[i].name == entity_name)
            return (ScriptEntity*) &(script_entity_[i]);
    }
    return nullptr;
}

const ScriptId ScriptManager::GetCurrentScriptId() const{return current_script_id_;}

void ScriptManager::ContinueScriptExecution(const ScriptId& script){
    QueueScript* script_pointer = GetScriptByScriptId(script);
    if (script_pointer == nullptr){
        // The script that waited for this is already gone, for example because the field changed
        // or the script was cancelled, so there is nothing to continue.
        LOG_TRIVIAL(
          "There is no script \"" + script.function + "\" for entity \""
          + script.entity + "\" to continue. It has already ended."
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
      "[SCRIPT] script:request: Request function \"" + Ogre::String(function) + "\" for entity \""
      + Ogre::String(entity) + "\" with priority " + Ogre::StringConverter::toString(priority) + "."
    );
    ScriptEntity* script_entity = GetScriptEntityByName(type, Ogre::String(entity));
    if (script_entity == nullptr){
        LOG_WARNING(
          "[SCRIPT] script:request: Entity \"" + Ogre::String(entity) + "\" doesn't exist."
        );
        return;
    }

    bool added = ScriptRequest(script_entity, function, priority, "", "", false, false);

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

    ScriptEntity* script_entity = GetScriptEntityByName(type, Ogre::String(entity));

    if (script_entity == nullptr){
        LOG_ERROR(
          "[SCRIPT] script:request_start_sync: Entity \"" + Ogre::String(entity)
          + "\" doesn't exist.");
        return 1;
    }

    // A script that already started can't be waited for to start: nothing would resume the caller.
    for (const QueueScript& queued : script_entity->queue){
        if (queued.function == function && queued.yield){
            LOG_TRIVIAL(
              "[SCRIPT] script:request_start_sync: Function \"" + Ogre::String(function)
              + "\" for entity \"" + Ogre::String(entity) + "\" already started."
            );
            return 1;
        }
    }

    bool added = ScriptRequest(script_entity, function, priority, "", "", true, false);

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

    ScriptEntity* script_entity = GetScriptEntityByName(type, Ogre::String(entity));

    if (script_entity == nullptr){
        LOG_ERROR(
          "[SCRIPT] script:request_end_sync: Entity \"" + Ogre::String(entity)
          + "\" doesn't exist."
        );
        return 1;
    }

    bool added = ScriptRequest(script_entity, function, priority, "", "", false, true);

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
  const Ogre::String& argument1, const Ogre::String& argument2, bool start_sync,
  bool end_sync, const Ogre::String& argument3, bool allow_duplicate
){
    if (script_entity == nullptr) return false;

    sol::object table
      = GetTableByEntityName(script_entity->type, script_entity->name, lua_state_);
    sol::object table_function;
    if (table.get_type() == sol::type::table)
        table_function = table.as<sol::table>()[function];
    if (table_function.get_type() == sol::type::function){
        QueueScript script;
        script.function = function;
        script.argument1 = argument1;
        script.argument2 = argument2;
        script.argument3 = argument3;
        script.priority = priority;
        script.state = lua_newthread(lua_state_);
        // The thread must not be garbage collected,
        // so it need to be stored.
        script.state_id = luaL_ref(lua_state_, LUA_REGISTRYINDEX);
        script.seconds_to_wait = 0;
        script.wait = false;
        script.yield = false;
        if (start_sync == true) script.paused_scripts_start.push_back(GetCurrentScriptId());
        if (end_sync == true) script.paused_scripts_end.push_back(GetCurrentScriptId());

        // If the script is already queued or running, don't queue it again. Whoever waits for the
        // request must wait for the existing script, because nothing else would resume it.
        if (allow_duplicate == false){
          for (int i = 0; i < script_entity->queue.size(); i ++){
             if (script_entity->queue[i].function == function){
               script_entity->resort = true;
               QueueScript& existing = script_entity->queue[i];
               for (const ScriptId& waiting : script.paused_scripts_end)
                   existing.paused_scripts_end.push_back(waiting);
               // A script that already started isn't waited for. See ScriptRequestStartSync.
               if (!existing.yield){
                   for (const ScriptId& waiting : script.paused_scripts_start)
                       existing.paused_scripts_start.push_back(waiting);
               }
               luaL_unref(lua_state_, LUA_REGISTRYINDEX, script.state_id);
               return true;
            }
            }
        }

        // A script that is only polling (a loop that waits for a key or a flag, with no pause)
        // is replaced by the request of one with a higher priority, as the scenes do to stop the
        // loops of the keys (the mimic of the parade of junin1a).
        if (function != "on_update" && function != "on_start" && function != "on_button"){
            for (QueueScript& other : script_entity->queue){
                if (
                  other.priority > priority && other.yield && !other.wait
                  && other.function != "on_update" && other.function != "on_start"
                  && other.function != "on_button"
                ) other.replaced = true;
            }
        }
        script_entity->queue.push_back(script);
        script_entity->resort = true;
        return true;
    }
    return false;
}

void ScriptManager::UpdateField(){}

void ScriptManager::UpdateBattle(){}

void ScriptManager::UpdateWorld(){}
