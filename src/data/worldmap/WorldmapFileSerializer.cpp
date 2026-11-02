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

#include "WorldmapFileSerializer.h"

#include <OgreException.h>
#include <string>
#include "common/LzsFile.h"

namespace VGears {

    namespace {

        const size_t WORLD_MAP_BLOCK_SIZE = 0xB800;
        const size_t WORLD_MAP_BLOCK_HEADER_SIZE = sizeof(uint32) * 16;
        const size_t WORLD_MAP_TRIANGLE_SIZE = 12;
        const size_t WORLD_MAP_VERTEX_SIZE = 8;

        uint16 ReadLittleEndian16(const std::vector<uint8>& data, size_t offset){
            return static_cast<uint16>(data[offset])
              | (static_cast<uint16>(data[offset + 1]) << 8);
        }

        uint32 ReadLittleEndian32(const std::vector<uint8>& data, size_t offset){
            return static_cast<uint32>(data[offset])
              | (static_cast<uint32>(data[offset + 1]) << 8)
              | (static_cast<uint32>(data[offset + 2]) << 16)
              | (static_cast<uint32>(data[offset + 3]) << 24);
        }

        sint16 ReadLittleEndianSigned16(
          const std::vector<uint8>& data, size_t offset
        ){
            const uint16 value = ReadLittleEndian16(data, offset);
            const sint32 signed_value = value <= 0x7FFF
              ? static_cast<sint32>(value)
              : static_cast<sint32>(value) - 0x10000;
            return static_cast<sint16>(signed_value);
        }

        void ThrowInvalidWorldmap(const std::string& reason){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "Invalid world map file: " + reason,
              "WorldmapFileSerializer::ImportWorldmapFile"
            );
        }

