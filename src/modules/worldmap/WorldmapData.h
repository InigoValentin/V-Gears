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

#include <cstddef>
#include <map>
#include <vector>
#include <OgreString.h>
#include "common/TypeDefine.h"
#include "formats/worldmap/WorldmapFileSerializer.h"

namespace VGears {

    /**
     * Resource group of the world map files.
     */
    static const char* const WORLD_MAP_RESOURCE_GROUP = "WORLD_MAP";

    /**
     * World map hardcoded texture information.
     */
    struct HarcodedTextureInfo{

        /**
         * Texture id.
         */
        uint32 id;

        /**
         * Texture name.
         */
        const char* name;

        /**
         * Texture width.
         */
        uint32 width;

        /**
         * Texture height.
         */
        uint32 height;

        /**
         * Texture U component.
         */
        uint32 u;

        /**
         * Texture V component
         */
        uint32 v;
    };

    /**
     * World map hardcoded textures.
     */
    extern const HarcodedTextureInfo WORLD_MAP_TEXTURES[];

    /**
     * Number of textures in {@see WORLD_MAP_TEXTURES}.
     */
    extern const size_t WORLD_MAP_TEXTURE_COUNT;

    /**
     * Gets what has to be added to the texture ids of a map to get its textures in the table.
     *
     * @param[in] map_id The map.
     * @return The offset.
     */
    uint16 GetTextureIdOffset(const uint8 map_id);

    /**
     * Tells if a terrain can be walked on foot.
     *
     * @param[in] walkability_type Type of the terrain.
     * @return True if it can be walked.
     */
    bool IsFootWalkable(uint8 walkability_type);

    /**
     * Gets the types of terrain, by name.
     */
    const std::map<Ogre::String, uint8>& GetWorldMapTerrainTypes();

    /**
     * Gets the name of a type of terrain.
     *
     * @param[in] walkability_type Type of the terrain.
     * @return The name, or an empty string if it is unknown.
     */
    Ogre::String GetWorldMapTerrainName(const uint8 walkability_type);

    /**
     * World map IDs and the files of the original game they load. The original game has no WM1.
     * 0 is the overworld, 1 is WM2 and 2 is WM3.
     */
    static const int WORLD_MAP_FILE_NUMBERS[] = {0, 2, 3};
    static const int WORLD_MAP_COUNT = sizeof(WORLD_MAP_FILE_NUMBERS) / sizeof(int);

    /**
     * Size and position of a map in blocks. The blocks of a MAP file are stored in order, row by
     * row. The maps are placed in the 9x7 block grid of the world, since the scripts index the
     * meshes with the position in that grid, and the maps share coordinates: the underwater map
     * WM2 is a part of the world of WM0, so a position of WM0 inside it is the same in WM2.
     *
     * The layout of WM2 was deduced from the edges of its blocks and the mesh coordinates in its
     * script. The one of WM3 is a guess.
     */
    struct MapLayout{
        int width;
        int height;
        int origin_x;
        int origin_y;
    };

    /**
     * Gets the size and position of a map.
     *
     * @param[in] map_id The map.
     * @return Its layout.
     */
    MapLayout GetMapLayout(const int map_id);

    /**
     * Gets if a position of the world is inside the blocks of a map.
     *
     * @param[in] map_id The map.
     * @param[in] x Position in X.
     * @param[in] z Position in Z.
     * @return True if it is inside.
     */
    bool IsInsideMap(const int map_id, const float x, const float z);

    /**
     * Loads and parses an installed world map file.
     *
     * @param[in] file_name Name of the MAP file, like "WM0.MAP".
     * @return The map blocks.
     */
    std::vector<VGears::WorldmapFileSerializer::Block> LoadWorldMap(const Ogre::String& file_name);

    /**
     * Creates a 16 bit point from coordinates.
     *
     * @param[in] coord The coordinates.
     * @return Coordinate point.
     */
    float From16BitFixedPoint(VGears::sint16 coord);
}
