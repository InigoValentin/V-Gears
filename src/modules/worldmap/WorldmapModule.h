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

#pragma once

#include <OgreFrameListener.h>
#include <OgreSingleton.h>
#include <OgreVector3.h>
#include <map>
#include <set>
#include <vector>
#include "common/TypeDefine.h"

class EntityModel;

namespace VGears {

    /**
     * The world map module.
     */
    class WorldmapModule : public Ogre::Singleton<WorldmapModule>, public Ogre::FrameListener{

        public:

            enum class VehicleType{
                ON_FOOT,
                CHOCOBO,
                BUGGY,
                HIGHWIND,
                SUBMARINE,
                TINY_BRONCO
            };

            WorldmapModule();

            virtual ~WorldmapModule();

            /**
             * Initializes the world map module
             *
             * @param[in] map_id 0 is the overworld (WM0), 1 is WM2 and 2 is WM3. If another map
             * was already loaded, it is replaced.
             */
            void Init(const int map_id = 0);
            void Suspend();

            bool frameStarted(const Ogre::FrameEvent& event) override;

            VehicleType GetCurrentVehicle() const;
            void ClearVehicleTerrains();
            void AddVehicleTerrain(const char* terrain_name);
            Ogre::String GetTerrainTypeUnderEntity(const char* entity_name) const;
            int GetTerrainTriangleUnderEntity(const char* entity_name) const;
            bool UnboardVehicle(const char* vehicle_name);

            /**
             * Hides the world map and stops it while a battle is on.
             */
            void PauseForBattle();

            /**
             * Shows the world map again after a battle.
             */
            void ResumeAfterBattle();

            /**
             * Prevents or allows random encounters (the SetBattleLock opcode).
             *
             * @param[in] locked True to prevent them.
             */
            void SetEncountersLocked(const bool locked);

            /**
             * Sets the orientation of the model of a world map entity, in degrees. It's the
             * orientation of the model itself, so the heading of the entity is added to it. It is
             * for finding the right orientation of a model.
             *
             * @param[in] entity_name Name of the entity, like "WorldMapHighwind".
             * @param[in] x Rotation in X.
             * @param[in] y Rotation in Y.
             * @param[in] z Rotation in Z.
             */
            /**
             * Moves the model of a world map entity up or down, in world units, without moving
             * the entity. It is for finding the right height of a model.
             *
             * @param[in] entity_name Name of the entity, like "WorldMapSubmarine".
             * @param[in] y Height of the model. Negative is down, 0 is the position of the entity.
             */
            void SetModelOffset(const char* entity_name, const float y);

            /**
             * Puts a world map entity at a fixed height of the world, keeping its x and z.
             *
             * @param[in] entity_name Name of the entity, like "WorldMapemerald_30".
             * @param[in] y Height in world units, not relative to the ground.
             */
            void SetEntityAltitude(const char* entity_name, const float y);

            /**
             * Sets where an entity of the overworld appears when the map is loaded, instead of its
             * default spawn point. It is used by the debug data, see debug_data.lua.
             *
             * @param[in] entity_name Name of the entity: "Cloud", "Chocobo", "Highwind", "Buggy",
             * "Submarine" or "TinyBronco".
             * @param[in] x Position in X, in world units.
             * @param[in] z Position in Z, in world units.
             */
            void SetSpawn(const char* entity_name, const float x, const float z);

            /**
             * Hides a vehicle of the overworld that the story hasn't given yet: it is not visible,
             * solid nor talkable, so it can't be boarded. Has to be called before the map is created.
             *
             * @param[in] entity_name Name of the vehicle: "Chocobo", "Highwind", "Buggy",
             * "Submarine" or "TinyBronco".
             */
            void SetVehicleHidden(const char* entity_name);

            /**
             * Enters a world map from a field: the map is always built again (its entities, scripts and
             * models are for the story of now), unlike Init, which keeps the one that was suspended.
             *
             * @param[in] map_id Map to enter, 0 WM0, 1 WM2, 2 WM3.
             */
            void Enter(const int map_id);

