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
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include "FF7Character.h"
#include "core/AudioManager.h"
#include "core/BattleManager.h"
#include "modules/worldmap/WorldmapModule.h"
#include "core/CameraManager.h"
#include "core/Console.h"
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

const float BattleManager::ATTACK_SCALE = 0.0032f;

const float BattleManager::PARTY_SCALE = 0.0039f;

const float BattleManager::SCENE_SCALE = 0.0012f;

const float BattleManager::PARTY_Y = 5.0f;

const float BattleManager::SIDE_DISTANCE = 14.0f;

// Model rotations, in degrees. Party members face the enemies at 0 degrees, and enemies face the
// party at 180. Left and right are seen from the battle camera. If the models face the wrong
// direction in side or pincer attacks, swap these two.
const int BattleManager::FACING_TO_ENEMIES = 0;
const int BattleManager::FACING_TO_PARTY = 180;
const int BattleManager::FACING_TO_RIGHT = 90;
const int BattleManager::FACING_TO_LEFT = -90;

BattleManager::BattleManager(){
    LOG_TRIVIAL("BattleManager created.");
    scene_node_ = Ogre::Root::getSingleton().getSceneManager("Scene")
      ->getRootSceneNode()->createChildSceneNode("BattleManager");
    // Build scene model map.
    scene_model_map_ = XmlBattleScenesFile("./data/models/battle/scenes.xml").GetScenes();
    // Build character model map.
    character_model_map_
      = XmlBattleCharactersFile("./data/models/battle/characters.xml").GetCharacters();
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
    enemy_entity_name_.clear();
    party_entity_name_.clear();
    party_animation_prefix_.clear();
    party_model_name_.clear();
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
    // Battles can start in the world map, which is hidden and stopped while the battle lasts.
    battle_from_world_ = EntityManager::getSingleton().IsWorldModule();
    if (battle_from_world_) VGears::WorldmapModule::getSingleton().PauseForBattle();
    SetBattleModule();
    AudioManager::getSingleton().SetBattleModule();
    CameraManager::getSingleton().SetBattleModule();
    DialogsManager::getSingleton().SetBattleModule();
    EntityManager::getSingleton().SetBattleModule();
    // The entities of the field must not be seen in the battle.
    EntityManager::getSingleton().HideFieldEntities();
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
    LogCamera("Battle camera at the start of the battle");
    // Something could change the camera right after the battle starts (it seems to happen in some
    // battles started from field scripts), so it's set again in a few frames.
    camera_check_frames_ = 3;
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

    // Stop the battle music, and let the music of the field continue.
    AudioManager::getSingleton().EndBattleMusic();
    AudioManager::getSingleton().SetPreviousModule();
    CameraManager::getSingleton().SetPreviousModule();
    DialogsManager::getSingleton().SetPreviousModule();
    ScriptManager::getSingleton().ClearBattle();
    EntityManager::getSingleton().ClearBattle();
    EntityManager::getSingleton().GetBackground2D()->Show();
    EntityManager::getSingleton().ShowFieldEntities();
    EntityManager::getSingleton().SetPreviousModule();
    InputManager::getSingleton().SetPreviousModule();
    ScriptManager::getSingleton().ClearBattle();
    ScriptManager::getSingleton().SetPreviousModule();
    UiManager::getSingleton().SetPreviousModule();
    if (battle_from_world_){
        battle_from_world_ = false;
        VGears::WorldmapModule::getSingleton().ResumeAfterBattle();
    }
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
    Enemy enemy(id, pos, front, visible, targeteable, active, cover);
    Ogre::Vector3 position = enemy.GetPos();
    const unsigned int index = enemies_.size();
    // Enemy positions come from the formation: in normal battles they are on the negative Y side,
    // facing the party. In back attacks they are on the positive Y side, with the party mirrored
    // to the negative one. In pincer attacks they are on both sides, with the party in the middle.
    int rotation = FACING_TO_PARTY;
    switch (layout_){
        case LAYOUT::PREEMPTIVE:
            // Enemies are caught off guard, facing away from the party.
            rotation = FACING_TO_ENEMIES;
            break;
        case LAYOUT::BACK_ATTACK:
            // Enemies on the party's usual side, facing the other way.
            rotation = FACING_TO_ENEMIES;
            break;
        case LAYOUT::PINCER_ATTACK:
            // Enemies on both sides of the party, facing the center.
            rotation = (position.y > 0 ? FACING_TO_ENEMIES : FACING_TO_PARTY);
            break;
        case LAYOUT::SIDE_ATTACK:
            // Enemies stay in the middle, facing in a random direction.
            rotation = (std::rand() % 2 == 0 ? FACING_TO_LEFT : FACING_TO_RIGHT);
            break;
        default:
            break;
    }
    enemies_.push_back(enemy);
    const std::string entity_name = enemy.GetName() + "_" + std::to_string(index);
    enemy_entity_name_.push_back(entity_name);
    EntityManager::getSingleton().AddEntity(
      entity_name, "enemies/" + enemy.GetModel() + ".mesh", position, Ogre::Degree(1),
      Ogre::Vector3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE), Ogre::Quaternion(1, 1, 0, 0), id
    );
    EntityManager::getSingleton().GetEntity(entity_name)->SetRotation(Ogre::Degree(rotation));
}

