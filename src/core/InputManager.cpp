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
#include "core/Console.h"
#include "core/InputManager.h"
#include "core/InputManagerCommands.h"
#include "core/Logger.h"
#include "core/Timer.h"
#include "core/Manager.h"
#include "ConfigCmdHandler.h"

/**
 * Input manager singleton.
 */
template<>InputManager *Ogre::Singleton<InputManager>::msSingleton = nullptr;

InputManager::InputManager():repeat_first_wait_(true), repeat_timer_(0){
    InitCmd();
    Reset();
    Update();
}

InputManager::~InputManager(){}

void InputManager::Input(const VGears::Event& event){}

void InputManager::UpdateDebug(){}

void InputManager::OnResize(){}

void InputManager::ClearField(){}

void InputManager::ClearBattle(){}

void InputManager::ClearWorld(){}

void InputManager::Reset(){
    for (int button = 0; button < 256; ++ button) button_state_[button] = false;
}

void InputManager::ButtonPressed(int button, char text, bool down){
    // A script asked for the next key (to let the player choose one): report it as an event and do
    // nothing else with it.
    if (capture_next_key_ == true && down == true){
        capture_next_key_ = false;
        VGears::Event capture_event;
        capture_event.type = VGears::ET_KEY_PRESS;
        capture_event.param1 = button;
        capture_event.param2 = text;
        capture_event.event = "KEY_CAPTURE";
        event_queue_.push_back(capture_event);
        return;
    }
    if (button_state_[button] != down){
        button_state_[button] = down;
        button_text_[button] = text;
        VGears::Event event;
        event.type = (down == true) ? VGears::ET_KEY_PRESS : VGears::ET_KEY_RELEASE;
        event.param1 = button;
        event.param2 = text;
        event_queue_.push_back(event);
        repeat_first_wait_ = true;
        repeat_timer_ = 0;
    }
    if (Console::getSingleton().IsVisible() != true){
        if (down == true){
            ActivateBinds(button);
            AddGameEvents(button, VGears::ET_KEY_PRESS);
        }
        else AddGameEvents(button, VGears::ET_KEY_RELEASE);
    }
}

void InputManager::MousePressed(int button, bool down){
    VGears::Event event;
    event.type = (down == true) ? VGears::ET_KEY_PRESS : VGears::ET_KEY_RELEASE;
    event.param1 = button;
    event_queue_.push_back(event);
}

void InputManager::MouseMoved(int x, int y){
    VGears::Event event;
    event.type = VGears::ET_MOUSE_MOVE;
    event.param1 = x;
    event.param2 = y;
    event_queue_.push_back(event);
}

void InputManager::MouseScrolled(int value){
    VGears::Event event;
    event.type = VGears::ET_MOUSE_SCROLL;
    event.param1 = value;
    event_queue_.push_back(event);
}

void InputManager::Update(){
    repeat_timer_ += Timer::getSingleton().GetSystemTimeDelta();

    if (
      (repeat_first_wait_ == true && repeat_timer_ >= 0.5)
      || (repeat_first_wait_ == false && repeat_timer_ >= 0.05)
    ){
        for (int button = 0; button < 256; ++ button){
            if (button_state_[button] == true){
                VGears::Event event;
                event.type = VGears::ET_KEY_REPEAT_WAIT;
                event.param1 = button;
                event.param2 = button_text_[button];
                event_queue_.push_back(event);
                if (Console::getSingleton().IsVisible() != true)
                    AddGameEvents(button, VGears::ET_KEY_REPEAT_WAIT);
            }
        }
        repeat_first_wait_ = false;
        repeat_timer_ = 0;
    }

    for (int button = 0; button < 256; ++ button){
        if (button_state_[button] == true){
            VGears::Event event;
            event.type = VGears::ET_KEY_REPEAT;
            event.param1 = button;
            event.param2 = button_text_[button];
            event_queue_.push_back(event);
            if (Console::getSingleton().IsVisible() != true)
                AddGameEvents(button, VGears::ET_KEY_REPEAT);
        }
    }
}

bool InputManager::IsButtonPressed(int button) const{return button_state_[button];}

bool InputManager::IsGameEventPressed(const Ogre::String& event) const{
    if (Console::getSingleton().IsVisible() == true) return false;
    for (const BindGameEventInfo& bind : bind_game_events_){
        if (bind.event != event || bind.buttons.empty()) continue;
        bool all_pressed = true;
        for (const int button : bind.buttons){
            if (IsButtonPressed(button) == false){
                all_pressed = false;
                break;
            }
        }
        if (all_pressed == true) return true;
    }
    return false;
}

void InputManager::GetInputEvents(InputEventArray& input_events){
    input_events = event_queue_;
    event_queue_.clear();
}

void InputManager::BindCommand(
  ConfigCmd* cmd, const Ogre::StringVector& params, const ButtonList& buttons
){
    BindInfo info;
    info.cmd = cmd;
    info.params = params;
    info.buttons = buttons;
    binds_.push_back(info);
}

void InputManager::BindGameEvent(const Ogre::String& event, const ButtonList& buttons){
    BindGameEventInfo info;
    info.event = event;
    info.buttons = buttons;
    bind_game_events_.push_back(info);
}

