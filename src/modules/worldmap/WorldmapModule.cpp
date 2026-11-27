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
#include <OgreCamera.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include "engine/entity/EntityManager.h"
#include "engine/entity/EntityModel.h"
#include "engine/graphics/CameraManager.h"
#include "engine/input/InputManager.h"
#include "engine/script/ScriptManager.h"
#include "engine/ui/DialogsManager.h"
#include "modules/worldmap/WorldmapConstants.h"
#include "modules/worldmap/WorldmapData.h"
#include "modules/worldmap/WorldmapTextures.h"

/**
 * World map module singleton.
 */
template<> VGears::WorldmapModule *Ogre::Singleton< VGears::WorldmapModule >::msSingleton = nullptr;

namespace VGears {

    using namespace Ogre;

    /** Distances (from the camera, 1 map block = 2 units) where the black fog of WM2 starts and is total. */
    static const float UNDERWATER_FOG_START = 2.0f;
    static const float UNDERWATER_FOG_END = 6.0f;

    WorldmapModule::WorldmapModule():
      player_entity_(nullptr),
      highwind_entity_(nullptr),
      buggy_entity_(nullptr),
      chocobo_entity_(nullptr),
      submarine_entity_(nullptr),
      tiny_bronco_entity_(nullptr),
      current_vehicle_(VehicleType::ON_FOOT),
      map_id_(-1),
      camera_yaw_(0),
      flight_active_(false),
      flight_entity_(nullptr),
      flight_heading_(0),
      flight_y_(0),
      player_height_offset_(0),
      active_function_mesh_index_(-1),
      active_function_id_(-1),
      initialized_(false),
      frame_listener_attached_(false),
      cancel_was_pressed_(false),
      confirm_was_pressed_(false),
      transition_stage_(TransitionStage::NONE),
      transition_dive_(true),
      transition_timer_(0),
      transition_start_y_(0),
      transition_target_y_(0),
      surface_requested_(false),
      surface_position_(Ogre::Vector3::ZERO),
      surface_heading_(0),
      dive_requested_(false),
      dive_position_(Ogre::Vector3::ZERO),
      dive_heading_(0),
      paused_for_battle_(false)
    {}

    WorldmapModule::~WorldmapModule(){
      if (frame_listener_attached_) Root::getSingleton().removeFrameListener(this);
    }

    bool WorldmapModule::frameStarted(const Ogre::FrameEvent& event){
        if (!initialized_ || player_entity_ == nullptr || paused_for_battle_) return true;
        const Ogre::Real delta_time = event.timeSinceLastFrame;
        AnimateWorldMapTextures(delta_time);

        // Diving or surfacing: the submarine moves by itself and the screen fades.
        if (transition_stage_ != TransitionStage::NONE){
            UpdateTransition(delta_time);
            if (initialized_ && player_entity_ != nullptr){
                UpdateVehicleAnimations(false);
                UpdateCamera(delta_time, false);
            }
            return true;
        }

        ::Entity* active_entity = EntityManager::getSingleton().ScriptGetPlayerEntity();
        if (!UpdateCurrentVehicle(active_entity)){
            // An entity of a scripted scene (the Warship): the camera follows it and nothing else happens.
            if (active_entity != nullptr) UpdateCamera(delta_time, false);
            return true;
        }
        UpdateMusic(delta_time);
        // The buttons can change the map: the entities are not valid after it.
        if (HandleActionButtons(active_entity)) return true;

        const bool highwind_active = active_entity == highwind_entity_;
        EntityModel* active_model = GetActiveModel(active_entity);

        // Flying vehicles (the Highwind, and the submarine of WM2) have their own controls.
        flight_active_ = highwind_active || (map_id_ == 1 && active_entity == player_entity_);
        if (flight_active_){
            UpdateFlight(active_model, delta_time, highwind_active);
            UpdateVehicleAnimations(false);
            UpdateCamera(delta_time, false);
            return true;
        }
        flight_entity_ = nullptr;

        UpdateCameraRotation(delta_time);
        const bool is_walking = UpdateWalking(active_entity, active_model, delta_time);

        // The submarine of WM2 has no walking animations.
        if (current_vehicle_ == VehicleType::ON_FOOT && map_id_ != 1){
            player_entity_->PlayAnimationContinue(is_walking ? "Walk" : "Idle");
        }

        UpdateVehicleAnimations(is_walking);
        UpdateCamera(delta_time, false);
        return true;
    }

