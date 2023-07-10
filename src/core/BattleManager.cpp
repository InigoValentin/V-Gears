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
#include "core/BattleManager.h"
#include "core/CameraManager.h"
#include "core/DialogsManager.h"
#include "core/Enemy.h"
#include "core/EntityManager.h"
#include "core/ConfigVar.h"
#include "core/InputManager.h"
#include "core/Logger.h"
#include "core/ScriptManager.h"
#include "core/TextHandler.h"
#include "core/UiManager.h"
#include "core/XmlFormationFile.h"

/**
 * Battle manager singleton.
 */
template<>BattleManager *Ogre::Singleton<BattleManager>::msSingleton = nullptr;

ConfigVar cv_debug_battle_grid("debug_battle_grid", "Draw debug battle grid", "false");
ConfigVar cv_debug_battle_axis("debug_battle_axis", "Draw debug battle axis", "false");

const float BattleManager::ENEMY_SCALE = 0.0032f;

const float BattleManager::PARTY_SCALE = 0.0039f;

const float BattleManager::SCENE_SCALE = 0.0012f;

BattleManager::BattleManager(){
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

void BattleManager::Input(const VGears::Event& event){}

void BattleManager::UpdateDebug(){}

void BattleManager::OnResize(){}

void BattleManager::ClearField(){
    LOG_TRIVIAL("Called empty method BattleManager::ClearField()");
}

void BattleManager::ClearBattle(){
    formation_id_ = -1;
    next_formation_id_ = -1;
    // TODO location_ = null;
    camera_.clear();
    initial_camera_ = 0;
    layout_ = LAYOUT::NORMAL;
    escape_difficulty_ = 0.0f;
    arena_battle_ = false;
    show_victory_pose_ = true;
    show_spoils_ = true;
    preemptive_ = true;
    money_ = 0;
    spoil_.clear();
    enemies_.clear();
    // TODO party_.clear();
}

void BattleManager::ClearWorld(){
    LOG_TRIVIAL("Called empty method BattleManager::ClearWorld()");
}

void BattleManager::StartBattle(const unsigned int id){
    if (module_ == Module::BATTLE){
        LOG_ERROR("Started battle in BattleManager, but the manager was already in battle mode.");
        return;
    }
    SetBattleModule();
    AudioManager::getSingleton().SetBattleModule();
    CameraManager::getSingleton().SetBattleModule();
    DialogsManager::getSingleton().SetBattleModule();
    EntityManager::getSingleton().SetBattleModule();
    EntityManager::getSingleton().GetBackground2D()->Hide();
    InputManager::getSingleton().SetBattleModule();
    ScriptManager::getSingleton().SetBattleModule();
    UiManager::getSingleton().SetBattleModule();
    SetFormationId(id);

    // Load the formation XML file (name is dddd.xml)
    std::string filename = std::to_string(formation_id_);
    while (filename.length() < 4) filename = "0" + filename;
    filename = "./data/gamedata/formation/" + filename + ".xml";
    // This sets the data in the battle manager singleton.
    XmlFormationFile(filename).LoadFormation();
    // TODO: Music
    if (camera_.size() == 0)
        LOG_ERROR("Unable to start battle camera. No cameras defined in the BattleManager.");
    else if (initial_camera_ > camera_.size()){
        LOG_ERROR(
          "Unable to set initial battle camera. Default camera is set to "
          + std::to_string(initial_camera_) + " but only " + std::to_string(camera_.size())
          + " cameras are defined in the CameraManager. Defaulting to first camera.");
        CameraManager::getSingleton().StartBattleCamera(
          camera_.at(0).location, camera_.at(0).orientation
        );
    }
    else
        CameraManager::getSingleton().StartBattleCamera(
          camera_.at(initial_camera_).location, camera_.at(initial_camera_).orientation
        );
    LoadParty();
    //EndBattle();// TODO: Debug
}

void BattleManager::EndBattle(){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Ended battle in BattleManager, but the manager was not in battle mode.");
        return;
    }
    SetPreviousModule();
    ClearBattle();
    CameraManager::getSingleton().EndBattleCamera();

    AudioManager::getSingleton().SetPreviousModule();
    CameraManager::getSingleton().SetPreviousModule();
    DialogsManager::getSingleton().SetPreviousModule();
    ScriptManager::getSingleton().ClearBattle();
    EntityManager::getSingleton().ClearBattle();
    EntityManager::getSingleton().GetBackground2D()->Show();
    EntityManager::getSingleton().SetPreviousModule();
    InputManager::getSingleton().SetPreviousModule();
    ScriptManager::getSingleton().ClearBattle();
    ScriptManager::getSingleton().SetPreviousModule();
    UiManager::getSingleton().SetPreviousModule();
}

std::vector<Enemy> BattleManager::GetEnemies() const{return enemies_;}

void BattleManager::AddEnemy(
  const unsigned int id, const Ogre::Vector3 pos, const bool front, const bool visible,
  const bool targeteable, const bool active, const std::string cover
){
    if (module_ != Module::BATTLE){
        LOG_ERROR(
          "Tried to add an enemy to the BattleManager, but the manager was not in battle mode."
        );
        return;
    }
    Enemy* enemy = new Enemy(id, pos, front, visible, targeteable, active, cover);
    enemies_.push_back(*enemy);
    EntityManager::getSingleton().AddEntity(
      enemy->GetName() + "_" + std::to_string(enemies_.size() - 1),
      "enemies/" + enemy->GetModel() + ".mesh", enemy->GetPos(), Ogre::Degree(1),
      Ogre::Vector3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE), Ogre::Quaternion(1, 1, 0, 0), id
    );
    EntityManager::getSingleton().GetEntity(
      enemy->GetName() + "_" + std::to_string(enemies_.size() - 1)
    )->SetRotation(Ogre::Degree(180));
}

