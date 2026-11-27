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
#include <OgreVector3.h>
#include "engine/entity/EntityManager.h"
#include "engine/script/ScriptManager.h"
#include "modules/worldmap/WorldmapConstants.h"
#include "modules/worldmap/WorldmapData.h"

namespace VGears {

    /**
     * Width in meshes of the grid that the world map scripts use to index meshes (mesh_y * 36 +
     * mesh_x). It is the same in every map, regardless of the size of the map.
     */
    static const int WORLD_MAP_SCRIPT_MESH_STRIDE = 36;

    bool WorldmapModule::FindWalkableSurface(
      const Ogre::Vector3& position, Ogre::Vector3& surface, bool grass_only,
      bool require_walkable, uint8* walkability_type, size_t* triangle_index
    ) const{
        WorldmapTerrain::WalkableCheck check;
        if (require_walkable){
            check = [this](const TerrainTriangle& triangle, const Ogre::Real height){
                // Underwater: every terrain type is allowed, but there is a height limit.
                if (map_id_ == 1){
                    return !UNDERWATER_HEIGHT_LIMIT_ENABLED
                      || height + FLIGHT_MIN_CLEARANCE <= UNDERWATER_MAX_HEIGHT;
                }
                return IsActiveTerrainAllowed(triangle.walkability_type);
            };
        }
        return terrain_.FindSurface(position, surface, grass_only, check, walkability_type, triangle_index);
    }

    void WorldmapModule::EnterTriangle(const TerrainTriangle& triangle){
        const int mesh_index = triangle.mesh_y * WORLD_MAP_SCRIPT_MESH_STRIDE + triangle.mesh_x;
        if (triangle.function_id >= 3){
            if (mesh_index != active_function_mesh_index_
              || triangle.function_id != active_function_id_){
                TriggerTriangleFunction(triangle);
            }
            active_function_mesh_index_ = mesh_index;
            active_function_id_ = triangle.function_id;
        }
        else{
            active_function_mesh_index_ = -1;
            active_function_id_ = -1;
        }
    }

    void WorldmapModule::TriggerTriangleFunction(const TerrainTriangle& triangle){
        ScriptEntity* world_map_script
          = ScriptManager::getSingleton().GetScriptEntityByName(
              ScriptManager::SYSTEM, "WorldMap"
            );
        if (world_map_script == nullptr) return;

        const int mesh_index = triangle.mesh_y * WORLD_MAP_SCRIPT_MESH_STRIDE + triangle.mesh_x;
        // What the function can know about who entered the triangle (see WorldVM.can_enter).
        ScriptManager::getSingleton().RunString(
          std::string("System.WorldMap.vehicle = \"") + GetVehicleScriptName(current_vehicle_)
          + "\"; System.WorldMap.landing = " + (landing_trigger_ ? "true" : "false")
        );
        ScriptManager::getSingleton().ScriptRequest(
          world_map_script, "on_triangle_enter", 1,
          std::to_string(mesh_index), std::to_string(triangle.function_id - 3), false, false
        );
    }

      Ogre::String WorldmapModule::GetTerrainTypeUnderEntity(const char* entity_name) const{
        if (entity_name == nullptr) return "";
        const ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return "";

        Ogre::Vector3 surface;
        uint8 walkability_type = 0;
        if (!FindWalkableSurface(entity->GetPosition(), surface, false, false, &walkability_type)){
          return "";
        }
        return GetWorldMapTerrainName(walkability_type);
      }

      int WorldmapModule::GetTerrainTriangleUnderEntity(const char* entity_name) const{
        if (entity_name == nullptr) return -1;
        const ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return -1;

        Ogre::Vector3 surface;
        size_t triangle_index = 0;
        if (!FindWalkableSurface(entity->GetPosition(), surface, false, false, nullptr, &triangle_index)){
          return -1;
        }
        return static_cast<int>(triangle_index);
      }

} // namespace VGears