    bool WorldmapModule::UpdateCurrentVehicle(const ::Entity* active_entity){
        if (active_entity == player_entity_) current_vehicle_ = VehicleType::ON_FOOT;
        else if (active_entity == chocobo_entity_) current_vehicle_ = VehicleType::CHOCOBO;
        else if (active_entity == buggy_entity_) current_vehicle_ = VehicleType::BUGGY;
        else if (active_entity == highwind_entity_) current_vehicle_ = VehicleType::HIGHWIND;
        else if (active_entity == submarine_entity_) current_vehicle_ = VehicleType::SUBMARINE;
        else if (active_entity == tiny_bronco_entity_) current_vehicle_ = VehicleType::TINY_BRONCO;
        else return false;
        return true;
    }

    void WorldmapModule::UpdateMusic(const Ogre::Real delta_time){
        // The music of the map (WorldMusic in world_music.lua): when the vehicle changes and every half second.
        music_timer_ += delta_time;
        if (music_timer_ < 0.5f && music_vehicle_ == current_vehicle_) return;
        music_timer_ = 0.0f;
        music_vehicle_ = current_vehicle_;
        ScriptManager::getSingleton().RunString(
          "if WorldMusic then WorldMusic.update(" + std::to_string(map_id_) + ", \""
          + GetVehicleScriptName(current_vehicle_) + "\") end"
        );
    }

    bool WorldmapModule::HandleActionButtons(::Entity* active_entity){
        const bool vehicle_active = current_vehicle_ != VehicleType::ON_FOOT;
        const bool cancel_pressed = InputManager::getSingleton().IsGameEventPressed("CANCEL");
        const bool cancel_edge = cancel_pressed && !cancel_was_pressed_;
        // The cancel button makes the submarine of WM2 go back to the surface.
        if (
          cancel_edge && map_id_ == 1 && active_entity == submarine_entity_
          && !EntityManager::getSingleton().IsPlayerLocked()
        ){
            cancel_was_pressed_ = true;
            if (SurfaceToOverworld(submarine_entity_)) return true;
        }
        if (vehicle_active && cancel_pressed && !cancel_was_pressed_){
            ScriptManager::getSingleton().ScriptRequest(
              ScriptManager::ENTITY, active_entity->GetName().c_str(), "unboard", 0
            );
        }
        cancel_was_pressed_ = cancel_pressed;
        // The action button places a pole in the snow map, and makes the submarine dive from WM0 to WM2.
        const bool confirm_pressed = InputManager::getSingleton().IsGameEventPressed("CONFIRM");
        const bool confirm_edge = confirm_pressed && !confirm_was_pressed_;
        confirm_was_pressed_ = confirm_pressed;
        if (
          confirm_edge && map_id_ == 2 && active_entity == player_entity_
          && !EntityManager::getSingleton().IsPlayerLocked()
        ){
            PlaceSnowPole();
        }
        if (
          confirm_edge && map_id_ == 0 && active_entity == submarine_entity_
          && !EntityManager::getSingleton().IsPlayerLocked()
        ){
            if (DiveToUnderwaterMap(submarine_entity_)) return true;
        }
        return false;
    }

    EntityModel* WorldmapModule::GetActiveModel(const ::Entity* active_entity) const{
        if (active_entity == player_entity_) return player_entity_;
        switch (current_vehicle_){
            case VehicleType::CHOCOBO: return chocobo_entity_;
            case VehicleType::BUGGY: return buggy_entity_;
            default: break;
        }
        if (active_entity == highwind_entity_) return highwind_entity_;
        if (current_vehicle_ == VehicleType::SUBMARINE) return submarine_entity_;
        return tiny_bronco_entity_;
    }

    void WorldmapModule::UpdateCameraRotation(const Ogre::Real delta_time){
        // The camera of the snow map (WM3) can't be moved by hand: it turns by itself.
        if (map_id_ == 2) camera_yaw_ += snow_.UpdateCamera(delta_time);
        else{
            // L1 and R1 rotate the camera around the active entity.
            InputManager& input = InputManager::getSingleton();
            if (input.IsGameEventPressed("L1")) camera_yaw_ -= CAMERA_ROTATION_SPEED * delta_time;
            if (input.IsGameEventPressed("R1")) camera_yaw_ += CAMERA_ROTATION_SPEED * delta_time;
        }
        if (camera_yaw_ > Ogre::Math::PI) camera_yaw_ -= Ogre::Math::TWO_PI;
        else if (camera_yaw_ < -Ogre::Math::PI) camera_yaw_ += Ogre::Math::TWO_PI;
    }