unsigned int BattleManager::ScriptGetEnemyCount() const{return enemies_.size();}

Enemy* BattleManager::ScriptGetEnemy(const unsigned int index){
    if (index >= enemies_.size()) return nullptr;
    return &enemies_[index];
}

void BattleManager::AddAttackModel(
  const unsigned int id, const std::string model, const Ogre::Vector3 pos,
  const Ogre::Degree orientation, const std::string animation
){
    if (module_ != Module::BATTLE){
        LOG_ERROR(
          "Tried to add an attack model to the BattleManager,"
          + " but the manager was not in battle mode."
        );
        return;
    }
    //Enemy* enemy = new Enemy(id, pos, front, visible, targeteable, active, cover);
    //enemies_.push_back(*enemy);
    EntityManager::getSingleton().AddEntity(
      model + "_" + std::to_string(id), "attacks/" + model + ".mesh", pos, orientation,
      Ogre::Vector3(ENEMY_SCALE, ENEMY_SCALE, ENEMY_SCALE), Ogre::Quaternion(1, 1, 0, 0), id
    );
    EntityManager::getSingleton().GetEntity(model + "_" + std::to_string(id))
      ->SetRotation(orientation);
    EntityManager::getSingleton().GetEntity(model + "_" + std::to_string(id))
      ->ScriptSetDefaultAnimation(animation.c_str());
}

void BattleManager::ScriptAddAttackModel(
  const unsigned int id, const char* model, const float x, const float y, const float z,
  const int orientation, const char* animation
){
    AddAttackModel(
      id, std::string(model), Ogre::Vector3(x, y, z),
      Ogre::Degree(orientation), std::string(animation)
    );
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
    // The field script can make the battle a preemptive attack (BTLMD, flag 0x04).
    if ((battle_mode_ & 0x04) != 0 && layout_ == LAYOUT::NORMAL) layout_ = LAYOUT::PREEMPTIVE;
}

bool BattleManager::HasAttackData(const int id){
    if (!attack_index_loaded_){
        attack_index_ = XmlAttackFile::LoadIndex("./data/gamedata/attack/_index.xml");
        attack_index_loaded_ = true;
    }
    return attack_index_.find(id) != attack_index_.end();
}

const AttackData* BattleManager::GetAttackData(const int id){
    auto cached = attack_cache_.find(id);
    if (cached != attack_cache_.end()) return &cached->second;
    if (!attack_index_loaded_){
        attack_index_ = XmlAttackFile::LoadIndex("./data/gamedata/attack/_index.xml");
        attack_index_loaded_ = true;
    }
    auto file = attack_index_.find(id);
    if (file == attack_index_.end()){
        LOG_ERROR("No attack data for attack " + std::to_string(id));
        return nullptr;
    }
    AttackData data;
    if (!XmlAttackFile("./data/gamedata/attack/" + file->second).LoadAttack(data)) return nullptr;
    return &(attack_cache_[id] = data);
}

