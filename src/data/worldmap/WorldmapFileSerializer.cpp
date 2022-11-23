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

#include "common/LzsFile.h"

namespace VGears {

    /**
     * Size of each block in a world map file.
     */
    const int WORLD_MAP_BLOCK_SIZE = 0xB800;

    WorldmapFileSerializer::WorldmapFileSerializer(){}

    WorldmapFileSerializer::~WorldmapFileSerializer(){}

    void WorldmapFileSerializer::ImportWorldmapFile(
      Ogre::DataStreamPtr& stream, WorldmapFile& dest
    ){
        const auto fileSize = stream->size();
        auto nublocks = fileSize / WORLD_MAP_BLOCK_SIZE;
        for (unsigned int j = 0; j < nublocks; j ++){
            Block block;
            const size_t basePos = WORLD_MAP_BLOCK_SIZE * j;
            stream->seek(basePos);
            // Read the offset to compressed data in this block.
            BlockHeader header = {};
            for (auto i = 0u; i < 16; i ++)
                ReadUInt32(stream, header.compressed_data_offsets[i]);
            for (auto i = 0u; i < 16; i ++){
                BlockPart block_part;
                // Go to the offset.
                stream->seek(basePos + header.compressed_data_offsets[i]);
                // Read the size of the compressed data.
                uint32 compressed_data_size = 0;
                ReadUInt32(stream, compressed_data_size);
                // Go back to before the compressed data size.
                stream->seek(basePos + header.compressed_data_offsets[i]);
                // Read the compressed data into a temp buffer, including the
                // compressed data size.
                std::vector<uint8> buffer(compressed_data_size + 4);
                stream->read(buffer.data(), buffer.size());
                // Decompress the data.
                auto decompressed = LzsBuffer::Decompress(buffer);
                Ogre::MemoryDataStream decompressed_stream(
                  decompressed.data(), decompressed.size(), false, true
                );
                ReadUInt16(
                  decompressed_stream, block_part.header.triengle_count
                );
                ReadUInt16(decompressed_stream, block_part.header.vertex_count);

                /*
                std::cout << "block: " << j
                  << " from offset " << header.compressed_data_offsets[i]
                  << " old size: " << buffer.size()
                  << " decompressed size is " << decompressed.size()
                  << " header is tris: " <<  block_part.header.triengle_count
                  << " verts " << block_part.header.vertex_count
                  << std::endl;
                */

                block_part.triangles.resize(block_part.header.triengle_count);
                for (int k = 0; k < block_part.header.triengle_count; k ++){
                    BlockTriangle& s = block_part.triangles[k];
                    ReadUInt8(decompressed_stream, s.vertex_0_index);
                    ReadUInt8(decompressed_stream, s.vertex_1_index);
                    ReadUInt8(decompressed_stream, s.vertex_2_index);
                    ReadUInt8(decompressed_stream, s.walkability_info_);
                    //ReadUInt8(decompressed_stream, s.unknown);
                    ReadUInt8(decompressed_stream, s.vertex_0_u);
                    ReadUInt8(decompressed_stream, s.vertex_0_v);
                    ReadUInt8(decompressed_stream, s.vertex_1_u);
                    ReadUInt8(decompressed_stream, s.vertex_1_v);
                    ReadUInt8(decompressed_stream, s.vertex_2_u);
                    ReadUInt8(decompressed_stream, s.vertex_2_v);
                    ReadUInt16(decompressed_stream, s.TextureInfo);
                    s.TextureInfo = s.TextureInfo & 0x1FF;
                    //ReadUInt16(decompressed_stream, s.Location);
                    /*
                    std::cout << "v0: " << int(s.vertex_0_index)
                      << " v1 " << int(s.vertex_1_index)
                      << " v2 " << int(s.vertex_2_index)
                      << " walk " << int(s.walkability_info_)
                      << " u1 " << int(s.vertex_1_u)
                      << " v1 " << int(s.vertex_1_v)
                      << " v2 " << int(s.vertex_2_u)
                      << " u2 " << int(s.vertex_2_v)
                      << " texture " << s.TextureInfo
                      << " locId " << s.Location
                      << std::endl;
                    */
                }
                block_part.normals.resize(block_part.header.vertex_count);
                block_part.vertices.resize(block_part.header.vertex_count);
                // All vertices.
                for (int k = 0; k < block_part.header.vertex_count; k ++){
                    Vertex& v = block_part.vertices[k];
                    ReadInt16(decompressed_stream, v.x);
                    ReadInt16(decompressed_stream, v.y);
                    ReadInt16(decompressed_stream, v.z);
                    ReadUInt16(decompressed_stream, v.unused);
                }
                // Then all normals.
                for (int k = 0; k < block_part.header.vertex_count; k ++){
                    Normal& n = block_part.normals[k];
                    ReadInt16(decompressed_stream, n.x);
                    ReadInt16(decompressed_stream, n.y);
                    ReadInt16(decompressed_stream, n.z);
                    ReadUInt16(decompressed_stream, n.unused);
                }
                block.meshes.push_back(block_part);
            }
            blocks.push_back(block);
        }
    }
}