    bool WorldmapModule::UpdateWalking(
      ::Entity* active_entity, EntityModel* active_model, const Ogre::Real delta_time
    ){
        // Scripts lock the player during events and transitions.
        if (EntityManager::getSingleton().IsPlayerLocked()) return false;
        // The game events of the movement, so the keys of the controller settings are used.
        Ogre::Vector3 movement = Ogre::Vector3::ZERO;
        InputManager& input = InputManager::getSingleton();
        if (input.IsGameEventPressed("walk_left")) movement.x -= 1.0f;
        if (input.IsGameEventPressed("walk_right")) movement.x += 1.0f;
        if (input.IsGameEventPressed("walk_up")) movement.z -= 1.0f;
        if (input.IsGameEventPressed("walk_down")) movement.z += 1.0f;
        if (movement.isZeroLength()) return false;
        movement.normalise();
        // The movement is relative to the camera.
        movement = Ogre::Quaternion(Ogre::Radian(camera_yaw_), Ogre::Vector3::UNIT_Y) * movement;

        const bool vehicle_active = current_vehicle_ != VehicleType::ON_FOOT;
        const Ogre::Vector3 current_position = active_entity->GetPosition();
        const Ogre::Real speed = GetSpeed(
          GetVehicleScriptName(current_vehicle_), From16BitFixedPoint(8182) * 0.45f * (vehicle_active ? 2.0f : 1.0f)
        );
        const Ogre::Vector3 requested_position = current_position + movement * speed * delta_time;
        Ogre::Vector3 surface;
        size_t triangle_index = 0;
        if (!FindWalkableSurface(requested_position, surface, false, true, nullptr, &triangle_index)) return false;
        if (active_entity == highwind_entity_){
            Ogre::Vector3 current_surface;
            if (FindWalkableSurface(current_position, current_surface, false, false)){
                surface.y += current_position.y - current_surface.y;
            }
        }
        else if (active_entity == player_entity_) surface.y += player_height_offset_;
        if (EntityManager::getSingleton().IsWorldPositionBlocked(active_entity, surface)) return false;
        active_entity->SetPosition(surface);
        const TerrainTriangle& triangle = terrain_.GetTriangles()[triangle_index];
        EnterTriangle(triangle);
        // There are no encounters underwater.
        if (current_vehicle_ == VehicleType::ON_FOOT && map_id_ != 1) encounters_.Update(delta_time, triangle);
        const Ogre::Vector3 model_forward(0.0f, 0.0f, 1.0f);
        const Ogre::Vector3 move_direction(movement.x, 0.0f, movement.z);
        const Ogre::Quaternion heading = model_forward.getRotationTo(move_direction, Ogre::Vector3::UNIT_Y);
        active_model->GetModel()->getParentSceneNode()->getParentSceneNode()->setOrientation(heading);
        return true;
    }

