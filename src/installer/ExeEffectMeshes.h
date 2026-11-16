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

#include <cstdlib>
#include <map>
#include <ostream>
#include <string>
#include <vector>
#include "ExeEffectCameras.h"

/**
 * Reads the meshes of the battle effects that are in ff7.exe, like the ground of Ifrit or the
 * mountain of Ramuh.
 *
 * The initialization code of the effect copies a block of the data of the executable to the place it
 * is used from (mov ecx, size/4 ; mov esi, source ; mov edi, destination ; rep movsd), and the block
 * is a mesh: the size of its vertices in bytes (u32), the vertices (x, y, z and a padding, signed 16
 * bits each, y goes down), and four lists of polygons: textured triangles (16 bytes each), textured
 * quads (20), triangles with colours (20) and quads with colours (24). Every list begins with the
 * number of polygons and a word of flags (both s16). The polygons use the vertices by their position
 * in bytes. The blocks that fit exactly (the lists end where the copied block ends) are meshes.
 *
 * The effect moves the vertices to key shapes: arrays of vertices of the same size, one after the
 * other, in a table of pointers somewhere in the data. The 16 words that follow the table are
 * kept too, because some effects have the durations of their moves there.
 *
 * Found by reading the functions 0x594022 (Ifrit) and 0x598f74 (Ramuh) of the 1998 executable.
 */
namespace ExeEffectMeshes{

    /**
     * A mesh of the executable.
     */
    struct Mesh{
        std::vector<int16_t> vertices; ///< x, y, z of every vertex.
        std::vector<std::vector<int16_t>> keys; ///< x, y, z of the vertices of every key shape.
        std::vector<int16_t> tail; ///< 16 words after the table of key shapes.
        std::vector<int16_t> triangles; ///< 10 numbers each: 3 vertices, 3 pairs of UV, grey level.
        std::vector<int16_t> gouraud; ///< 12 numbers each: 3 vertices, 3 colours (r, g, b).
        size_t shards = 0; ///< Shard meshes: how many of the triangles are the ones of the first list.
    };

