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
#include "core/CameraManager.h"
#include "core/ConfigVar.h"
#include "core/DebugDraw.h"
#include "core/EntityManager.h"
#include "core/EntityModel.h"
#include "core/InputManager.h"
#include "core/Logger.h"
#include "core/ScriptManager.h"
#include "core/Timer.h"
#include "core/DialogsManager.h"

/**
 * Entity manager singleton.
 */
template<>EntityManager *Ogre::Singleton<EntityManager>::msSingleton = nullptr;

ConfigVar cv_debug_grid("debug_grid", "Draw debug grid", "false");
ConfigVar cv_debug_axis("debug_axis", "Draw debug axis", "false");

const float EntityManager::SCENE_SCALE = 0.0012f;

const unsigned int EntityManager::BATTLE_BACKGROUND_ID = 1000;

const unsigned int EntityManager::WORLD_MAP_BACKGROUND_ID = 1001;

float EntityManager::PointElevation(
  const Ogre::Vector2& point, const Ogre::Vector3& a, const Ogre::Vector3& b, const Ogre::Vector3& c
){
    float _a = a.z * (b.y - c.y) + b.z * (c.y - a.y) + c.z * (a.y - b.y);
    float _b = a.y * (b.x - c.x) + b.y * (c.x - a.x) + c.y * (a.x - b.x);
    float _c = a.x * (b.z - c.z) + b.x * (c.z - a.z) + c.x * (a.z - b.z);
    float _d
      = a.x * (b.z * c.y - c.z * b.y) + b.x * (c.z * a.y - a.z * c.y)
        + c.x * (a.z * b.y - b.z * a.y);
    return (_d - _c * point.y - _a * point.x) / _b;
}

float EntityManager::SideOfVector(
  const Ogre::Vector2& point, const Ogre::Vector2& p1, const Ogre::Vector2& p2
){
    Ogre::Vector2 AB = p2    - p1;
    Ogre::Vector2 AP = point - p1;
    return AB.x * AP.y - AB.y * AP.x;
}

float EntityManager::SquareDistanceToLine(
  const Ogre::Vector3& point, const Ogre::Vector3& point_a, const Ogre::Vector3& point_b,
  Ogre::Vector3& proj
){
    float temp =
      -((point_a.x - point.x) * (point_b.x - point_a.x) + (point_a.y - point.y)
      * (point_b.y - point_a.y) + (point_a.z - point.z) * (point_b.z - point_a.z))
      / ((point_b.x - point_a.x) * (point_b.x - point_a.x) + (point_b.y - point_a.y)
      * (point_b.y - point_a.y) + (point_b.z - point_a.z) * (point_b.z - point_a.z));

    proj.x = temp * (point_b.x - point_a.x) + point_a.x;
    proj.y = temp * (point_b.y - point_a.y) + point_a.y;
    proj.z = temp * (point_b.z - point_a.z) + point_a.z;

    if (
      ((point_a.x >= proj.x && point_b.x <= proj.x) || (point_a.x < proj.x && point_b.x >= proj.x))
      && (
        (point_a.y >= proj.y && point_b.y <= proj.y) || (point_a.y < proj.y && point_b.y >= proj.y)
      )
    ){
        return (proj.x - point.x) * (proj.x - point.x) + (proj.y - point.y)
          * (proj.y - point.y) + (proj.z - point.z) * (proj.z - point.z);
    }
    return -1;
}

Ogre::Degree EntityManager::GetDirectionToPoint(
  const Ogre::Vector3& current_point, const Ogre::Vector3& direction_point
){
    // TODO: Declare function in header file.
    Ogre::Vector2 up(0.0f, -1.0f);
    Ogre::Vector2 dir(direction_point.x - current_point.x, direction_point.y - current_point.y);
    // Angle between vectors
    Ogre::Degree angle(Ogre::Radian(acosf(dir.dotProduct(up) / (dir.length() * up.length()))));
    return (dir.x < 0) ? Ogre::Degree(360) - angle : angle;
}

EntityManager::EntityManager():
  player_entity_(nullptr),
  player_move_(Ogre::Vector3::ZERO),
  player_move_rotation_(0),
  player_lock_(false),
  player_run_(false),
  random_encounters_(false),
  encounter_rate_(0)
{
    LOG_TRIVIAL("EntityManager created.");
    scene_node_ = Ogre::Root::getSingleton().getSceneManager("Scene")
      ->getRootSceneNode()->createChildSceneNode("EntityManager");

    //grid_ = Ogre::Root::getSingleton().getSceneManager("Scene")
    //  ->createEntity("Grid", "system/grid.mesh");
    grid_ = Ogre::Root::getSingleton().getSceneManager("Scene")->createEntity("Grid", "grid.mesh");
    scene_node_->attachObject(grid_);
    //axis_ = Ogre::Root::getSingleton().getSceneManager("Scene")
    //  ->createEntity("Axis", "system/axis.mesh");
    axis_ = Ogre::Root::getSingleton().getSceneManager("Scene")->createEntity("Axis", "axis.mesh");
    scene_node_->attachObject(axis_);
}

EntityManager::~EntityManager(){
    if (grid_ != nullptr) Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(grid_);
    if (axis_ != nullptr) Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(axis_);
    Clear();
    Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->removeAndDestroyChild(
      "EntityManager"
    );
    LOG_TRIVIAL("EntityManager destroyed.");
}

void EntityManager::Input(const VGears::Event& event){
    background_2d_.InputDebug(event);
    if (paused_ == true) return;
    if (player_entity_ != NULL && player_lock_ == false){
        if (event.type == VGears::ET_KEY_REPEAT && event.event == "walk_left") player_move_.x = -1;
        else if (event.type == VGears::ET_KEY_REPEAT && event.event == "walk_right")
            player_move_.x = 1;
        if (event.type == VGears::ET_KEY_REPEAT && event.event == "walk_down") player_move_.y = -1;
        else if (event.type == VGears::ET_KEY_REPEAT && event.event == "walk_up")
            player_move_.y = 1;
        if (event.type == VGears::ET_KEY_REPEAT && event.event == "run") player_run_ = true;
        if (event.type == VGears::ET_KEY_PRESS && event.event == "interact"){
            CheckEntityInteract();
            for (unsigned int i = 0; i < entity_triggers_.size(); ++ i){
                if (
                  entity_triggers_[i]->IsEnabled() == true
                  && entity_triggers_[i]->CheckApproached() == true
                ){
                    ScriptEntity* scr_entity = ScriptManager::getSingleton().GetScriptEntityByName(
                      ScriptManager::ENTITY, entity_triggers_[i]->GetName()
                    );
                    bool added = ScriptManager::getSingleton().ScriptRequest(
                      scr_entity, "on_interact", 1, "", "", false, false
                    );
                    if (added == false){
                        LOG_WARNING(
                          "Script 'on_interact' for entity '" + entity_triggers_[i]->GetName()
                          + "' doesn't exist."
                        );
                    }
                }
            }
        }
        // Rotate move vector to field move rotation
        Ogre::Quaternion q1;
        q1.FromAngleAxis(player_move_rotation_, Ogre::Vector3::UNIT_Z);
        player_move_ = q1 * player_move_;
    }
}