            /**
             * Creates a model of the world that the scripts place (scenery like the rocket or the
             * Gold Saucer), at a position of the world. The model is not solid.
             *
             * @param[in] name Name of the entity, without the prefix "WorldMap".
             * @param[in] hrc Name of the HRC file of the model, like "cfc".
             * @param[in] x Position in X, in world units.
             * @param[in] z Position in Z, in world units.
             * @param[in] raw_scale Scale that the game stores with the model: 0x200 is 1.
             * @param[in] direction Direction that the script gives, from 0 to 255 for a full turn.
             * @param[in] rotation_x Rotation of the model itself in X, in degrees.
             * @param[in] rotation_y Rotation of the model itself in Y, in degrees.
             * @param[in] rotation_z Rotation of the model itself in Z, in degrees.
             */
            void AddWorldModelFromScript(
              const char* name, const char* hrc, const float x, const float z, const int raw_scale,
              const int direction, const float rotation_x, const float rotation_y,
              const float rotation_z
            );

            void SetModelOrientation(const char* entity_name, const float x, const float y, const float z);

        private:

            struct TerrainTriangle{
                Ogre::Vector3 vertices[3];
                uint8 walkability_type;
                uint8 function_id;
                int mesh_x;
                int mesh_y;
                /// Encounter region of the triangle, 0-15.
                uint8 region;
            };

            /**
             * Random encounters of a region and terrain group, from ENC_W.BIN.
             */
            struct EncounterRecord{
                bool enabled;
                /// Lower values mean more frequent encounters.
                int rate;
                /// Walkability type that selects this record.
                int terrain;
                /// Words 1-15 of the record: battle ID (10 bits) and probability (6 bits).
                uint16 words[15];
            };

            void DestroyWorldMap();
            /**
             * State of an entity of the overworld, kept while another map is loaded.
             */
            struct EntityState{
                Ogre::String name;
                Ogre::Vector3 position;
                Ogre::Quaternion orientation;
                bool visible;
                bool solid;
                bool talkable;
            };

            /**
             * Saves the state of the entities of the overworld, to restore it when it is loaded
             * again.
             */
            void SaveOverworldSnapshot();

            /**
             * Puts the entities of the overworld where they were when the snapshot was saved.
             */
            void RestoreOverworldSnapshot();

            /**
             * Gets the spawn point of an entity: the one that the debug data sets, or the default.
             */
            Ogre::Vector3 GetSpawn(
              const Ogre::String& entity_name, const Ogre::Vector3& default_spawn
            ) const;

            /// Spawn points that replace the default ones.
            std::map<Ogre::String, Ogre::Vector2> spawn_overrides_;

            /// Vehicles that the story hasn't given.
            std::set<Ogre::String> hidden_vehicles_;

            void CreateWorldMap(Ogre::SceneManager* scene_manager);
            void CreatePlayer();
                        void CreateCompanionModels();
                        void AddWorldModel(
                            const Ogre::String& name, const Ogre::String& mesh_name,
                            const Ogre::Vector3& desired_position, Ogre::Real target_size,
                            const Ogre::Quaternion& orientation,
                            Ogre::Real collision_radius_scale = 1.0f,
                            bool talkable = false,
                            Ogre::Real explicit_scale = 0.0f
                        );
            void UpdateCamera(Ogre::Real delta_time, bool snap_to_player);
            bool FindWalkableSurface(
                            const Ogre::Vector3& position, Ogre::Vector3& surface,
                            bool grass_only = false, bool require_walkable = true,
                            uint8* walkability_type = nullptr,
                            size_t* triangle_index = nullptr
            ) const;
            bool IsActiveTerrainAllowed(uint8 walkability_type) const;
            /**
             * Runs the function of a triangle when it is entered, if it changed.
             */
            void EnterTriangle(const TerrainTriangle& triangle);

            /**
             * Plays the animation that each vehicle has in its state: parked, boarded, or boarded
             * and moving.
             *
             * @param[in] moving True if the active entity is moving.
             */
            void UpdateVehicleAnimations(const bool moving);

            /**
             * Takes the submarine from WM0 to WM2, if it is over deep water inside the underwater
             * map. It keeps the position and the heading.
             *
             * @param[in] submarine The submarine. The entities are replaced if it dives.
             * @return True if it dived.
             */
            bool DiveToUnderwaterMap(EntityModel* submarine);

            /**
             * Takes the submarine from WM2 back to WM0, if the same position is deep water in
             * WM0. It keeps the position and the heading.
             *
             * @param[in] submarine The submarine. The entities are replaced if it surfaces.
             * @return True if it surfaced.
             */
            bool SurfaceToOverworld(EntityModel* submarine);