    /**
     * Reads a block as a mesh.
     *
     * @return False if it doesn't fit exactly.
     */
    inline bool ParseBlock(
      const ExeEffectCameras::Image& image, const uint32_t address, const uint32_t bytes, Mesh& mesh
    ){
        using ExeEffectCameras::Read;
        const std::vector<uint8_t>& data = image.data;
        const int64_t start = ExeEffectCameras::FileOffset(image.sections, address);
        if (start < 0 || static_cast<size_t>(start) + bytes > data.size()) return false;
        const uint32_t vertex_bytes = Read<uint32_t>(data, start);
        if (vertex_bytes == 0 || vertex_bytes % 8 != 0 || vertex_bytes + 4 > bytes) return false;
        mesh = Mesh();
        for (uint32_t i = 0; i < vertex_bytes / 8; ++ i){
            for (int c = 0; c < 3; ++ c) mesh.vertices.push_back(Read<int16_t>(data, start + 4 + 8 * i + 2 * c));
        }
        size_t position = start + 4 + vertex_bytes;
        const size_t end = start + bytes;
        static const int strides[4] = {16, 20, 20, 24};
        for (int list = 0; list < 4; ++ list){
            if (position + 4 > end) return false;
            const int16_t count = Read<int16_t>(data, position);
            position += 4;
            if (count < 0 || position + static_cast<size_t>(count) * strides[list] > end) return false;
            for (int i = 0; i < count; ++ i, position += strides[list]){
                // The vertices are positions in bytes of the array, of 8 bytes each.
                int vertices[4];
                const int number = (list == 0 || list == 2) ? 3 : 4;
                for (int v = 0; v < number; ++ v){
                    vertices[v] = Read<int16_t>(data, position + 2 * v);
                    if (vertices[v] < 0 || vertices[v] % 8 != 0 || vertices[v] / 8 >= static_cast<int>(vertex_bytes / 8)) return false;
                    vertices[v] /= 8;
                }
                if (list == 0){
                    // 3 vertices, grey at +6, UV of the vertices at +8, +12 and +14.
                    mesh.triangles.insert(mesh.triangles.end(), {
                      static_cast<int16_t>(vertices[0]), static_cast<int16_t>(vertices[1]), static_cast<int16_t>(vertices[2]),
                      data[position + 8], data[position + 9], data[position + 12], data[position + 13],
                      data[position + 14], data[position + 15], data[position + 6]
                    });
                }
                else if (list == 1){
                    // 4 vertices, UV at +8, +12, +14 and +16, grey at +18. Two triangles.
                    const int16_t uv[4][2] = {
                      {data[position + 8], data[position + 9]}, {data[position + 12], data[position + 13]},
                      {data[position + 14], data[position + 15]}, {data[position + 16], data[position + 17]}
                    };
                    const int order[2][3] = {{0, 1, 2}, {1, 3, 2}};
                    for (int t = 0; t < 2; ++ t){
                        mesh.triangles.push_back(static_cast<int16_t>(vertices[order[t][0]]));
                        mesh.triangles.push_back(static_cast<int16_t>(vertices[order[t][1]]));
                        mesh.triangles.push_back(static_cast<int16_t>(vertices[order[t][2]]));
                        for (int v = 0; v < 3; ++ v){
                            mesh.triangles.push_back(uv[order[t][v]][0]);
                            mesh.triangles.push_back(uv[order[t][v]][1]);
                        }
                        mesh.triangles.push_back(data[position + 18]);
                    }
                }
                else{
                    // Colours (r, g, b and a code) at +8, +12, +16 (and +20 for quads).
                    const int offsets[4] = {8, 12, 16, 20};
                    auto colour = [&](const int v){
                        for (int c = 0; c < 3; ++ c) mesh.gouraud.push_back(data[position + offsets[v] + c]);
                    };
                    const int order[2][3] = {{0, 1, 2}, {1, 3, 2}};
                    const int triangles = list == 2 ? 1 : 2;
                    for (int t = 0; t < triangles; ++ t){
                        for (int v = 0; v < 3; ++ v) mesh.gouraud.push_back(static_cast<int16_t>(vertices[order[t][v]]));
                        for (int v = 0; v < 3; ++ v) colour(order[t][v]);
                    }
                }
            }
        }
        // The lists have to end where the block ends.
        return position == end;
    }

