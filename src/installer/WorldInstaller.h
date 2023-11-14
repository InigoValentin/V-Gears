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

#include <string>
#include <vector>
#include <common/TypeDefine.h>
#include "common/File.h"

/**
 * The world map data installer.
 */
class WorldInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] input_dir Path to the directory containing the original data to parse.
         * @param[in] output_dir Path to the directory of the installation data.
         * @param[in] keep_originals True to keep original data after conversion, false to remove.
         */
        WorldInstaller(
          std::string input_dir, std::string output_dir, const bool keep_originals
        );

        /**
         * Destructor.
         */
        ~WorldInstaller();

        /**
         * Initializes the installer.
         *
         * @return The number of maps to process.
         */
        unsigned int Initialize();

        /**
         * Processes the next map to process.
         *
         * @return True if, once the map has been processed, all maps are processed. False if there
         * are more maps to process.
         */
        bool ProcessMap();

        //void ReadMapFile(std::string file_name);

        //void GenerateMap(std::string file_name, std::string output);

    private:

        /**
         * UV coordinates for a vertex.
         */
        struct UVCoord{

            /**
             * U coordinate.
             */
            u8 u;

            /**
             * V coordinate.
             */
            u8 v;
        };

        /**
         * Mesh triangle data.
         */
        struct Triangle{

            /**
             * Index for each triangle vertex.
             */
            u8 vertex_index[3];

            /**
             * Walkability info.
             *
             * Shares byte with {@see function_id}, 5 bytes.
             */
            u8 walkability;

            /**
             * ID of the function triggered when entering the triangle.
             *
             * Shares byte with {@see walkability}, 3 bytes.
             */
            u8 function_id;

            /**
             * UV coordinates in texture for each vertex.
             */
            UVCoord vertex_coord[3];

            /**
             * Walkability info.
             *
             * Shares bytes with {@see location_id}, 9 bytes.
             */
            u16 texture_info;

            /**
             * Location message ID.
             *
             * Shares bytes with {@see texture_info}, 7 bytes.
             */
            u16 location;
        };

        /**
         * Mesh vertex data.
         *
         * Can also be used for nromal nada.
         */
        struct Vertex{

            /**
             * X coordinate.
             */
            u16 x;

            /**
             * Y coordinate.
             */
            u16 y;

            /**
             * Z coordinate.
             */
            u16 z;

            /**
             * Unknown value, unused.
             */
            u16 unknown;
        };

        /**
         * Mesh data.
         */
        struct Mesh{ // 0xB800 / 0x0F

            /**
             * Number of triangles in the mesh.
             */
            u16 triangle_count;

            /**
             * Number of vertices in the mesh.
             */
            u16 vertex_count;

            /**
             * Mesh triangle data.
             *
             * Size must equal {@see triangle_count}.
             */
            std::vector<Triangle> triangles;

            /**
             * Mesh vertex data.
             *
             * Size must equal {@see vertex_count}.
             */
            std::vector<Vertex> vertices;

            /**
             * Mesh normal data.
             *
             * Size must equal {@see vertex_count}.
             */
            std::vector<Vertex> normals;
        };

        /**
         * Each block of a map.
         *
         * A block is always 0xB800 bytes and it's always composed of 16 meshes.
         */
        struct Block{ // 0xB800

            /**
             * Block meshes.
             */
            Mesh mesh[16];
        };

        /**
         * A map.
         *
         * A map is defined in a WM*.MAP file. They are divided in 0xB800 byte blocks.
         */
        struct Map {

            /**
             * Map blocks.
             */
            std::vector<Block> blocks;
        };

        std::vector<u8> DecompressLZSSData(u32* compressed_data);

        /**
         * Each of the original WM*.MAP in the installation disk.
         */
        std::vector<File> wm_map_;

        /**
         * The path to the directory from which to read the PC game data.
         */
        std::string input_dir_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

        /**
         * If true, original files will not be deleted after conversion.
         */
        bool keep_originals_;

        /**
         * Number of maps already processed.
         */
        unsigned int processed_maps_;
};
