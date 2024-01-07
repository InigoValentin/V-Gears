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
#include "FF7Character.h"
#include "core/AudioManager.h"
#include "core/WorldMapManager.h"
#include "core/WorldMapManager.h"
#include "core/CameraManager.h"
#include "core/Console.h"
#include "core/DialogsManager.h"
#include "core/EntityManager.h"
#include "core/ConfigVar.h"
#include "core/InputManager.h"
#include "core/Logger.h"
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

void WorldMapManager::AddTerrain(
  const unsigned int index, const Ogre::String mesh, const Ogre::Vector3 pos
){
    if (module_ != Module::WORLD){
        LOG_ERROR(
          "Tried to add terrain to the WorldMapManager, but the manager was not in world map mode."
        );
        return;
    }
    /*Enemy* enemy = new Enemy(id, pos, front, visible, targeteable, active, cover);
    enemies_.push_back(*enemy);
    EntityManager::getSingleton().AddEntity(
      enemy->GetName() + "_" + std::to_string(enemies_.size() - 1),
      "enemies/" + enemy->GetModel() + ".mesh", enemy->GetPos(), Ogre::Degree(1),
      Ogre::Vector3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE), Ogre::Quaternion(1, 1, 0, 0), id
    );
    EntityManager::getSingleton().GetEntity(
      enemy->GetName() + "_" + std::to_string(enemies_.size() - 1)
    )->SetRotation(Ogre::Degree(180));
    */
}

void WorldMapManager::UpdateField(){}

void WorldMapManager::UpdateBattle(){}

void WorldMapManager::UpdateWorld(){}