    /**
     * Reads a block of the shards of Shiva's ice (the ones the ice breaks into, at 0x8aa590 and the
     * next ones of the 1998 executable): the size of the vertices, the vertices, a list of
     * triangles with texture of 20 bytes (flags 172) and a list of quads with texture of 24 bytes
     * (flags 0). Triangles: 3 vertices (positions in bytes), the colour word at +8 (r, g, b and a
     * code), the UV of the vertices at +12, +16 and +18. Quads: 4 vertices, the colour word at +8,
     * the UV at +12, +16, +18 and +20. The effect breaks only the triangles (function 0x591455).
     *
     * @param size Size in bytes of the block, if it is one.
     * @return False if it isn't a block of this kind.
     */
    inline bool ParseShardBlock(
      const ExeEffectCameras::Image& image, const uint32_t address, Mesh& mesh, uint32_t& size
    ){
        using ExeEffectCameras::Read;
        const std::vector<uint8_t>& data = image.data;
        const int64_t start = ExeEffectCameras::FileOffset(image.sections, address);
        if (start < 0 || static_cast<size_t>(start) + 16 > data.size()) return false;
        const uint32_t vertex_bytes = Read<uint32_t>(data, start);
        if (vertex_bytes == 0 || vertex_bytes % 8 != 0 || vertex_bytes < 64 || vertex_bytes > 64000) return false;
        const uint32_t vertices = vertex_bytes / 8;
        size_t position = static_cast<size_t>(start) + 4 + vertex_bytes;
        if (position + 4 > data.size()) return false;
        const int16_t triangles = Read<int16_t>(data, position);
        if (triangles < 8 || triangles > 1500 || Read<int16_t>(data, position + 2) != 172) return false;
        position += 4;
        if (position + static_cast<size_t>(triangles) * 20 + 4 > data.size()) return false;
        mesh = Mesh();
        for (uint32_t i = 0; i < vertices; ++ i){
            for (int c = 0; c < 3; ++ c) mesh.vertices.push_back(Read<int16_t>(data, start + 4 + 8 * i + 2 * c));
        }
        auto vertex = [&](const size_t at, const int number, int* out){
            for (int v = 0; v < number; ++ v){
                const int value = Read<int16_t>(data, at + 2 * v);
                if (value < 0 || value % 8 != 0 || value / 8 >= static_cast<int>(vertices)) return false;
                out[v] = value / 8;
            }
            return true;
        };
        for (int i = 0; i < triangles; ++ i, position += 20){
            int v[3];
            if (!vertex(position, 3, v)) return false;
            mesh.triangles.insert(mesh.triangles.end(), {
              static_cast<int16_t>(v[0]), static_cast<int16_t>(v[1]), static_cast<int16_t>(v[2]),
              data[position + 12], data[position + 13], data[position + 16], data[position + 17],
              data[position + 18], data[position + 19], data[position + 8]
            });
        }
        mesh.shards = triangles;
        const int16_t quads = Read<int16_t>(data, position);
        if (quads < 0 || quads > 400 || Read<int16_t>(data, position + 2) != 0) return false;
        position += 4;
        if (position + static_cast<size_t>(quads) * 24 > data.size()) return false;
        for (int i = 0; i < quads; ++ i, position += 24){
            int v[4];
            if (!vertex(position, 4, v)) return false;
            const int16_t uv[4][2] = {
              {data[position + 12], data[position + 13]}, {data[position + 16], data[position + 17]},
              {data[position + 18], data[position + 19]}, {data[position + 20], data[position + 21]}
            };
            const int order[2][3] = {{0, 1, 2}, {1, 3, 2}};
            for (int t = 0; t < 2; ++ t){
                for (int k = 0; k < 3; ++ k) mesh.triangles.push_back(static_cast<int16_t>(v[order[t][k]]));
                for (int k = 0; k < 3; ++ k){
                    mesh.triangles.push_back(uv[order[t][k]][0]);
                    mesh.triangles.push_back(uv[order[t][k]][1]);
                }
                mesh.triangles.push_back(data[position + 8]);
            }
        }
        size = static_cast<uint32_t>(position - start);
        return true;
    }

    /**
     * Gets the size of a block of a mesh that starts at an address, walking its lists of polygons.
     *
     * @return 0 if it isn't a mesh (the sizes or the vertices of the polygons are not valid).
     */
    inline uint32_t BlockSize(const ExeEffectCameras::Image& image, const uint32_t address){
        using ExeEffectCameras::Read;
        const std::vector<uint8_t>& data = image.data;
        const int64_t start = ExeEffectCameras::FileOffset(image.sections, address);
        if (start < 0 || static_cast<size_t>(start) + 8 > data.size()) return 0;
        const uint32_t vertex_bytes = Read<uint32_t>(data, start);
        if (vertex_bytes == 0 || vertex_bytes % 8 != 0 || vertex_bytes > 64000) return 0;
        const int vertices = vertex_bytes / 8;
        size_t position = static_cast<size_t>(start) + 4 + vertex_bytes;
        static const int strides[4] = {16, 20, 20, 24};
        static const int numbers[4] = {3, 4, 3, 4};
        int total = 0;
        for (int list = 0; list < 4; ++ list){
            if (position + 4 > data.size()) return 0;
            const int16_t count = Read<int16_t>(data, position);
            position += 4;
            if (count < 0 || count > 4000 || position + static_cast<size_t>(count) * strides[list] > data.size()) return 0;
            for (int i = 0; i < count; ++ i){
                for (int v = 0; v < numbers[list]; ++ v){
                    const int vertex = Read<int16_t>(data, position + static_cast<size_t>(i) * strides[list] + 2 * v);
                    if (vertex < 0 || vertex % 8 != 0 || vertex / 8 >= vertices) return 0;
                }
            }
            position += static_cast<size_t>(count) * strides[list];
            total += count;
        }
        // Small blocks are too easy to find by chance.
        if (vertices < 8 || total < 8) return 0;
        return static_cast<uint32_t>(position - start);
    }

