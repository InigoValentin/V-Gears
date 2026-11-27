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
#include <map>
#include <set>
#include <OgreException.h>
#include <OgreLogManager.h>
#include <OgreManualObject.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include "engine/entity/EntityManager.h"
#include "engine/graphics/CameraManager.h"
#include "engine/script/ScriptManager.h"
#include "modules/worldmap/WorldmapConstants.h"
#include "modules/worldmap/WorldmapData.h"
#include "modules/worldmap/WorldmapTextures.h"

namespace VGears {

    using namespace Ogre;

    /**
     * Width in meshes of the grid that the world map scripts use to index meshes (mesh_y * 36 +
     * mesh_x). It is the same in every map, regardless of the size of the map.
     */
    static const int WORLD_MAP_SCRIPT_MESH_STRIDE = 36;

    void WorldmapModule::CreateWorldMap(SceneManager* scene_manager){
      scene_manager_ = scene_manager;
      // The node is kept when a map is replaced.
      if (world_map_node_ == nullptr){
        world_map_node_ = scene_manager_->getRootSceneNode()
          ->createChildSceneNode("WorldMapTerrain");
      }
      world_map_node_->setVisible(true);
        std::vector<VGears::WorldmapFileSerializer::Block> blocks
          = LoadWorldMap("WM" + std::to_string(WORLD_MAP_FILE_NUMBERS[map_id_]) + ".MAP");
        if (GetTextureIdOffset(map_id_) != 0){
          for (auto& block : blocks) for (auto& mesh : block.meshes) for (auto& triangle : mesh.triangles){
            triangle.TextureInfo += GetTextureIdOffset(map_id_);
          }
        }
        const size_t texture_count = WORLD_MAP_TEXTURE_COUNT;
        // WM0 has extra blocks after the 9x7 grid: the alternate shapes of the story progress.
        const size_t base_block_count = map_id_ == 0
          ? std::min(blocks.size(), size_t(63)) : blocks.size();
        const float space = From16BitFixedPoint(8182);
        std::map<uint16, ManualObject*> texture_objects;
        std::set<uint16> unavailable_texture_ids;
        unsigned int manual_index = 0;

        for (size_t block_index = 0; block_index < base_block_count; ++block_index){
            const VGears::WorldmapFileSerializer::Block& block = blocks[block_index];
            const size_t map_x = origin_x_ + block_index % grid_width_;
            const size_t map_y = origin_y_ + block_index / grid_width_;
            for (size_t mesh_index = 0; mesh_index < block.meshes.size(); ++mesh_index){
                const VGears::WorldmapFileSerializer::BlockPart& part
                  = block.meshes[mesh_index];
                const size_t mesh_x = mesh_index % 4;
                const size_t mesh_y = mesh_index / 4;
                const float pos_x = (mesh_x * space) + (map_x * space * 4);
                const float pos_z = (mesh_y * space) + (map_y * space * 4);

                for (const VGears::WorldmapFileSerializer::BlockTriangle& triangle
                  : part.triangles){
                    const VGears::WorldmapFileSerializer::Vertex& vertex_0
                      = part.vertices.at(triangle.vertex_0_index);
                    const VGears::WorldmapFileSerializer::Vertex& vertex_1
                      = part.vertices.at(triangle.vertex_1_index);
                    const VGears::WorldmapFileSerializer::Vertex& vertex_2
                      = part.vertices.at(triangle.vertex_2_index);
                    TerrainTriangle terrain_triangle;
                    terrain_triangle.vertices[0] = Ogre::Vector3(
                      From16BitFixedPoint(vertex_0.x) + pos_x,
                      From16BitFixedPoint(vertex_0.y),
                      From16BitFixedPoint(vertex_0.z) + pos_z
                    );
                    terrain_triangle.vertices[1] = Ogre::Vector3(
                      From16BitFixedPoint(vertex_1.x) + pos_x,
                      From16BitFixedPoint(vertex_1.y),
                      From16BitFixedPoint(vertex_1.z) + pos_z
                    );
                    terrain_triangle.vertices[2] = Ogre::Vector3(
                      From16BitFixedPoint(vertex_2.x) + pos_x,
                      From16BitFixedPoint(vertex_2.y),
                      From16BitFixedPoint(vertex_2.z) + pos_z
                    );
                    terrain_triangle.walkability_type = triangle.walkability_info_;
                    terrain_triangle.function_id = triangle.function_id;
                    terrain_triangle.mesh_x = static_cast<int>(map_x * 4 + mesh_x);
                    terrain_triangle.mesh_y = static_cast<int>(map_y * 4 + mesh_y);
                    // The 5 lower bits of the location are the region of the random encounters.
                    terrain_triangle.region = std::min<int>(triangle.Location & 0x1F, 15);
                    terrain_triangles_.push_back(terrain_triangle);

                    if (unavailable_texture_ids.find(triangle.TextureInfo)
                      != unavailable_texture_ids.end()){
                        continue;
                    }
                    if (triangle.TextureInfo >= texture_count){
                        if (unavailable_texture_ids.insert(triangle.TextureInfo).second){
                            LogManager::getSingleton().logMessage(
                              "Skipping world-map texture ID "
                                + std::to_string(triangle.TextureInfo)
                                + ": no PC texture mapping is available."
                            );
                        }
                        continue;
                    }

                    ManualObject* manual = nullptr;
                    auto object = texture_objects.find(triangle.TextureInfo);
                    if (object == texture_objects.end()){
                        MaterialPtr material;
                        try{
                            material = LoadTextureMaterial(triangle.TextureInfo);
                        }
                        catch (const Ogre::Exception& exception){
                            unavailable_texture_ids.insert(triangle.TextureInfo);
                            LogManager::getSingleton().logMessage(
                              "Failed to load world-map texture "
                                + std::string(WORLD_MAP_TEXTURES[triangle.TextureInfo].name)
                                + ": " + exception.getDescription()
                            );
                            continue;
                        }

                        manual = scene_manager->createManualObject(
                          "world_map_surface_" + std::to_string(++manual_index)
                        );
                        manual->begin(material->getName(), RenderOperation::OT_TRIANGLE_LIST);
                        manual->setDebugDisplayEnabled(false);
                        manual->setDynamic(false);
                        texture_objects[triangle.TextureInfo] = manual;
                    }
                    else manual = object->second;

                    manual->position(
                      terrain_triangle.vertices[2].x,
                      terrain_triangle.vertices[2].y,
                      terrain_triangle.vertices[2].z
                    );
                    manual->textureCoord(
                      ToTextureCoordU(triangle.vertex_2_u, triangle.TextureInfo),
                      ToTextureCoordV(triangle.vertex_2_v, triangle.TextureInfo)
                    );

                    manual->position(
                      terrain_triangle.vertices[1].x,
                      terrain_triangle.vertices[1].y,
                      terrain_triangle.vertices[1].z
                    );
                    manual->textureCoord(
                      ToTextureCoordU(triangle.vertex_1_u, triangle.TextureInfo),
                      ToTextureCoordV(triangle.vertex_1_v, triangle.TextureInfo)
                    );

                    manual->position(
                      terrain_triangle.vertices[0].x,
                      terrain_triangle.vertices[0].y,
                      terrain_triangle.vertices[0].z
                    );
                    manual->textureCoord(
                      ToTextureCoordU(triangle.vertex_0_u, triangle.TextureInfo),
                      ToTextureCoordV(triangle.vertex_0_v, triangle.TextureInfo)
                    );
                }
            }
        }

        for (const auto& texture_object : texture_objects){
            texture_object.second->end();
            world_map_node_->attachObject(texture_object.second);
        }

        BuildTerrainIndex();

        const float map_center_x = space * (origin_x_ * 4.0f + grid_width_ * 2.0f);
        const float map_center_z = space * (origin_y_ * 4.0f + grid_height_ * 2.0f);
        Ogre::Camera* camera = CameraManager::getSingleton().GetCurrentCamera();
        camera->setPosition(Ogre::Vector3(map_center_x, 80.0f, map_center_z + 90.0f));
        camera->lookAt(Ogre::Vector3(map_center_x, 0.0f, map_center_z));
    }