      void WorldmapModule::UpdateCamera(Ogre::Real delta_time, bool snap_to_player){
        ::Entity* active_entity = EntityManager::getSingleton().ScriptGetPlayerEntity();
        if (active_entity == nullptr) return;
        const Ogre::Vector3 target
          = active_entity->GetPosition() + Ogre::Vector3(0, active_entity->GetHeight() * 0.5f, 0);
        // The camera of a flying vehicle stays behind it. L1 and R1 rotate it from there.
        const float yaw = camera_yaw_ + (flight_active_ ? flight_heading_ + Ogre::Math::PI : 0.0f);
        // The camera of the entity: distance and angle over the ground (the default one is the offset 1.5, 2.25).
        const char* camera_name = "walk";
        if (map_id_ == 1) camera_name = "submarine_underwater";
        else if (active_entity == chocobo_entity_) camera_name = "chocobo";
        else if (active_entity == buggy_entity_) camera_name = "buggy";
        else if (active_entity == highwind_entity_) camera_name = "highwind";
        else if (active_entity == submarine_entity_) camera_name = "submarine";
        else if (active_entity == tiny_bronco_entity_) camera_name = "tiny_bronco";
        else if (active_entity != player_entity_) camera_name = "scene";
        float distance = std::sqrt(1.5f * 1.5f + 2.25f * 2.25f);
        float pitch = std::atan2(1.5f, 2.25f);
        const auto camera_settings = camera_settings_.find(camera_name);
        if (camera_settings != camera_settings_.end()){
          distance = camera_settings->second.distance;
          pitch = Ogre::Math::DegreesToRadians(camera_settings->second.pitch_degrees);
        }
        const Ogre::Vector3 camera_offset
          = Ogre::Quaternion(Ogre::Radian(yaw), Ogre::Vector3::UNIT_Y)
            * Ogre::Vector3(0.0f, distance * std::sin(pitch), distance * std::cos(pitch));
        Ogre::Camera* camera = CameraManager::getSingleton().GetCurrentCamera();
        const Ogre::Vector3 desired_position = target + camera_offset;
        if (snap_to_player){
          camera->setPosition(desired_position);
        }
        else{
          const Ogre::Real follow_amount = std::min(1.0f, delta_time * 8.0f);
          camera->setPosition(
            camera->getPosition()
            + (desired_position - camera->getPosition()) * follow_amount
          );
        }
        camera->lookAt(target);
      }

    void WorldmapModule::PauseForBattle(){
        paused_for_battle_ = true;
        terrain_.SetVisible(false);
    }

    void WorldmapModule::SetEncountersLocked(const bool locked){encounters_.SetLocked(locked);}

    void WorldmapModule::ResumeAfterBattle(){
        paused_for_battle_ = false;
        encounters_.ResetTimer();
        // The battle had its own music: the one of the map starts again.
        music_timer_ = 1.0f;
        ScriptManager::getSingleton().RunString("if WorldMusic then WorldMusic.reset() end");
        terrain_.SetVisible(true);
        if (player_entity_ != nullptr) UpdateCamera(0, true);
    }

    /// Names of the entities of the overworld whose state is kept when the map is replaced.
    static const char* const OVERWORLD_ENTITY_NAMES[] = {
      "WorldMapCloud", "WorldMapChocobo", "WorldMapHighwind", "WorldMapBuggy",
      "WorldMapSubmarine", "WorldMapTinyBronco"
    };

    void WorldmapModule::SaveOverworldSnapshot(){
      overworld_snapshot_.clear();
      for (const char* name : OVERWORLD_ENTITY_NAMES){
        ::Entity* entity = EntityManager::getSingleton().GetEntity(name);
        if (entity == nullptr) continue;
        EntityState state;
        state.name = name;
        state.position = entity->GetPosition();
        state.orientation = static_cast<EntityModel*>(entity)->GetModel()->getParentSceneNode()
          ->getParentSceneNode()->getOrientation();
        state.visible = entity->IsVisible();
        state.solid = entity->IsSolid();
        state.talkable = entity->IsTalkable();
        overworld_snapshot_.push_back(state);
      }
    }

    void WorldmapModule::RestoreOverworldSnapshot(){
      for (const EntityState& state : overworld_snapshot_){
        ::Entity* entity = EntityManager::getSingleton().GetEntity(state.name);
        if (entity == nullptr) continue;
        entity->SetPosition(state.position);
        static_cast<EntityModel*>(entity)->GetModel()->getParentSceneNode()->getParentSceneNode()
          ->setOrientation(state.orientation);
        entity->SetVisible(state.visible);
        entity->SetSolid(state.solid);
        entity->SetTalkable(state.talkable);
      }
      overworld_snapshot_.clear();
    }

    void WorldmapModule::DestroyWorldMap(){
      // The overworld is built again when the submarine surfaces: keep where everything was.
      if (map_id_ == 0) SaveOverworldSnapshot();
      Suspend();
      terrain_.Clear();
      encounters_.Clear();
      // The entities of the old map (player, vehicles, models and scripts).
      EntityManager::getSingleton().ClearWorld();
      initialized_ = false;
      map_id_ = -1;
    }

    void WorldmapModule::Enter(const int map_id){
      if (initialized_) DestroyWorldMap();
      Init(map_id);
    }

