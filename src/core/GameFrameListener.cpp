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

#include <OgreStringConverter.h>
#include "VGearsGameState.h"
#include "core/BattleManager.h"
#include "core/GameFrameListener.h"
#include "core/CameraManager.h"
#include "core/ConfigVar.h"
#include "core/Console.h"
#include "core/DebugDraw.h"
#include "core/EntityManager.h"
#include "core/InputManager.h"
#include "core/Logger.h"
#include "core/ScriptManager.h"
#include "core/Timer.h"
#include "core/UiManager.h"
#include "core/DialogsManager.h"

ConfigVar cv_debug_fps("debug_fps", "Debug FPS", "false");

GameFrameListener::GameFrameListener(Ogre::RenderWindow* win):
  window_(win), input_manager_(0), keyboard_(0)
{
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;
    win->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
#if defined __WIN32__
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
#else
    pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
    pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
    pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
#endif
    input_manager_ = OIS::InputManager::createInputSystem(pl);
    keyboard_ = static_cast<OIS::Keyboard*>(
      input_manager_->createInputObject(OIS::OISKeyboard, true)
    );
    keyboard_->setEventCallback(this);
    mouse_ = static_cast<OIS::Mouse*>(input_manager_->createInputObject(OIS::OISMouse, true));
    mouse_->setEventCallback(this);
    this->windowResized(window_);
    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(window_, this);
}

GameFrameListener::~GameFrameListener(){
    input_manager_->destroyInputObject(keyboard_);
    input_manager_->destroyInputObject(mouse_);
    OIS::InputManager::destroyInputSystem(input_manager_);
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(window_, this);
    this->windowClosed(window_);
}

bool GameFrameListener::frameStarted(const Ogre::FrameEvent& evt){
    Timer::getSingleton().AddTime(evt.timeSinceLastFrame);
    if(VGears::g_ApplicationState == VGears::G_EXIT) return false;
    if(keyboard_) keyboard_->capture();
    if(mouse_) mouse_->capture();
    InputManager::getSingleton().Update();
    InputEventArray input_event_array;
    input_event_array.clear();
    InputManager::getSingleton().GetInputEvents(input_event_array);
    bool console_active = Console::getSingleton().IsVisible();
    for (size_t i = 0; i < input_event_array.size(); ++ i){
        Console::getSingleton().Input(input_event_array[ i ]);
        if(console_active != true){
            EntityManager::getSingleton().Input(input_event_array[ i ]);
            DialogsManager::getSingleton().Input( input_event_array[ i ] );
            ScriptManager::getSingleton().Input(input_event_array[ i ]);
            CameraManager::getSingleton().Input(input_event_array[ i ], evt.timeSinceLastFrame);
        }
    }
    Console::getSingleton().Update();
    ScriptManager::getSingleton().Update(ScriptManager::SYSTEM);
    UiManager::getSingleton().Update();
    CameraManager::getSingleton().Update();
    EntityManager::getSingleton().Update();
    DialogsManager::getSingleton().Update();
    BattleManager::getSingleton().Update();
    return true;
}

bool GameFrameListener::frameEnded(const Ogre::FrameEvent& evt){
    if(cv_debug_fps.GetB() == true){
        const Ogre::RenderTarget::FrameStats& stats = window_->getStatistics();
        DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.LEFT);
        DEBUG_DRAW.SetScreenSpace(true);
        DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 1, 1, 1));
        DEBUG_DRAW.Text(10, 10, "Current FPS:" + Ogre::StringConverter::toString(stats.lastFPS));
    }
    return true;
}

void GameFrameListener::windowMoved(Ogre::RenderWindow* rw){}

void GameFrameListener::windowResized(Ogre::RenderWindow *rw){
    if(mouse_){
        const OIS::MouseState& ms = mouse_->getMouseState();
        ms.width = (int)rw->getWidth();
        ms.height = (int)rw->getHeight();
    }
    Console::getSingleton().OnResize();
    UiManager::getSingleton().OnResize();
    CameraManager::getSingleton().OnResize();
    EntityManager::getSingleton().OnResize();
}

void GameFrameListener::windowClosed(Ogre::RenderWindow* rw){
    VGears::g_ApplicationState = VGears::G_EXIT;
}

void GameFrameListener::windowFocusChange(Ogre::RenderWindow* rw){}

bool GameFrameListener::keyPressed(const OIS::KeyEvent& event){
    InputManager::getSingleton().ButtonPressed(event.key, event.text, true);
    return true;
}


bool GameFrameListener::keyReleased(const OIS::KeyEvent& event){
    InputManager::getSingleton().ButtonPressed(event.key, event.text, false);
    return true;
}


bool GameFrameListener::mouseMoved(const OIS::MouseEvent& e){
    if(e.state.Z.rel != 0) InputManager::getSingleton().MouseScrolled(e.state.Z.rel);
    else InputManager::getSingleton().MouseMoved(e.state.X.rel, e.state.Y.rel);
    return true;
}

bool GameFrameListener::mousePressed(const OIS::MouseEvent& e, OIS::MouseButtonID id){
    InputManager::getSingleton().MousePressed(id, true);
    return true;
}

bool GameFrameListener::mouseReleased(const OIS::MouseEvent& e, OIS::MouseButtonID id){
    InputManager::getSingleton().MousePressed(id, false);
    return true;
}