void EntityManager::UpdateField(){
    // Update all entity scripts
    ScriptManager::getSingleton().Update(ScriptManager::ENTITY);

    // Set move point for player entity
    if (player_entity_ != nullptr && player_move_ != Ogre::Vector3::ZERO){
        Entity::State state = player_entity_->GetState();
        if (state == Entity::WALKMESH || state == Entity::NONE){
            player_move_
              *= player_entity_->GetMoveWalkSpeed() * Timer::getSingleton().GetGameTimeDelta();
            player_entity_->SetMovePosition(player_entity_->GetPosition() + player_move_);
            player_entity_->SetState(Entity::WALKMESH);
        }

        // HACK: If manually moving, make playable entity solid.
        // Some scripts 'forget' to make the playable entity solid at the end
        // of them, and if it remains non-solid, it can't interact with lines
        // (i.e. it cant' use gateways).
        player_entity_->SetSolid(true);
    }

    for (unsigned int i = 0; i < entity_.size(); ++ i){
        entity_[i]->Update();

        // It's not needed to update movements it delta time == 0 (for example
        // if time is stopped)
        if (Timer::getSingleton().GetGameTimeDelta() == 0) continue;

        // Update screen scroll.
        Entity* scroll_entity = background_2d_.GetAutoScrollEntity();
        if (scroll_entity == entity_[i]){
            CameraManager &camera_manager(CameraManager::getSingleton());
            Ogre::Vector3 view = camera_manager.ProjectPointToScreen(scroll_entity->GetPosition());
            Ogre::Vector2 pos = background_2d_.GetScreenScroll();
            Ogre::Viewport *viewport(camera_manager.getViewport());
            float width = float(viewport->getActualWidth());
            float height = float(viewport->getActualHeight());
            view.x = view.x - width / 2 - pos.x;
            view.y = view.y - height / 2 - pos.y;
            background_2d_.SetScreenScroll(Ogre::Vector2(-view.x, -view.y));
        }
        // Update offset.
        if (entity_[i]->GetOffsetType() != AT_NONE) SetNextOffsetStep(entity_[i]);
        // Update turn.
        if (entity_[i]->GetTurnType() != AT_NONE) SetNextTurnStep(entity_[i]);
        // Update movement.
        switch(entity_[i]->GetState()){
            case Entity::WALKMESH:
                {
                    // Try set entity on walkmesh if it's still don't has triangle.
                    if (entity_[i]->GetMoveTriangleId() == -1){
                        if (SetEntityOnWalkmesh(entity_[i]) == false){
                            LOG_TRIVIAL(
                              "Can't set entity '" + entity_[i]->GetName()
                              + "' on walkmesh. Finish move."
                            );
                            entity_[i]->UnsetMove();
                        }
                    }
                    // Perform move
                    if (entity_[i]->GetState() == Entity::WALKMESH){
                        float speed = 0;
                        if (player_lock_ == false && player_entity_ == entity_[i]){
                            speed = entity_[i]->GetMoveWalkSpeed();
                            // TODO: Set as a constant.
                            // Or even better, as a configurable variable.
                            if (player_run_ == true) speed *= 4;
                        }
                        else speed = entity_[i]->GetMoveAutoSpeed();
                        bool is_move = PerformWalkmeshMove(entity_[i], speed);
                        if (entity_[i]->GetMoveAutoAnimation() == true){
                            if (is_move == true){
                                if (speed >= entity_[i]->GetMoveRunSpeed()){
                                    entity_[i]->PlayAnimationContinue(
                                      entity_[i]->GetMoveAnimationRunName()
                                    );
                                }
                                else{
                                    entity_[i]->PlayAnimationContinue(
                                      entity_[i]->GetMoveAnimationWalkName()
                                    );
                                }
                            }
                            else if (
                              entity_[i]->GetAnimationState() != Entity::REQUESTED_ANIMATION
                            ){
                                entity_[i]->PlayAnimationContinue(
                                  entity_[i]->GetDefaultAnimationName()
                                );
                            }
                        }
                    }
                }
                break;
            case Entity::LINEAR:
                CheckTriggers(entity_[i], entity_[i]->GetPosition());
                SetNextLinearStep(entity_[i]);
                break;
            case Entity::JUMP:
                SetNextJumpStep(entity_[i]);
                break;
            case Entity::NEEDS_TO_REATTACH:
                if (SetEntityOnWalkmesh(entity_[i])) entity_[i]->SetState(Entity::WALKMESH);
                else entity_[i]->SetState(Entity::NONE);
            case Entity::NONE:
                CheckTriggers(entity_[i], entity_[i]->GetPosition());
                if (entity_[i]->GetAnimationState() != Entity::REQUESTED_ANIMATION)
                    entity_[i]->PlayAnimationContinue(entity_[i]->GetDefaultAnimationName());
                break;
        }
    }
    // Reset player move. It already must be handled in update
    player_move_ = Ogre::Vector3::ZERO;
    player_run_ = false;
    if (background_2d_.GetScrollType() != Background2D::NONE) SetNextScrollStep();
    background_2d_.Update();
}

void EntityManager::UpdateBattle(){
    ScriptManager::getSingleton().Update(ScriptManager::BATTLE);
    if (background_3d_ != nullptr){
        background_3d_->Update();
        background_3d_->PlayAnimationContinue(background_3d_->GetDefaultAnimationName());
    }
    for (unsigned int i = 0; i < battle_entity_.size(); ++ i){
        battle_entity_[i]->Update();
        battle_entity_[i]->PlayAnimationContinue(battle_entity_[i]->GetDefaultAnimationName());
    }
    //CameraManager::getSingleton().GetCurrentCamera()->roll(Ogre::Radian(0.01));
}

void EntityManager::UpdateWorld(){
    // TODO: Implement
}

void EntityManager::AddFieldOrWorldMapEntity(
  const Ogre::String& name, const Ogre::String& file_name, const Ogre::Vector3& position,
  const Ogre::Degree& rotation, const Ogre::Vector3& scale,
  const Ogre::Quaternion& root_orientation, const int index
){
    if (module_ != Module::FIELD){
        LOG_ERROR("Tried to add field or world Entity but the EntityManager is in battle mode.");
        return;
    }
    Ogre::SceneNode* node = scene_node_->createChildSceneNode("Model_" + name);
    EntityModel* entity = new EntityModel(name, file_name, node);
    entity->SetPosition(position);
    entity->SetRotation(rotation);
    entity->setScale(scale);
    entity->SetIndex(index);
    entity->setRootOrientation(root_orientation);
    entity_.push_back(entity);
    ScriptManager::getSingleton().AddEntity(ScriptManager::ENTITY, entity->GetName(), entity);
}

