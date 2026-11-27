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
#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include "engine/graphics/CameraManager.h"
#include "engine/config/ConfigVar.h"
#include "engine/graphics/DebugDraw.h"
#include "engine/entity/EntityManager.h"
#include "engine/entity/EntityModel.h"
#include "common/Logger.h"
#include "engine/script/ScriptManager.h"
#include "engine/Timer.h"
#include "engine/ui/DialogsManager.h"

/**
 * Entity manager singleton.
 */
template<>EntityManager *Ogre::Singleton<EntityManager>::msSingleton = nullptr;

ConfigVar cv_debug_grid("debug_grid", "Draw debug grid", "false");
ConfigVar cv_debug_axis("debug_axis", "Draw debug axis", "false");

const float EntityManager::SCENE_SCALE = 1.0f / 255.0f;

const unsigned int EntityManager::BATTLE_BACKGROUND_ID = 1000;

const unsigned int EntityManager::WORLD_MAP_BACKGROUND_ID = 1001;

EntityManager::EntityManager():
    background_3d_(nullptr),
  player_entity_(nullptr),
  player_move_(Ogre::Vector3::ZERO),
  player_move_rotation_(0),
  player_lock_(false),
  player_run_(false),
  random_encounters_(false),
  encounter_rate_(0),
  encounter_table_(1),
  encounter_distance_(0.0f),
  encounter_last_position_(Ogre::Vector3::ZERO),
  encounter_last_position_valid_(false),
  encounter_safe_steps_(0)
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