    /**
     * Gets the data of an image, to know if it looks like the vertices of a mesh.
     */
    inline bool LooksLike(
      const ExeEffectCameras::Image& image, const Mesh& mesh, const uint32_t address
    ){
        using ExeEffectCameras::Read;
        const int64_t start = ExeEffectCameras::FileOffset(image.sections, address);
        const size_t count = mesh.vertices.size() / 3;
        if (start < 0 || static_cast<size_t>(start) + count * 8 > image.data.size()) return false;
        size_t matching = 0;
        for (size_t i = 0; i < count; ++ i){
            const int x = Read<int16_t>(image.data, start + 8 * i);
            const int z = Read<int16_t>(image.data, start + 8 * i + 4);
            const int pad = Read<int16_t>(image.data, start + 8 * i + 6);
            if (pad == 0 && std::abs(x - mesh.vertices[3 * i]) < 300 && std::abs(z - mesh.vertices[3 * i + 2]) < 500) ++ matching;
        }
        return matching * 2 >= count;
    }

    /**
     * Extracts the meshes of the effects.
     *
     * @param[in] path Path to ff7.exe.
     * @param[out] meshes The meshes, by the address of their block in the executable.
     * @return False if the file couldn't be read or it isn't a 32 bit executable.
     */
    inline bool Extract(const std::string& path, std::map<uint32_t, Mesh>& meshes){
        using ExeEffectCameras::Read;
        meshes.clear();
        ExeEffectCameras::Image image;
        if (!ExeEffectCameras::LoadImage(path, image)) return false;
        const std::vector<uint8_t>& data = image.data;
        const ExeEffectCameras::Section& code = image.code;
        // mov ecx, N ; mov esi, S ; mov edi, D ; rep movsd
        std::vector<uint32_t> copies; // The places the blocks are copied to: working copies, not meshes.
        for (size_t i = 0; i + 17 <= code.file_size; ++ i){
            const size_t o = code.file_offset + i;
            if (data[o] != 0xb9 || data[o + 5] != 0xbe || data[o + 10] != 0xbf || data[o + 15] != 0xf3 || data[o + 16] != 0xa5) continue;
            const uint32_t count = Read<uint32_t>(data, o + 1);
            const uint32_t source = Read<uint32_t>(data, o + 6);
            copies.push_back(Read<uint32_t>(data, o + 11));
            Mesh mesh;
            if (count < 64 || !ParseBlock(image, source, count * 4, mesh) || mesh.vertices.empty()) continue;
            // The table of the key shapes, in a section of data that isn't code.
            const size_t vertices = mesh.vertices.size() / 3;
            const uint32_t vertex_bytes = static_cast<uint32_t>(vertices * 8);
            bool found = false;
            for (const ExeEffectCameras::Section& section : image.sections){
                if (section.code || found) continue;
                for (uint32_t k = 0; k + 8 <= section.file_size && !found; k += 4){
                    const size_t entry = section.file_offset + k;
                    const uint32_t first = Read<uint32_t>(data, entry);
                    if (first == source || !LooksLike(image, mesh, first)) continue;
                    // The shapes are one after the other.
                    std::vector<uint32_t> table = {first};
                    while (entry + 4 * (table.size() + 1) <= data.size()){
                        const uint32_t next = Read<uint32_t>(data, entry + 4 * table.size());
                        // Only the first one has to be like the mesh: some shapes aren't used.
                        if (next != table.back() + vertex_bytes) break;
                        const int64_t at_next = ExeEffectCameras::FileOffset(image.sections, next);
                        if (at_next < 0 || static_cast<size_t>(at_next) + vertex_bytes > data.size()) break;
                        table.push_back(next);
                    }
                    if (table.size() < 2) continue;
                    // The first one can be followed by shapes that aren't in the table: the table is
                    // what is a pointer at a position of the section.
                    found = true;
                    for (const uint32_t pointer : table){
                        const int64_t at = ExeEffectCameras::FileOffset(image.sections, pointer);
                        std::vector<int16_t> shape;
                        for (size_t v = 0; v < vertices; ++ v){
                            for (int c = 0; c < 3; ++ c) shape.push_back(Read<int16_t>(data, at + 8 * v + 2 * c));
                        }
                        mesh.keys.push_back(shape);
                    }
                    for (int w = 0; w < 16; ++ w) mesh.tail.push_back(Read<int16_t>(data, entry + 4 * table.size() + 2 * w));
                }
            }
            if (found) meshes[source] = mesh;
        }
        // The meshes that don't change, that aren't copied by the code: blocks of the data that fit
        // exactly. The working copies of the others have the same polygons as their template.
        for (const ExeEffectCameras::Section& section : image.sections){
            if (section.code) continue;
            for (uint32_t k = 0; k + 16 <= section.file_size;){
                const uint32_t address = section.address + k;
                const uint32_t size = BlockSize(image, address);
                if (size == 0){
                    k += 4;
                    continue;
                }
                bool is_copy = false;
                for (const uint32_t copy : copies) is_copy = is_copy || copy == address;
                Mesh mesh;
                if (!is_copy && meshes.count(address) == 0 && ParseBlock(image, address, size, mesh)) meshes[address] = mesh;
                k += size;
            }
        }
        // The shards of the ice of Shiva.
        for (const ExeEffectCameras::Section& section : image.sections){
            if (section.code) continue;
            for (uint32_t k = 0; k + 16 <= section.file_size; k += 4){
                const uint32_t address = section.address + k;
                if (meshes.count(address) != 0) continue;
                Mesh mesh;
                uint32_t size = 0;
                if (ParseShardBlock(image, address, mesh, size)) meshes[address] = mesh;
            }
        }
        return true;
    }