void EntityManager::AddBattleEntity(
  const Ogre::String& name, const Ogre::String& file_name, const Ogre::Vector3& position,
  const Ogre::Degree& orientation, const Ogre::Vector3& scale, const int index
){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to add battle Entity but the EntityManager is not in battle mode.");
        return;
    }
    Ogre::SceneNode* node = scene_node_->createChildSceneNode("Battle_" + name);
    EntityModel* entity = new EntityModel(name, file_name, node);
    entity->SetPosition(position);
    entity->SetRotation(orientation);
    entity->setScale(scale);
    entity->SetIndex(index);
    entity->SetVisible(true);
    // Battle models must be reoriented.
    node->setOrientation(1, 1, 0, 0);
    battle_entity_.push_back(entity);
    ScriptManager::getSingleton().AddEntity(ScriptManager::BATTLE, entity->GetName(), entity);

    /*std::cout << "BATTLE ANIMATIONS FOR " << name << std::endl;
    const std::map<Ogre::String, Ogre::AnimationState*>& anims
      = entity->GetModel()->getAllAnimationStates()->getAnimationStates();
    int size = anims.size();
    std::cout << "    " << std::to_string(size) << " TOTAL " << std::endl;
    for (auto anim = anims.begin(); anim != anims.end(); ++ anim){
        std::cout << "    - " << anim->first << ", " << anim->second << std::endl;
    }*/

    /* Animations have the following names:
     *  XX_00,
     *  XX_01,
     *  XX_02,
     *  ...
     * where XX is the first two characters of file_name, without the path.
     */
    Ogre::String anim = file_name.substr(file_name.find_last_of("/\\") + 1).substr(0, 2) + "_00";
    std::cout << "    Playing " << anim << std::endl;
    entity->ScriptSetDefaultAnimation(anim.c_str());


}

Entity* EntityManager::GetBackground3D() const{
    if (IsFieldModule()){
        LOG_WARNING("Tried to retrieve a 3D background while the EntityManager is in field mode.");
        return nullptr;
    }
    return background_3d_;
}

void EntityManager::SetBackground3D(const Ogre::String& name, const Ogre::String& file_name){
    if (IsFieldModule()){
        LOG_ERROR("Tried to add a 3D background while the EntityManager is in field mode.");
        return;
    }
    Ogre::SceneNode* node = scene_node_->createChildSceneNode("bg_" + name);
    background_3d_ = new EntityModel(name, file_name, node);
    background_3d_->SetPosition(Ogre::Vector3(0, 0, -2));
    background_3d_->SetRotation(Ogre::Degree(0));
    background_3d_->setScale(Ogre::Vector3(SCENE_SCALE, SCENE_SCALE, SCENE_SCALE));
    background_3d_->SetIndex(IsBattleModule() ? BATTLE_BACKGROUND_ID : WORLD_MAP_BACKGROUND_ID);
    background_3d_->SetVisible(true);
    // Backgrounds must be reoriented.
    node->setOrientation(1, 1, 0, 0);
    // Probably don't need a script for the 3d background.
    /*ScriptManager::getSingleton().AddEntity(
      ScriptManager::BATTLE, background_3d_->GetName(), background_3d_
    );*/
}

void EntityManager::UpdateDebug(){
    grid_->setVisible(cv_debug_grid.GetB());
    axis_->setVisible(cv_debug_axis.GetB());
    if (module_ == Module::BATTLE){
        if (background_3d_ != nullptr) background_3d_->UpdateDebug();
        for (unsigned int i = 0; i < battle_entity_.size(); ++ i) battle_entity_[i]->UpdateDebug();
    }
    else{
        for (unsigned int i = 0; i < entity_.size(); ++ i) entity_[i]->UpdateDebug();
        for (unsigned int i = 0; i < entity_triggers_.size(); ++ i)
            entity_triggers_[i]->UpdateDebug();
        for (unsigned int i = 0; i < entity_points_.size(); ++ i) entity_points_[i]->UpdateDebug();
        walkmesh_.UpdateDebug();
        background_2d_.UpdateDebug();
    }
}

void EntityManager::OnResize(){background_2d_.OnResize();}

void EntityManager::ClearField(){
    walkmesh_.Clear();
    background_2d_.Clear();
    DialogsManager::getSingleton().Clear();
    for (unsigned int i = 0; i < entity_.size(); ++ i){
        ScriptManager::getSingleton().RemoveEntity(ScriptManager::ENTITY, entity_[i]->GetName());
        delete entity_[i];
    }
    entity_.clear();
    player_entity_ = nullptr;
    player_move_ = Ogre::Vector3::ZERO;
    player_move_rotation_ = 0;
    player_lock_ = false;
    for (unsigned int i = 0; i < entity_triggers_.size(); ++ i){
        ScriptManager::getSingleton().RemoveEntity(
          ScriptManager::ENTITY, entity_triggers_[i]->GetName()
        );
        delete entity_triggers_[i];
    }
    entity_triggers_.clear();
    for (unsigned int i = 0; i < entity_points_.size(); ++ i) delete entity_points_[i];
    entity_points_.clear();
    for (unsigned int i = 0; i < entity_scripts_.size(); ++ i)
        ScriptManager::getSingleton().RemoveEntity(ScriptManager::ENTITY, entity_scripts_[i]);
    scene_node_->removeAndDestroyAllChildren();
}

void EntityManager::ClearBattle(){
    for (unsigned int i = 0; i < battle_entity_.size(); ++ i){
        ScriptManager::getSingleton().RemoveEntity(
          ScriptManager::ENTITY, battle_entity_[i]->GetName()
        );
        Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(
          battle_entity_[i]->GetName()
        );
        scene_node_->removeAndDestroyChild("Battle_" + battle_entity_[i]->GetName());
    }
    battle_entity_.clear();
    if (background_3d_ != nullptr){
        scene_node_->removeAndDestroyChild("bg_" + background_3d_->GetName());
        delete background_3d_;
    }
}

void EntityManager::ClearWorld(){
    // TODO implement
}

Walkmesh* EntityManager::GetWalkmesh(){return &walkmesh_;}

Background2D* EntityManager::GetBackground2D(){return &background_2d_;}

void EntityManager::AddEntity(
  const Ogre::String& name, const Ogre::String& file_name,
  const Ogre::Vector3& position, const Ogre::Degree& rotation, int index
){
    AddEntity(
      name, file_name, position, rotation,
      Ogre::Vector3::UNIT_SCALE, Ogre::Quaternion::IDENTITY, index
    );
}

void EntityManager::AddEntity(
  const Ogre::String& name, const Ogre::String& file_name, const Ogre::Vector3& position,
  const Ogre::Degree& rotation, const Ogre::Vector3& scale,
  const Ogre::Quaternion& root_orientation, const int index
){
    if (IsBattleModule())
        AddBattleEntity(name, file_name, position, rotation, scale, index);
    else
        AddFieldOrWorldMapEntity(
          name, file_name, position, rotation, scale, root_orientation, index
        );
}

void EntityManager::ScriptAddEntity(
  const char* name, const char* file_name,
  const float x, const float y, const float z, const float rotation, int index
){AddEntity(name, file_name, Ogre::Vector3(x, y, z), Ogre::Degree(rotation), index);}

void EntityManager::AddEntityTrigger(
  const Ogre::String& name,
  const Ogre::Vector3& point1, const Ogre::Vector3& point2, const bool enabled
){
    if (IsBattleModule()){
        LOG_WARNING("Tried to add an entity trigger but the EntityManager is in battle mode.");
        return;
    }
    EntityTrigger* trigger = new EntityTrigger(name);
    trigger->SetPoints(point1, point2);
    trigger->SetEnabled(enabled);
    entity_triggers_.push_back(trigger);
    ScriptManager::getSingleton().AddEntity(ScriptManager::ENTITY, trigger->GetName(), nullptr);
}