    void WorldmapModule::BuildTerrainIndex(){
        terrain_grid_.clear();
        terrain_grid_columns_ = 0;
        terrain_grid_rows_ = 0;
        if (terrain_triangles_.empty()) return;
        float min_x = std::numeric_limits<float>::max(), min_z = min_x;
        float max_x = -min_x, max_z = -min_x;
        for (const TerrainTriangle& triangle : terrain_triangles_){
            for (const Ogre::Vector3& vertex : triangle.vertices){
                min_x = std::min(min_x, vertex.x); max_x = std::max(max_x, vertex.x);
                min_z = std::min(min_z, vertex.z); max_z = std::max(max_z, vertex.z);
            }
        }
        // A cell is as big as a mesh of the map (a triangle almost always touches only one).
        terrain_grid_cell_ = From16BitFixedPoint(8182);
        terrain_grid_min_x_ = min_x;
        terrain_grid_min_z_ = min_z;
        terrain_grid_columns_ = static_cast<int>((max_x - min_x) / terrain_grid_cell_) + 1;
        terrain_grid_rows_ = static_cast<int>((max_z - min_z) / terrain_grid_cell_) + 1;
        terrain_grid_.resize(static_cast<size_t>(terrain_grid_columns_) * terrain_grid_rows_);
        // The tolerance of the search (a point a little outside of the edge still counts).
        const float margin = 0.01f;
        for (size_t i = 0; i < terrain_triangles_.size(); ++i){
            const TerrainTriangle& triangle = terrain_triangles_[i];
            float t_min_x = triangle.vertices[0].x, t_max_x = t_min_x;
            float t_min_z = triangle.vertices[0].z, t_max_z = t_min_z;
            for (const Ogre::Vector3& vertex : triangle.vertices){
                t_min_x = std::min(t_min_x, vertex.x); t_max_x = std::max(t_max_x, vertex.x);
                t_min_z = std::min(t_min_z, vertex.z); t_max_z = std::max(t_max_z, vertex.z);
            }
            const int column_0 = std::max(0, static_cast<int>(std::floor((t_min_x - margin - min_x) / terrain_grid_cell_)));
            const int column_1 = std::min(terrain_grid_columns_ - 1, static_cast<int>(std::floor((t_max_x + margin - min_x) / terrain_grid_cell_)));
            const int row_0 = std::max(0, static_cast<int>(std::floor((t_min_z - margin - min_z) / terrain_grid_cell_)));
            const int row_1 = std::min(terrain_grid_rows_ - 1, static_cast<int>(std::floor((t_max_z + margin - min_z) / terrain_grid_cell_)));
            for (int row = row_0; row <= row_1; ++row){
                for (int column = column_0; column <= column_1; ++column){
                    terrain_grid_[static_cast<size_t>(row) * terrain_grid_columns_ + column].push_back(static_cast<uint32>(i));
                }
            }
        }
        size_t entries = 0;
        for (const auto& cell : terrain_grid_) entries += cell.size();
        LogManager::getSingleton().logMessage(
          "World map terrain index: " + std::to_string(terrain_triangles_.size()) + " triangles in "
          + std::to_string(terrain_grid_columns_) + "x" + std::to_string(terrain_grid_rows_) + " cells, "
          + std::to_string(entries) + " entries"
        );
    }

