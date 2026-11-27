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

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>
#include "MovieCamera.h"

namespace{

    uint32_t ReadU32(const std::vector<uint8_t>& data, const size_t offset){
        uint32_t value = 0;
        std::memcpy(&value, data.data() + offset, sizeof(value));
        return value;
    }

    // Header of the archive: 12 bytes of padding and "SQUARESOFT", then the number of files.
    const size_t COUNT_OFFSET = 12;
    const size_t TABLE_OFFSET = 16;

    // Each entry of the table: name (20), offset (4), type (1) and duplicate counter (2).
    const size_t ENTRY_SIZE = 27;
    const size_t NAME_SIZE = 20;

    // Each file starts with its name (20) and its size (4).
    const size_t FILE_HEADER_SIZE = 24;
}

namespace MovieCamera{

    int FirstCameraFrame(const std::vector<uint8_t>& camera){
        const size_t frames = camera.size() / FRAME_SIZE;
        for (size_t frame = 0; frame < frames; ++ frame){
            const auto begin = camera.begin() + frame * FRAME_SIZE;
            // The last 8 bytes are the projection, which can be set even if there is no camera.
            if (std::any_of(begin, begin + 32, [](uint8_t byte){return byte != 0;}))
                return static_cast<int>(frame);
        }
        return -1;
    }

    bool ReadLgpFile(
      const std::vector<uint8_t>& archive, const std::string& name, std::vector<uint8_t>& content
    ){
        if (archive.size() < TABLE_OFFSET) return false;
        const uint32_t count = ReadU32(archive, COUNT_OFFSET);
        if (archive.size() < TABLE_OFFSET + static_cast<size_t>(count) * ENTRY_SIZE) return false;
        for (uint32_t i = 0; i < count; ++ i){
            const size_t entry = TABLE_OFFSET + static_cast<size_t>(i) * ENTRY_SIZE;
            const char* entry_name = reinterpret_cast<const char*>(archive.data() + entry);
            if (std::string(entry_name, strnlen(entry_name, NAME_SIZE)) != name) continue;
            const size_t offset = ReadU32(archive, entry + NAME_SIZE);
            if (archive.size() < offset + FILE_HEADER_SIZE) return false;
            const size_t size = ReadU32(archive, offset + NAME_SIZE);
            if (archive.size() < offset + FILE_HEADER_SIZE + size) return false;
            const auto begin = archive.begin() + offset + FILE_HEADER_SIZE;
            content.assign(begin, begin + size);
            return true;
        }
        return false;
    }

    std::vector<uint8_t> LoadCamera(const std::string& lgp_path, const std::string& movie){
        std::vector<uint8_t> camera;
        std::ifstream file(lgp_path, std::ios::binary);
        if (!file) return camera;
        const std::vector<uint8_t> archive(
          (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()
        );
        if (!ReadLgpFile(archive, movie + ".cam", camera)) camera.clear();
        return camera;
    }
}