void EntityManager::AddEntityPoint(
  const Ogre::String& name, const Ogre::Vector3& position, const float rotation
){
    if (IsBattleModule()){
        LOG_WARNING("Tried to add an entity point but the EntityManager is in battle mode.");
        return;
    }
    EntityPoint* entity_point = new EntityPoint(name);
    entity_point->SetPosition(position);
    entity_point->SetRotation(rotation);
    entity_points_.push_back(entity_point);
}

void EntityManager::AddEntityScript(const Ogre::String& name){
    // TODO: Should call to this be prevented while in battle mode?
    entity_scripts_.push_back(name);
    ScriptManager::getSingleton().AddEntity(ScriptManager::ENTITY, name, nullptr);
}

void EntityManager::ScriptAddEntityScript(const char* name){AddEntityScript(name);}

Entity* EntityManager::GetEntity(const Ogre::String& name) const{
    if (IsBattleModule()){
        for (unsigned int i = 0; i < battle_entity_.size(); ++ i)
            if (battle_entity_[i]->GetName() == name) return battle_entity_[i];
    }
    else{
        for (unsigned int i = 0; i < entity_.size(); ++ i)
            if (entity_[i]->GetName() == name) return entity_[i];
    }
    return nullptr;
}

Entity* EntityManager::GetEntityFromIndex(const int index) const{
    if (IsBattleModule()){
        if (background_3d_ != nullptr && background_3d_->GetIndex() == index)
            return background_3d_;
        for (unsigned int i = 0; i < battle_entity_.size(); ++ i)
            if (battle_entity_[i]->GetIndex() == index) return battle_entity_[i];
    }
    else{
        for (unsigned int i = 0; i < entity_.size(); ++ i)
            if (entity_[i]->GetIndex() == index) return entity_[i];
    }
    return nullptr;
}

Entity* EntityManager::GetEntityFromCharacterId(const int id) const{
    if (IsBattleModule()){
        for (unsigned int i = 0; i < battle_entity_.size(); ++ i)
            if (battle_entity_[i]->IsCharacter() && battle_entity_[i]->GetCharacterId() == id)
                return battle_entity_[i];
    }
    else{
        for (unsigned int i = 0; i < entity_.size(); ++ i)
            if (entity_[i]->IsCharacter() && entity_[i]->GetCharacterId() == id) return entity_[i];
    }
    return nullptr;
}

Entity* EntityManager::ScriptGetEntity(const char* name) const{
    return GetEntity(Ogre::String(name));
}

EntityPoint* EntityManager::ScriptGetEntityPoint(const char* name) const{
    for (unsigned int i = 0; i < entity_points_.size(); ++ i)
        if (entity_points_[i]->GetName() == name) return entity_points_[i];
    return nullptr;
}

void EntityManager::ScriptSetPlayerEntity(const char* name){
    if (IsBattleModule()){
        LOG_WARNING("Tried to set a player entity but the EntityManager is in battle mode.");
        return;
    }
    for (unsigned int i = 0; i < entity_.size(); ++ i){
        if (entity_[i]->GetName() == name){
            player_entity_ = entity_[i];
            // Also, make is solid, visible, and non-talkable.
            entity_[i]->SetSolid(true);
            entity_[i]->SetVisible(true);
            entity_[i]->SetTalkable(false);
        }
    }
}

Entity* EntityManager::ScriptGetPlayerEntity() const{
    if (IsBattleModule()){
        LOG_WARNING("Tried to set a player entity but the EntityManager is in battle mode.");
        return nullptr;
    }
    return player_entity_;
}

void EntityManager::ScriptUnsetPlayerEntity(){
    if (IsBattleModule()){
        LOG_WARNING("Tried to unset a player entity but the EntityManager is in battle mode.");
        return;
    }
    player_entity_ = nullptr;
}

void EntityManager::ScriptPlayerLock(const bool lock){
    if (IsBattleModule()){
        LOG_WARNING("Tried to set player lock but the EntityManager is in battle mode.");
        return;
    }
    player_lock_ = lock;
    if (lock == true) player_move_ = Ogre::Vector3::ZERO;
}

void EntityManager::SetPlayerMoveRotation(const Ogre::Radian rotation){
    if (IsBattleModule()){
        LOG_WARNING("Tried to set player move rotation the EntityManager is in battle mode.");
        return;
    }
    player_move_rotation_ = rotation;
}

bool EntityManager::GetRandomEncounters(){return random_encounters_;}

void EntityManager::SetRandomEncounters(bool active){random_encounters_ = active;}

float EntityManager::GetEncounterRate(){return encounter_rate_;}

void EntityManager::SetEncounterRate(float rate){
    if (rate >= 1.0f) encounter_rate_ = 1.0f;
    else if (rate <= 0.0f) encounter_rate_ = 0.0f;
    else encounter_rate_ = rate;
}

bool EntityManager::IsKeyOn(unsigned int key_code){
    // If the player is locked, always false.
    // TODO: Maybe this will interfere with something down the road?
    // Some scene where the player can't move but must press a button?
    if (player_lock_) return false;
    unsigned int code = key_code;
    // Translate keycodes to game key codes.
    // For example 32 is standard for "Enter", but in game is "Circle/Action".
    // TODO: Finish this list.
    switch (key_code){
        case 32: code = OIS::KC_SPACE; break;
    }
    bool pressed = InputManager::getSingleton().IsButtonPressed(code);
    return pressed;
}

bool EntityManager::IsKeyOff(unsigned int key_code){return !IsKeyOn(key_code);}

void EntityManager::SetEntityToCharacter(const char* entity_name, unsigned int char_id){
    if (IsBattleModule()){
        for (unsigned int i = 0; i < battle_entity_.size(); ++ i){
            if (battle_entity_[i]->GetName() == entity_name){
                battle_entity_[i]->SetCharacter(char_id);
                return;
            }
        }
    }
    else{
        for (unsigned int i = 0; i < entity_.size(); ++ i){
            if (entity_[i]->GetName() == entity_name){
                entity_[i]->SetCharacter(char_id);
                return;
            }
        }
    }
    LOG_WARNING(
      "Unabe to assigne character ID " + std::to_string(char_id) + " to entity '" + entity_name
      + "'. No entity by that name exists."
    );
}