void EntityManager::UpdateField(){
    // Update all entity scripts
    ScriptManager::getSingleton().Update(ScriptManager::ENTITY);
    // The player is walking if there is a move set by the input.
    const bool walked = player_entity_ != nullptr && player_move_ != Ogre::Vector3::ZERO;

    // Set move point for player entity
    if (player_entity_ != nullptr && player_move_ != Ogre::Vector3::ZERO){
        Entity::State state = player_entity_->GetState();
        if (state == Entity::WALKMESH || state == Entity::NONE){
            // The directional keys are relative to the screen. Rotate them to the field's move
            // rotation here, once per frame. This can't be done in Input, which runs once for each
            // event, so the rotation would be applied several times when there is more than one
            // event in the frame (diagonals, running, ...). player_move_ stays as the raw input,
            // which is what ladders need.
            // A scripted walk of an entity that isn't the player keeps the last triangle when it
            // goes over an edge, so the stored triangle can be far from the position. Reattach to
            // the walkmesh before moving manually, otherwise every move would be blocked.
            const int current_triangle = player_entity_->GetMoveTriangleId();
            bool in_current_triangle = false;
            if (current_triangle >= 0 && current_triangle < walkmesh_.GetNumberOfTriangles()){
                const Ogre::Vector3 position = player_entity_->GetPosition();
                const Ogre::Vector3 A = walkmesh_.GetA(current_triangle);
                const Ogre::Vector3 B = walkmesh_.GetB(current_triangle);
                const Ogre::Vector3 C = walkmesh_.GetC(current_triangle);
                in_current_triangle = Ogre::Math::pointInTri2D(
                  Ogre::Vector2(position.x, position.y), Ogre::Vector2(A.x, A.y),
                  Ogre::Vector2(B.x, B.y), Ogre::Vector2(C.x, C.y)
                );
            }
            if (!in_current_triangle) SetEntityOnWalkmesh(player_entity_);

            Ogre::Quaternion rotation;
            rotation.FromAngleAxis(player_move_rotation_, Ogre::Vector3::UNIT_Z);
            // Normalized, so diagonals aren't faster than straight moves.
            Ogre::Vector3 input = player_move_;
            input.normalise();
            const Ogre::Vector3 walk_move = rotation * input
              * (player_entity_->GetMoveWalkSpeed() * Timer::getSingleton().GetGameTimeDelta());
            player_entity_->SetMovePosition(player_entity_->GetPosition() + walk_move);
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
        if (scroll_entity == entity_[i])
            background_2d_.SetScroll(background_2d_.GetEntityScroll(scroll_entity));
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
                            // The run key inverts the default movement.
                            if (player_run_ != player_default_run_) speed *= 4;
                        }
                        else speed = entity_[i]->GetMoveAutoSpeed();
                        float remaining_time = Timer::getSingleton().GetGameTimeDelta();
                        bool is_move = false;
                        if (entity_[i]->IsScriptedMove() && speed > 0.0f){
                            const float max_step_time = 0.2f / speed;
                            while (remaining_time > 0.0f && entity_[i]->GetState() == Entity::WALKMESH){
                                const float step_time = std::min(remaining_time, max_step_time);
                                const bool moved = PerformWalkmeshMove(entity_[i], speed, step_time);
                                is_move = is_move || moved;
                                if (!moved) break;
                                remaining_time -= step_time;
                            }
                        }
                        else is_move = PerformWalkmeshMove(entity_[i], speed, remaining_time);
                        // The scripts set the animation of a scene as the default one (DFANM) and
                        // don't set it back, but the player stands normally again as soon as they
                        // walk in the original game (blin67_3, where Cloud ends the scene crouched).
                        if (
                          is_move == true && player_lock_ == false && player_entity_ == entity_[i]
                          && entity_[i]->GetDefaultAnimationName() != "Idle"
                        ){
                            entity_[i]->ScriptSetDefaultAnimation("Idle");
                        }
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
    CheckEntityContacts();
    // Reset player move. It already must be handled in update
    player_move_ = Ogre::Vector3::ZERO;
    player_run_ = false;
    if (background_2d_.GetScrollType() != Background2D::NONE) SetNextScrollStep();
    background_2d_.Update();
    // This can start a battle and change the module, so it's the last thing to do.
    UpdateEncounters(walked);
    // The movie follows the speed of the game, so the scenes that are timed with it stay in sync.
    // Its sound doesn't, so at any other speed they drift apart.
    movie_player_.Update(Timer::getSingleton().GetGameTimeDelta());
    screen_fade_.Update(Timer::getSingleton().GetGameTimeDelta());
}

void EntityManager::UpdateBattle(){
    ScriptManager::getSingleton().Update(ScriptManager::BATTLE);
    if (background_3d_ != nullptr){
        background_3d_->Update();
        background_3d_->PlayAnimationContinue(background_3d_->GetDefaultAnimationName());
    }
    for (unsigned int i = 0; i < battle_entity_.size(); ++ i){
        battle_entity_[i]->Update();
        // Go back to the default animation, unless an animation has been requested. Requested
        // animations that are not looped go back to the default one by themselves when they end.
        if (battle_entity_[i]->GetAnimationState() != Entity::REQUESTED_ANIMATION){
            battle_entity_[i]->PlayAnimationContinue(battle_entity_[i]->GetDefaultAnimationName());
        }
    }
    //CameraManager::getSingleton().GetCurrentCamera()->roll(Ogre::Radian(0.01));
}

void EntityManager::UpdateWorld(){
    ScriptManager::getSingleton().Update(ScriptManager::ENTITY);
    for (Entity* entity : entity_){
        entity->Update();
    }
    player_move_ = Ogre::Vector3::ZERO;
    player_run_ = false;
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
    // The models of a field are visible, solid and talkable until a script changes it (VISI 0,
    // SOLID 1, TLKON 1). Many entities, such as the doors of mds6_2 or the people of mrkt2, are
    // never turned on by a script. Setting a position doesn't turn them on.
    entity->SetVisible(true);
    entity->SetSolid(true);
    entity->SetTalkable(true);
    entity_.push_back(entity);
    ScriptManager::getSingleton().AddEntity(ScriptManager::ENTITY, entity->GetName(), entity);
}

EntityModel* EntityManager::AddWorldMapEntity(
  const Ogre::String& name, const Ogre::String& file_name,
  const Ogre::Vector3& position, const Ogre::Degree& rotation,
  const Ogre::Vector3& scale, const Ogre::String& resource_group
){
    if (!IsWorldModule()){
        LOG_ERROR("Tried to add a world-map entity outside world-map mode.");
        return nullptr;
    }
    if (GetEntity(name) != nullptr){
        LOG_ERROR("Tried to add duplicate world-map entity '" + name + "'.");
        return nullptr;
    }

    Ogre::SceneNode* node = scene_node_->createChildSceneNode("Model_" + name);
    EntityModel* entity = new EntityModel(name, file_name, node, resource_group);
    entity->SetPosition(position);
    entity->SetRotation(rotation);
    entity->setScale(scale);
    entity->SetVisible(true);
    entity_.push_back(entity);
    ScriptManager::getSingleton().AddEntity(ScriptManager::ENTITY, entity->GetName(), entity);
    return entity;
}

void EntityManager::AddBattleEntity(
  const Ogre::String& name, const Ogre::String& file_name, const Ogre::Vector3& position,
  const Ogre::Degree& rotation, const Ogre::Vector3& scale,
  const Ogre::Quaternion& root_orientation, const int index
){
    if (module_ != Module::BATTLE){
        LOG_ERROR("Tried to add battle Entity but the EntityManager is not in battle mode.");
        return;
    }
    Ogre::SceneNode* node = scene_node_->createChildSceneNode("Battle_" + name);
    // Battle models must be reoriented.
    node->setOrientation(root_orientation);
    EntityModel* entity = new EntityModel(name, file_name, node);
    entity->SetPosition(position);
    entity->SetRotation(rotation);
    entity->setScale(scale);
    entity->SetIndex(index);
    entity->SetVisible(true);
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
    //std::cout << "    Playing " << anim << std::endl;
    entity->ScriptSetDefaultAnimation(anim.c_str());


}

void EntityManager::RemoveBattleEntity(const Ogre::String& name){
    if (module_ != Module::BATTLE) return;
    for (auto it = battle_entity_.begin(); it != battle_entity_.end(); ++ it){
        Entity* entity = *it;
        if (entity->GetName() != name) continue;
        ScriptManager::getSingleton().RemoveEntity(ScriptManager::BATTLE, name);
        battle_entity_.erase(it);
        // The entity is not deleted, as its destructor would destroy the model again, like in
        // ClearBattle. So what the model holds (a weapon) has to be destroyed here.
        entity->ScriptAttachModel("", -1, 0.0f, 0.0f, 0.0f);
        Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(name);
        scene_node_->removeAndDestroyChild("Battle_" + name);
        entity->ReleaseResources();
        return;
    }
}

void EntityManager::RemoveWorldMapEntity(const Ogre::String& name){
    if (!IsWorldModule()) return;
    for (auto it = entity_.begin(); it != entity_.end(); ++ it){
        Entity* entity = *it;
        if (entity->GetName() != name) continue;
        if (entity == player_entity_) return;
        ScriptManager::getSingleton().RemoveEntity(ScriptManager::ENTITY, name);
        entity_.erase(it);
        // The destructor of the model destroys it.
        delete entity;
        scene_node_->removeAndDestroyChild("Model_" + name);
        return;
    }
}

void EntityManager::SetBackground3D(const Ogre::String& name, const Ogre::String& file_name){
    if (IsFieldModule()){
        LOG_ERROR("Tried to add a 3D background while the EntityManager is in field mode.");
        return;
    }
    Ogre::SceneNode* node = scene_node_->createChildSceneNode("bg_" + name);
    background_3d_ = new EntityModel(name, file_name, node);
    background_3d_->SetPosition(Ogre::Vector3(0, 0, 0));
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
    // Only the world map fades the debug text with the distance to the camera.
    DEBUG_DRAW.SetFadeEnabled(module_ == Module::WORLD);
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
    ClearEncounterTables();
    entities_hidden_by_battle_.clear();
    entities_in_contact_.clear();
    walkmesh_.Clear();
    background_2d_.Clear();
    screen_fade_.Clear();
    player_slip_ = true;
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
        // The model held in a bone (a weapon) is another Ogre entity, and the next battle creates
        // one with the same name.
        battle_entity_[i]->ScriptAttachModel("", -1, 0.0f, 0.0f, 0.0f);
        Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(
          battle_entity_[i]->GetName()
        );
        scene_node_->removeAndDestroyChild("Battle_" + battle_entity_[i]->GetName());
    }
    battle_entity_.clear();
    if (background_3d_ != nullptr){
        Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(
          "bg_" + background_3d_->GetName()
        );
        /*ScriptManager::getSingleton().RemoveEntity(
          ScriptManager::ENTITY, background_3d_->GetName()
        );*/
        scene_node_->getChild("bg_" + background_3d_->GetName())->removeAllChildren();
        scene_node_->removeAndDestroyChild("bg_" + background_3d_->GetName());
        Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(
          background_3d_->GetName()
        );
        background_3d_ = nullptr;
    }
}

