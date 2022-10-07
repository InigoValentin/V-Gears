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

#include "common/TypeDefine.h"
#include "data/VGearsSerializer.h"

namespace VGears {

    class WorldMapFile;

    /**
     * Handles map file serialization.
     */
    class MapFileSerializer : public Serializer{

        public:

            /**
             * Block header of map files.
             */
            struct BlockHeader{
                /**
                 * Offset in this block of the compressed data.
                 *
                 * At the compressed data offset we have another uint32 which
                 * is the size of the uncompressed data.
                 */
                uint32 compressed_data_offsets[16];
            };

            /**
             * Block header of a mesh section in map files.
             */
            struct BlockMeshHeader{

                /**
                 * Number of BlockTriangle's in the mesh.
                 */
                uint16 triengle_count;

                /**
                 * Number of vertices and normals.
                 */
                uint16 vertex_count;
            };

            /**
             * A vertex.
             */
            struct Vertex{

                /**
                 * X coordinate.
                 */
                sint16 x;

                /**
                 * Y coordinate.
                 */
                sint16 y;

                /**
                 * Z coordinate.
                 */
                sint16 z;

                /**
                 * Fill space to fit the structure in 32 bit.
                 */
                uint16 unused;
            };

            /**
             * A normal.
             */
            struct Normal{
                /**
                 * X coordinate.
                 */
                sint16 x;

                /**
                 * Y coordinate.
                 */
                sint16 y;

                /**
                 * Z coordinate.
                 */
                sint16 z;

                /**
                 * Fill space to fit the structure in 32 bit.
                 */
                uint16 unused;
            };

            /**
             * A triangle block in a map file.
             */
            struct BlockTriangle{

                /**
                 * Index of the first vertex of the triangle.
                 */
                uint8 vertex_0_index;

                /**
                 * Index of the second vertex of the triangle.
                 */
                uint8 vertex_1_index;

                /**
                 * Index of the third vertex of the triangle.
                 */
                uint8 vertex_2_index;

                /**
                 * Triangle walkability.
                 *
                 * Only 5 bits are valid.
                 *
                 * @todo Document walkability modes.
                 */
                uint8 walkability_info_;

                /**
                 * Unknown data.
                 *
                 * Only 3 bits are valid.
                 *
                 * @todo Find out what this is for.
                 */
                uint8 unknown;

                /**
                 * First side vector U component.
                 */
                uint8 vertex_0_u;

                /**
                 * First side vector V component.
                 */
                uint8 vertex_0_v;

                /**
                 * Second side vector U component.
                 */
                uint8 vertex_1_u;

                /**
                 * Second side vector V component.
                 */
                uint8 vertex_1_v;

                /**
                 * Third side vector U component.
                 */
                uint8 vertex_2_u;

                /**
                 * Third side vector V component.
                 */
                uint8 vertex_2_v;

                /**
                 * Triangle texture info.
                 *
                 * Only 9 bits are valid.
                 */
                uint16 TextureInfo;

                /**
                 * Triangle location.
                 *
                 * Only 7 bits are valid.
                 */
                uint16 Location;
            };

            /**
             * Type of walkmesh triangle.
             *
             * Determines the triangle walkability under different conditions.
             */
            enum TriangleWalkMapTypes{

                /**
                 * Grass.
                 *
                 * Most things can go here.
                 */
                GRASS = 0,

                /**
                 * Forest.
                 *
                 * No landing here, but anything else goes.
                 */
                FOREST = 1,

                /**
                 * Mountain.
                 *
                 * Chocobos and flying machines only.
                 */
                MOUNTAIN = 2,

                /**
                 * Deep water.
                 *
                 * Only gold chocobo and submarine can go here.
                 */
                SEA_DEEP_WATER = 3,

                /**
                 * River crossing.
                 *
                 * Buggy, Tiny Bronco and water-capable chocobos.
                 */
                RIVER_CROSSING = 4,

                /**
                 * River.
                 *
                 * Tiny Bronco and chocobos.
                 */
                RIVER = 5,

                /**
                 * Shallow water.
                 *
                 * Only Tiny Bronco and chocobos.
                 */
                WATER = 6,

                /**
                 * Swamp.
                 *
                 * The Midgar zolom can only move in swamp areas.
                 */
                SWAMP = 7,

                /**
                 * Desert.
                 *
                 * No landing.
                 */
                DESERT = 8,

                /**
                 * Wasteland.
                 *
                 * Found around Midgar, Wutai and misc other. No landing.
                 */
                WASTELAND = 9,