    /**
     * Writes the meshes as Lua tables: Game.EffectMeshes["0x8af9b0"] = {...}.
     */
    inline void WriteLua(std::ostream& out, const std::map<uint32_t, Mesh>& meshes){
        auto list = [&](const std::vector<int16_t>& values, const size_t per_line){
            for (size_t i = 0; i < values.size(); ++ i){
                if (i % per_line == 0) out << "\n        ";
                out << values[i] << ", ";
            }
        };
        out << "-- Meshes of the battle effects that are in the data of ff7.exe, with the vertices of the key shapes\n"
            << "-- they change through. Game.EffectMeshes[address] has the vertices of the first shape (x, y, z, in\n"
            << "-- the units of the effects, y goes down), the key shapes, the 16 words that follow the table of\n"
            << "-- shapes (some effects have the durations of the moves there), the triangles with texture (3 vertex\n"
            << "-- numbers, the UV of each vertex in bytes of the texture page and the grey level, 128 is 1.0) and the\n"
            << "-- triangles with colours (3 vertex numbers and the r, g, b of each vertex).\n"
            << "Game.EffectMeshes = Game.EffectMeshes or {}\n";
        for (const auto& item : meshes){
            const Mesh& m = item.second;
            out << "Game.EffectMeshes[\"0x" << std::hex << item.first << std::dec << "\"] = {\n    vertices = {";
            list(m.vertices, 24);
            out << "\n    },\n    keys = {\n";
            for (const std::vector<int16_t>& key : m.keys){
                out << "        {";
                list(key, 24);
                out << "\n        },\n";
            }
            out << "    },\n    tail = {";
            list(m.tail, 16);
            out << "\n    },\n    triangles = {";
            list(m.triangles, 20);
            out << "\n    },\n    gouraud = {";
            list(m.gouraud, 24);
            out << "\n    },\n";
            if (m.shards > 0) out << "    shards = " << m.shards << ",\n";
            out << "}\n";
        }
    }

}
