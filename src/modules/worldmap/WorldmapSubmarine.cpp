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
#include <vector>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreSceneNode.h>
#include "engine/entity/EntityManager.h"
#include "engine/entity/EntityModel.h"
#include "engine/ui/UiManager.h"
#include "engine/ui/UiWidget.h"
#include "modules/worldmap/WorldmapData.h"

namespace VGears {

    using namespace Ogre;

    /// How deep the submarine goes under the surface of WM0 when diving, in world units.
    static const float DIVE_SINK_DEPTH = 0.5f;

    /// Time the submarine takes to go under the surface of WM0, or to come up to it.
    static const float DIVE_SINK_TIME = 1.0f;

    /// Height where the submarine appears in WM2 when diving, and where it goes when surfacing.
    /// It's over the top layer of the map.
    static const float DIVE_HIGH_HEIGHT = 1.0f;

    /// Time the submarine takes to go down from, or up to, the height above.
    static const float DIVE_HIGH_TIME = 1.5f;

    /// Time of the fade to black before the map changes.
    static const float DIVE_FADE_OUT_TIME = 0.5f;

    /**
     * Gets if a position of the overworld (WM0) is over deep water. Used when the submarine
     * comes back from WM2, when WM0 is not loaded. Only the part of the world that WM2 covers is
     * read, and it's done once.
     */
    static bool IsOverworldDeepWater(const float x, const float z){
        struct Triangle{
            float x[3];
            float z[3];
        };
        static std::vector<Triangle> triangles;
        static bool loaded = false;
        if (!loaded){
            loaded = true;
            const MapLayout overworld = GetMapLayout(0);
            const MapLayout underwater = GetMapLayout(1);
            const float space = From16BitFixedPoint(8182);
            const std::vector<VGears::WorldmapFileSerializer::Block> blocks = LoadWorldMap("WM0.MAP");
            for (size_t block_index = 0; block_index < std::min(blocks.size(), size_t(63)); ++block_index){
                const int map_x = block_index % overworld.width;
                const int map_y = block_index / overworld.width;
                if (map_x < underwater.origin_x || map_x >= underwater.origin_x + underwater.width
                  || map_y < underwater.origin_y || map_y >= underwater.origin_y + underwater.height){
                    continue;
                }
                const VGears::WorldmapFileSerializer::Block& block = blocks[block_index];
                for (size_t mesh_index = 0; mesh_index < block.meshes.size(); ++mesh_index){
                    const VGears::WorldmapFileSerializer::BlockPart& part = block.meshes[mesh_index];
                    const float pos_x = ((mesh_index % 4) + map_x * 4) * space;
                    const float pos_z = ((mesh_index / 4) + map_y * 4) * space;
                    for (const VGears::WorldmapFileSerializer::BlockTriangle& triangle : part.triangles){
                        if (triangle.walkability_info_ != VGears::WorldmapFileSerializer::SEA_DEEP_WATER){
                            continue;
                        }
                        const size_t indices[3] = {
                            triangle.vertex_0_index, triangle.vertex_1_index, triangle.vertex_2_index
                        };
                        Triangle result;
                        for (int i = 0; i < 3; ++i){
                            const VGears::WorldmapFileSerializer::Vertex& vertex = part.vertices.at(indices[i]);
                            result.x[i] = From16BitFixedPoint(vertex.x) + pos_x;
                            result.z[i] = From16BitFixedPoint(vertex.z) + pos_z;
                        }
                        triangles.push_back(result);
                    }
                }
            }
        }
        for (const Triangle& t : triangles){
            const float denominator
              = (t.z[1] - t.z[2]) * (t.x[0] - t.x[2]) + (t.x[2] - t.x[1]) * (t.z[0] - t.z[2]);
            if (Ogre::Math::Abs(denominator) < 1e-6f) continue;
            const float a = ((t.z[1] - t.z[2]) * (x - t.x[2]) + (t.x[2] - t.x[1]) * (z - t.z[2]))
              / denominator;
            const float b = ((t.z[2] - t.z[0]) * (x - t.x[2]) + (t.x[0] - t.x[2]) * (z - t.z[2]))
              / denominator;
            const float c = 1.0f - a - b;
            if (a >= -1e-4f && b >= -1e-4f && c >= -1e-4f) return true;
        }
        return false;
    }

    bool WorldmapModule::DiveToUnderwaterMap(EntityModel* submarine){
        const Ogre::Vector3 position = submarine->GetPosition();
        Ogre::Vector3 surface;
        uint8 walkability_type = 0;
        if (!FindWalkableSurface(position, surface, false, false, &walkability_type)
          || walkability_type != VGears::WorldmapFileSerializer::SEA_DEEP_WATER){
            return false;
        }
        // WM2 only covers a part of the world.
        if (!IsInsideMap(1, position.x, position.z)){
            LogManager::getSingleton().logMessage(
              "The submarine can't dive here: it's outside the underwater map."
            );
            return false;
        }
        dive_requested_ = true;
        dive_position_ = position;
        dive_heading_ = submarine->GetModel()->getParentSceneNode()->getParentSceneNode()
          ->getOrientation().getYaw().valueRadians();
        BeginTransition(true, submarine);
        return true;
    }

    /**
     * Sets how opaque the black cover of the screen is.
     *
     * @param[in] alpha 0 shows the world map, 1 is all black.
     */
    static void SetScreenCover(const float alpha){
        UiWidget* cover = UiManager::getSingleton().GetWidget("Fade.Screen");
        if (cover == nullptr) return;
        cover->SetColour(0, 0, 0);
        cover->SetAlpha(std::max(0.0f, std::min(1.0f, alpha)));
    }

