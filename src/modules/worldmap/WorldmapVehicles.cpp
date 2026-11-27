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

#include "modules/worldmap/WorldmapModule.h"
#include <algorithm>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include "engine/entity/EntityManager.h"
#include "engine/entity/EntityModel.h"
#include "engine/input/InputManager.h"
#include "modules/worldmap/WorldmapConstants.h"
#include "modules/worldmap/WorldmapData.h"

namespace VGears {

    using namespace Ogre;

    /// Turning speed of the flying vehicles, in radians per second.
    static const float FLIGHT_TURN_SPEED = 1.5f;

    /// Speed up and down of the flying vehicles, in world units per second.
    static const float FLIGHT_CLIMB_SPEED = 1.0f;

    /// Highest height over the ground where the Highwind can be unboarded. It lands there.
    static const float HIGHWIND_MAX_LANDING_HEIGHT = 1.0f;

    /// Highest point the Highwind can reach.
    static const float HIGHWIND_MAX_HEIGHT = 1.0f;

    /**
     * Animations of a vehicle in the world map, from the A files of the model: when it's parked,
     * when it's boarded, and when it's boarded and moving. An empty name is no animation (the
     * model in its pose). The roles were found by playing them, and for the models with only one
     * animation it's the one of the model.
     */
    struct VehicleAnimations{
        const char* parked;
        const char* boarded;
        const char* moving;
        /// The parked animation shows its first frame and doesn't move.
        bool parked_paused;
    };

    static const VehicleAnimations CHOCOBO_ANIMATIONS = {"asc", "asc", "asd", false};
    static const VehicleAnimations BUGGY_ANIMATIONS = {"age", "aha", "aha", false};
    static const VehicleAnimations TINY_BRONCO_ANIMATIONS = {"dya", "dya", "dya", true};
    static const VehicleAnimations HIGHWIND_ANIMATIONS = {"cic", "cic", "cic", true};
    static const VehicleAnimations SUBMARINE_ANIMATIONS = {"dfe", "dfe", "dfe", true};

    /**
     * Plays the animation of a vehicle in its current state.
     */
    static void PlayVehicleAnimation(
      EntityModel* vehicle, const ::Entity* active_entity, const bool moving,
      const VehicleAnimations& animations
    ){
        if (vehicle == nullptr) return;
        const bool parked = vehicle != active_entity;
        const char* name = parked ? animations.parked
          : (moving ? animations.moving : animations.boarded);
        if (name[0] == '\0'){
            vehicle->StopAnimation();
            return;
        }
        if (parked && animations.parked_paused){
            vehicle->PlayAnimationPaused(name);
            return;
        }
        vehicle->UnfreezeAnimation();
        if (vehicle->HasAnimation(name)) vehicle->PlayAnimationContinue(name);
    }

    void WorldmapModule::UpdateVehicleAnimations(const bool moving){
        const ::Entity* active_entity = EntityManager::getSingleton().ScriptGetPlayerEntity();
        PlayVehicleAnimation(chocobo_entity_, active_entity, moving, CHOCOBO_ANIMATIONS);
        PlayVehicleAnimation(buggy_entity_, active_entity, moving, BUGGY_ANIMATIONS);
        PlayVehicleAnimation(tiny_bronco_entity_, active_entity, moving, TINY_BRONCO_ANIMATIONS);
        PlayVehicleAnimation(highwind_entity_, active_entity, moving, HIGHWIND_ANIMATIONS);
        PlayVehicleAnimation(submarine_entity_, active_entity, moving, SUBMARINE_ANIMATIONS);
    }

    void WorldmapModule::UpdateFlight(
      EntityModel* entity, const Ogre::Real delta_time, const bool highwind
    ){
        Ogre::SceneNode* node = entity->GetModel()->getParentSceneNode()->getParentSceneNode();
        if (flight_entity_ != entity){
            // Starts flying: continue from the current heading and height.
            flight_entity_ = entity;
            flight_heading_ = node->getOrientation().getYaw().valueRadians();
            flight_y_ = entity->GetPosition().y;
            camera_yaw_ = 0;
        }

        float turn = 0;
        float climb = 0;
        bool forward = false;
        // Scripts lock the player during events and transitions.
        if (!EntityManager::getSingleton().IsPlayerLocked()){
            InputManager& input = InputManager::getSingleton();
            // The heading decreases to turn right: the model faces +Z at heading 0 and the
            // right side of the camera behind it is -X.
            if (input.IsGameEventPressed("walk_left")) turn += 1.0f;
            if (input.IsGameEventPressed("walk_right")) turn -= 1.0f;
            // Inverted, like the controls of a plane: down climbs and up descends.
            if (input.IsGameEventPressed("walk_up")) climb -= 1.0f;
            if (input.IsGameEventPressed("walk_down")) climb += 1.0f;
            forward = input.IsGameEventPressed("CONFIRM");
        }

        flight_heading_ += turn * FLIGHT_TURN_SPEED * delta_time;
        if (flight_heading_ > Ogre::Math::PI) flight_heading_ -= Ogre::Math::TWO_PI;
        else if (flight_heading_ < -Ogre::Math::PI) flight_heading_ += Ogre::Math::TWO_PI;
        node->setOrientation(Ogre::Quaternion(Ogre::Radian(flight_heading_), Ogre::Vector3::UNIT_Y));

        const Ogre::Vector3 current_position = entity->GetPosition();
        Ogre::Vector3 requested_position = current_position;
        if (forward){
            const Ogre::Vector3 direction(
              Ogre::Math::Sin(Ogre::Radian(flight_heading_)), 0.0f,
              Ogre::Math::Cos(Ogre::Radian(flight_heading_))
            );
            requested_position += direction * delta_time * GetSpeed(
              highwind ? "highwind" : "submarine", From16BitFixedPoint(8182) * 0.45f * 2.0f
            );
        }

        // The terrain below. If the vehicle can't go there, it stays but can still go up or down.
        Ogre::Vector3 surface;
        size_t triangle_index = 0;
        if (!FindWalkableSurface(requested_position, surface, false, true, nullptr, &triangle_index)){
            requested_position = current_position;
            if (!FindWalkableSurface(
              requested_position, surface, false, false, nullptr, &triangle_index
            )) return;
            forward = false;
        }

        // The ground has priority over the maximum height, so the vehicle never goes through it.
        const Ogre::Real ceiling = highwind ? HIGHWIND_MAX_HEIGHT : UNDERWATER_MAX_HEIGHT;
        Ogre::Real new_y = flight_y_ + climb * FLIGHT_CLIMB_SPEED * delta_time;
        new_y = std::min(new_y, ceiling);
        new_y = std::max(new_y, surface.y + FLIGHT_MIN_CLEARANCE);
        const Ogre::Vector3 new_position(requested_position.x, new_y, requested_position.z);
        if (EntityManager::getSingleton().IsWorldPositionBlocked(entity, new_position)) return;
        flight_y_ = new_y;
        entity->SetPosition(new_position);
        if (forward) EnterTriangle(terrain_triangles_[triangle_index]);
    }

