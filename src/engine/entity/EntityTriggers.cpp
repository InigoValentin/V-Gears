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
#include "common/Logger.h"
#include "engine/entity/Entity.h"
#include "engine/entity/EntityTrigger.h"
#include "engine/entity/EntityTriggers.h"
#include "engine/script/ScriptManager.h"

const int EntityTriggers::INTERACT_SCRIPT_PRIORITY = 4;

float EntityTriggers::SquareDistanceToLine(
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

Ogre::Degree EntityTriggers::GetDirectionToPoint(
  const Ogre::Vector3& current_point, const Ogre::Vector3& direction_point
){
    Ogre::Vector2 up(0.0f, -1.0f);
    Ogre::Vector2 dir(direction_point.x - current_point.x, direction_point.y - current_point.y);
    // Angle between vectors
    Ogre::Degree angle(Ogre::Radian(SafeAcos(dir.dotProduct(up), dir.length() * up.length())));
    return (dir.x < 0) ? Ogre::Degree(360) - angle : angle;
}

float EntityTriggers::Reach(float solid_radius){
    // The radius an entity has when a script doesn't change it.
    const float default_radius = 0.21f;
    return solid_radius < 0.01f ? default_radius : solid_radius;
}

bool EntityTriggers::IsBetterInteractionTarget(
  float angle, float distance, float best_angle, float best_distance
){
    const float same_angle = 0.001f;
    if (angle < best_angle - same_angle) return true;
    return std::fabs(angle - best_angle) <= same_angle && distance < best_distance;
}

void EntityTriggers::ClearContacts(){contacts_.clear();}

void EntityTriggers::CheckLines(
  Entity* entity, const Ogre::Vector3& position, const std::vector<EntityTrigger*>& lines
){
    bool is_moving = true;
    if (
      std::fabs(entity->GetPosition().x - position.x) < 0.005f
      && std::fabs(entity->GetPosition().y - position.y) < 0.005f
      && std::fabs(entity->GetPosition().z - position.z) < 0.005f
    ){
        is_moving = false;
    }

    for (unsigned int i = 0; i < lines.size(); ++ i){
        ScriptEntity* trigger = ScriptManager::getSingleton().GetScriptEntityByName(
          ScriptManager::ENTITY, lines[i]->GetName()
        );

        if (trigger != nullptr && lines[i]->IsEnabled() == true){
            Ogre::Vector3 point_a = lines[i]->GetPoint1();
            Ogre::Vector3 point_b = lines[i]->GetPoint2();
            Ogre::Vector3 entity_position = entity->GetPosition();
            Ogre::Vector3 proj;

            // Calculate distance
            float square_dist = SquareDistanceToLine(entity_position, point_a, point_b, proj);
            float solid_radius = Reach(entity->GetSolidRadius());

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
              && lines[i]->CheckApproached() == false
              && is_moving
            ){
                lines[i]->SetApproached(true);
                //std::cout << "    [TRIGGER] " << entity->GetName() << " on_approach "
                //  << lines[i]->GetName() << std::endl;
                ScriptManager::getSingleton().ScriptRequest(
                  trigger, "on_approach", 1, entity->GetName(), "", false, false
                );
            }

            // Check on move to line.
            // Must happen only once when the entity center touches the line, if the entity has
            // previously entered the line.
            if (
              square_dist != -1 && square_dist < 1
              && lines[i]->CheckCrossed() == false
              && lines[i]->CheckApproached() == true
            ){
                lines[i]->SetCrossed(true);
                //std::cout << "    [TRIGGER] " << entity->GetName() << " on_cross "
                //  << lines[i]->GetName() << std::endl;
                ScriptManager::getSingleton().ScriptRequest(
                  trigger, "on_cross", 1, entity->GetName(), "", false, false
                );
            }

            // Check on cross line.
            // Must happen on every frame while the entity solid radius collides with the line, if
            // the entity has previously entered the line.
            if (
              square_dist != -1 && square_dist <= solid_radius * solid_radius
              && lines[i]->CheckApproached() == true
            ){
                // TODO: Fix this hack.
                // After the on_near script is run, set a cooltime of 5 frames before running
                // it again. This is because most of the time this trigger checks for a key, and
                // if its pressed, is run multiple times.
                if (lines[i]->GetNearEventCooldown() == 0){
                    //std::cout << "    [TRIGGER] " << entity->GetName()
                    //  << " on_near " << lines[i]->GetName() << std::endl;
                    ScriptManager::getSingleton().ScriptRequest(
                      trigger, "on_near", 1, entity->GetName(), "", true, true
                    );
                    lines[i]->ResetNearEventCooldown();
                }
                else lines[i]->DecreaseNearEventCooldown();
            }

            // Check on cross line once.
            // The same as on cross line, but only once.
            if (
              square_dist != -1 && square_dist < solid_radius * solid_radius
              && lines[i]->CheckNearSingleEventTriggered() == false
              && lines[i]->CheckApproached() == true
            ){
                lines[i]->SetNearSingleEventTriggered(true);
                //std::cout << "    [TRIGGER] " << entity->GetName()
                //  << " on_near_once " << lines[i]->GetName() << std::endl;
                ScriptManager::getSingleton().ScriptRequest(
                  trigger, "on_near_once", 1, entity->GetName(), "", false, false
                );

            }

            // Check on leave line.
            // Must be triggered once when the line is not in the entity solid radius, but only if
            // on enter script has been triggered previously.
            if (
              square_dist != -1 && square_dist > solid_radius * solid_radius
              && lines[i]->CheckApproached()
            ){
                //std::cout << "    [TRIGGER] " << entity->GetName() << " on_leave "
                //  << lines[i]->GetName() << std::endl;
                ScriptManager::getSingleton().ScriptRequest(
                  trigger, "on_leave", 1, entity->GetName(), "", false, false
                );
            }

            // If the entity is far enough from the line, mark every trigger as inactive
            if (square_dist == -1 || square_dist > solid_radius * solid_radius){
                lines[i]->Clear();
                //std::cout << "    [TRIGGER] " << entity->GetName() << " clear of "
                //  << lines[i]->GetName() << std::endl;
            }

        }
    }
}

