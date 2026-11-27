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
#include <cmath>
#include <limits>
#include <OgreEntity.h>
#include <OgreException.h>
#include <OgreLogManager.h>
#include <OgreSceneNode.h>
#include "engine/entity/EntityManager.h"
#include "engine/entity/EntityModel.h"
#include "modules/worldmap/WorldmapConstants.h"
#include "modules/worldmap/WorldmapData.h"

namespace VGears {

    using namespace Ogre;

    /// How much the model of the submarine is under the surface of the water in WM0, in world
    /// units. Found by looking at it.
    static const float SUBMARINE_SUBMERGED_DEPTH = 0.09f;

    /**
     * Moves the model of an entity up or down, without moving the entity.
     *
     * The model is scaled to the size of the world, so the offset has to be divided by that scale
     * to be in world units.
     *
     * @param[in] entity The entity.
     * @param[in] offset Distance in world units. Negative is down.
     */
    static void SetModelHeightOffset(EntityModel* entity, const float offset){
        Ogre::SceneNode* model_node = entity->GetModel()->getParentSceneNode();
        float scale = 1.0f;
        if (model_node->getParentSceneNode() != nullptr){
            scale = model_node->getParentSceneNode()->_getDerivedScale().y;
        }
        if (scale <= 0.0f) scale = 1.0f;
        model_node->setPosition(Ogre::Vector3(0.0f, offset / scale, 0.0f));
    }

    /// Height over the ground where the submarine starts, so it floats.
    static const float SUBMARINE_START_CLEARANCE = 0.3f;