int BattleManager::ScriptGetLayout() const{return static_cast<int>(layout_);}

int BattleManager::GetNextFormationId() const{return next_formation_id_;}

void BattleManager::ScriptSetMode(const int mode){battle_mode_ = mode;}

int BattleManager::GetMode() const{return battle_mode_;}

int BattleManager::GetFormationId() const{return formation_id_;}

float BattleManager::GetEscapeDifficulty() const{return escape_difficulty_;}

bool BattleManager::IsPreemptiveAllowed() const{return preemptive_;}

void BattleManager::SetPreemptiveDisabled(const bool disabled){preemptive_ = !disabled;}

bool BattleManager::ShowVictoryPose() const{return show_victory_pose_;}

bool BattleManager::ShowSpoils() const{return show_spoils_;}

std::string BattleManager::ScriptGetEnemyEntityName(const unsigned int index) const{
    if (index >= enemy_entity_name_.size()) return "";
    return enemy_entity_name_[index];
}

std::string BattleManager::ScriptGetPartyEntityName(const unsigned int index) const{
    if (index >= party_entity_name_.size()) return "";
    return party_entity_name_[index];
}

std::string BattleManager::ScriptGetPartyModelName(const unsigned int index) const{
    if (index >= party_model_name_.size()) return "";
    return party_model_name_[index];
}

std::string BattleManager::ScriptGetPartyAnimationPrefix(const unsigned int index) const{
    if (index >= party_animation_prefix_.size()) return "";
    return party_animation_prefix_[index];
}

void BattleManager::ScriptSetEnemyVisible(const unsigned int index, const bool visible){
    if (index >= enemy_entity_name_.size()) return;
    Entity* entity = EntityManager::getSingleton().GetEntity(enemy_entity_name_[index]);
    if (entity != nullptr) entity->SetVisible(visible);
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
    std::string model = "";
    for (XmlBattleScenesFile::BattleScene scene : scene_model_map_){
        if (scene.id == id){
            model = "scenes/" + scene.model;
            break;
        }
    }
    if (model == ""){
        LOG_ERROR("No 3D model assigned to battle location " + std::to_string(id));
        return;
    }
    EntityManager::getSingleton().SetBackground3D(name, model);
}

void BattleManager::SetArenaBattle(const bool arena){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to set arena info in the BattleManager, but it's not in battle mode.");
        return;
    }
    arena_battle_ = arena;
}