void EntityManager::ClearWorld(){
    walkmesh_.Clear();
    for (Entity* entity : entity_){
        ScriptManager::getSingleton().RemoveEntity(ScriptManager::ENTITY, entity->GetName());
        delete entity;
    }
    entity_.clear();
    player_entity_ = nullptr;
    player_move_ = Ogre::Vector3::ZERO;
    player_move_rotation_ = 0;
    player_lock_ = false;
    player_run_ = false;
    for (EntityTrigger* trigger : entity_triggers_){
        ScriptManager::getSingleton().RemoveEntity(ScriptManager::ENTITY, trigger->GetName());
        delete trigger;
    }
    entity_triggers_.clear();
    for (EntityPoint* point : entity_points_) delete point;
    entity_points_.clear();
    for (const Ogre::String& script : entity_scripts_){
        ScriptManager::getSingleton().RemoveEntity(ScriptManager::ENTITY, script);
    }
    entity_scripts_.clear();
    scene_node_->removeAndDestroyAllChildren();
}

Walkmesh* EntityManager::GetWalkmesh(){return &walkmesh_;}

Background2D* EntityManager::GetBackground2D(){return &background_2d_;}

ScreenFade* EntityManager::GetScreenFade(){return &screen_fade_;}

MoviePlayer* EntityManager::GetMoviePlayer(){return &movie_player_;}

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
        AddBattleEntity(name, file_name, position, rotation, scale, root_orientation, index);
    else
        AddFieldOrWorldMapEntity(
          name, file_name, position, rotation, scale, root_orientation, index
        );
}

void EntityManager::ScriptAddEntity(
  const char* name, const char* file_name,
  const float x, const float y, const float z, const float rotation, int index
){AddEntity(name, file_name, Ogre::Vector3(x, y, z), Ogre::Degree(rotation), index);}

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

void EntityManager::HideFieldEntities(){
    entities_hidden_by_battle_.clear();
    for (Entity* entity : entity_){
        if (entity->IsVisible()){
            entities_hidden_by_battle_.push_back(entity);
            entity->SetVisible(false);
        }
    }
}

void EntityManager::ShowFieldEntities(){
    // Only the ones that still exist: the field could have been changed during the battle.
    for (Entity* hidden : entities_hidden_by_battle_){
        if (std::find(entity_.begin(), entity_.end(), hidden) != entity_.end()) hidden->SetVisible(true);
    }
    entities_hidden_by_battle_.clear();
}

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