void BattleManager::AddCamera(
  const unsigned int id, const Ogre::Vector3 pos, const Ogre::Vector3 dir
){
    if (module_ != Module::BATTLE){
        LOG_ERROR(
          "Tried to add a camera to the BattleManager, but the manager was not in battle mode."
        );
        return;
    }
    BattleCamera camera;
    camera.id = id;
    camera.location = pos;
    camera.orientation = dir;
    camera_.push_back(camera);
}

void BattleManager::SetLayout(const LAYOUT layout){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to set a layout the BattleManager, but it's not in battle mode.");
        return;
    }
    if (layout == LAYOUT::UNKNOWN_0 || layout == LAYOUT::UNKNOWN_1) layout_ = LAYOUT::NORMAL;
    else layout_ = layout;
}

void BattleManager::SetFormationId(const int id){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to set formation int the BattleManager, but it's not in battle mode.");
        return;
    }
    if (id < 0) formation_id_ = -1;
    else formation_id_ = id;
}

void BattleManager::SetNextFormationId(const int id){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to set next formation in the BattleManager, but it's not in battle mode.");
        return;
    }
    if (id < 0) next_formation_id_ = -1;
    else next_formation_id_ = id;
}

void BattleManager::SetEscapeability(const float difficulty){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to set escapability in the BattleManager, but it's not in battle mode.");
        return;
    }
    if (difficulty < 0.0f || difficulty > 1.0f) escape_difficulty_ = -1.0f;
    else escape_difficulty_ = difficulty;
}

void BattleManager::SetSkipVictoryPose(const bool skip){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to set pose mode in the BattleManager, but it's not in battle mode.");
        return;
    }
    show_victory_pose_ = !skip;
}

void BattleManager::SetSkipSpoils(const bool skip){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to set spoils mode in the BattleManager, but it's not in battle mode.");
        return;
    }
    show_spoils_ = !skip;
}

void BattleManager::SetLocation(const int id, const Ogre::String name){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to set a location in the BattleManager, but it's not in battle mode.");
        return;
    }
    EntityManager::getSingleton().SetBackground3D(name, "scenes/oi_grassland.mesh");
    /**EntityManager::getSingleton().AddBattleEntity(
      "Background", "scenes/oi_grassland.mesh", Ogre::Vector3(0, 0, 0), Ogre::Degree(0),
      Ogre::Vector3(SCENE_SCALE, SCENE_SCALE, SCENE_SCALE), 999, true, true
    );*/
}

void BattleManager::SetArenaBattle(const bool arena){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to set arena info in the BattleManager, but it's not in battle mode.");
        return;
    }
    arena_battle_ = arena;
}

void BattleManager::SetInitialCamera(const unsigned int id){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to set the camera in the BattleManager, but it's not in battle mode.");
        return;
    }
    initial_camera_ = id;
}

void BattleManager::LoadParty(){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to load party in the BattleManager, but it's not in battle mode.");
        return;
    }
    std::vector<int> party = TextHandler::getSingleton().GetParty();
    std::vector<int> positions {};
    for (int i = 0; i < party.size(); i ++) positions.push_back(i);
    Ogre::Vector3 position = Ogre::Vector3(0, 5, 0);
    // Randomize party member positions.
    std::random_shuffle(positions.begin(), positions.end());

    // TODO: Read this from data/gamedata/characters.xml
    for (int i = 0; i < positions.size(); i ++){
        std::string name = "";
        std::string model = "";
        switch (party.at(positions.at(i))){
            case Character::CLOUD:
                name = "party_cloud";
                model = "characters/rt_cloud.mesh";
                break;
            case Character::TIFA:
                name = "party_tifa";
                model = "characters/ru_tifa.mesh";
                break;
            case Character::AERIS:
                name = "party_aeris";
                model = "characters/rv_aeris.mesh";
                break;
            case Character::BARRET:
                name = "party_barret";
                model = "characters/sb_barret.mesh";
                break;
            case Character::RED:
                name = "party_red";
                model = "characters/rw_red_xiii.mesh";
                break;
            // TODO: Cid data not installed
            case Character::CID:
                name = "party_cid";
                model = "characters/rz_cid.mesh";
                break;
            case Character::VINCENT:
                name = "party_vincent";
                model = "characters/sh_vincent.mesh";
                break;
            case Character::YUVI:
                name = "party_yuvi";
                model = "characters/rx_yuffie.mesh";
                break;
            case Character::KETCY:
                name = "party_ketcy";
                model = "characters/ry_cait_sith.mesh";
                break;
            default: continue;
        }
        EntityManager::getSingleton().AddEntity(
          name, model, position, Ogre::Degree(0),
          Ogre::Vector3(PARTY_SCALE, PARTY_SCALE, PARTY_SCALE),
          Ogre::Quaternion(1, 1, 0, 0), 100 + i
        );

        // Next position in Y axis
        position.x += ((i + 1) * (i % 2 == 0 ? 5 : -5));

    }
}

void BattleManager::UpdateField(){}

void BattleManager::UpdateBattle(){}

void BattleManager::UpdateWorld(){}
