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

#include <functional>
#include <vector>
#include <OgreVector3.h>
#include "common/TypeDefine.h"
#include "formats/worldmap/WorldmapFileSerializer.h"

namespace Ogre{
    class SceneManager;
    class SceneNode;
}

namespace VGears {

    /**
     * A triangle of the terrain of a world map.
     */
    struct TerrainTriangle{
        /// Corners of the triangle, in world units.
        Ogre::Vector3 vertices[3];
        /// Type of the terrain.
        uint8 walkability_type;
        /// Function of the world map script that runs when the triangle is entered, if 3 or more.
        uint8 function_id;
        /// Column of its mesh in the 9x7 block grid of the world.
        int mesh_x;
        /// Row of its mesh in the 9x7 block grid of the world.
        int mesh_y;
        /// Encounter region of the triangle, 0-15.
        uint8 region;
    };

    /**
     * The terrain of the loaded world map: its surfaces in the scene, and the search of the ground
     * under a position.
     */
    class WorldmapTerrain{

        public:

            /**
             * Tells if a triangle can be walked, at the height of the searched position.
             */
            typedef std::function<bool(const TerrainTriangle& triangle, Ogre::Real height)> WalkableCheck;

            /**
             * Constructor.
             */
            WorldmapTerrain();

            /**
             * Destructor. Removes the terrain from the scene.
             */
            ~WorldmapTerrain();

            /**
             * Loads the terrain of a map and shows it. {@see Clear} must be called before loading another one.
             *
             * @param[in] scene_manager Scene where the terrain is added.
             * @param[in] map_id 0 is WM0, 1 WM2 and 2 WM3.
             */
            void Load(Ogre::SceneManager* scene_manager, const int map_id);

            /**
             * Removes the surfaces and the triangles of the loaded map.
             */
            void Clear();

            /**
             * Shows or hides the terrain.
             *
             * @param[in] visible True to show it.
             */
            void SetVisible(const bool visible);

            /**
             * Searches the ground under a position: of the triangles over or under it, the one with the closest
             * height.
             *
             * @param[in] position Position to search at.
             * @param[out] surface The position on the ground. Only set if there is a triangle.
             * @param[in] grass_only True to only search grass.
             * @param[in] check Tells if the triangle can be walked. If empty, any triangle can.
             * @param[out] walkability_type Type of the terrain found. Can be nullptr.
             * @param[out] triangle_index Index of the triangle found. Can be nullptr.
             * @return True if the closest triangle can be walked.
             */
            bool FindSurface(
              const Ogre::Vector3& position, Ogre::Vector3& surface, bool grass_only,
              const WalkableCheck& check, uint8* walkability_type, size_t* triangle_index
            ) const;

            /**
             * Gets the triangles of the loaded map.
             */
            const std::vector<TerrainTriangle>& GetTriangles() const;

            /**
             * Gets the center of the loaded map, at height 0.
             */
            Ogre::Vector3 GetCenter() const;

        private:

            /**
             * Groups the triangles by the cell of a grid they touch, so the search of the triangle under a
             * position only checks a few of them.
             */
            void BuildIndex();

            std::vector<TerrainTriangle> triangles_;
            /// Indices of the triangles (ascending) that touch each cell of the grid, row by row.
            std::vector<std::vector<uint32>> terrain_grid_;
            float terrain_grid_min_x_ = 0.0f;
            float terrain_grid_min_z_ = 0.0f;
            float terrain_grid_cell_ = 1.0f;
            int terrain_grid_columns_ = 0;
            int terrain_grid_rows_ = 0;
            Ogre::SceneManager* scene_manager_ = nullptr;
            Ogre::SceneNode* world_map_node_ = nullptr;
            /// Size of the map in blocks.
            size_t grid_width_ = 9;
            size_t grid_height_ = 7;
            /// Position of the first block of the map in the 9x7 block grid of the world.
            size_t origin_x_ = 0;
            size_t origin_y_ = 0;
    };
}