        void ValidateLzsData(
          const std::vector<uint8>& data, size_t max_output_size
        ){
            if (data.size() <= 4
              || ReadLittleEndian32(data, 0) != data.size() - 4){
                ThrowInvalidWorldmap("invalid LZS payload size");
            }

            size_t input_offset = 4;
            size_t output_size = 0;
            while (input_offset < data.size()){
                const uint8 control = data[input_offset++];
                bool has_token = false;
                for (unsigned int bit = 0;
                  bit < 8 && input_offset < data.size(); ++bit){
                    has_token = true;
                    size_t token_output_size = 1;
                    if ((control & (1u << bit)) == 0){
                        if (data.size() - input_offset < 2){
                            ThrowInvalidWorldmap("truncated LZS back-reference");
                        }
                        token_output_size
                          = static_cast<size_t>(data[input_offset + 1] & 0x0F) + 3;
                        input_offset += 2;
                    }
                    else ++input_offset;

                    if (token_output_size > max_output_size - output_size){
                        ThrowInvalidWorldmap("LZS output exceeds a MAP block");
                    }
                    output_size += token_output_size;
                }
                if (!has_token){
                    ThrowInvalidWorldmap("truncated LZS control byte");
                  }
            }
        }

    }

    WorldmapFileSerializer::WorldmapFileSerializer(){}

    WorldmapFileSerializer::~WorldmapFileSerializer(){}

    void WorldmapFileSerializer::ImportWorldmapFile(
      Ogre::DataStreamPtr& stream, WorldmapFile& dest
    ){
        static_cast<void>(dest);
        if (stream.isNull()){
            ThrowInvalidWorldmap("input stream is null");
        }

        const size_t file_size = stream->size();
        if (file_size == 0 || file_size % WORLD_MAP_BLOCK_SIZE != 0){
            ThrowInvalidWorldmap("file size is not a whole number of MAP blocks");
        }

        std::vector<uint8> file_data(file_size);
        stream->seek(0);
        if (stream->read(file_data.data(), file_data.size()) != file_data.size()){
            ThrowInvalidWorldmap("could not read the complete file");
        }

        std::vector<Block> parsed_blocks;
        const size_t block_count = file_size / WORLD_MAP_BLOCK_SIZE;
        parsed_blocks.reserve(block_count);
        for (size_t block_index = 0; block_index < block_count; ++block_index){
            Block block;
            const size_t block_start = block_index * WORLD_MAP_BLOCK_SIZE;
            block.meshes.reserve(16);
            for (size_t mesh_index = 0; mesh_index < 16; ++mesh_index){
                const uint32 mesh_offset = ReadLittleEndian32(
                  file_data, block_start + mesh_index * sizeof(uint32)
                );
                if (mesh_offset < WORLD_MAP_BLOCK_HEADER_SIZE
                  || mesh_offset % 4 != 0
                  || mesh_offset > WORLD_MAP_BLOCK_SIZE - sizeof(uint32)){
                    ThrowInvalidWorldmap("mesh offset is invalid");
                }

                const size_t mesh_start = block_start + mesh_offset;
                const uint32 compressed_data_size
                  = ReadLittleEndian32(file_data, mesh_start);
                const size_t max_compressed_size
                  = WORLD_MAP_BLOCK_SIZE - mesh_offset - sizeof(uint32);
                if (compressed_data_size > max_compressed_size){
                    ThrowInvalidWorldmap("compressed mesh extends past its MAP block");
                }

                const size_t compressed_size
                  = static_cast<size_t>(compressed_data_size) + sizeof(uint32);
                std::vector<uint8> compressed_data(
                  file_data.begin() + mesh_start,
                  file_data.begin() + mesh_start + compressed_size
                );
                ValidateLzsData(compressed_data, WORLD_MAP_BLOCK_SIZE);
                const std::vector<uint8> decompressed
                  = LzsBuffer::Decompress(compressed_data);
                if (decompressed.size() < 4){
                    ThrowInvalidWorldmap("decompressed mesh header is truncated");
                }

                BlockPart block_part;
                block_part.header.triengle_count
                  = ReadLittleEndian16(decompressed, 0);
                block_part.header.vertex_count
                  = ReadLittleEndian16(decompressed, 2);
                const size_t expected_size = 4
                  + static_cast<size_t>(block_part.header.triengle_count)
                    * WORLD_MAP_TRIANGLE_SIZE
                  + static_cast<size_t>(block_part.header.vertex_count)
                    * WORLD_MAP_VERTEX_SIZE * 2;
                if (expected_size > decompressed.size()){
                    ThrowInvalidWorldmap("decompressed mesh data is truncated");
                }

                size_t data_offset = 4;
                block_part.triangles.resize(block_part.header.triengle_count);
                for (size_t triangle_index = 0;
                  triangle_index < block_part.triangles.size(); ++triangle_index){
                    BlockTriangle& triangle = block_part.triangles[triangle_index];
                    triangle.vertex_0_index = decompressed[data_offset++];
                    triangle.vertex_1_index = decompressed[data_offset++];
                    triangle.vertex_2_index = decompressed[data_offset++];
                    const uint8 walkmap_and_function = decompressed[data_offset++];
                    triangle.walkability_info_ = walkmap_and_function & 0x1F;
                    triangle.unknown = (walkmap_and_function >> 5) & 0x07;
                    triangle.vertex_0_u = decompressed[data_offset++];
                    triangle.vertex_0_v = decompressed[data_offset++];
                    triangle.vertex_1_u = decompressed[data_offset++];
                    triangle.vertex_1_v = decompressed[data_offset++];
                    triangle.vertex_2_u = decompressed[data_offset++];
                    triangle.vertex_2_v = decompressed[data_offset++];
                    const uint16 texture_and_location
                      = ReadLittleEndian16(decompressed, data_offset);
                    data_offset += sizeof(uint16);
                    triangle.TextureInfo = texture_and_location & 0x1FF;
                    triangle.Location = (texture_and_location >> 9) & 0x7F;

                    if (triangle.vertex_0_index >= block_part.header.vertex_count
                      || triangle.vertex_1_index >= block_part.header.vertex_count
                      || triangle.vertex_2_index >= block_part.header.vertex_count){
                        ThrowInvalidWorldmap("triangle references a missing vertex");
                    }
                }

                block_part.vertices.resize(block_part.header.vertex_count);
                for (size_t vertex_index = 0;
                  vertex_index < block_part.vertices.size(); ++vertex_index){
                    Vertex& vertex = block_part.vertices[vertex_index];
                    vertex.x = ReadLittleEndianSigned16(decompressed, data_offset);
                    data_offset += sizeof(uint16);
                    vertex.y = ReadLittleEndianSigned16(decompressed, data_offset);
                    data_offset += sizeof(uint16);
                    vertex.z = ReadLittleEndianSigned16(decompressed, data_offset);
                    data_offset += sizeof(uint16);
                    vertex.unused = ReadLittleEndian16(decompressed, data_offset);
                    data_offset += sizeof(uint16);
                }

                block_part.normals.resize(block_part.header.vertex_count);
                for (size_t normal_index = 0;
                  normal_index < block_part.normals.size(); ++normal_index){
                    Normal& normal = block_part.normals[normal_index];
                    normal.x = ReadLittleEndianSigned16(decompressed, data_offset);
                    data_offset += sizeof(uint16);
                    normal.y = ReadLittleEndianSigned16(decompressed, data_offset);
                    data_offset += sizeof(uint16);
                    normal.z = ReadLittleEndianSigned16(decompressed, data_offset);
                    data_offset += sizeof(uint16);
                    normal.unused = ReadLittleEndian16(decompressed, data_offset);
                    data_offset += sizeof(uint16);
                }

                block.meshes.push_back(block_part);
            }
            parsed_blocks.push_back(block);
        }

        blocks.swap(parsed_blocks);
    }
}