                /**
                 * Snow.
                 *
                 * Leaves footprints, no landing.
                 */
                SNOW = 10,

                /**
                 * Riverside.
                 *
                 * Beach-like area where river and land meet.
                 */
                RIVERSIDE = 11,

                /**
                 * Cliff.
                 *
                 * Sharp drop, usually where the player can be on either side.
                 */
                CLIFF = 12,

                /**
                 * Corel bridge.
                 *
                 * Tiny bridge over the waterfall from Costa del Sol to Corel.
                 */
                COREL_BRIDGE = 13,

                /**
                 * Bridges in Wutai.
                 *
                 * Rickety rope bridges south of Wutai.
                 */
                WUTAI_BRIDGE = 14,

                /**
                 * Unused.
                 *
                 * It doesn't seem to be used anywhere in the original data.
                 */
                UNUSED_1 = 15,

                /**
                 * Hill side.
                 *
                 * This is the tiny walkable part at the foot of a mountain.
                 */
                HILL_SIDE = 16,

                /**
                 * Beach.
                 *
                 * Where land and shallow water meet.
                 */
                BEACH = 17,

                /**
                 * Submarine pens.
                 *
                 * The only place where the submarine can be boarded.
                 */
                SUBMARINE_PEN = 18,

                /**
                 * Cosmo Canyon area.
                 *
                 * The ground in cosmo canyon has this type, walkability seems
                 * to be the same as {@see Wasteland} (no landing).
                 */
                CANYON = 19,

                /**
                 * Mountain pass.
                 *
                 * The small path through the mountains connecting Costa del
                 * Sol and Corel.
                 */
                MOUNTAIN_PASS = 20,

                /**
                 * Unknown.
                 *
                 * Present around bridges, may have some special meaning.
                 */
                UNKNOWN_1 = 21,

                /**
                 * Waterfall.
                 *
                 * River type where the Tiny Bronco can't go.
                 */
                WATERFALL = 22,

                /**
                 * Unused.
                 *
                 * It doesn't seem to be used anywhere in the original data.
                 */
                UNUSED_2 = 23,

                /**
                 * Gold Saucer desert.
                 *
                 * Special desert type for the Gold Saucer desert.
                 */
                GOLD_SAUCER_DESERT = 24,

                /**
                 * Jungle.
                 *
                 * Same walkability as {@see FOREST}, used in southern parts of
                 * the map.
                 */
                JUNGLE = 25,

                /**
                 * Deep sea.
                 *
                 * Special type of deep water, only used in one small spot next
                 * to HP-MP cave, possibly related to the underwater
                 * map/submarine.
                 */
                SEA = 26,

                /**
                 * The northernd cave.
                 *
                 * Inside part of the crater, where the Highwind can descend.
                 */
                NORTHERND_CAVE = 27,

                /**
                 * Borders of the Gold Saucer desert.
                 *
                 * Narrow strip of land surrounding the golden saucer desert.
                 * Probably related to the "quicksand" script.
                 */
                GOLD_SAUCER_DESERT_BORDER = 28,

                /**
                 * Bridge heads.
                 *
                 * Small area at both ends of every bridge. May have some
                 * special meaning.
                 */
                BRIDGE_HEAD = 29,

                /**
                 * Entrances.
                 *
                 * Special type that can be set unwalkable from the script.
                 */
                BACK_ENTRANCE = 30,

                /**
                 * Unused.
                 *
                 * It doesn't seem to be used anywhere in the original data.
                 */
                UNUSED_3 = 31
            };

            /**
             * Constructor.
             */
            MapFileSerializer();

            /**
             * Destructor.
             */
            virtual ~MapFileSerializer();

            /**
             * Imports and parses a map file.
             *
             * @param[in] stream Map file contents.
             * @param[in] dest Unused
             */
            void ImportMapFile(Ogre::DataStreamPtr& stream, WorldMapFile& dest);

            /**
             * Part of a file block.
             */
            struct BlockPart{

                /**
                 * Block part header.
                 */
                BlockMeshHeader header;

                /**
                 * List of triangles in the block part.
                 */
                std::vector<BlockTriangle> triangles;

                /**
                 * List of vertices in the block part.
                 */
                std::vector<Vertex> vertices;

                /**
                 * List of normals in the block part.
                 */
                std::vector<Normal> normals;
            };

            /**
             * A file block.
             */
            struct Block{

                /**
                 * List of meshes in the block.
                 */
                std::vector<BlockPart> meshes;
            };

            /**
             * The list of file blocks.
             */
            std::vector<Block> blocks;
    };

}