bool EntityManager::SetEntityOnWalkmesh(Entity* entity){
    if (IsBattleModule()){
        LOG_WARNING("Tried to set entity on the walkmesh, but the EntityManager is in battle mode");
        return false;
    }
    Ogre::Vector3 position3 = entity->GetPosition();
    Ogre::Vector2 position2;
    position2.x = position3.x;
    position2.y = position3.y;
    std::vector<std::pair<int, float>> triangles;
    // Search for possible triangles.
    for (int i = 0; i < walkmesh_.GetNumberOfTriangles(); ++ i){
        Ogre::Vector3 A3 = walkmesh_.GetA(i);
        Ogre::Vector3 B3 = walkmesh_.GetB(i);
        Ogre::Vector3 C3 = walkmesh_.GetC(i);
        Ogre::Vector2 A2(A3.x, A3.y);
        Ogre::Vector2 B2(B3.x, B3.y);
        Ogre::Vector2 C2(C3.x, C3.y);
        if (Ogre::Math::pointInTri2D(position2, A2, B2, C2) == true){
            triangles.push_back(std::make_pair(i, PointElevation(
              position2, walkmesh_.GetA(i), walkmesh_.GetB(i), walkmesh_.GetC(i)
            )));
        }
    }
    // If the coordinates doesn't match any triangle, exit.
    if (triangles.size() == 0 && entity == player_entity_){
        LOG_ERROR(
          "Can't find any triangle to place entity '" + entity->GetName() + "' on walkmesh."
        );
        return false;
    }
    // From the possible triangles, select the closest one (on the Z edge).
    int closest_z_triangle = -1;
    float closest_z_distance = 9999.0f;
    float pos_z = position3.z;
    for (size_t i = 0; i < triangles.size(); ++ i){
        if (std::abs(pos_z - triangles[i].second) < closest_z_distance){
            closest_z_triangle = triangles[i].first;
            closest_z_distance = std::abs(pos_z - triangles[i].second);
        }
    }
    if (closest_z_triangle == -1 && entity == player_entity_){
        LOG_ERROR(
          "Can't find nearby triangle to place entity '" + entity->GetName() + "' on walkmesh."
        );
        return false;
    }
    entity->SetPosition(Ogre::Vector3(position2.x, position2.y, pos_z));
    entity->SetMoveTriangleId(closest_z_triangle);
    return true;
}

bool EntityManager::PerformWalkmeshMove(Entity* entity, const float speed){
    if (IsBattleModule()){
        LOG_WARNING("Tried to perform a walkmesh move, but the EntityManager is in battle mode");
        return false;
    }
    Ogre::Vector3 start_point = entity->GetPosition();
    Ogre::Vector3 move_vector = entity->GetMovePosition() - start_point;
    Ogre::Vector2 direction(move_vector.x, move_vector.y);
    direction.normalise();
    direction *= Timer::getSingleton().GetGameTimeDelta();
    direction *= speed;
    // If movement is still needed but speed or time don't allow it, just return for now.
    if (direction.isZeroLength() == true){
        if (player_entity_ == entity) CheckTriggers(entity, entity->GetPosition());
        return false;
    }
    int current_triangle = entity->GetMoveTriangleId();
    if (current_triangle == -1){
        LOG_ERROR("Entity '" + entity->GetName() + "' not placed on walkmesh and can't move.");
        return false;
    }
    Ogre::Vector3 rotation(0.0f, 0.0f, 0.0f);
    Ogre::Quaternion q1(0.0f, 0.0f, 0.0f, 1.0f);
    Ogre::Vector3 end_point(0.0f, 0.0f, 0.0f);
    Ogre::Vector3 end_point2(0.0f, 0.0f, 0.0f);
    bool first_triangle_check = false;
    bool second_triangle_check = false;
    bool third_triangle_check = false;
    bool last_triangle_check = false;
    bool first_entity_check = false;
    bool second_entity_check = false;
    bool third_entity_check = false;
    // Shorten move vector by triangle angle
    end_point.x = start_point.x + direction.x;
    end_point.y = start_point.y + direction.y;
    Ogre::Vector3 A3 = walkmesh_.GetA(current_triangle);
    Ogre::Vector3 B3 = walkmesh_.GetB(current_triangle);
    Ogre::Vector3 C3 = walkmesh_.GetC(current_triangle);
    end_point.z = PointElevation(Ogre::Vector2(end_point.x, end_point.y), A3, B3, C3);
    Ogre::Vector3 temp = end_point - start_point;
    temp.normalise();
    temp = temp * direction.length();
    direction.x = temp.x;
    direction.y = temp.y;
    // Set the direction for the entity if it has to move.
    if (entity->GetMoveAutoRotation() == true){
        Ogre::Vector2 up(0.0f, -1.0f);
        // Angle between vectors
        Ogre::Degree angle(Ogre::Radian(acosf(
          direction.dotProduct(up) / (direction.length() * up.length())
        )));
        angle = (direction.x < 0) ? Ogre::Degree(360) - angle : angle;
        entity->SetRotation(Ogre::Degree(angle));
    }
    float solid = (entity->IsSolid() == true) ? entity->GetSolidRadius() : 0.01f;

    // For not playable entities, be a bit more lenient with solid collisions.
    // Don't use the solid radius.
    if (entity != player_entity_) solid /= 10;

    // Get ending point.
    end_point.z = start_point.z;
    for (
      int i = 0;
      (entity == player_entity_ && i < 16)
        || (entity == player_entity_ &&  i < 3)
        || (entity != player_entity_ && i <= 16);
      ++ i
    ){
        end_point = Ogre::Vector3(
          start_point.x + direction.x, start_point.y + direction.y, start_point.z
        );
        // 1st check.
        // Rotate move_vector +45.
        q1.FromAngleAxis(Ogre::Radian(Ogre::Degree(45)), Ogre::Vector3::UNIT_Z);
        rotation.x = direction.x;
        rotation.y = direction.y;
        rotation.z = 0;
        rotation.normalise();
        rotation = q1 * rotation;
        // Multiply move_vector by solid range
        rotation = rotation * solid;
        end_point2.x = end_point.x + rotation.x;
        end_point2.y = end_point.y + rotation.y;
        // Check_triangle.
        first_triangle_check = WalkmeshBorderCross(entity, end_point2, direction);
        entity->SetMoveTriangleId(current_triangle);
        // Model check.
        first_entity_check = CheckSolidCollisions(entity, end_point2);

        // 2nd check.
        // Rotate move_vector +45.
        q1.FromAngleAxis(Ogre::Radian(Ogre::Degree(-45)), Ogre::Vector3::UNIT_Z);
        rotation.x = direction.x;
        rotation.y = direction.y;
        rotation.z = 0;
        rotation.normalise();
        rotation = q1 * rotation;
        // Multiply move_vector by solid range.
        rotation = rotation * solid;
        end_point2.x = end_point.x + rotation.x;
        end_point2.y = end_point.y + rotation.y;
        // Check triangle.
        second_triangle_check = WalkmeshBorderCross(entity, end_point2, direction);
        entity->SetMoveTriangleId(current_triangle);
        // Model check.
        second_entity_check = CheckSolidCollisions(entity, end_point2);

        // 3rd check.
        rotation.x = direction.x;
        rotation.y = direction.y;
        rotation.z = 0;
        rotation.normalise();
        rotation = rotation * solid;
        end_point2.x = end_point.x + rotation.x;
        end_point2.y = end_point.y + rotation.y;
        // Check triangle.
        third_triangle_check = WalkmeshBorderCross(entity, end_point2, direction);
        entity->SetMoveTriangleId(current_triangle);
        // Model check.
        third_entity_check = CheckSolidCollisions(entity, end_point2);

        // Check condition and modify move_vector.
        if (
          first_triangle_check != false || second_triangle_check != false
          || third_triangle_check != false || first_entity_check != false
          || second_entity_check != false || third_entity_check != false
        ){
            // Only for NPC.
            if (entity != player_entity_){
                // If the entity collides only directly into a triangle border.
                if (
                  first_triangle_check  == false
                  && second_triangle_check == false
                  && third_triangle_check != false
                ){
                    q1.FromAngleAxis(Ogre::Radian(Ogre::Degree(-7)), Ogre::Vector3::UNIT_Z);
                    rotation.x = direction.x;
                    rotation.y = direction.y;
                    rotation = q1 * rotation;
                    direction.x = rotation.x;
                    direction.y = rotation.y;
                }
                // Or if the entity only collides into other entity directly.
                else if (
                  first_entity_check == false && second_entity_check == false
                  && third_entity_check != false
                ){
                    q1.FromAngleAxis(Ogre::Radian(Ogre::Degree(-11.25f)), Ogre::Vector3::UNIT_Z);
                    rotation.x = direction.x;
                    rotation.y = direction.y;
                    rotation = q1 * rotation;
                    direction.x = rotation.x;
                    direction.y = rotation.y;
                }

                // If not both left and right check failed.
                if (first_triangle_check == false || second_triangle_check == false){
                    if (
                      (first_triangle_check == false && first_entity_check != false)
                      || (first_triangle_check != false && second_triangle_check == false)
                    ){
                        q1.FromAngleAxis(
                          Ogre::Radian(Ogre::Degree(-11.25f)), Ogre::Vector3::UNIT_Z
                        );
                        rotation.x = direction.x;
                        rotation.y = direction.y;
                        rotation = q1 * rotation;
                        direction.x = rotation.x;
                        direction.y = rotation.y;
                    }

                    if (
                      first_triangle_check == false
                      && first_entity_check == false
                      && (second_triangle_check != false || second_entity_check != false
                      )
                    ){
                        q1.FromAngleAxis(Ogre::Radian(Ogre::Degree(11.25f)), Ogre::Vector3::UNIT_Z);
                        rotation.x = direction.x;
                        rotation.y = direction.y;
                        rotation = q1 * rotation;
                        direction.x = rotation.x;
                        direction.y = rotation.y;
                    }
                    continue;
                }
                break;
            }

            // Only for playable character.
            else{
                if (
                  first_entity_check  == false
                  && second_entity_check == false
                  && third_entity_check  == false
                ){
                    // If not both left and right check failed
                    if (first_triangle_check == false || second_triangle_check == false){
                        if (first_triangle_check != false && second_triangle_check == false){
                            q1.FromAngleAxis(
                              Ogre::Radian(Ogre::Degree(-11.25f)), Ogre::Vector3::UNIT_Z
                            );
                            rotation.x = direction.x;
                            rotation.y = direction.y;
                            rotation = q1 * rotation;
                            direction.x = rotation.x;
                            direction.y = rotation.y;
                        }

                        if (first_triangle_check == false && second_triangle_check != false){
                            q1.FromAngleAxis(
                              Ogre::Radian(Ogre::Degree(11.25f)), Ogre::Vector3::UNIT_Z
                            );
                            rotation.x = direction.x;
                            rotation.y = direction.y;
                            rotation = q1 * rotation;
                            direction.x = rotation.x;
                            direction.y = rotation.y;
                        }
                        continue;
                    }
                    break;
                }
            }
        }
    }

    // Set new X, Y and Z
    last_triangle_check = WalkmeshBorderCross(entity, end_point, direction);

    if (
      first_triangle_check != false || second_triangle_check != false
      || third_triangle_check != false || last_triangle_check != false
      || first_entity_check != false || second_entity_check != false
      || third_entity_check != false
    ){
        // Event if the entity can't move, check for triggers, but use current position.
        if (player_entity_ == entity) CheckTriggers(entity, entity->GetPosition());
        return false;
    }

    // Check triggers before set entity position because move line is checked.
    if (player_entity_ == entity) CheckTriggers(entity, end_point);
    entity->SetPosition(end_point);

    // If the entity has come to destination point, finish movement.
    Ogre::Vector3 final = entity->GetMovePosition() - end_point;
    if (Ogre::Vector2(final.x, final.y).length() <= entity->GetMoveStopDistance() + speed / 10)
        entity->UnsetMove();

    return true;
}