void EntityTriggers::CheckContacts(
  Entity* player, const bool player_locked, const std::vector<Entity*>& entities
){
    if (player == nullptr || player_locked || !player->IsSolid()){
        contacts_.clear();
        return;
    }
    const Ogre::Vector3 player_position = player->GetPosition();
    for (Entity* entity : entities){
        if (entity == player || entity->IsLine() || !entity->IsSolid()) continue;
        const Ogre::Vector3 position = entity->GetPosition();
        const float height = (position.z < player_position.z)
          ? entity->GetHeight() : player->GetHeight();
        const float distance_z = position.z - player_position.z + height;
        bool touching = false;
        if (distance_z < height * 2 && distance_z >= 0){
            const float reach = player->GetSolidRadius() + entity->GetSolidRadius() + 0.05f;
            const float dx = position.x - player_position.x;
            const float dy = position.y - player_position.y;
            touching = dx * dx + dy * dy < reach * reach;
        }
        if (!touching){
            contacts_.erase(entity);
            continue;
        }
        if (!contacts_.insert(entity).second) continue;
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

void EntityTriggers::Interact(
  Entity* player, const std::vector<Entity*>& entities, const bool world_map
){
    Entity* entity_to_interact = world_map
      ? FindWorldMapTarget(player, entities) : FindFieldTarget(player, entities);
    if (entity_to_interact != nullptr && entity_to_interact->IsLine() == false){
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

void EntityTriggers::InteractWithLines(const std::vector<EntityTrigger*>& lines){
    for (EntityTrigger* line : lines){
        if (line->IsEnabled() == true && line->CheckApproached() == true){
            ScriptEntity* script_entity = ScriptManager::getSingleton().GetScriptEntityByName(
              ScriptManager::ENTITY, line->GetName()
            );
            const bool added = ScriptManager::getSingleton().ScriptRequest(
              script_entity, "on_interact", INTERACT_SCRIPT_PRIORITY, "", "", false, false
            );
            if (added == false){
                LOG_WARNING(
                  "Script 'on_interact' for entity '" + line->GetName() + "' doesn't exist."
                );
            }
        }
    }
}

Entity* EntityTriggers::FindWorldMapTarget(Entity* player, const std::vector<Entity*>& entities){
    Entity* target = nullptr;
    Ogre::Real nearest_distance = -1.0f;
    const Ogre::Vector3 player_position = player->GetPosition();
    for (Entity* entity : entities){
        if (entity->IsTalkable() == false || entity == player) continue;
        const Ogre::Vector3 position = entity->GetPosition();
        const Ogre::Real talk_range = entity->GetTalkRadius() + player->GetSolidRadius() + 0.05f;
        // The world map has the height in the Y axis.
        const Ogre::Real distance
          = (position.x - player_position.x) * (position.x - player_position.x)
          + (position.z - player_position.z) * (position.z - player_position.z);
        const bool height_overlap
          = player_position.y <= position.y + entity->GetHeight()
          && player_position.y + player->GetHeight() >= position.y;
        if (
          height_overlap && distance <= talk_range * talk_range
          && (nearest_distance < 0 || distance < nearest_distance)
        ){
            target = entity;
            nearest_distance = distance;
        }
    }
    return target;
}

Entity* EntityTriggers::FindFieldTarget(Entity* player, const std::vector<Entity*>& entities){
    Entity* target = nullptr;
    Ogre::Degree less_angle(90);
    float best_distance = std::numeric_limits<float>::max();
    const Ogre::Degree player_angle = player->GetRotation();
    const Ogre::Vector3 player_position = player->GetPosition();
    const float solid_range = player->GetSolidRadius();
    for (Entity* entity : entities){
        if (entity->IsTalkable() == false || entity == player) continue;
        const Ogre::Vector3 position = entity->GetPosition();
        const int height = (position.z < player_position.z)
          ? entity->GetHeight() : player->GetHeight();
        const float distance_z = position.z - player_position.z + height;
        if (!((distance_z < (height * 2)) && (distance_z >= 0))) continue;
        float interact_range = entity->GetTalkRadius();
        interact_range = (interact_range + solid_range) * (interact_range + solid_range);
        const float distance
          = (position.x - player_position.x) * (position.x - player_position.x)
          + (position.y - player_position.y) * (position.y - player_position.y);
        if (!(distance < interact_range + solid_range)) continue;
        Ogre::Degree angle = player_angle - GetDirectionToPoint(player_position, position);
        angle = (angle < Ogre::Degree(0)) ? -angle : angle;
        angle = (angle >= Ogre::Degree(180)) ? Ogre::Degree(360) - angle : angle;
        // With several entities in range, interact with the one the player faces most. Nobody 90
        // degrees or more away from where the player faces is considered.
        if (
          angle < Ogre::Degree(90) && IsBetterInteractionTarget(
            angle.valueDegrees(), distance, less_angle.valueDegrees(), best_distance
          )
        ){
            less_angle = angle;
            best_distance = distance;
            target = entity;
        }
    }
    return target;
}