    void WorldmapModule::CreatePlayer(){
        Ogre::Vector3 requested_spawn = GetSpawn("Cloud", Ogre::Vector3(42.0f, 0.0f, 36.5f));
        Ogre::Vector3 spawn_position;
        bool spawn_found;
        if (map_id_ == 0){
            // A given spawn point (from a field exit) can be on any terrain, like a town or a forest.
            const bool given_spawn = spawn_overrides_.find("Cloud") != spawn_overrides_.end();
            spawn_found = given_spawn
              ? FindWalkableSurface(requested_spawn, spawn_position, false, false)
              : FindWalkableSurface(requested_spawn, spawn_position, true);
        }
        else{
            // Spawn points of the other maps are unknown: use the walkable triangle closest to the
            // center of the map, or to the position where the submarine dived.
            const Ogre::Vector3 center = dive_requested_ ? dive_position_ : terrain_.GetCenter();
            Ogre::Real best_distance = std::numeric_limits<Ogre::Real>::max();
            // The exact position of the dive, if the submarine can be there.
            const bool exact_dive_position = dive_requested_
              && FindWalkableSurface(dive_position_, spawn_position, false, true);
            if (exact_dive_position) best_distance = 0;
            else for (const TerrainTriangle& triangle : terrain_.GetTriangles()){
                const Ogre::Vector3 centroid
                  = (triangle.vertices[0] + triangle.vertices[1] + triangle.vertices[2]) / 3.0f;
                if (map_id_ == 1 ? (UNDERWATER_HEIGHT_LIMIT_ENABLED
                  && centroid.y + FLIGHT_MIN_CLEARANCE > UNDERWATER_MAX_HEIGHT)
                  : !IsActiveTerrainAllowed(triangle.walkability_type)) continue;
                const Ogre::Real distance = Ogre::Vector2(
                  centroid.x - center.x, centroid.z - center.z
                ).squaredLength();
                if (distance < best_distance){
                    best_distance = distance;
                    requested_spawn = centroid;
                }
            }
            spawn_found = exact_dive_position || (
              best_distance < std::numeric_limits<Ogre::Real>::max()
              && FindWalkableSurface(requested_spawn, spawn_position, false)
            );
        }
        if (!spawn_found){
            LogManager::getSingleton().logMessage(
              "World map " + std::to_string(map_id_) + " has no walkable surface for Cloud's spawn."
            );
            return;
        }

        if (map_id_ == 1){
            // The underwater map is played with the submarine only.
            AddWorldModel(
              "Submarine", "ddd.mesh", spawn_position, From16BitFixedPoint(8182) * 0.1125f,
              Ogre::Quaternion::IDENTITY, 1.0f, false
            );
            submarine_entity_ = static_cast<EntityModel*>(
              EntityManager::getSingleton().GetEntity("WorldMapSubmarine")
            );
            if (submarine_entity_ == nullptr) return;
            player_entity_ = submarine_entity_;
            player_height_offset_ = 0;
            if (dive_requested_){
              // Same heading as in WM0.
              submarine_entity_->GetModel()->getParentSceneNode()->getParentSceneNode()
                ->setOrientation(Ogre::Quaternion(Ogre::Radian(dive_heading_), Ogre::Vector3::UNIT_Y));
              dive_requested_ = false;
            }
            // It floats over the seabed.
            submarine_entity_->SetPosition(
              submarine_entity_->GetPosition() + Ogre::Vector3(0, SUBMARINE_START_CLEARANCE, 0)
            );
            EntityManager::getSingleton().ScriptSetPlayerEntity("WorldMapSubmarine");
            UpdateCamera(0, true);
            return;
        }

        LogManager::getSingleton().logMessage(
          "Spawning Cloud at "
            + Ogre::StringConverter::toString(spawn_position.x) + ", "
            + Ogre::StringConverter::toString(spawn_position.z)
        );

        player_entity_ = EntityManager::getSingleton().AddWorldMapEntity(
          "WorldMapCloud", "cloud.mesh", spawn_position, Ogre::Degree(0),
          Ogre::Vector3::UNIT_SCALE, "FIELD_MODELS"
        );
        if (player_entity_ == nullptr) return;

        // Cloud's bounds without animations. The bounds in the mesh can't be used for sizing: the
        // installer enlarges them with the animations' movement, so the model is not culled.
        const Ogre::AxisAlignedBox bounds(
          -8.7473f, -8.1671f, -4.1878f, 7.8685f, 5.6837f, 22.8135f
        );
        const Ogre::Vector3 model_size = bounds.getSize();
        const Ogre::Real model_height = model_size.y;
        if (model_height > 0){
            const Ogre::Real scale = From16BitFixedPoint(8182) * 0.45f * 0.25f * 0.25f
              / model_height;
            player_entity_->setScale(Ogre::Vector3::UNIT_SCALE * scale);
            player_entity_->SetHeight(model_height * scale);
            player_entity_->SetSolidRadius(
              std::max(model_size.x, model_size.z) * scale * 0.5f
            );
            player_entity_->SetCollisionDebugOrientation(
              Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_X)
            );
            // Only solid entities can interact with the vehicles.
            player_entity_->SetSolid(true);
            player_height_offset_ = -bounds.getMinimum().y * scale;
            spawn_position.y += player_height_offset_;
            player_entity_->SetPosition(spawn_position);
            Ogre::AnimationStateSet* animations
              = player_entity_->GetModel()->getAllAnimationStates();
            if (animations == nullptr || !animations->hasAnimationState("Walk")){
                LogManager::getSingleton().logMessage(
                  "Cloud model has no Walk animation state."
                );
            }
            EntityManager::getSingleton().ScriptSetPlayerEntity("WorldMapCloud");
            player_entity_->PlayAnimationContinue("Idle");
            // Vehicles and companions are only placed in the overworld.
            if (map_id_ == 0){
                CreateCompanionModels();
                // Coming back from WM2: everything is where it was, except for the submarine.
                if (surface_requested_){
                    RestoreOverworldSnapshot();
                    PlaceSurfacedSubmarine();
                }
                overworld_snapshot_.clear();
            }
            UpdateCamera(0, true);
        }
    }

    void WorldmapModule::AddWorldModel(
      const Ogre::String& name, const Ogre::String& mesh_name,
      const Ogre::Vector3& desired_position, Ogre::Real target_size,
      const Ogre::Quaternion& orientation, Ogre::Real collision_radius_scale,
      bool talkable, Ogre::Real explicit_scale
    ){
        Ogre::Vector3 surface_position;
        if (!FindWalkableSurface(desired_position, surface_position, false, false)){
            LogManager::getSingleton().logMessage(
              "No terrain surface found for world-map model '" + name + "'."
            );
            return;
        }

        EntityModel* entity = nullptr;
        try{
            entity = EntityManager::getSingleton().AddWorldMapEntity(
              "WorldMap" + name, mesh_name, surface_position,
              Ogre::Degree(0), Ogre::Vector3::UNIT_SCALE, "WORLD_MODELS"
            );
        }
        catch (const Ogre::Exception& exception){
            LogManager::getSingleton().logMessage(
              "Failed to load world-map model '" + mesh_name + "': "
                + exception.getDescription()
            );
            return;
        }
        if (entity == nullptr) return;

        const Ogre::Vector3 bounds_size = entity->GetModel()->getBoundingBox().getSize();
        const Ogre::Real largest_dimension = std::max(
          bounds_size.x, std::max(bounds_size.y, bounds_size.z)
        );
        if (largest_dimension <= 0){
            LogManager::getSingleton().logMessage(
              "World-map model '" + mesh_name + "' has empty bounds."
            );
            return;
        }

        // The scale of the model: the size of its largest dimension, or the one that is given.
        const Ogre::Real model_scale = explicit_scale > 0 ? explicit_scale
          : target_size / largest_dimension;
        const Ogre::Vector3 oriented_x = orientation * Ogre::Vector3::UNIT_X;
        const Ogre::Vector3 oriented_y = orientation * Ogre::Vector3::UNIT_Y;
        const Ogre::Vector3 oriented_z = orientation * Ogre::Vector3::UNIT_Z;
        const Ogre::Real world_width =
          Ogre::Math::Abs(oriented_x.x) * bounds_size.x
          + Ogre::Math::Abs(oriented_y.x) * bounds_size.y
          + Ogre::Math::Abs(oriented_z.x) * bounds_size.z;
        const Ogre::Real world_height =
          Ogre::Math::Abs(oriented_x.y) * bounds_size.x
          + Ogre::Math::Abs(oriented_y.y) * bounds_size.y
          + Ogre::Math::Abs(oriented_z.y) * bounds_size.z;
        const Ogre::Real world_depth =
          Ogre::Math::Abs(oriented_x.z) * bounds_size.x
          + Ogre::Math::Abs(oriented_y.z) * bounds_size.y
          + Ogre::Math::Abs(oriented_z.z) * bounds_size.z;
        entity->setRootOrientation(orientation);
        entity->setScale(Ogre::Vector3::UNIT_SCALE * model_scale);
        entity->SetSolid(true);
        entity->SetHeight(world_height * model_scale);
        entity->SetSolidRadius(
          std::max(world_width, world_depth) * model_scale * 0.5f
            * collision_radius_scale
        );
        entity->SetTalkable(talkable);
        if (talkable) entity->SetTalkRadius(entity->GetSolidRadius());
        // The submarine of the overworld is partially under the surface of the water. Only the
        // model is lowered, so its position and collision are the same.
        if (map_id_ == 0 && name == "Submarine"){
            SetModelHeightOffset(entity, -SUBMARINE_SUBMERGED_DEPTH);
        }
        entity->SetCollisionDebugOrientation(
          Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_X)
        );
        LogManager::getSingleton().logMessage(
          "Placed world-map model '" + name + "' at "
            + Ogre::StringConverter::toString(surface_position.x) + ", "
            + Ogre::StringConverter::toString(surface_position.z)
        );
    }

    void WorldmapModule::CreateCompanionModels(){
        // These models and their spawn points are now hardcoded, they mmust be
        // loaded according to the story point and save game data.
        const Ogre::Real space = From16BitFixedPoint(8182);
        AddWorldModel(
          "Chocobo", "aja.mesh",
          GetSpawn("Chocobo", Ogre::Vector3(42.5f, 0.0f, 37.5f)),
          space * 0.14f,
          // The animations of the model (asc) already have it the right way up.
          Ogre::Quaternion::IDENTITY, 1.0f, true
        );
        AddWorldModel(
          "Highwind", "cgd.mesh",
          GetSpawn("Highwind", Ogre::Vector3(43.0f, 0.0f, 36.5f)),
          space * 0.25f,
          // The animation of the model (cic) already has it the right way up.
          Ogre::Quaternion::IDENTITY, 0.5f, true
        );
        AddWorldModel(
          "Buggy", "aba.mesh",
          GetSpawn("Buggy", Ogre::Vector3(42.0f, 0.0f, 36.0f)),
          space * 0.3f, Ogre::Quaternion::IDENTITY, 0.5f, true
        );
        AddWorldModel(
          "Submarine", "ddd.mesh",
          GetSpawn("Submarine", Ogre::Vector3(41.5f, 0.0f, 36.5f)),
          space * 0.1125f, Ogre::Quaternion::IDENTITY, 1.0f, true
        );
        AddWorldModel(
          "TinyBronco", "dva.mesh",
          GetSpawn("TinyBronco", Ogre::Vector3(42.0f, 0.0f, 37.37f)),
          space * 0.15f, Ogre::Quaternion::IDENTITY, 0.75f, true
        );
        highwind_entity_ = static_cast<EntityModel*>(
          EntityManager::getSingleton().GetEntity("WorldMapHighwind")
        );
        buggy_entity_ = static_cast<EntityModel*>(
          EntityManager::getSingleton().GetEntity("WorldMapBuggy")
        );
        chocobo_entity_ = static_cast<EntityModel*>(
          EntityManager::getSingleton().GetEntity("WorldMapChocobo")
        );
        submarine_entity_ = static_cast<EntityModel*>(
          EntityManager::getSingleton().GetEntity("WorldMapSubmarine")
        );
        tiny_bronco_entity_ = static_cast<EntityModel*>(
          EntityManager::getSingleton().GetEntity("WorldMapTinyBronco")
        );
        for (const Ogre::String& name : hidden_vehicles_){
            ::Entity* entity = EntityManager::getSingleton().GetEntity("WorldMap" + name);
            if (entity == nullptr) continue;
            entity->SetVisible(false);
            entity->SetSolid(false);
            entity->SetTalkable(false);
        }
    }

    void WorldmapModule::AddWorldModelFromScript(
      const char* name, const char* hrc, const float x, const float z, const int raw_scale,
      const int direction, const float rotation_x, const float rotation_y, const float rotation_z
    ){
        if (name == nullptr || hrc == nullptr) return;
        // The game scale is for coordinates of 4096 units per world unit, and 0x200 is 1 (15.2).
        // The 1.1 is what the models that were adjusted by looking at them have more.
        const Ogre::Real scale = (static_cast<Ogre::Real>(raw_scale) / 512.0f) * 15.2f / 4096.0f * 1.1f;
        AddWorldModel(
          name, Ogre::String(hrc) + ".mesh", Ogre::Vector3(x, 0.0f, z), 1.0f,
          Ogre::Quaternion::IDENTITY, 1.0f, false, scale
        );
        ::Entity* entity = EntityManager::getSingleton().GetEntity(Ogre::String("WorldMap") + name);
        if (entity == nullptr) return;
        entity->SetSolid(false);
        // The rotation of the model itself.
        entity->setRootOrientation(
          Ogre::Quaternion(Ogre::Degree(rotation_x), Ogre::Vector3::UNIT_X)
            * Ogre::Quaternion(Ogre::Degree(rotation_y), Ogre::Vector3::UNIT_Y)
            * Ogre::Quaternion(Ogre::Degree(rotation_z), Ogre::Vector3::UNIT_Z)
        );
        // The direction of the script is a full turn in 256: 0 faces +Z (south), 64 east (+X), 128 north and
        // 192 west. Checked with the sea around Junon in the scene of the Warship, which sails at 192.
        const Ogre::Radian heading(Ogre::Degree(direction * 360.0f / 256.0f));
        static_cast<EntityModel*>(entity)->GetModel()->getParentSceneNode()->getParentSceneNode()
          ->setOrientation(Ogre::Quaternion(heading, Ogre::Vector3::UNIT_Y));
    }

    void WorldmapModule::SetVehicleHidden(const char* entity_name){
        if (entity_name != nullptr) hidden_vehicles_.insert(entity_name);
    }

    void WorldmapModule::SetSpeed(const char* name, const float speed){
        if (name != nullptr && speed > 0.0f) speeds_[name] = speed;
    }

    void WorldmapModule::SetCamera(const char* name, const float distance, const float pitch_degrees){
        if (name != nullptr && distance > 0.0f) camera_settings_[name] = {distance, pitch_degrees};
    }

    float WorldmapModule::GetSpeed(const char* name, const float default_speed) const{
        const auto speed = speeds_.find(name);
        return speed == speeds_.end() ? default_speed : speed->second;
    }

    void WorldmapModule::SetSpawn(const char* entity_name, const float x, const float z){
        if (entity_name == nullptr) return;
        spawn_overrides_[entity_name] = Ogre::Vector2(x, z);
    }

    Ogre::Vector3 WorldmapModule::GetSpawn(
      const Ogre::String& entity_name, const Ogre::Vector3& default_spawn
    ) const{
        const auto spawn = spawn_overrides_.find(entity_name);
        if (spawn == spawn_overrides_.end()) return default_spawn;
        return Ogre::Vector3(spawn->second.x, 0.0f, spawn->second.y);
    }

    void WorldmapModule::SetModelOffset(const char* entity_name, const float y){
        if (entity_name == nullptr) return;
        ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return;
        SetModelHeightOffset(static_cast<EntityModel*>(entity), y);
    }

    void WorldmapModule::SetEntityAltitude(const char* entity_name, const float y){
        if (entity_name == nullptr) return;
        ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return;
        const Ogre::Vector3 position = entity->GetPosition();
        entity->SetPosition(Ogre::Vector3(position.x, y, position.z));
    }

    void WorldmapModule::SetModelOrientation(
      const char* entity_name, const float x, const float y, const float z
    ){
        if (entity_name == nullptr) return;
        ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return;
        entity->setRootOrientation(
          Ogre::Quaternion(Ogre::Degree(x), Ogre::Vector3::UNIT_X)
            * Ogre::Quaternion(Ogre::Degree(y), Ogre::Vector3::UNIT_Y)
            * Ogre::Quaternion(Ogre::Degree(z), Ogre::Vector3::UNIT_Z)
        );
    }

    void WorldmapModule::SetModelHeading(const char* entity_name, const float direction){
        if (entity_name == nullptr) return;
        ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return;
        // The same heading as AddWorldModelFromScript.
        const Ogre::Radian heading(Ogre::Degree(direction * 360.0f / 256.0f));
        static_cast<EntityModel*>(entity)->GetModel()->getParentSceneNode()->getParentSceneNode()
          ->setOrientation(Ogre::Quaternion(heading, Ogre::Vector3::UNIT_Y));
    }

    int WorldmapModule::GetModelHeading(const char* entity_name){
        if (entity_name == nullptr) return 0;
        ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return 0;
        const Ogre::Quaternion orientation = static_cast<EntityModel*>(entity)->GetModel()
          ->getParentSceneNode()->getParentSceneNode()->getOrientation();
        const Ogre::Vector3 forward = orientation * Ogre::Vector3(0.0f, 0.0f, 1.0f);
        const float turns = std::atan2(forward.x, forward.z) / Ogre::Math::TWO_PI;
        return static_cast<int>(std::floor(turns * 256.0f + 0.5f)) & 255;
    }

    void WorldmapModule::PlaceSnowPole(){
        if (player_entity_ == nullptr) return;
        const Ogre::Vector3 position = player_entity_->GetPosition();
        snow_.PlacePole([this, position](const std::string& name){
            // The model and scale of the snow poles of the game (WorldMapModels: djc, 0x300).
            AddWorldModelFromScript(name.c_str(), "djc", position.x, position.z, 0x300, 0, 0.0f, 0.0f, 0.0f);
        });
    }

} // namespace VGears