bool EntityManager::WalkmeshBorderCross(
  Entity* entity, Ogre::Vector3& position, const Ogre::Vector2& move_vector
){
    if (IsBattleModule()){
        LOG_WARNING("Tried to check for walkmesh cross, but the EntityManager is in battle mode");
        return false;
    }
    int current_triangle = entity->GetMoveTriangleId();
    if (current_triangle == -1) return true;
    Ogre::Vector2 pos = Ogre::Vector2(position.x, position.y);
    for (;;){
        Ogre::Vector3 A3 = walkmesh_.GetA(current_triangle);
        Ogre::Vector3 B3 = walkmesh_.GetB(current_triangle);
        Ogre::Vector3 C3 = walkmesh_.GetC(current_triangle);
        Ogre::Vector2 A(A3.x, A3.y);
        Ogre::Vector2 B(B3.x, B3.y);
        Ogre::Vector2 C(C3.x, C3.y);
        float sign1 = SideOfVector(pos, B, A);
        float sign2 = SideOfVector(pos, C, B);
        float sign3 = SideOfVector(pos, A, C);
        int next_triangle = -1;
        if (sign1 < 0) next_triangle = walkmesh_.GetAccessSide(current_triangle, 0);
        else if (sign2 < 0) next_triangle = walkmesh_.GetAccessSide(current_triangle, 1);
        else if (sign3 < 0) next_triangle = walkmesh_.GetAccessSide(current_triangle, 2);
        else{
            position.z = PointElevation(pos, A3, B3, C3);
            entity->SetMoveTriangleId(current_triangle);
            return false;
        }
        if (next_triangle >= 0){
            bool lock = walkmesh_.IsLocked(next_triangle);
            if (lock == false){
                current_triangle = next_triangle;
                continue;
            }
        }
        position.z = PointElevation(pos, A3, B3, C3);
        entity->SetMoveTriangleId(current_triangle);
        return true;
    }
}

bool EntityManager::CheckSolidCollisions(Entity* entity, Ogre::Vector3& position){
    if (IsBattleModule()){
        LOG_WARNING("Tried to check for solid collisions, but the EntityManager is in battle mode");
        return false;
    }
    if (entity->IsSolid() == false) return false;
    for (size_t i = 0; i < entity_.size(); ++ i){
        if (entity_[i]->IsSolid() == false) continue;
        if (entity_[i] == entity) continue;
        Ogre::Vector3 pos1 = entity_[i]->GetPosition();
        float solid_range = entity_[i]->GetSolidRadius();
        solid_range *= solid_range;
        float height = (pos1.z < position.z) ? entity_[i]->GetHeight() : entity->GetHeight();
        if (
          ((pos1.z - position.z + height) < (height * 2)) && ((pos1.z - position.z + height) >= 0)
        ){
            float distance
              = (pos1.x - position.x) * (pos1.x - position.x)
              + (pos1.y - position.y) * (pos1.y - position.y);
            if (distance < solid_range) return true;
        }
    }
    return false;
}

