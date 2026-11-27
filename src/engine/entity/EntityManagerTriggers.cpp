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
#include <limits>
#include "engine/entity/EntityManager.h"
#include "common/Logger.h"
#include "engine/script/ScriptManager.h"

const int EntityManager::INTERACT_SCRIPT_PRIORITY = 4;

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
    Ogre::Degree angle(Ogre::Radian(SafeAcos(dir.dotProduct(up), dir.length() * up.length())));
    return (dir.x < 0) ? Ogre::Degree(360) - angle : angle;
}

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

float EntityManager::TriggerReach(float solid_radius){
    // The radius an entity has when a script doesn't change it.
    const float default_radius = 0.21f;
    return solid_radius < 0.01f ? default_radius : solid_radius;
}

bool EntityManager::IsBetterInteractionTarget(
  float angle, float distance, float best_angle, float best_distance
){
    const float same_angle = 0.001f;
    if (angle < best_angle - same_angle) return true;
    return std::fabs(angle - best_angle) <= same_angle && distance < best_distance;
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
            float solid_radius = TriggerReach(entity->GetSolidRadius());

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

void EntityManager::ScriptSetLine(
  const char* name, const float x1, const float y1, const float z1, const float x2, const float y2,
  const float z2
){
    for (EntityTrigger* trigger : entity_triggers_){
        if (trigger->GetName() == name){
            trigger->SetPoints(Ogre::Vector3(x1, y1, z1), Ogre::Vector3(x2, y2, z2));
            return;
        }
    }
}

void EntityManager::CheckEntityContacts(){
    if (IsBattleModule() || !IsFieldModule()) return;
    if (player_entity_ == nullptr || player_lock_ || !player_entity_->IsSolid()){
        entities_in_contact_.clear();
        return;
    }
    const Ogre::Vector3 player_position = player_entity_->GetPosition();
    for (Entity* entity : entity_){
        if (entity == player_entity_ || entity->IsLine() || !entity->IsSolid()) continue;
        const Ogre::Vector3 position = entity->GetPosition();
        const float height = (position.z < player_position.z)
          ? entity->GetHeight() : player_entity_->GetHeight();
        const float distance_z = position.z - player_position.z + height;
        bool touching = false;
        if (distance_z < height * 2 && distance_z >= 0){
            const float reach = player_entity_->GetSolidRadius() + entity->GetSolidRadius() + 0.05f;
            const float dx = position.x - player_position.x;
            const float dy = position.y - player_position.y;
            touching = dx * dx + dy * dy < reach * reach;
        }
        if (!touching){
            entities_in_contact_.erase(entity);
            continue;
        }
        if (!entities_in_contact_.insert(entity).second) continue;
        ScriptEntity* script_entity = ScriptManager::getSingleton().GetScriptEntityByName(
          ScriptManager::ENTITY, entity->GetName()
        );
        // Most entities have no script for it: nothing to say then.
        if (script_entity != nullptr){
            ScriptManager::getSingleton().ScriptRequest(
              script_entity, "script_2", INTERACT_SCRIPT_PRIORITY, "", "", false, false
            );
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
    float best_distance = std::numeric_limits<float>::max();
    Ogre::Real nearest_worldmap_distance = -1.0f;
    for (unsigned int i = 0; i < entity_.size(); ++ i){
        if (entity_[i]->IsTalkable() == false) continue;
        if (entity_[i] == player_entity_) continue;
        Ogre::Vector3 pos1 =  entity_[i]->GetPosition();
        float interact_range =  entity_[i]->GetTalkRadius();
        Ogre::Vector3 pos2 = player_entity_->GetPosition();
        float solid_range = player_entity_->GetSolidRadius();
                if (IsWorldModule()){
                        const Ogre::Real target_bottom = pos1.y;
                        const Ogre::Real player_bottom = pos2.y;
                                                const Ogre::Real talk_range
                                                    = interact_range + solid_range + 0.05f;
                        const Ogre::Real distance
                            = (pos1.x - pos2.x) * (pos1.x - pos2.x)
                                + (pos1.z - pos2.z) * (pos1.z - pos2.z);
                        const bool height_overlap
                            = player_bottom <= target_bottom + entity_[i]->GetHeight()
                                && player_bottom + player_entity_->GetHeight() >= target_bottom;
                        if (
                                height_overlap && distance <= talk_range * talk_range
                                && (nearest_worldmap_distance < 0 || distance < nearest_worldmap_distance)
                        ){
                                entity_to_interact = entity_[i];
                                nearest_worldmap_distance = distance;
                        }
                        continue;
                }
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
                // With several entities in range, interact with the one the player faces most.
                // Nobody 90 degrees or more away from where the player faces is considered.
                if (
                  angle < Ogre::Degree(90) && IsBetterInteractionTarget(
                    angle.valueDegrees(), distance, less_angle.valueDegrees(), best_distance
                  )
                ){
                    less_angle = angle;
                    best_distance = distance;
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
          scr_entity, "on_interact", INTERACT_SCRIPT_PRIORITY, "", "", false, false
        );
        if (added == false){
            LOG_WARNING("Script 'on_interact' for entity '" +  name + "' doesn't exist.");
        }
    }
}
