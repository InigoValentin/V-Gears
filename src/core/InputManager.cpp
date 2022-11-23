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

#include "core/ConfigCmdManager.h"
#include "core/Console.h"
#include "core/InputManager.h"
#include "core/InputManagerCommands.h"
#include "core/Logger.h"
#include "core/Timer.h"

/**
 * Input manager singleton.
 */
template<>InputManager *Ogre::Singleton<InputManager>::msSingleton = nullptr;

InputManager::InputManager():
  repeat_first_wait_(true),
  repeat_timer_(0)
{
    InitCmd();
    Reset();
}

InputManager::~InputManager(){}


void InputManager::Reset(){
    for (int button = 0; button < 256; ++ button) button_state_[button] = false;
}

void InputManager::ButtonPressed(int button, char text, bool down){
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
          )
          != bind_game_events_[i].buttons.end()
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
        event.event = bind_game_events_[binds_to_activate[i]].event;
        event_queue_.push_back(event);
    }
}
