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

template<>EntityManager *Ogre::Singleton<EntityManager>::msSingleton = nullptr;

ConfigVar cv_debug_grid("debug_grid", "Draw debug grid", "false");
ConfigVar cv_debug_axis("debug_axis", "Draw debug axis", "false");

/**
 * Calculates the point elevation.
 *
 * @param point[in] Point to calculate the elevation of.
 * @param A[in] @todo
 * @param B[in] @todo
 * @param C[in] @todo
 * @return The elevation of the point.
 * @todo Understand and document better.
 */
float PointElevation(
  const Ogre::Vector2& point,
  const Ogre::Vector3& A, const Ogre::Vector3& B, const Ogre::Vector3& C
){
    // TODO: Declare function in header file.
    float _A = A.z * (B.y - C.y) + B.z * (C.y - A.y) + C.z * (A.y - B.y);
    float _B = A.y * (B.x - C.x) + B.y * (C.x - A.x) + C.y * (A.x - B.x);
    float _C = A.x * (B.z - C.z) + B.x * (C.z - A.z) + C.x * (A.z - B.z);
    float _D
      = A.x * (B.z * C.y - C.z * B.y) + B.x * (C.z * A.y - A.z * C.y)
        + C.x * (A.z * B.y - B.z * A.y);
    return (_D - _C * point.y - _A * point.x) / _B;
}

/**
 * @todo Understand and document.
 *
 * @param point[in] @todo
 * @param p1[in] @todo
 * @param p2[in] @todo
 * @return @todo
 */
float SideOfVector(
  const Ogre::Vector2& point, const Ogre::Vector2& p1, const Ogre::Vector2& p2
){
    // TODO: Declare function in header file.
    Ogre::Vector2 AB = p2    - p1;
    Ogre::Vector2 AP = point - p1;
    return AB.x * AP.y - AB.y * AP.x;
}

/**
 * @todo Understand and document.
 *
 * @param point[in] @todo
 * @param p1[in] @todo
 * @param p2[in] @todo
 * @return @todo
 */
float SquareDistanceToLine(
  const Ogre::Vector3& p, const Ogre::Vector3& p1, const Ogre::Vector3& p2,
  Ogre::Vector3& proj
){
    // TODO: Declare function in header file.
    float temp =
      -((p1.x - p.x) * (p2.x - p1.x) + (p1.y - p.y)
      * (p2.y - p1.y) + (p1.z - p.z) * (p2.z - p1.z))
      / ((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y)
      * (p2.y - p1.y) + (p2.z - p1.z) * (p2.z - p1.z));

    proj.x = temp * (p2.x - p1.x) + p1.x;
    proj.y = temp * (p2.y - p1.y) + p1.y;
    proj.z = temp * (p2.z - p1.z) + p1.z;

    if (
      ((p1.x >= proj.x && p2.x <= proj.x) || (p1.x < proj.x && p2.x >= proj.x))
      && (
        (p1.y >= proj.y && p2.y <= proj.y) || (p1.y < proj.y && p2.y >= proj.y)
     )
   ){
        return (proj.x - p.x) * (proj.x - p.x) + (proj.y - p.y)
          * (proj.y - p.y) + (proj.z - p.z) * (proj.z - p.z);
    }
    return -1;
}

/**
 * Calculates the direction degree between to points.
 *
 * @param current_point[in] Origin point.
 * @param current_point[in] Destination point.
 * @return Degrees between the two points
 */
Ogre::Degree GetDirectionToPoint(
  const Ogre::Vector3& current_point, const Ogre::Vector3& direction_point
){
    // TODO: Declare function in header file.
    Ogre::Vector2 up(0.0f, -1.0f);
    Ogre::Vector2 dir(
      direction_point.x - current_point.x, direction_point.y - current_point.y
   );
    // Angle between vectors
    Ogre::Degree angle(
      Ogre::Radian(acosf(dir.dotProduct(up) / (dir.length() * up.length())))
   );
    return (dir.x < 0) ? Ogre::Degree(360) - angle : angle;
}

EntityManager::EntityManager():
  paused_(false),
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
    grid_ = Ogre::Root::getSingleton().getSceneManager("Scene")
      ->createEntity("Grid", "grid.mesh");
    scene_node_->attachObject(grid_);
    //axis_ = Ogre::Root::getSingleton().getSceneManager("Scene")
    //  ->createEntity("Axis", "system/axis.mesh");
    axis_ = Ogre::Root::getSingleton().getSceneManager("Scene")
      ->createEntity("Axis", "axis.mesh");
    scene_node_->attachObject(axis_);
}

EntityManager::~EntityManager(){
    if (grid_ != nullptr){
        Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(
          grid_
        );
    }
    if (axis_ != nullptr){
        Ogre::Root::getSingleton().getSceneManager("Scene")->destroyEntity(
          axis_
        );
    }
    Clear();
    Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()
      ->removeAndDestroyChild("EntityManager");
    LOG_TRIVIAL("EntityManager destroyed.");
}

