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
             */
            void Init();
            void Suspend();

            bool frameStarted(const Ogre::FrameEvent& event) override;

            VehicleType GetCurrentVehicle() const;
            void ClearVehicleTerrains();
            void AddVehicleTerrain(const char* terrain_name);
            Ogre::String GetTerrainTypeUnderEntity(const char* entity_name) const;
            int GetTerrainTriangleUnderEntity(const char* entity_name) const;
            bool UnboardVehicle(const char* vehicle_name);

        private:

            struct TerrainTriangle{
                Ogre::Vector3 vertices[3];
                uint8 walkability_type;
                uint8 function_id;
                int mesh_x;
                int mesh_y;
            };

            void CreateWorldMap(Ogre::SceneManager* scene_manager);
            void CreatePlayer();
                        void CreateCompanionModels();
                        void AddWorldModel(
                            const Ogre::String& name, const Ogre::String& mesh_name,
                            const Ogre::Vector3& desired_position, Ogre::Real target_size,
                            const Ogre::Quaternion& orientation,
                            Ogre::Real collision_radius_scale = 1.0f,
                            bool talkable = false
                        );
            void UpdateCamera(Ogre::Real delta_time, bool snap_to_player);
            bool FindWalkableSurface(
                            const Ogre::Vector3& position, Ogre::Vector3& surface,
                            bool grass_only = false, bool require_walkable = true,
                            uint8* walkability_type = nullptr,
                            size_t* triangle_index = nullptr
            ) const;
            bool IsActiveTerrainAllowed(uint8 walkability_type) const;
            void TriggerTriangleFunction(const TerrainTriangle& triangle);

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
            bool initialized_;
            bool frame_listener_attached_;
            bool cancel_was_pressed_;
    };

}