void EntityManager::CheckTriggers(Entity* entity, const Ogre::Vector3& position){
    if (IsBattleModule()){
        LOG_WARNING("Tried to check for triggers, but the EntityManager is in battle mode");
        return;
    }
    if (entity->IsSolid() == false) return;
    if (player_entity_ != entity) return;
    if (player_entity_->IsSolid() == false) return;
    if (player_lock_) return;
    bool is_moving = true;
    if (
      std::fabs(entity->GetPosition().x - position.x) < 0.005f
      && std::fabs(entity->GetPosition().y - position.y) < 0.005f
      && std::fabs(entity->GetPosition().z - position.z) < 0.005f
    ){
        is_moving = false;
    }

    for (unsigned int i = 0; i < entity_triggers_.size(); ++ i){
        ScriptEntity* trigger = ScriptManager::getSingleton().GetScriptEntityByName(
          ScriptManager::ENTITY, entity_triggers_[i]->GetName()
        );

        if (trigger != nullptr && entity_triggers_[i]->IsEnabled() == true){
            Ogre::Vector3 point_a = entity_triggers_[i]->GetPoint1();
            Ogre::Vector3 point_b = entity_triggers_[i]->GetPoint2();
            Ogre::Vector3 entity_position = entity->GetPosition();
            Ogre::Vector3 proj;

            // Calculate distance
            float square_dist = SquareDistanceToLine(entity_position, point_a, point_b, proj);
            float solid_radius = entity->GetSolidRadius();

            // Calculate direction.
            const Ogre::Degree direction_to_line = GetDirectionToPoint(entity_position, proj);
            const Ogre::Degree destination_direction_to_line = GetDirectionToPoint(position, proj);
            const Ogre::Degree movement_direction = entity->GetRotation();
            Ogre::Degree angle = direction_to_line - movement_direction + Ogre::Degree(90);
            angle = (angle > Ogre::Degree(360)) ? angle - Ogre::Degree(360) : angle;


            // Check on enter line.
            // Must happen only once when the solid radius touches the line, while facing the line,
            // while the entity is manually moving, and only if not already triggered.
            if (
              square_dist != -1 && square_dist <= solid_radius * solid_radius
              && angle < Ogre::Degree(180) && angle > Ogre::Degree(0)
              && entity_triggers_[i]->CheckApproached() == false
              && is_moving
            ){
                entity_triggers_[i]->SetApproached(true);
                //std::cout << "    [TRIGGER] " << entity->GetName() << " on_approach "
                //  << entity_triggers_[i]->GetName() << std::endl;
                ScriptManager::getSingleton().ScriptRequest(
                  trigger, "on_approach", 1, entity->GetName(), "", false, false
                );
            }

            // Check on move to line.
            // Must happen only once when the entity center touches the line, if the entity has
            // previously entered the line.
            if (
              square_dist != -1 && square_dist < 1
              && entity_triggers_[i]->CheckCrossed() == false
              && entity_triggers_[i]->CheckApproached() == true
            ){
                entity_triggers_[i]->SetCrossed(true);
                //std::cout << "    [TRIGGER] " << entity->GetName() << " on_cross "
                //  << entity_triggers_[i]->GetName() << std::endl;
                ScriptManager::getSingleton().ScriptRequest(
                  trigger, "on_cross", 1, entity->GetName(), "", false, false
                );
            }

            // Check on cross line.
            // Must happen on every frame while the entity solid radius collides with the line, if
            // the entity has previously entered the line.
            if (
              square_dist != -1 && square_dist <= solid_radius * solid_radius
              && entity_triggers_[i]->CheckApproached() == true
            ){
                // TODO: Fix this hack.
                // After the on_near script is run, set a cooltime of 5 frames before running
                // it again. This is because most of the time this trigger checks for a key, and
                // if its pressed, is run multiple times.
                if (entity_triggers_[i]->GetNearEventCooldown() == 0){
                    //std::cout << "    [TRIGGER] " << entity->GetName()
                    //  << " on_near " << entity_triggers_[i]->GetName() << std::endl;
                    ScriptManager::getSingleton().ScriptRequest(
                      trigger, "on_near", 1, entity->GetName(), "", true, true
                    );
                    entity_triggers_[i]->ResetNearEventCooldown();
                }
                else entity_triggers_[i]->DecreaseNearEventCooldown();
            }

            // Check on cross line once.
            // The same as on cross line, but only once.
            if (
              square_dist != -1 && square_dist < solid_radius * solid_radius
              && entity_triggers_[i]->CheckNearSingleEventTriggered() == false
              && entity_triggers_[i]->CheckApproached() == true
            ){
                entity_triggers_[i]->SetNearSingleEventTriggered(true);
                //std::cout << "    [TRIGGER] " << entity->GetName()
                //  << " on_near_once " << entity_triggers_[i]->GetName() << std::endl;
                ScriptManager::getSingleton().ScriptRequest(
                  trigger, "on_near_once", 1, entity->GetName(), "", false, false
                );

            }

            // Check on leave line.
            // Must be triggered once when the line is not in the entity solid radius, but only if
            // on enter script has been triggered previously.
            if (
              square_dist != -1 && square_dist > solid_radius * solid_radius
              && entity_triggers_[i]->CheckApproached()
            ){
                //std::cout << "    [TRIGGER] " << entity->GetName() << " on_leave "
                //  << entity_triggers_[i]->GetName() << std::endl;
                ScriptManager::getSingleton().ScriptRequest(
                  trigger, "on_leave", 1, entity->GetName(), "", false, false
                );
            }

            // If the entity is far enough from the line, mark every trigger as inactive
            if (square_dist == -1 || square_dist > solid_radius * solid_radius){
                entity_triggers_[i]->Clear();
                //std::cout << "    [TRIGGER] " << entity->GetName() << " clear of "
                //  << entity_triggers_[i]->GetName() << std::endl;
            }

        }
    }
}

void EntityManager::CheckEntityInteract(){
    if (IsBattleModule()){
        LOG_WARNING(
          "Tried to check for entity interaction, but the EntityManager is in battle mode"
        );
        return;
    }
    if (player_entity_ == NULL || player_lock_ == true || player_entity_->IsSolid() == false)
        return;
    Ogre::Degree angle_pc = player_entity_->GetRotation();
    Entity* entity_to_interact = NULL;
    Ogre::Degree less_angle(90);
    for (unsigned int i = 0; i < entity_.size(); ++ i){
        if (entity_[i]->IsTalkable() == false) continue;
        if (entity_[i] == player_entity_) continue;
        Ogre::Vector3 pos1 =  entity_[i]->GetPosition();
        float interact_range =  entity_[i]->GetTalkRadius();
        Ogre::Vector3 pos2 = player_entity_->GetPosition();
        float solid_range = player_entity_->GetSolidRadius();
        int height = (pos1.z < pos2.z) ? entity_[i]->GetHeight() : player_entity_->GetHeight();
        if (((pos1.z - pos2.z + height) < (height * 2)) && ((pos1.z - pos2.z + height) >= 0)){
            interact_range = (interact_range + solid_range) * (interact_range + solid_range);
            float distance
              = (pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y);
            if (distance < interact_range + solid_range){
                Ogre::Degree angle_to_model = GetDirectionToPoint(pos2, pos1);
                Ogre::Degree angle = angle_pc - angle_to_model;
                angle = (angle < Ogre::Degree(0)) ? -angle : angle;
                angle = (angle >= Ogre::Degree(180)) ? Ogre::Degree(360) - angle : angle;
                if (angle < less_angle){
                    angle = less_angle;
                    entity_to_interact = entity_[i];
                }
            }
        }
    }
    if (entity_to_interact != NULL && entity_to_interact->IsLine() == false){
        Ogre::String name = entity_to_interact->GetName();
        ScriptEntity* scr_entity = ScriptManager::getSingleton().GetScriptEntityByName(
          ScriptManager::ENTITY, name
        );
        bool added = ScriptManager::getSingleton().ScriptRequest(
          scr_entity, "on_interact", 1, "", "", false, false
        );
        if (added == false){
            LOG_WARNING("Script 'on_interact' for entity '" +  name + "' doesn't exist.");
        }
    }
}

