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

#include <iostream>
#include <cmath>
#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include "core/BattleManager.h"
#include "core/ConfigVar.h"
#include "core/Logger.h"

/**
 * Battle manager singleton.
 */
template<>BattleManager *Ogre::Singleton<BattleManager>::msSingleton = nullptr;

ConfigVar cv_debug_battle_grid("debug_battle_grid", "Draw debug battle grid", "false");
ConfigVar cv_debug_battle_axis("debug_battle_axis", "Draw debug battle axis", "false");

BattleManager::BattleManager(): paused_(false){
    LOG_TRIVIAL("BattleManager created.");
    scene_node_ = Ogre::Root::getSingleton().getSceneManager("Scene")
      ->getRootSceneNode()->createChildSceneNode("BattleManager");
}

BattleManager::~BattleManager(){
    Clear();
    Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->removeAndDestroyChild(
      "BattleManager"
    );
    LOG_TRIVIAL("BattleManager destroyed.");
}

void BattleManager::Input(const VGears::Event& event){
    // TODO: Change to battle input commands.
    //background_2d_.InputDebug(event);
    if (paused_ == true) return;
    //if (event.type == VGears::ET_KEY_PRESS && event.event == "interact"){
        // TODO
    //}
}

void BattleManager::Update(){
    UpdateDebug();
    if (paused_ == true) return;

    // TODO: Update all entity scripts
    // for (unsigned int i = 0; i < party_.size(); ++ i) party_[i]->Update();
    // for (unsigned int i = 0; i < enemy_.size(); ++ i) enemy_[i]->Update();
    // TODO: Environment model update
}

void BattleManager::UpdateDebug(){
    // TODO: Update all entity scripts
    // for (unsigned int i = 0; i < party_.size(); ++ i) party_[i]->UpdateDebug();
    // for (unsigned int i = 0; i < enemy_.size(); ++ i) enemy_[i]->UpdateDebug();
    // TODO: Environment model update
}

void BattleManager::Clear(){
    paused_ = false;
    formation_id_ = -1;
    next_formation_id_ = -1;
    // TODO location_ = null;
    // TODO camera_.clear();
    initial_camera_ = 0;
    layout_ = LAYOUT::NORMAL;
    escape_difficulty_ = 0.0f;
    arena_battle_ = false;
    show_victory_pose_ = true;
    show_spoils_ = true;
    preemptive_ = true;
    money_ = 0;
    spoil_.clear();
    // TODO enemy_.clear();
    // TODO party_.clear();
    scene_node_->removeAndDestroyAllChildren();
}

void BattleManager::ScriptSetPaused(const bool paused){paused_ = paused;}

void BattleManager::SetLayout(const LAYOUT layout){
    if (layout == LAYOUT::UNKNOWN_0 || layout == LAYOUT::UNKNOWN_1) layout_ = LAYOUT::NORMAL;
    else layout_ = layout;
}

void BattleManager::SetFormationId(const int id){
    if (id < 0) formation_id_ = -1;
    else formation_id_ = id;
}

void BattleManager::SetNextFormationId(const int id){
    if (id < 0) next_formation_id_ = -1;
    else next_formation_id_ = id;
}

void BattleManager::SetEscapeability(const float difficulty){
    if (difficulty < 0.0f || difficulty > 1.0f) escape_difficulty_ = -1.0f;
    else escape_difficulty_ = difficulty;
}

void BattleManager::SetSkipVictoryPose(const bool skip){show_victory_pose_ = !skip;}

void BattleManager::SetSkipSpoils(const bool skip){show_spoils_ = !skip;}

void BattleManager::SetLocation(const int id, const Ogre::String name){
    // TODO
}

void BattleManager::SetArenaBattle(const bool arena){arena_battle_ = arena;}

void BattleManager::AddCamera(
  const unsigned int id, const int x, const int y, const int z,
  const int direction_x, const int direction_y, const int direction_z
){
    // TODO
}

void BattleManager::SetInitialCamera(const unsigned int id){initial_camera_ = id;}

void BattleManager::AddEnemy(
  const unsigned int id, const int x, const int y, const int z, const bool front_row,
  const Ogre::String cover, const bool visible, const bool target, const int script
){
    // TODO
}