int BattleManager::ScriptSetCamera(const int index){
    if (index >= 0 && index < static_cast<int>(camera_.size())){
        CameraManager::getSingleton().SetBattleCamera(
          camera_.at(index).location, camera_.at(index).orientation
        );
        LOG_TRIVIAL(
          "Battle camera " + std::to_string(index) + ": position ("
          + std::to_string(camera_.at(index).location.x) + ", "
          + std::to_string(camera_.at(index).location.y) + ", "
          + std::to_string(camera_.at(index).location.z) + "), looking at ("
          + std::to_string(camera_.at(index).orientation.x) + ", "
          + std::to_string(camera_.at(index).orientation.y) + ", "
          + std::to_string(camera_.at(index).orientation.z) + ")"
        );
    }
    return camera_.size();
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
    // Party members actually in the battle, in party order.
    std::vector<int> members {};
    for (int i = 0; i < party.size(); i ++) if (party.at(i) != -1) members.push_back(i);

    // Center of the enemies, used when the party surrounds them.
    float enemy_center_x = 0.0f;
    float enemy_center_y = -10.0f;
    if (enemies_.size() > 0){
        enemy_center_x = 0.0f;
        enemy_center_y = 0.0f;
        for (Enemy& enemy : enemies_){
            enemy_center_x += enemy.GetPos().x;
            enemy_center_y += enemy.GetPos().y;
        }
        enemy_center_x /= enemies_.size();
        enemy_center_y /= enemies_.size();
    }

    party_entity_name_.assign(party.size(), "");
    party_animation_prefix_.assign(party.size(), "");
    party_model_name_.assign(party.size(), "");
    // TODO: Read this from data/gamedata/characters.xml
    for (int slot = 0; slot < members.size(); slot ++){
        const int index = members.at(slot);
        std::string name = "";
        std::string model = "";
        for (XmlBattleCharactersFile::BattleCharacter character : character_model_map_){
            if (character.id == party.at(index)){
                model = "characters/" + character.models.at(0).model;
                name = "party_" + character.models.at(0).name;
                break;
            }
        }
        if (model == ""){
            LOG_ERROR("No 3D model assigned to battle character " + std::to_string(party.at(index)));
            return;
        }
        const float line_x = (slot - (members.size() - 1) / 2.0f) * PARTY_SPACING;
        Ogre::Vector3 position(line_x, PARTY_Y, 0);
        int rotation = FACING_TO_ENEMIES;
        switch (layout_){
            case LAYOUT::BACK_ATTACK:
                // Enemies come from behind: the party is on the other side of the field, facing
                // away from them.
                position.y = -PARTY_Y;
                break;
            case LAYOUT::SIDE_ATTACK:
                // Party members alternate on both sides of the enemies, facing the center.
                position.x = enemy_center_x + (slot % 2 == 0 ? -1 : 1) * SIDE_DISTANCE;
                position.y = enemy_center_y - 2 * (slot / 2);
                rotation = (slot % 2 == 0 ? FACING_TO_RIGHT : FACING_TO_LEFT);
                break;
            case LAYOUT::PINCER_ATTACK:
                // Party in the middle, between the two enemy groups, facing one of them.
                position.y = 0;
                rotation = (std::rand() % 2 == 0 ? FACING_TO_ENEMIES : FACING_TO_PARTY);
                break;
            default:
                break;
        }
        EntityManager::getSingleton().AddEntity(
          name, model, position, Ogre::Degree(0),
          Ogre::Vector3(PARTY_SCALE, PARTY_SCALE, PARTY_SCALE),
          Ogre::Quaternion(1, 1, 0, 0), 100 + slot
        );
        EntityManager::getSingleton().GetEntity(name)->SetRotation(Ogre::Degree(rotation));
        party_entity_name_[index] = name;
        {
            std::string base = model.substr(model.find_last_of("/\\") + 1);
            party_model_name_[index] = base.substr(0, base.find_last_of('.'));
        }
        party_animation_prefix_[index] = model.substr(model.find_last_of("/\\") + 1).substr(0, 2);
    }
}

void BattleManager::UpdateField(){}

void BattleManager::LogCamera(const std::string& title) const{
    Ogre::Camera* camera = CameraManager::getSingleton().GetCurrentCamera();
    if (camera == nullptr) return;
    const Ogre::Vector3 position = camera->getPosition();
    const Ogre::Vector3 direction = camera->getDirection();
    LOG_TRIVIAL(
      title + ": position (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ", "
      + std::to_string(position.z) + "), direction (" + std::to_string(direction.x) + ", "
      + std::to_string(direction.y) + ", " + std::to_string(direction.z) + "), fov "
      + std::to_string(camera->getFOVy().valueDegrees()) + " degrees"
    );
}

void BattleManager::UpdateBattle(){
    if (camera_check_frames_ > 0){
        camera_check_frames_ --;
        if (camera_check_frames_ == 0 && camera_.size() > 0){
            LogCamera("Battle camera three frames after the start");
            ScriptSetCamera(initial_camera_ < camera_.size() ? initial_camera_ : 0);
            LogCamera("Battle camera set again");
        }
    }
    ScriptManager::getSingleton().RunString("UiContainer.BattleUi:tick()");
}

void BattleManager::UpdateWorld(){}