void EntityManager::SetNextOffsetStep(Entity* entity){
    // TODO: Allow this while on battle mode?
    ActionType type = entity->GetOffsetType();
    float total = entity->GetOffsetSeconds();
    float current = entity->GetOffsetCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Vector3 start = entity->GetOffsetPositionStart();
    Ogre::Vector3 end = entity->GetOffsetPositionEnd();
    // TODO: Prevent division by 0:
    float x = current / total;
    if (std::isnan(x)) x = 0.0f;
    float smooth_modifier = (type == AT_SMOOTH) ? -2 * x * x * x + 3 * x * x : x;
    Ogre::Vector3 offset = start + (end - start) * smooth_modifier;
    entity->SetOffset(offset);
    if (current == total) entity->UnsetOffset();
    else entity->SetOffsetCurrentSeconds(current);
}

void EntityManager::SetNextTurnStep(Entity* entity){
    // TODO: Allow this while on battle mode?
    ActionType type = entity->GetTurnType();
    float total = entity->GetTurnSeconds();
    float current = entity->GetTurnCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Degree start = entity->GetTurnDirectionStart();
    Ogre::Degree end = entity->GetTurnDirectionEnd();
    // TODO: Prevent division by 0:
    float x = current / total;
    float smooth_modifier = (type == AT_SMOOTH) ? -2 * x * x * x + 3 * x * x : x;
    Ogre::Degree rotation = start + (end - start) * smooth_modifier;
    entity->SetRotation(rotation);
    if (current == total) entity->UnsetTurn();
    else entity->SetTurnCurrentSeconds(current);
}

void EntityManager::SetNextLinearStep(Entity* entity){
    if (IsBattleModule()){
        LOG_WARNING("Tried to set next linear step, but the EntityManager is in battle mode");
        return;
    }
    bool to_end = true;
    bool is_move = false;
    Ogre::Vector3 start = entity->GetLinearStart();
    Ogre::Vector3 end = entity->GetLinearEnd();
    Ogre::Vector3 current = entity->GetPosition();
    float delta = Timer::getSingleton().GetGameTimeDelta();
    if (entity == player_entity_){
        LinearMovement move = entity->GetLinearMovement();
        if (move == LM_DOWN_TO_UP || move == LM_UP_TO_DOWN){
            if (player_move_.y > 0){ // move up
                to_end = (move == LM_DOWN_TO_UP) ? true : false;
                is_move = true;
            }
            else if (player_move_.y < 0){ // move down
                to_end = (move == LM_DOWN_TO_UP) ? false : true;
                is_move = true;
            }
        }
        else if (move == LM_LEFT_TO_RIGHT || move == LM_RIGHT_TO_LEFT){
            if (player_move_.x > 0){ // move right
                to_end = (move == LM_LEFT_TO_RIGHT) ? true : false;
                is_move = true;
            }
            else if (player_move_.x < 0){ // move left
                to_end = (move == LM_LEFT_TO_RIGHT) ? false : true;
                is_move = true;
            }
        }
    }
    else is_move = true;
    if (is_move == true){
        Ogre::Vector3 position;
        if (to_end == true){
            Ogre::Vector3 end_start = end - start;
            // Move across the whole line by this number of seconds.
            float time = end_start.length() / 1.2f;
            position.x = current.x + (end_start.x / time) * delta;
            position.y = current.y + (end_start.y / time) * delta;
            position.z = current.z + (end_start.z / time) * delta;
            position.x = (end_start.x < 0)
              ? std::max(position.x, end.x) : std::min(position.x, end.x);
            position.y = (end_start.y < 0)
              ? std::max(position.y, end.y) : std::min(position.y, end.y);
            position.z = (end_start.z < 0)
              ? std::max(position.z, end.z) : std::min(position.z, end.z);
            if (position == end){
                entity->UnsetLinear();
                entity->SetMoveTriangleId(entity->GetLinearDestTriangle());
                SetEntityOnWalkmesh(entity);
            }
        }
        else{
            Ogre::Vector3 end_start = start - end;
            // Move across the whole line by this number of seconds.
            float time = end_start.length() / 1.2f;
            position.x = current.x + (end_start.x / time) * delta;
            position.y = current.y + (end_start.y / time) * delta;
            position.z = current.z + (end_start.z / time) * delta;
            position.x = (end_start.x < 0)
              ? std::max(position.x, start.x) : std::min(position.x, start.x);
            position.y = (end_start.y < 0)
              ? std::max(position.y, start.y) : std::min(position.y, start.y);
            position.z = (end_start.z < 0)
              ? std::max(position.z, start.z) : std::min(position.z, start.z);
            if (position == start){
                entity->UnsetLinear();
                SetEntityOnWalkmesh(entity);
            }
        }
        entity->SetPosition(position);
        entity->UpdateAnimation((to_end == true) ? delta : -delta);
    }
}

void EntityManager::SetNextJumpStep(Entity* entity){
    if (IsBattleModule()){
        LOG_WARNING("Tried to set next jump step, but the EntityManager is in battle mode");
        return;
    }
    float total = entity->GetJumpSeconds();
    float current = entity->GetJumpCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Vector3 start_position = entity->GetJumpStart();
    Ogre::Vector3 end_position   = entity->GetJumpEnd();
    Ogre::Vector3 position;
    position.x = start_position.x + ((end_position.x - start_position.x) / total) * current;
    position.y = start_position.y + ((end_position.y - start_position.y) / total) * current;
    position.z = current * current * -1.0f + current
      * ((end_position.z - start_position.z) / total + total * 1.0f) + start_position.z;
    entity->SetPosition(position);
    if (total == current){
        entity->UnsetJump();
        entity->SetMoveTriangleId(entity->GetJumpDestTriangle());
        SetEntityOnWalkmesh(entity);
    }
    else entity->SetJumpCurrentSeconds(current);
}

void EntityManager::SetNextScrollStep(){
    // TODO: Allow this on battle mode?
    Background2D::SCROLL_TYPE type = background_2d_.GetScrollType();
    float total = background_2d_.GetScrollSeconds();
    float current = background_2d_.GetScrollCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Vector2 start = background_2d_.GetScrollPositionStart();
    Ogre::Vector2 end = background_2d_.GetScrollPositionEnd();
    float x = current / total;
    float smooth_modifier = (type == Background2D::SMOOTH) ? -2 * x * x * x + 3 * x * x : x;
    Ogre::Vector2 scroll = start + (end - start) * smooth_modifier;
    background_2d_.SetScroll(scroll);
    if (current == total) background_2d_.UnsetScroll();
    else background_2d_.SetScrollCurrentSeconds(current);
}