    /// Name of a vehicle in the scripts (WorldMapSettings).
    const char* WorldmapModule::GetVehicleScriptName(const VehicleType vehicle){
        switch (vehicle){
            case VehicleType::CHOCOBO: return "chocobo";
            case VehicleType::BUGGY: return "buggy";
            case VehicleType::HIGHWIND: return "highwind";
            case VehicleType::SUBMARINE: return "submarine";
            case VehicleType::TINY_BRONCO: return "tiny_bronco";
            default: return "walk";
        }
    }

      bool WorldmapModule::UnboardVehicle(const char* vehicle_name){
        if (vehicle_name == nullptr) return false;

        EntityManager& entity_manager = EntityManager::getSingleton();
        ::Entity* player_entity = entity_manager.ScriptGetPlayerEntity();
        ::Entity* vehicle_entity = entity_manager.GetEntity(vehicle_name);
        if (
          player_entity == nullptr || vehicle_entity == nullptr
          || player_entity != vehicle_entity || player_entity == player_entity_
        ){
          return false;
        }

        const Ogre::Vector3 vehicle_position = vehicle_entity->GetPosition();
        const Ogre::Real unboard_distance
          = vehicle_entity->GetSolidRadius() + player_entity_->GetSolidRadius() + 0.1f;
        const Ogre::Real diagonal_distance = unboard_distance * 0.70710678f;
        const Ogre::Vector3 directions[] = {
          Ogre::Vector3(1, 0, 0), Ogre::Vector3(-1, 0, 0),
          Ogre::Vector3(0, 0, 1), Ogre::Vector3(0, 0, -1),
          Ogre::Vector3(1, 0, 1), Ogre::Vector3(-1, 0, 1),
          Ogre::Vector3(1, 0, -1), Ogre::Vector3(-1, 0, -1)
        };

        for (size_t i = 0; i < sizeof(directions) / sizeof(directions[0]); ++i){
          const Ogre::Real distance = i < 4 ? unboard_distance : diagonal_distance;
          const Ogre::Vector3 requested_position
            = vehicle_position + directions[i] * distance;
          Ogre::Vector3 surface;
          uint8 walkability_type = 0;
          if (!FindWalkableSurface(
              requested_position, surface, false, false, &walkability_type
            ) || !IsFootWalkable(walkability_type)){
            continue;
          }
          surface.y += player_height_offset_;
          if (entity_manager.IsWorldPositionBlocked(player_entity_, surface)) continue;

          if (vehicle_entity == highwind_entity_){
            // The Highwind lands. It can't be left higher up, Cloud couldn't board it again.
            Ogre::Vector3 ground;
            if (!FindWalkableSurface(vehicle_position, ground, false, false)) return false;
            if (vehicle_position.y - ground.y > HIGHWIND_MAX_LANDING_HEIGHT) return false;
            vehicle_entity->SetPosition(Ogre::Vector3(vehicle_position.x, ground.y, vehicle_position.z));
            // Landing on the terrain of the northern cave enters it (the function of the triangle runs).
            uint8 landing_terrain = 0;
            size_t landing_triangle = 0;
            if (FindWalkableSurface(vehicle_position, ground, false, false, &landing_terrain, &landing_triangle)
              && landing_terrain == VGears::WorldmapFileSerializer::NORTHERND_CAVE
              && terrain_triangles_[landing_triangle].function_id >= 3){
              landing_after_unboard_ = &terrain_triangles_[landing_triangle];
            }
          }
          player_entity_->SetPosition(surface);
          // The boarding scripts hide Cloud and make him non-solid.
          player_entity_->SetVisible(true);
          player_entity_->SetSolid(true);
          vehicle_entity->SetTalkable(true);
          entity_manager.ScriptSetPlayerEntity("WorldMapCloud");
          ClearVehicleTerrains();
          if (landing_after_unboard_ != nullptr){
            // The Highwind is what entered (the vehicle of the trigger).
            current_vehicle_ = VehicleType::HIGHWIND;
            landing_trigger_ = true;
            TriggerTriangleFunction(*landing_after_unboard_);
            landing_trigger_ = false;
            landing_after_unboard_ = nullptr;
          }
          current_vehicle_ = VehicleType::ON_FOOT;
          return true;
        }
        return false;
      }

} // namespace VGears