    bool WorldmapModule::FindWalkableSurface(
      const Ogre::Vector3& position, Ogre::Vector3& surface, bool grass_only,
      bool require_walkable, uint8* walkability_type, size_t* triangle_index, bool use_index
    ) const{
        bool found = false;
        Ogre::Real closest_height_distance = std::numeric_limits<Ogre::Real>::max();
        // Only the triangles of the cell of the position (the index is in ascending order, so the result
        // is the same as with all of them).
        const std::vector<uint32>* cell_triangles = nullptr;
        if (use_index && !terrain_grid_.empty()){
            const int column = static_cast<int>(std::floor((position.x - terrain_grid_min_x_) / terrain_grid_cell_));
            const int row = static_cast<int>(std::floor((position.z - terrain_grid_min_z_) / terrain_grid_cell_));
            if (column < 0 || row < 0 || column >= terrain_grid_columns_ || row >= terrain_grid_rows_) return false;
            cell_triangles = &terrain_grid_[static_cast<size_t>(row) * terrain_grid_columns_ + column];
        }
        const size_t candidate_count = cell_triangles != nullptr ? cell_triangles->size() : terrain_triangles_.size();
        for (size_t candidate = 0; candidate < candidate_count; ++candidate){
            const size_t i = cell_triangles != nullptr ? (*cell_triangles)[candidate] : candidate;
            const TerrainTriangle& triangle = terrain_triangles_[i];
            if (grass_only
              && triangle.walkability_type != VGears::WorldmapFileSerializer::GRASS){
                continue;
            }
            const Ogre::Vector3& a = triangle.vertices[0];
            const Ogre::Vector3& b = triangle.vertices[1];
            const Ogre::Vector3& c = triangle.vertices[2];
            const Ogre::Real denominator
              = (b.z - c.z) * (a.x - c.x) + (c.x - b.x) * (a.z - c.z);
            if (Ogre::Math::Abs(denominator) < 1e-6f) continue;

            const Ogre::Real weight_a
              = ((b.z - c.z) * (position.x - c.x)
                + (c.x - b.x) * (position.z - c.z)) / denominator;
            const Ogre::Real weight_b
              = ((c.z - a.z) * (position.x - c.x)
                + (a.x - c.x) * (position.z - c.z)) / denominator;
            const Ogre::Real weight_c = 1.0f - weight_a - weight_b;
            if (weight_a < -1e-4f || weight_b < -1e-4f || weight_c < -1e-4f){
                continue;
            }

            const Ogre::Real height
              = weight_a * a.y + weight_b * b.y + weight_c * c.y;
            const Ogre::Real height_distance = Ogre::Math::Abs(height - position.y);
            if (height_distance >= closest_height_distance) continue;
            closest_height_distance = height_distance;
            found = !require_walkable || (map_id_ == 1
              // Underwater: every terrain type is allowed, but there is a height limit.
              ? (!UNDERWATER_HEIGHT_LIMIT_ENABLED
                || height + FLIGHT_MIN_CLEARANCE <= UNDERWATER_MAX_HEIGHT)
              : IsActiveTerrainAllowed(triangle.walkability_type));
            surface = Ogre::Vector3(position.x, height, position.z);
            if (walkability_type != nullptr) *walkability_type = triangle.walkability_type;
            if (triangle_index != nullptr) *triangle_index = i;
        }
        return found;
    }

    void WorldmapModule::ClearVehicleTerrains(){
        active_vehicle_terrain_types_.clear();
    }

    void WorldmapModule::AddVehicleTerrain(const char* terrain_name){
        if (terrain_name == nullptr) return;
        const auto terrain = GetWorldMapTerrainTypes().find(terrain_name);
        if (terrain == GetWorldMapTerrainTypes().end()){
            LogManager::getSingleton().logMessage(
              "Unknown world-map vehicle terrain '" + Ogre::String(terrain_name) + "'."
            );
            return;
        }
        if (std::find(
              active_vehicle_terrain_types_.begin(), active_vehicle_terrain_types_.end(),
              terrain->second
            ) == active_vehicle_terrain_types_.end()){
            active_vehicle_terrain_types_.push_back(terrain->second);
        }
    }

    bool WorldmapModule::IsActiveTerrainAllowed(uint8 walkability_type) const{
        if (current_vehicle_ == VehicleType::ON_FOOT){
            return IsFootWalkable(walkability_type);
        }
        return std::find(
          active_vehicle_terrain_types_.begin(), active_vehicle_terrain_types_.end(),
          walkability_type
        ) != active_vehicle_terrain_types_.end();
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
