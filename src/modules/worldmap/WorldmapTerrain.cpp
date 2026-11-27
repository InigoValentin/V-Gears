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

#include "modules/worldmap/WorldmapTerrain.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <OgreException.h>
#include <OgreLogManager.h>
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include "modules/worldmap/WorldmapData.h"
#include "modules/worldmap/WorldmapTextures.h"

namespace VGears {

    using namespace Ogre;

    WorldmapTerrain::WorldmapTerrain(){}

    WorldmapTerrain::~WorldmapTerrain(){
        if (scene_manager_ != nullptr && world_map_node_ != nullptr){
            scene_manager_->getRootSceneNode()->removeAndDestroyChild("WorldMapTerrain");
        }
    }

    void WorldmapTerrain::Clear(){
        if (world_map_node_ != nullptr && scene_manager_ != nullptr){
            // Destroy the surfaces. The node is kept and reused.
            while (world_map_node_->numAttachedObjects() > 0){
                Ogre::MovableObject* object = world_map_node_->detachObject(static_cast<unsigned short>(0));
                scene_manager_->destroyMovableObject(object);
            }
        }
        triangles_.clear();
        terrain_grid_.clear();
    }

    void WorldmapTerrain::SetVisible(const bool visible){
        if (world_map_node_ != nullptr) world_map_node_->setVisible(visible);
    }

    const std::vector<TerrainTriangle>& WorldmapTerrain::GetTriangles() const{return triangles_;}

    Ogre::Vector3 WorldmapTerrain::GetCenter() const{
        const float space = From16BitFixedPoint(8182);
        return Ogre::Vector3(
          space * (origin_x_ * 4.0f + grid_width_ * 2.0f), 0.0f,
          space * (origin_y_ * 4.0f + grid_height_ * 2.0f)
        );
    }

    void WorldmapTerrain::Load(SceneManager* scene_manager, const int map_id){
        scene_manager_ = scene_manager;
        // The node is kept when a map is replaced.
        if (world_map_node_ == nullptr){
            world_map_node_ = scene_manager_->getRootSceneNode()
              ->createChildSceneNode("WorldMapTerrain");
        }
        world_map_node_->setVisible(true);
        const MapLayout layout = GetMapLayout(map_id);
        grid_width_ = layout.width;
        grid_height_ = layout.height;
        origin_x_ = layout.origin_x;
        origin_y_ = layout.origin_y;
        std::vector<VGears::WorldmapFileSerializer::Block> blocks
          = LoadWorldMap("WM" + std::to_string(WORLD_MAP_FILE_NUMBERS[map_id]) + ".MAP");
        if (GetTextureIdOffset(map_id) != 0){
            for (auto& block : blocks) for (auto& mesh : block.meshes) for (auto& triangle : mesh.triangles){
                triangle.TextureInfo += GetTextureIdOffset(map_id);
            }
        }
        const size_t texture_count = WORLD_MAP_TEXTURE_COUNT;
        // WM0 has extra blocks after the 9x7 grid: the alternate shapes of the story progress.
        const size_t base_block_count = map_id == 0
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
                    triangles_.push_back(terrain_triangle);

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

        BuildIndex();
    }

    void WorldmapTerrain::BuildIndex(){
        terrain_grid_.clear();
        terrain_grid_columns_ = 0;
        terrain_grid_rows_ = 0;
        if (triangles_.empty()) return;
        float min_x = std::numeric_limits<float>::max(), min_z = min_x;
        float max_x = -min_x, max_z = -min_x;
        for (const TerrainTriangle& triangle : triangles_){
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
        for (size_t i = 0; i < triangles_.size(); ++i){
            const TerrainTriangle& triangle = triangles_[i];
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
          "World map terrain index: " + std::to_string(triangles_.size()) + " triangles in "
          + std::to_string(terrain_grid_columns_) + "x" + std::to_string(terrain_grid_rows_) + " cells, "
          + std::to_string(entries) + " entries"
        );
    }

    bool WorldmapTerrain::FindSurface(
      const Ogre::Vector3& position, Ogre::Vector3& surface, bool grass_only,
      const WalkableCheck& check, uint8* walkability_type, size_t* triangle_index
    ) const{
        bool found = false;
        Ogre::Real closest_height_distance = std::numeric_limits<Ogre::Real>::max();
        // Only the triangles of the cell of the position (the index is in ascending order, so the result
        // is the same as with all of them).
        const std::vector<uint32>* cell_triangles = nullptr;
        if (!terrain_grid_.empty()){
            const int column = static_cast<int>(std::floor((position.x - terrain_grid_min_x_) / terrain_grid_cell_));
            const int row = static_cast<int>(std::floor((position.z - terrain_grid_min_z_) / terrain_grid_cell_));
            if (column < 0 || row < 0 || column >= terrain_grid_columns_ || row >= terrain_grid_rows_) return false;
            cell_triangles = &terrain_grid_[static_cast<size_t>(row) * terrain_grid_columns_ + column];
        }
        const size_t candidate_count = cell_triangles != nullptr ? cell_triangles->size() : triangles_.size();
        for (size_t candidate = 0; candidate < candidate_count; ++candidate){
            const size_t i = cell_triangles != nullptr ? (*cell_triangles)[candidate] : candidate;
            const TerrainTriangle& triangle = triangles_[i];
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
            found = !check || check(triangle, height);
            surface = Ogre::Vector3(position.x, height, position.z);
            if (walkability_type != nullptr) *walkability_type = triangle.walkability_type;
            if (triangle_index != nullptr) *triangle_index = i;
        }
        return found;
    }

} // namespace VGears