    void WorldmapModule::Init(const int map_id){
      if (map_id < 0 || map_id >= WORLD_MAP_COUNT){
        LogManager::getSingleton().logMessage(
          "Invalid world map " + std::to_string(map_id) + ". Valid maps are 0 to "
            + std::to_string(WORLD_MAP_COUNT - 1) + "."
        );
        return;
      }
      if (initialized_ && map_id != map_id_){
        DestroyWorldMap();
      }
      if (initialized_){
        if (frame_listener_attached_) return;
        EntityManager::getSingleton().SetWorldModule();
        DialogsManager::getSingleton().SetWorldModule();
        terrain_.SetVisible(true);
        // The music starts again (a field or a battle had its own).
        music_timer_ = 1.0f;
        ScriptManager::getSingleton().RunString("if WorldMusic then WorldMusic.reset() end");
        CreatePlayer();
        Root::getSingleton().addFrameListener(this);
        frame_listener_attached_ = true;
        return;
      }
      map_id_ = map_id;
      camera_yaw_ = 0;
      EntityManager::getSingleton().SetWorldModule();
      DialogsManager::getSingleton().SetWorldModule();
      terrain_.Load(Root::getSingleton().getSceneManager("Scene"), map_id_);
      const Ogre::Vector3 center = terrain_.GetCenter();
      Ogre::Camera* camera = CameraManager::getSingleton().GetCurrentCamera();
      camera->setPosition(Ogre::Vector3(center.x, 80.0f, center.z + 90.0f));
      camera->lookAt(Ogre::Vector3(center.x, 0.0f, center.z));
      // Underwater (WM2): everything fades to black with the distance to the camera.
      if (map_id_ == 1){
        Root::getSingleton().getSceneManager("Scene")->setFog(
          Ogre::FOG_LINEAR, Ogre::ColourValue::Black, 0.0f, UNDERWATER_FOG_START, UNDERWATER_FOG_END
        );
      }
      else Root::getSingleton().getSceneManager("Scene")->setFog(Ogre::FOG_NONE);
      // Snow map (WM3): it always snows.
      snow_.SetVisible(map_id_ == 2);
      // The music of the map starts in the first frame.
      music_timer_ = 1.0f;
      ScriptManager::getSingleton().RunString("if WorldMusic then WorldMusic.reset() end");
      // The button that was used to get here (a menu, a field) must not count as the first pole.
      confirm_was_pressed_ = InputManager::getSingleton().IsGameEventPressed("CONFIRM");
      snow_.Reset();
      ScriptManager::getSingleton().RunFile(
        "game/world/scripts/wm" + std::to_string(WORLD_MAP_FILE_NUMBERS[map_id_]) + ".lua"
      );
      // The debug spawn points, if debug_data.lua has them.
      spawn_overrides_.clear();
      hidden_vehicles_.clear();
      ScriptManager::getSingleton().RunString(
        "if WorldMapDebug and WorldMapDebug.spawn then "
        "for name, point in pairs(WorldMapDebug.spawn) do "
        "world_map_module:set_spawn(name, point[1], point[2]) end end"
      );
      // The positions that the scripts give to the vehicles, for the story (debug ones have priority).
      ScriptManager::getSingleton().RunString(
        "if WorldVM then WorldVM.prepare(" + std::to_string(map_id_) + ") end"
      );
      CreatePlayer();
      // The models that the scripts place (scenery).
      ScriptManager::getSingleton().RunString(
        "if WorldVM then WorldVM.place_models(" + std::to_string(map_id_) + ") end"
      );
      encounters_.Load();
      initialized_ = true;
      Root::getSingleton().addFrameListener(this);
      frame_listener_attached_ = true;
    }

    void WorldmapModule::Suspend(){
      if (!frame_listener_attached_) return;
      Root::getSingleton().removeFrameListener(this);
      frame_listener_attached_ = false;
      terrain_.SetVisible(false);
      snow_.SetVisible(false);
      player_entity_ = nullptr;
      highwind_entity_ = nullptr;
      buggy_entity_ = nullptr;
      chocobo_entity_ = nullptr;
      submarine_entity_ = nullptr;
      tiny_bronco_entity_ = nullptr;
      current_vehicle_ = VehicleType::ON_FOOT;
      active_vehicle_terrain_types_.clear();
      active_function_mesh_index_ = -1;
      active_function_id_ = -1;
      flight_active_ = false;
      flight_entity_ = nullptr;
      cancel_was_pressed_ = false;
      confirm_was_pressed_ = false;
    }

} // namespace VGears
