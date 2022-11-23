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
/*#include <vector>
#include <common/TypeDefine.h>

class WorldInstaller{

    public:

        WorldInstaller(std::string input_dir, std::string output_dir);

        ~WorldInstaller();

        void ReadMapFile(std::string file_name);

        void GenerateMap(std::string file_name, std::string output);

    private:

        struct Mesh{ // 0xB800 / 0x0F

            u16 triangle_count;

            u16 vertex_count;

            u8 compressed_data[(0xB800 / 0x0F)];
        };

        struct Map{ // 0xB800

            Mesh mesh[16];
        };

        struct WMFile {
            Map map[64];
        };

        std::vector<uint8> DecompressLZSSData(u32* compressed_data);

        std::string input_dir_;

        std::string output_dir_;
};*/