void EntityManager::Input(const QGears::Event& event){
    background_2d_.InputDebug(event);
    if (paused_ == true) return;

    if (player_entity_ != NULL && player_lock_ == false){
        if (event.type == QGears::ET_KEY_REPEAT && event.event == "walk_left")
            player_move_.x = -1;
        else if (
          event.type == QGears::ET_KEY_REPEAT && event.event == "walk_right"
        ){
            player_move_.x = 1;
        }
        if (event.type == QGears::ET_KEY_REPEAT && event.event == "walk_down")
            player_move_.y = -1;
        else if (
          event.type == QGears::ET_KEY_REPEAT && event.event == "walk_up"
        ){
            player_move_.y = 1;
        }
        if (event.type == QGears::ET_KEY_REPEAT && event.event == "run")
            player_run_ = true;

        if (event.type == QGears::ET_KEY_PRESS && event.event == "interact"){
            CheckEntityInteract();
            for (unsigned int i = 0; i < entity_triggers_.size(); ++ i){
                if (
                  entity_triggers_[i]->IsEnabled() == true
                  && entity_triggers_[i]->IsActivator(player_entity_) == true
                ){
                    ScriptEntity* scr_entity
                      = ScriptManager::getSingleton().GetScriptEntityByName(
                        ScriptManager::ENTITY, entity_triggers_[i]->GetName()
                      );
                    bool added = ScriptManager::getSingleton().ScriptRequest(
                      scr_entity, "on_interact", 1, "", "", false, false
                    );
                    if (added == false){
                        LOG_WARNING(
                          "Script \"on_interact\" for entity \""
                          + entity_triggers_[i]->GetName()
                          + "\" doesn't exist."
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

void EntityManager::Update(){
    UpdateDebug();
    if (paused_ == true) return;

    // Update all entity scripts
    ScriptManager::getSingleton().Update(ScriptManager::ENTITY);

    // Set move point for player entity
    if (player_entity_ != nullptr && player_move_ != Ogre::Vector3::ZERO){
        Entity::State state = player_entity_->GetState();
        if (state == Entity::WALKMESH || state == Entity::NONE){
            player_move_
              *= player_entity_->GetMoveWalkSpeed()
              * Timer::getSingleton().GetGameTimeDelta();
            player_entity_->SetMovePosition(
              player_entity_->GetPosition() + player_move_
            );
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

        // update screen scroll
        Entity* scroll_entity = background_2d_.GetAutoScrollEntity();
        if (scroll_entity == entity_[i]){
            CameraManager &camera_manager(CameraManager::getSingleton());
            Ogre::Vector3 view = camera_manager.ProjectPointToScreen(
              scroll_entity->GetPosition()
            );
            Ogre::Vector2 pos = background_2d_.GetScreenScroll();
            Ogre::Viewport *viewport(camera_manager.getViewport());
            float width = float(viewport->getActualWidth());
            float height = float(viewport->getActualHeight());
            view.x = view.x - width / 2 - pos.x;
            view.y = view.y - height / 2 - pos.y;
            background_2d_.SetScreenScroll(Ogre::Vector2(-view.x, -view.y));
        }

        // Update offseting
        if (entity_[i]->GetOffsetType() != AT_NONE)
            SetNextOffsetStep(entity_[i]);

        // Update turning
        if (entity_[i]->GetTurnType() != AT_NONE) SetNextTurnStep(entity_[i]);

        // Update movement
        switch(entity_[i]->GetState()){
            case Entity::WALKMESH:
                {
                    bool is_move = false;

                    // Try set entity on walkmesh if it's still don't has triangle
                    if (entity_[i]->GetMoveTriangleId() == -1){
                        if (SetEntityOnWalkmesh(entity_[i]) == false){
                            LOG_TRIVIAL(
                              "Can't set entity \"" + entity_[i]->GetName()
                              + "\" on walkmesh. Finish move."
                            );
                            entity_[i]->UnsetMove();
                        }
                    }

                    // Perform move
                    if (entity_[i]->GetState() == Entity::WALKMESH){
                        float speed = 0;
                        if (player_lock_ == false && player_entity_ == entity_[i]){
                            speed = entity_[i]->GetMoveWalkSpeed();
                            if (player_run_ == true) speed *= 4;
                        }
                        else speed = entity_[i]->GetMoveAutoSpeed();
                        is_move = PerformWalkmeshMove(entity_[i], speed);
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
                              entity_[i]->GetAnimationState()
                              != Entity::REQUESTED_ANIMATION
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
                SetNextLinearStep(entity_[i]);
                break;

            case Entity::JUMP:
                SetNextJumpStep(entity_[i]);
                break;

            case Entity::NONE:
                if (
                  entity_[i]->GetAnimationState()
                  != Entity::REQUESTED_ANIMATION
                ){
                    entity_[i]->PlayAnimationContinue(
                      entity_[i]->GetDefaultAnimationName()
                    );
                }
            break;
        }
    }

    // Reset player move. It already must be handled in update
    player_move_ = Ogre::Vector3::ZERO;
    player_run_ = false;

    if (background_2d_.GetScrollType() != Background2D::NONE)
        SetNextScrollStep();
    background_2d_.Update();
}


void EntityManager::UpdateDebug(){
    grid_->setVisible(cv_debug_grid.GetB());
    axis_->setVisible(cv_debug_axis.GetB());
    for (unsigned int i = 0; i < entity_.size(); ++ i)
        entity_[i]->UpdateDebug();
    for (unsigned int i = 0; i < entity_triggers_.size(); ++ i)
        entity_triggers_[i]->UpdateDebug();
    for (unsigned int i = 0; i < entity_points_.size(); ++ i)
        entity_points_[i]->UpdateDebug();
    walkmesh_.UpdateDebug();
    background_2d_.UpdateDebug();
}

void EntityManager::OnResize(){background_2d_.OnResize();}

void EntityManager::Clear(){
    walkmesh_.Clear();
    background_2d_.Clear();
    DialogsManager::getSingleton().Clear();
    for (unsigned int i = 0; i < entity_.size(); ++ i){
        ScriptManager::getSingleton().RemoveEntity(
          ScriptManager::ENTITY, entity_[i]->GetName()
        );
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

    for (unsigned int i = 0; i < entity_points_.size(); ++ i)
        delete entity_points_[i];
    entity_points_.clear();

    for (unsigned int i = 0; i < entity_scripts_.size(); ++ i){
        ScriptManager::getSingleton().RemoveEntity(
          ScriptManager::ENTITY, entity_scripts_[i]
        );
    }

    scene_node_->removeAndDestroyAllChildren();
}

void EntityManager::ScriptSetPaused(const bool paused){paused_ = paused;}

Walkmesh* EntityManager::GetWalkmesh(){return &walkmesh_;}

Background2D* EntityManager::GetBackground2D(){return &background_2d_;}

void EntityManager::AddEntity(
  const Ogre::String& name, const Ogre::String& file_name,
  const Ogre::Vector3& position, const Ogre::Degree& rotation
){
    AddEntity(
      name, file_name, position, rotation,
      Ogre::Vector3::UNIT_SCALE, Ogre::Quaternion::IDENTITY
    );
}

void EntityManager::AddEntity(
  const Ogre::String& name, const Ogre::String& file_name,
  const Ogre::Vector3& position, const Ogre::Degree& rotation,
  const Ogre::Vector3& scale, const Ogre::Quaternion& root_orientation
){
    Ogre::SceneNode* node = scene_node_->createChildSceneNode("Model_" + name);
    EntityModel* entity = new EntityModel(name, file_name, node);
    entity->SetPosition(position);
    entity->SetRotation(rotation);
    entity->setScale(scale);
    entity->setRootOrientation(root_orientation);
    entity_.push_back(entity);
    ScriptManager::getSingleton().AddEntity(
      ScriptManager::ENTITY, entity->GetName(), entity
    );
}

void EntityManager::ScriptAddEntity(
  const char* name, const char* file_name,
  const float x, const float y, const float z, const float rotation
){
    AddEntity(name, file_name, Ogre::Vector3(x, y, z), Ogre::Degree(rotation));
}

void EntityManager::AddEntityTrigger(
  const Ogre::String& name,
  const Ogre::Vector3& point1, const Ogre::Vector3& point2, const bool enabled
){
    EntityTrigger* trigger = new EntityTrigger(name);
    trigger->SetPoints(point1, point2);
    trigger->SetEnabled(enabled);
    entity_triggers_.push_back(trigger);
    ScriptManager::getSingleton().AddEntity(
      ScriptManager::ENTITY, trigger->GetName(), nullptr
    );
}

void EntityManager::AddEntityPoint(
  const Ogre::String& name, const Ogre::Vector3& position, const float rotation
){
    EntityPoint* entity_point = new EntityPoint(name);
    entity_point->SetPosition(position);
    entity_point->SetRotation(rotation);
    entity_points_.push_back(entity_point);
}


void EntityManager::AddEntityScript(const Ogre::String& name){
    entity_scripts_.push_back(name);
    ScriptManager::getSingleton().AddEntity(
      ScriptManager::ENTITY, name, nullptr
    );
}

void EntityManager::ScriptAddEntityScript(const char* name){
    AddEntityScript(name);
}

Entity* EntityManager::GetEntity(const Ogre::String& name) const{
    for (unsigned int i = 0; i < entity_.size(); ++ i){
        if (entity_[i]->GetName() == name) return entity_[i];
    }
    return nullptr;
}

Entity* EntityManager::GetEntityFromCharacterId(const char* id) const{
    for (unsigned int i = 0; i < entity_.size(); ++ i){
        if (entity_[i]->IsCharacter() && entity_[i]->GetCharacterId() == atoi(id))
            return entity_[i];
    }
    return nullptr;
}

Entity* EntityManager::ScriptGetEntity(const char* name) const{
    return GetEntity(Ogre::String(name));
}


EntityPoint* EntityManager::ScriptGetEntityPoint(const char* name) const{
    for (unsigned int i = 0; i < entity_points_.size(); ++ i){
        if (entity_points_[i]->GetName() == name) return entity_points_[i];
    }
    return nullptr;
}

void EntityManager::ScriptSetPlayerEntity(const char* name){
    for (unsigned int i = 0; i < entity_.size(); ++ i){
        if (entity_[i]->GetName() == name) player_entity_ = entity_[i];
    }
}

Entity* EntityManager::ScriptGetPlayerEntity() const{return player_entity_;}

void EntityManager::ScriptUnsetPlayerEntity(){player_entity_ = nullptr;}

void EntityManager::ScriptPlayerLock(const bool lock){
    player_lock_ = lock;
    if (lock == true) player_move_ = Ogre::Vector3::ZERO;
}

void EntityManager::SetPlayerMoveRotation(const Ogre::Radian rotation){
    player_move_rotation_ = rotation;
}

bool EntityManager::GetRandomEncounters(){return random_encounters_;}

void EntityManager::SetRandomEncounters(bool active){
  random_encounters_ = active;
}

float EntityManager::GetEncounterRate(){return encounter_rate_;}

void EntityManager::SetEncounterRate(float rate){
    if (rate >= 1.0f) encounter_rate_ = 1.0f;
    else if (rate <= 0.0f) encounter_rate_ = 0.0f;
    else encounter_rate_ = rate;
}
bool EntityManager::StartBattleForResult(unsigned int formation){
    std::cout << "[BATTLE] Start battle for result ID " << formation << "\n";
    return true;
}
void EntityManager::StartBattle(unsigned int formation){
    std::cout << "[BATTLE] Start battle ID " << formation << "\n";
}

bool EntityManager::IsKeyOn(unsigned int key_code){
    // TODO: Translate keycodes to game key codes.
    // For example 32 is standard for "Enter", but in game is "Circle/Action".
    return InputManager::getSingleton().IsButtonPressed(key_code);
}

bool EntityManager::IsKeyOff(unsigned int key_code){return !IsKeyOn(key_code);}

void EntityManager::SetEntityToCharacter(const char* entity_name, const char* character_name){
    for (unsigned int i = 0; i < entity_.size(); ++ i){
        if (entity_[i]->GetName() == entity_name){
            entity_[i]->SetCharacter(character_name);
        }
    }
}

bool EntityManager::SetEntityOnWalkmesh(Entity* entity){
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

    // If the coordnates doesn't match any triangle
    if (triangles.size() == 0){
        LOG_ERROR(
          "Can't find any triangle to place entity \""
          + entity->GetName() + "\" on walkmesh."
        );
        return false;
    }

    /*
    int triangle = m_Models[entity_id]->GetTriangle();
    if (triangle != -1){
        for (int i = 0; i < triangles.size(); ++ i){
            if (triangles[i].first == triangle){
                m_Models[entity_id]->SetPosition(
                  Ogre::Vector3(position2.x, position2.y, triangles[i].second)
                );
                return;
            }
        }
    }
    */

    // Place the entity on the found triangles.
    // First, try to set it on top triangle less than current pos of entity.
    float pos_z = position3.z;
    int triangle_id = -1;
    for (size_t i = 0; i < triangles.size(); ++ i){
        if (triangles[i].second <= pos_z){
            pos_z = triangles[i].second;
            triangle_id = triangles[i].first;
            break;
        }
    }
    // If every triangle are higher than the entity:
    if (triangle_id == -1){
        for (size_t i = 0; i < triangles.size(); ++ i){
            if (triangles[i].second >= pos_z){
                pos_z = triangles[i].second;
                triangle_id = triangles[i].first;
            }
        }
    }
    entity->SetPosition(Ogre::Vector3(position2.x, position2.y, pos_z));
    entity->SetMoveTriangleId(triangle_id);
    return true;
}

bool EntityManager::PerformWalkmeshMove(Entity* entity, const float speed){
    Ogre::Vector3 start_point = entity->GetPosition();
    Ogre::Vector3 move_vector = entity->GetMovePosition() - start_point;
    Ogre::Vector2 direction(move_vector.x, move_vector.y);
    direction.normalise();
    direction *= Timer::getSingleton().GetGameTimeDelta();
    direction *= speed;

    // If movement is still needed but speed ot time don't allow it, just
    // return for now
    if (direction.isZeroLength() == true) return false;

    int current_triangle = entity->GetMoveTriangleId();
    if (current_triangle == -1){
        LOG_ERROR(
          "Entity \"" + entity->GetName()
          + "\" not placed on walkmesh and can't move."
        );
        return false;
    }

    //LOG_TRIVIAL("Start position calculation.");
    //LOG_TRIVIAL(
    //  "Start point: " + Ogre::StringConverter::toString(start_point) + "."
    //);
    //LOG_TRIVIAL(
    //  "Move vector: " + Ogre::StringConverter::toString(direction) + "."
    //);

    Ogre::Vector3 rotation(0.0f, 0.0f, 0.0f);
    Ogre::Quaternion q1(0.0f, 0.0f, 0.0f, 1.0f);
    Ogre::Vector3 end_point(0.0f, 0.0f, 0.0f);
    Ogre::Vector3 end_point2(0.0f, 0.0f, 0.0f);
    bool first_triangle_check  = false;
    bool second_triangle_check = false;
    bool third_triangle_check  = false;
    bool last_triangle_check   = false;
    bool first_entity_check    = false;
    bool second_entity_check   = false;
    bool third_entity_check    = false;

    // Shorten move vector by triangle angle
    end_point.x = start_point.x + direction.x;
    end_point.y = start_point.y + direction.y;

    Ogre::Vector3 A3 = walkmesh_.GetA(current_triangle);
    Ogre::Vector3 B3 = walkmesh_.GetB(current_triangle);
    Ogre::Vector3 C3 = walkmesh_.GetC(current_triangle);

    end_point.z
      = PointElevation(Ogre::Vector2(end_point.x, end_point.y), A3, B3, C3);
    Ogre::Vector3 temp = end_point - start_point;
    temp.normalise();
    temp = temp * direction.length();
    direction.x = temp.x;
    direction.y = temp.y;

    //LOG_TRIVIAL(
    //  "Shortened Direction:("
    //  + Ogre::StringConverter::toString(direction) + ")."
    //);

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

    float solid
      = (entity->IsSolid() == true) ? entity->GetSolidRadius() : 0.01f;

    // Get ending point
    end_point.z = start_point.z;
    //LOG_TRIVIAL(
    //  "End point: (" + Ogre::StringConverter::toString(end_point) + ")."
    //);

    //LOG_TRIVIAL("Start cycle.");
    for (
      int i = 0;
      (entity == player_entity_ && /*m_PlayerModelSlip == true &&*/ i < 16)
        || (entity == player_entity_ && /*m_PlayerModelSlip == false &&*/ i < 3)
        || (entity != player_entity_ && i <= 16);
      ++ i
    ){
        //LOG_TRIVIAL("Cycle " + Ogre::StringConverter::toString(i) + ".");
        end_point = Ogre::Vector3(
          start_point.x + direction.x, start_point.y + direction.y,
          start_point.z
        );

        // 1st check.
        // Rotate move_vector +45.
        q1.FromAngleAxis(Ogre::Radian(Ogre::Degree(45)), Ogre::Vector3::UNIT_Z);
        rotation.x = direction.x;
        rotation.y = direction.y;
        rotation.z = 0;
        rotation.normalise();
        rotation = q1 * rotation;
        //LOG_TRIVIAL(
        //  "Move vector length: "
        //  + Ogre::StringConverter::toString(rotation.length()) + "."
        //);

        // Multiply move_vector by solid range
        rotation = rotation * solid;
        end_point2.x = end_point.x + rotation.x;
        end_point2.y = end_point.y + rotation.y;
        //LOG_TRIVIAL(
        //  "Left sector part end point:("
        //  + Ogre::StringConverter::toString(end_point2) + ")."
        // );

        // Check_triangle.
        first_triangle_check
          = WalkmeshBorderCross(entity, end_point2, direction);
        entity->SetMoveTriangleId(current_triangle);

        // Model check.
        first_entity_check = CheckSolidCollisions(entity, end_point2);
        //LOG_TRIVIAL(
        //  "Entity \"" + entity->GetName() + "\" check1: "
        //  + Ogre::StringConverter::toString(first_triangle_check) + " "
        // + Ogre::StringConverter::toString(first_entity_check) + "."
        //);

        // 2nd check.
        // Rotate move_vector +45
        q1.FromAngleAxis(
          Ogre::Radian(Ogre::Degree(-45)), Ogre::Vector3::UNIT_Z
        );
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
        second_triangle_check = WalkmeshBorderCross(
          entity, end_point2, direction
        );
        entity->SetMoveTriangleId(current_triangle);

        // Model check.
        second_entity_check = CheckSolidCollisions(entity, end_point2);
        //LOG_TRIVIAL(
        //  "Entity \"" + entity->GetName() + "\" check2: "
        //  + Ogre::StringConverter::toString(second_triangle_check) + " "
        //  + Ogre::StringConverter::toString(second_entity_check) + "."
        //);

        // 3rd check.
        rotation.x = direction.x;
        rotation.y = direction.y;
        rotation.z = 0;
        rotation.normalise();
        rotation = rotation * solid;
        end_point2.x = end_point.x + rotation.x;
        end_point2.y = end_point.y + rotation.y;

        // Check triangle.
        third_triangle_check
          = WalkmeshBorderCross(entity, end_point2, direction);
        entity->SetMoveTriangleId(current_triangle);

        // Model check.
        third_entity_check = CheckSolidCollisions(entity, end_point2);
        //LOG_TRIVIAL(
        //  "Entity \"" + entity->GetName() + "\" check3: "
        //  + Ogre::StringConverter::toString(third_triangle_check) + " "
        //  + Ogre::StringConverter::toString(third_entity_check) + "."
        //);

        // Check condition and modify move_vector.
        if (
          first_triangle_check != false || second_triangle_check != false
          || third_triangle_check != false || first_entity_check != false
          || second_entity_check != false || third_entity_check != false
        ){
            // Only for NPC
            if (entity != player_entity_){

                // If the entity collides only directly into a triangle border.
                if (
                  first_triangle_check  == false
                  && second_triangle_check == false
                  && third_triangle_check != false
                ){
                    q1.FromAngleAxis(
                      Ogre::Radian(Ogre::Degree(-7)), Ogre::Vector3::UNIT_Z
                    );
                    rotation.x = direction.x;
                    rotation.y = direction.y;
                    rotation = q1 * rotation;
                    direction.x = rotation.x;
                    direction.y = rotation.y;
                }

                // Or if the entity only collides into other entity directly.
                else if (
                  first_entity_check  == false && second_entity_check == false
                  && third_entity_check  != false
                ){
                    //LOG_TRIVIAL(
                    //  "Entity \"" + entity->GetName() + "\" try to rotate."
                    //);
                    q1.FromAngleAxis(
                      Ogre::Radian(Ogre::Degree(-11.25f)), Ogre::Vector3::UNIT_Z
                    );
                    rotation.x = direction.x;
                    rotation.y = direction.y;
                    rotation = q1 * rotation;
                    direction.x = rotation.x;
                    direction.y = rotation.y;
                }

                // If not both left and right check failed
                if (
                  first_triangle_check == false
                  || second_triangle_check == false
                ){
                    if (
                      (
                        first_triangle_check == false
                        && first_entity_check != false
                      ) || (
                        first_triangle_check != false
                        && second_triangle_check == false
                      )
                    ){
                        q1.FromAngleAxis(
                          Ogre::Radian(Ogre::Degree(-11.25f)),
                          Ogre::Vector3::UNIT_Z
                        );
                        rotation.x = direction.x;
                        rotation.y = direction.y;
                        rotation = q1 * rotation;
                        direction.x = rotation.x;
                        direction.y = rotation.y;
                        //LOGGER->Log(
                        //  LOGGER_WARNING,
                        // "New move vector: %f %f.", direction.x, direction.y
                        //);
                    }

                    if (
                      first_triangle_check == false
                      && first_entity_check == false
                      && (
                        second_triangle_check != false
                        || second_entity_check != false
                      )
                    ){
                        q1.FromAngleAxis(
                          Ogre::Radian(Ogre::Degree(11.25f)),
                          Ogre::Vector3::UNIT_Z
                        );
                        rotation.x = direction.x;
                        rotation.y = direction.y;
                        rotation = q1 * rotation;
                        direction.x = rotation.x;
                        direction.y = rotation.y;
                        //LOGGER->Log(
                        //  LOGGER_WARNING,
                        //  "New move vector: %f %f.", direction.x, direction.y
                        //);
                    }
                    continue;
                }
                break;
            }

            // Only for playable character.
            else{
                if (
                  first_entity_check  == false && second_entity_check == false
                  && third_entity_check  == false
                ){
                    // If not both left and right check failed
                    if (
                      first_triangle_check == false
                      || second_triangle_check == false
                    ){
                        if (
                          (
                            first_triangle_check == false
                            && first_entity_check != false
                          )
                          || (
                            first_triangle_check != false
                            && second_triangle_check == false
                          )
                        ){
                            q1.FromAngleAxis(
                              Ogre::Radian(Ogre::Degree(-11.25f)),
                              Ogre::Vector3::UNIT_Z
                            );
                            rotation.x = direction.x;
                            rotation.y = direction.y;
                            rotation = q1 * rotation;
                            direction.x = rotation.x;
                            direction.y = rotation.y;
                            //LOG_TRIVIAL(
                            //  "Entity \"" + entity->GetName()
                            //  + "\" new move vector: "
                            //  + Ogre::StringConverter::toString(direction.x)
                            //  + " "
                            //  + Ogre::StringConverter::toString(direction.y)
                            //  + "."
                            //);
                        }

                        if (
                          first_triangle_check == false
                          && first_entity_check == false
                          && (
                            second_triangle_check != false
                            || second_entity_check != false
                          )
                        ){
                            q1.FromAngleAxis(
                              Ogre::Radian(Ogre::Degree(11.25f)),
                              Ogre::Vector3::UNIT_Z
                            );
                            rotation.x = direction.x;
                            rotation.y = direction.y;
                            rotation = q1 * rotation;
                            direction.x = rotation.x;
                            direction.y = rotation.y;
                            //LOG_TRIVIAL(
                            //  "Entity \"" + entity->GetName()
                            //  + "\" new move vector: "
                            //  + Ogre::StringConverter::toString(direction.x)
                            //  + " "
                            //  + Ogre::StringConverter::toString(direction.y)
                            //  + "."
                            //);
                        }
                        continue;
                    }
                    break;
                }
            }
        }
    }

    //LOG_TRIVIAL("Stop cycle.");

    // Set new X, Y and Z
    last_triangle_check = WalkmeshBorderCross(entity, end_point, direction);

    if (
      first_triangle_check != false || second_triangle_check != false
      || third_triangle_check != false || last_triangle_check != false
      || first_entity_check != false || second_entity_check != false
      || third_entity_check != false
    ){
        LOG_TRIVIAL(
          "Entity \"" + entity->GetName()
          + "\" can't move to specified position."
        );
        return false;
    }

    // Check triggers before set entity position because move line is checked.
    CheckTriggers(entity, end_point);
    entity->SetPosition(end_point);

    // If the entity has come to destination point, finish movement.
    Ogre::Vector3 final = entity->GetMovePosition() - end_point;
    if (
      Ogre::Vector2(final.x, final.y).length()
      <= entity->GetMoveStopDistance() + speed / 10
    ){
        entity->UnsetMove();
    }

    return true;
}

bool EntityManager::WalkmeshBorderCross(
  Entity* entity, Ogre::Vector3& position, const Ogre::Vector2& move_vector
){
    int current_triangle = entity->GetMoveTriangleId();
    if (current_triangle == -1) return true;

    Ogre::Vector2 pos = Ogre::Vector2(position.x, position.y);

    for (;;){
        Ogre::Vector3 A3 = walkmesh_.GetA(current_triangle);
        Ogre::Vector3 B3 = walkmesh_.GetB(current_triangle);
        Ogre::Vector3 C3 = walkmesh_.GetC(current_triangle);
        /*
        LOG_ERROR(
          "Triangle: A(" + Ogre::StringConverter::toString(A3.x) + ", "
          + Ogre::StringConverter::toString(A3.y) + ", "
          + Ogre::StringConverter::toString(A3.z) + ") B("
          + Ogre::StringConverter::toString(B3.x) + ", "
          + Ogre::StringConverter::toString(B3.y) + ", "
          + Ogre::StringConverter::toString(B3.z) + ") C(
          + Ogre::StringConverter::toString(C3.x) + ", "
          + Ogre::StringConverter::toString(C3.y) + ", "
          + Ogre::StringConverter::toString(C3.z) + ")."
        );
        */
        Ogre::Vector2 A(A3.x, A3.y);
        Ogre::Vector2 B(B3.x, B3.y);
        Ogre::Vector2 C(C3.x, C3.y);

        float sign1 = SideOfVector(pos, B, A);
        float sign2 = SideOfVector(pos, C, B);
        float sign3 = SideOfVector(pos, A, C);

        int next_triangle = -1;

        if (sign1 < 0)
            next_triangle = walkmesh_.GetAccessSide(current_triangle, 0);
        else if (sign2 < 0)
            next_triangle = walkmesh_.GetAccessSide(current_triangle, 1);
        else if (sign3 < 0)
            next_triangle = walkmesh_.GetAccessSide(current_triangle, 2);
        else{
            position.z = PointElevation(pos, A3, B3, C3);
            //LOG_ERROR("In triangle.");
            //LOG_ERROR(
            //  "Stop CheckTriangles with false and triangle "
            // + Ogre::StringConverter::toString(current_triangle) + "."
            //);
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

bool EntityManager::CheckSolidCollisions(
  Entity* entity, Ogre::Vector3& position
){
    if (entity->IsSolid() == false) return false;

    for (size_t i = 0; i < entity_.size(); ++ i){
        if (entity_[i]->IsSolid() == false) continue;
        if (entity_[i] == entity) continue;
        Ogre::Vector3 pos1 = entity_[i]->GetPosition();
        float solid_range = entity_[i]->GetSolidRadius();
        solid_range *= solid_range;
        float height
          = (pos1.z < position.z)
          ? entity_[i]->GetHeight() : entity->GetHeight();

        //log->logMessage(
        //  "Height collision(" + Ogre::StringConverter::toString(pos1.z) + " "
        //  + Ogre::StringConverter::toString(position.z) + " "
        //  + Ogre::StringConverter::toString(height) + ")."
        //);

        if (
          ((pos1.z - position.z + height) < (height * 2))
          && ((pos1.z - position.z + height) >= 0)
        ){
            float distance
              = (pos1.x - position.x) * (pos1.x - position.x)
              + (pos1.y - position.y) * (pos1.y - position.y);

            //log->logMessage(
            //  "Dist collision(" + Ogre::StringConverter::toString(solid_range)
            //  + " " + Ogre::StringConverter::toString(distance) + ")."
            //);

            if (distance < solid_range){
                //log->logMessage("Collide with entity.");
                // If this is player and is moved by controller:
                /*
                if (model_id == m_PlayerModelId && m_PlayerModelLock == false){
                    m_FieldScriptManager->AddScript(
                      m_Models[i]->GetName(), "on_collide", 1
                    );
                }
                */
                return true;
            }
        }
    }

    //LOGGER->Log(LOGGER_INFO, "Not collide with entity");
    return false;
}

void EntityManager::CheckTriggers(Entity* entity, Ogre::Vector3& position){
    if (entity->IsSolid() == false) return;

    for (unsigned int i = 0; i < entity_triggers_.size(); ++ i){
        ScriptEntity* scr_entity
          = ScriptManager::getSingleton().GetScriptEntityByName(
            ScriptManager::ENTITY, entity_triggers_[i]->GetName()
        );

        if (scr_entity != nullptr && entity_triggers_[i]->IsEnabled() == true){
            Ogre::Vector3 lp1 = entity_triggers_[i]->GetPoint1();
            Ogre::Vector3 lp2 = entity_triggers_[i]->GetPoint2();
            Ogre::Vector3 mp1 = entity->GetPosition();

            Ogre::Vector3 proj;

            // Calculate distance
            float square_dist = SquareDistanceToLine(mp1, lp1, lp2, proj);
            float solid = entity->GetSolidRadius();

            if (square_dist != -1 && square_dist < solid * solid){
                if (entity_triggers_[i]->IsActivator(entity) == false){
                    bool added = ScriptManager::getSingleton().ScriptRequest(
                      scr_entity, "on_enter_line", 1, entity->GetName(),
                      "", false, false
                    );
                    if (added == false){
                        LOG_WARNING(
                          "Script \"on_enter_line\" for entity \""
                          + entity_triggers_[i]->GetName() + "\" doesn't exist."
                        );
                    }
                    entity_triggers_[i]->AddActivator(entity);
                }

                // Check that 1st and 2nd points are on different side of line.
                float cond1
                  = ((lp2.x - lp1.x) * (mp1.y - lp1.y))
                  - ((mp1.x - lp1.x) * (lp2.y - lp1.y));
                float cond2
                  = ((lp2.x - lp1.x) * (position.y - lp1.y))
                  - ((position.x - lp1.x) * (lp2.y - lp1.y));

                // If the line is crossed.
                if (
                  (cond1 > 0 && cond2 <= 0) || (cond2 > 0 && cond1 <= 0)
                  || (cond1 >= 0 && cond2 < 0) || (cond2 >= 0 && cond1 < 0)
                ){
                    bool added = ScriptManager::getSingleton().ScriptRequest(
                      scr_entity, "on_cross_line", 1, entity->GetName(),
                      "", false, false
                    );
                    if (added == false){
                        LOG_WARNING(
                          "Script \"on_cross_line\" for entity \""
                          + entity_triggers_[i]->GetName() + "\" doesn't exist."
                        );
                    }
                }

                // If not moving into line.
                if (mp1 == position){
                    bool added = ScriptManager::getSingleton().ScriptRequest(
                      scr_entity, "on_move_to_line", 1, entity->GetName(),
                      "", false, false
                    );
                    if (added == false){
                        LOG_WARNING(
                          "Script \"on_move_to_line\" for entity \""
                          + entity_triggers_[i]->GetName() + "\" doesn't exist."
                        );
                    }
                }
                else{
                    const Ogre::Degree direction_to_line
                      = GetDirectionToPoint(mp1, proj);
                    const Ogre::Degree movement_direction
                      = GetDirectionToPoint(mp1, position);

                    // If moving into line.
                    Ogre::Degree angle
                      = direction_to_line - movement_direction
                      + Ogre::Degree(90);
                    angle
                      = (angle > Ogre::Degree(360))
                      ? angle - Ogre::Degree(360) : angle;

                    if (angle < Ogre::Degree(180) && angle > Ogre::Degree(0)){
                        bool added
                          = ScriptManager::getSingleton().ScriptRequest(
                            scr_entity, "on_move_to_line", 1, entity->GetName(),
                            "", false, false
                          );
                        if (added == false){
                            LOG_WARNING(
                              "Script \"on_move_to_line\" for entity \""
                              + entity_triggers_[i]->GetName()
                              + "\" doesn't exist."
                            );
                        }
                    }
                }
            }
            else{
                if (entity_triggers_[i]->IsActivator(entity) == true){
                    bool added = ScriptManager::getSingleton().ScriptRequest(
                      scr_entity, "on_leave_line", 1, entity->GetName(),
                      "", false, false
                    );
                    if (added == false){
                        LOG_WARNING(
                          "Script \"on_leave_line\" for entity \""
                          + entity_triggers_[i]->GetName() + "\" doesn't exist."
                        );
                    }
                    entity_triggers_[i]->RemoveActivator(entity);
                }
            }
        }
    }
}

void EntityManager::CheckEntityInteract(){
    if (
      player_entity_ == NULL || player_lock_ == true
      || player_entity_->IsSolid() == false
    ){
        return;
    }

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

        int height = (pos1.z < pos2.z)
          ? entity_[i]->GetHeight() : player_entity_->GetHeight();

        if (
          ((pos1.z - pos2.z + height) < (height * 2))
          && ((pos1.z - pos2.z + height) >= 0)
        ){
            interact_range
              = (interact_range + solid_range) * (interact_range + solid_range);
            float distance
              = (pos1.x - pos2.x) * (pos1.x - pos2.x)
              + (pos1.y - pos2.y) * (pos1.y - pos2.y);

            if (distance < interact_range + solid_range){
                Ogre::Degree angle_to_model = GetDirectionToPoint(pos2, pos1);
                Ogre::Degree angle = angle_pc - angle_to_model;
                angle = (angle < Ogre::Degree(0)) ? -angle : angle;
                angle = (angle >= Ogre::Degree(180))
                  ? Ogre::Degree(360) - angle : angle;

                if (angle < less_angle){
                    angle = less_angle;
                    entity_to_interact = entity_[i];
                }
            }
        }
    }

    if (entity_to_interact != NULL){
        Ogre::String name = entity_to_interact->GetName();
        ScriptEntity* scr_entity
          = ScriptManager::getSingleton().GetScriptEntityByName(
            ScriptManager::ENTITY, name
          );
        bool added = ScriptManager::getSingleton().ScriptRequest(
          scr_entity, "on_interact", 1, "", "", false, false
        );
        if (added == false){
            LOG_WARNING(
              "Script \"on_interact\" for entity \"" +  name
              + "\" doesn't exist."
            );
        }
    }
}

void EntityManager::SetNextOffsetStep(Entity* entity){
    ActionType type = entity->GetOffsetType();
    float total = entity->GetOffsetSeconds();
    float current = entity->GetOffsetCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Vector3 start = entity->GetOffsetPositionStart();
    Ogre::Vector3 end = entity->GetOffsetPositionEnd();
    float x = current / total;
    // Prevent division by 0:
    if (std::isnan(x)) x = 0.0f;
    float smooth_modifier
      = (type == AT_SMOOTH) ? -2 * x * x * x + 3 * x * x : x;
    Ogre::Vector3 offset = start + (end - start) * smooth_modifier;
    entity->SetOffset(offset);
    if (current == total) entity->UnsetOffset();
    else entity->SetOffsetCurrentSeconds(current);
}

void EntityManager::SetNextTurnStep(Entity* entity){
    ActionType type = entity->GetTurnType();
    float total = entity->GetTurnSeconds();
    float current = entity->GetTurnCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Degree start = entity->GetTurnDirectionStart();
    Ogre::Degree end = entity->GetTurnDirectionEnd();
    float x = current / total;
    float smooth_modifier
      = (type == AT_SMOOTH) ? -2 * x * x * x + 3 * x * x : x;
    Ogre::Degree rotation = start + (end - start) * smooth_modifier;
    entity->SetRotation(rotation);
    if (current == total) entity->UnsetTurn();
    else entity->SetTurnCurrentSeconds(current);
}

void EntityManager::SetNextLinearStep(Entity* entity){
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

            if (position == end) entity->UnsetLinear();
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

            if (position == start) entity->UnsetLinear();
        }

        entity->SetPosition(position);
        entity->UpdateAnimation((to_end == true) ? delta : -delta);
    }
}

void EntityManager::SetNextJumpStep(Entity* entity){
    float total = entity->GetJumpSeconds();
    float current = entity->GetJumpCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Vector3 start_position = entity->GetJumpStart();
    Ogre::Vector3 end_position   = entity->GetJumpEnd();
    Ogre::Vector3 position;
    position.x = start_position.x
      + ((end_position.x - start_position.x) / total) * current;
    position.y = start_position.y
      + ((end_position.y - start_position.y) / total) * current;
    position.z = current * current * -13.08f + current
      * ((end_position.z - start_position.z) / total + total * 13.08f)
      + start_position.z;
    entity->SetPosition(position);
    if (total == current) entity->UnsetJump();
    else entity->SetJumpCurrentSeconds(current);
}

void EntityManager::SetNextScrollStep(){
    Background2D::ScrollType type = background_2d_.GetScrollType();
    float total = background_2d_.GetScrollSeconds();
    float current = background_2d_.GetScrollCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Vector2 start = background_2d_.GetScrollPositionStart();
    Ogre::Vector2 end = background_2d_.GetScrollPositionEnd();
    float x = current / total;
    float smooth_modifier = (type == Background2D::SMOOTH)
      ? -2 * x * x * x + 3 * x * x : x;
    Ogre::Vector2 scroll = start + (end - start) * smooth_modifier;
    background_2d_.SetScroll(scroll);
    if (current == total) background_2d_.UnsetScroll();
    else background_2d_.SetScrollCurrentSeconds(current);
}