            /**
             * Starts the animation of a dive or a surfacing: the submarine goes under the surface
             * (or up), the screen fades to black, the map changes, and the screen fades back while
             * the submarine arrives. The request of the dive or the surfacing has to be set.
             *
             * @param[in] dive True for a dive from WM0 to WM2, false for a surfacing.
             * @param[in] submarine The submarine.
             */
            void BeginTransition(const bool dive, EntityModel* submarine);

            /**
             * Moves the animation of the dive or the surfacing forward. It changes the map in the
             * middle of it.
             *
             * @param[in] delta_time Seconds since the last frame.
             */
            void UpdateTransition(const Ogre::Real delta_time);

            /**
             * Ends the animation: the screen is shown and the player can move again.
             */
            void EndTransition();

            /**
             * Puts the submarine at the position of a surfacing and makes it the one in control.
             */
            void PlaceSurfacedSubmarine();

            /**
             * Moves a flying vehicle: forward with the action button, left and right turn, up and
             * down change the height.
             *
             * @param[in] entity The vehicle.
             * @param[in] delta_time Seconds since the last frame.
             * @param[in] highwind True for the Highwind, false for the submarine of WM2.
             */
            void UpdateFlight(EntityModel* entity, const Ogre::Real delta_time, const bool highwind);

            void TriggerTriangleFunction(const TerrainTriangle& triangle);
            void LoadEncounters();
            void UpdateEncounters(const float delta_time, const TerrainTriangle& triangle);
            int RollEncounterBattle(const EncounterRecord& record) const;

            std::vector<EncounterRecord> encounter_records_;
            float encounter_frames_;
            int encounter_accumulator_;
            int encounter_last_battle_;
            bool encounters_locked_;
            bool paused_for_battle_;

            std::vector<TerrainTriangle> terrain_triangles_;
            Ogre::SceneManager* scene_manager_;
            Ogre::SceneNode* world_map_node_;
            EntityModel* player_entity_;
            EntityModel* highwind_entity_;
            EntityModel* buggy_entity_;
            EntityModel* chocobo_entity_;
            EntityModel* submarine_entity_;
            EntityModel* tiny_bronco_entity_;
            VehicleType current_vehicle_;
            std::vector<uint8> active_vehicle_terrain_types_;
            Ogre::Real player_height_offset_;
            int active_function_mesh_index_;
            int active_function_id_;
            /// Loaded map ID, -1 if none.
            int map_id_;
            /// Size of the map in blocks.
            size_t grid_width_;
            size_t grid_height_;
            /// Rotation of the camera around the active entity, in radians. Positive is to the right.
            float camera_yaw_;
            /// True while the active entity is a flying vehicle.
            bool flight_active_;
            /// Flying vehicle that the flight state belongs to.
            EntityModel* flight_entity_;
            /// Rotation of the vehicle around the vertical axis, in radians.
            float flight_heading_;
            /// Height of the vehicle.
            float flight_y_;
            /// Position of the first block of the map in the 9x7 block grid of the world.
            size_t origin_x_;
            size_t origin_y_;
            bool initialized_;
            bool frame_listener_attached_;
            bool cancel_was_pressed_;
            bool confirm_was_pressed_;
            enum class TransitionStage{
                /// No animation going on.
                NONE,
                /// The submarine moves away, in the map where it was.
                LEAVING,
                /// The screen goes black.
                FADING_OUT,
                /// The submarine comes into the new map and the screen is shown again.
                ARRIVING
            };
            TransitionStage transition_stage_;
            /// True for a dive, false for a surfacing.
            bool transition_dive_;
            float transition_timer_;
            /// Heights where the submarine is moving between in the current stage.
            float transition_start_y_;
            float transition_target_y_;
            /// State of the entities of WM0 while WM2 is loaded.
            std::vector<EntityState> overworld_snapshot_;
            /// A surfacing to WM0 is going on: the submarine appears at the position and heading
            /// below.
            bool surface_requested_;
            Ogre::Vector3 surface_position_;
            float surface_heading_;
            /// A dive to WM2 is going on: the submarine appears at the position and heading below.
            bool dive_requested_;
            Ogre::Vector3 dive_position_;
            float dive_heading_;
    };

}
