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
#include <iostream>
#include <cmath>
#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include "common/FF7Character.h"
#include "engine/audio/AudioManager.h"
#include "core/WorldMapManager.h"
#include "core/WorldMapManager.h"
#include "engine/graphics/CameraManager.h"
#include "core/Console.h"
#include "core/DialogsManager.h"
#include "engine/entity/EntityManager.h"
#include "engine/config/ConfigVar.h"
#include "engine/input/InputManager.h"
#include "common/Logger.h"
#include "core/ScriptManager.h"
#include "core/TextHandler.h"
#include "core/UiManager.h"

/**
 * World Map manager singleton.
 */
template<>WorldMapManager *Ogre::Singleton<WorldMapManager>::msSingleton = nullptr;

WorldMapManager::WorldMapManager(){
    LOG_TRIVIAL("WorldMapManager created.");
    scene_node_ = Ogre::Root::getSingleton().getSceneManager("Scene")
      ->getRootSceneNode()->createChildSceneNode("WorldMapManager");
}

WorldMapManager::~WorldMapManager(){
    Clear();
    Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->removeAndDestroyChild(
      "WorldMapManager"
    );
    LOG_TRIVIAL("WorldMapManager destroyed.");
}

void WorldMapManager::Input(const VGears::Event& event){}

void WorldMapManager::UpdateDebug(){}

void WorldMapManager::OnResize(){}

void WorldMapManager::ClearField(){}

void WorldMapManager::ClearBattle(){}

void WorldMapManager::ClearWorld(){
    // TODO
}

void WorldMapManager::UpdateField(){}

void WorldMapManager::UpdateBattle(){}

void WorldMapManager::UpdateWorld(){}