    /// Eases a progress from 0 to 1.
    static float Smooth(const float t){
        const float clamped = std::max(0.0f, std::min(1.0f, t));
        return clamped * clamped * (3.0f - 2.0f * clamped);
    }

    void WorldmapModule::BeginTransition(const bool dive, EntityModel* submarine){
        transition_stage_ = TransitionStage::LEAVING;
        transition_dive_ = dive;
        transition_timer_ = 0;
        transition_start_y_ = submarine->GetPosition().y;
        // Diving: it sinks under the surface. Surfacing: it goes up, out of the map.
        transition_target_y_ = dive ? transition_start_y_ - DIVE_SINK_DEPTH : DIVE_HIGH_HEIGHT;
        EntityManager::getSingleton().ScriptPlayerLock(true);
    }

    void WorldmapModule::EndTransition(){
        transition_stage_ = TransitionStage::NONE;
        SetScreenCover(0);
        EntityManager::getSingleton().ScriptPlayerLock(false);
    }

    void WorldmapModule::UpdateTransition(const Ogre::Real delta_time){
        transition_timer_ += delta_time;
        ::Entity* submarine = EntityManager::getSingleton().ScriptGetPlayerEntity();
        if (submarine == nullptr){
            EndTransition();
            return;
        }
        const auto set_height = [submarine](const float y){
            Ogre::Vector3 position = submarine->GetPosition();
            position.y = y;
            submarine->SetPosition(position);
        };

        if (transition_stage_ == TransitionStage::LEAVING){
            const float duration = transition_dive_ ? DIVE_SINK_TIME : DIVE_HIGH_TIME;
            set_height(
              transition_start_y_
              + (transition_target_y_ - transition_start_y_) * Smooth(transition_timer_ / duration)
            );
            if (transition_timer_ >= duration){
                transition_stage_ = TransitionStage::FADING_OUT;
                transition_timer_ = 0;
            }
        }
        else if (transition_stage_ == TransitionStage::FADING_OUT){
            SetScreenCover(transition_timer_ / DIVE_FADE_OUT_TIME);
            if (transition_timer_ >= DIVE_FADE_OUT_TIME){
                // The map changes under the black screen. The entities are not valid after it.
                Init(transition_dive_ ? 1 : 0);
                ::Entity* arrived = EntityManager::getSingleton().GetEntity("WorldMapSubmarine");
                if (arrived == nullptr){
                    EndTransition();
                    return;
                }
                transition_target_y_ = arrived->GetPosition().y;
                // Diving: it comes from high up. Surfacing: it comes from under the surface.
                transition_start_y_ = transition_dive_
                  ? DIVE_HIGH_HEIGHT : transition_target_y_ - DIVE_SINK_DEPTH;
                Ogre::Vector3 position = arrived->GetPosition();
                position.y = transition_start_y_;
                arrived->SetPosition(position);
                transition_stage_ = TransitionStage::ARRIVING;
                transition_timer_ = 0;
                EntityManager::getSingleton().ScriptPlayerLock(true);
                UpdateCamera(0, true);
            }
        }
        else if (transition_stage_ == TransitionStage::ARRIVING){
            const float duration = transition_dive_ ? DIVE_HIGH_TIME : DIVE_SINK_TIME;
            set_height(
              transition_start_y_
              + (transition_target_y_ - transition_start_y_) * Smooth(transition_timer_ / duration)
            );
            // The screen is shown again by the time the submarine is at its height.
            SetScreenCover(1.0f - transition_timer_ / duration);
            if (transition_timer_ >= duration){
                set_height(transition_target_y_);
                EndTransition();
            }
        }
    }

    bool WorldmapModule::SurfaceToOverworld(EntityModel* submarine){
        const Ogre::Vector3 position = submarine->GetPosition();
        // The same position of WM0 has to be deep water.
        if (!IsOverworldDeepWater(position.x, position.z)){
            LogManager::getSingleton().logMessage(
              "The submarine can't surface here: it's not deep water in the overworld."
            );
            return false;
        }
        surface_requested_ = true;
        surface_position_ = position;
        surface_heading_ = submarine->GetModel()->getParentSceneNode()->getParentSceneNode()
          ->getOrientation().getYaw().valueRadians();
        BeginTransition(false, submarine);
        return true;
    }

    void WorldmapModule::PlaceSurfacedSubmarine(){
        surface_requested_ = false;
        if (submarine_entity_ == nullptr) return;
        Ogre::Vector3 surface;
        if (!FindWalkableSurface(surface_position_, surface, false, false)) return;
        submarine_entity_->SetPosition(surface);
        submarine_entity_->GetModel()->getParentSceneNode()->getParentSceneNode()->setOrientation(
          Ogre::Quaternion(Ogre::Radian(surface_heading_), Ogre::Vector3::UNIT_Y)
        );
        // Cloud boards the submarine, like the boarding script does.
        player_entity_->SetVisible(false);
        player_entity_->SetSolid(false);
        player_entity_->SetTalkable(false);
        ClearVehicleTerrains();
        for (const char* terrain : {"SEA_DEEP_WATER", "SEA", "SUBMARINE_PEN"}) AddVehicleTerrain(terrain);
        EntityManager::getSingleton().ScriptSetPlayerEntity("WorldMapSubmarine");
        UpdateCamera(0, true);
    }

} // namespace VGears