void InputManager::ScriptSetGameEventKeys(const char* event, const char* keys){
    const Ogre::String event_name(event);
    bind_game_events_.erase(
      std::remove_if(
        bind_game_events_.begin(), bind_game_events_.end(),
        [&event_name](const BindGameEventInfo& bind){return bind.event == event_name;}
      ),
      bind_game_events_.end()
    );
    Ogre::StringVector combinations = Ogre::StringUtil::split(Ogre::String(keys), ",");
    for (Ogre::String& combination : combinations){
        Ogre::StringUtil::trim(combination);
        if (combination.empty()) continue;
        ButtonList key_codes;
        if (ParseKeys(combination, key_codes) == false) BindGameEvent(event_name, key_codes);
    }
}

std::string InputManager::ScriptGetGameEventKeys(const char* event) const{
    const Ogre::String event_name(event);
    std::string keys;
    for (const BindGameEventInfo& bind : bind_game_events_){
        if (bind.event != event_name) continue;
        if (!keys.empty()) keys += ",";
        for (unsigned int i = 0; i < bind.buttons.size(); ++ i){
            if (i > 0) keys += "+";
            keys += KeyToString(static_cast<OIS::KeyCode>(bind.buttons[i]));
        }
    }
    return keys;
}

void InputManager::ScriptCaptureNextKey(){capture_next_key_ = true;}

void InputManager::SnapshotDefaultGameEvents(){
    default_game_event_keys_.clear();
    for (const BindGameEventInfo& bind : bind_game_events_){
        if (default_game_event_keys_.count(bind.event) == 0){
            default_game_event_keys_[bind.event] = ScriptGetGameEventKeys(bind.event.c_str());
        }
    }
}

std::string InputManager::ScriptGetDefaultGameEventKeys(const char* event) const{
    const auto it = default_game_event_keys_.find(Ogre::String(event));
    return (it == default_game_event_keys_.end()) ? std::string() : it->second;
}

void InputManager::EnsureDefaultGameEvents(){
    const struct{const char* event; OIS::KeyCode key;} defaults[] = {
      {"Up", OIS::KC_UP}, {"Down", OIS::KC_DOWN}, {"Left", OIS::KC_LEFT}, {"Right", OIS::KC_RIGHT}
    };
    for (const auto& entry : defaults){
        if (ScriptGetGameEventKeys(entry.event).empty()){
            BindGameEvent(entry.event, ButtonList(1, static_cast<int>(entry.key)));
        }
    }
}

void InputManager::ActivateBinds(const int button) {
    std::vector< int > binds_indexes;
    for (unsigned int i = 0; i < binds_.size(); ++ i){
        // if we found pressed button in this bind
        if (
          std::find(binds_[i].buttons.begin(), binds_[i].buttons.end(), button)
          != binds_[i].buttons.end()
        ){
            unsigned int j = 0;
            for (; j < binds_[i].buttons.size(); ++ j)
                if (IsButtonPressed(binds_[i].buttons[j]) == false) break;
            if (j >= binds_[i].buttons.size()) binds_indexes.push_back(i);
        }
    }

    std::vector< int > binds_to_activate;
    unsigned int max_size = 0;
    for (unsigned int i = 0; i < binds_indexes.size(); ++ i){
        if (max_size < binds_[binds_indexes[i]].buttons.size()){
            max_size = binds_[binds_indexes[i]].buttons.size();
            binds_to_activate.clear();
            binds_to_activate.push_back(binds_indexes[i]);
        }
        else if (max_size == binds_[binds_indexes[i]].buttons.size())
            binds_to_activate.push_back(binds_indexes[i]);
    }

    for (unsigned int i = 0; i < binds_to_activate.size(); ++ i){
         binds_[binds_to_activate[i]].cmd->GetHandler()(
           binds_[binds_to_activate[i]].params
         );
    }
}

void InputManager::AddGameEvents(const int button, const VGears::EventType type){
    std::vector< int > binds_indexes;
    for (unsigned int i = 0; i < bind_game_events_.size(); ++ i){
        // If the button is found in this bind
        if (
          std::find(
            bind_game_events_[i].buttons.begin(), bind_game_events_[i].buttons.end(), button
          ) != bind_game_events_[i].buttons.end()
        ){
            unsigned int j = 0;
            for (; j < bind_game_events_[i].buttons.size(); ++ j){
                if (bind_game_events_[i].buttons[j] != button)
                    if (IsButtonPressed(bind_game_events_[i].buttons[j]) == false) break;
            }
            if (j >= bind_game_events_[i].buttons.size()) binds_indexes.push_back(i);
        }
    }

    std::vector< int > binds_to_activate;
    unsigned int max_size = 0;
    for (unsigned int i = 0; i < binds_indexes.size(); ++ i){
        if (max_size < bind_game_events_[binds_indexes[i]].buttons.size()){
            max_size = bind_game_events_[binds_indexes[i]].buttons.size();
            binds_to_activate.clear();
            binds_to_activate.push_back(binds_indexes[i]);
        }
        else if (max_size == bind_game_events_[binds_indexes[i]].buttons.size())
            binds_to_activate.push_back(binds_indexes[i]);
    }

    for (unsigned int i = 0; i < binds_to_activate.size(); ++ i){
        VGears::Event event;
        event.type = type;
        event.param1 = button;
        event.event = bind_game_events_[binds_to_activate[i]].event;
        event_queue_.push_back(event);
    }
}

void InputManager::UpdateField(){Update();}

void InputManager::UpdateBattle(){Update();}

void InputManager::UpdateWorld(){Update();}
