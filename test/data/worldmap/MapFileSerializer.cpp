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

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <vector>
#include "data/worldmap/WorldmapFile.h"
#include "../../../src/data/worldmap/WorldmapFileSerializer.h"

namespace {

        const size_t WORLD_MAP_BLOCK_SIZE = 0xB800;

        class TestWorldmapFile : public VGears::WorldmapFile{

                public:

                        TestWorldmapFile() : VGears::WorldmapFile(nullptr, "", 0, "") {}
        };

        void AppendUInt16(std::vector<VGears::uint8>& data, VGears::uint16 value){
                data.push_back(static_cast<VGears::uint8>(value & 0xFF));
                data.push_back(static_cast<VGears::uint8>(value >> 8));
        }

        void AppendUInt32(std::vector<VGears::uint8>& data, VGears::uint32 value){
                data.push_back(static_cast<VGears::uint8>(value & 0xFF));
                data.push_back(static_cast<VGears::uint8>((value >> 8) & 0xFF));
                data.push_back(static_cast<VGears::uint8>((value >> 16) & 0xFF));
                data.push_back(static_cast<VGears::uint8>((value >> 24) & 0xFF));
        }

        void WriteUInt32(
            std::vector<VGears::uint8>& data, size_t offset, VGears::uint32 value
        ){
                data[offset] = static_cast<VGears::uint8>(value & 0xFF);
                data[offset + 1] = static_cast<VGears::uint8>((value >> 8) & 0xFF);
                data[offset + 2] = static_cast<VGears::uint8>((value >> 16) & 0xFF);
                data[offset + 3] = static_cast<VGears::uint8>((value >> 24) & 0xFF);
        }

        VGears::uint32 ReadUInt32(
            const std::vector<VGears::uint8>& data, size_t offset
        ){
                return static_cast<VGears::uint32>(data[offset])
                    | (static_cast<VGears::uint32>(data[offset + 1]) << 8)
                    | (static_cast<VGears::uint32>(data[offset + 2]) << 16)
                    | (static_cast<VGears::uint32>(data[offset + 3]) << 24);
        }

        std::vector<VGears::uint8> CreateMeshData(
            bool truncated, bool invalid_vertex
        ){
                std::vector<VGears::uint8> data;
                AppendUInt16(data, 1);
                AppendUInt16(data, 3);
                if (truncated) return data;

                data.push_back(invalid_vertex ? 3 : 0);
                data.push_back(1);
                data.push_back(2);
                data.push_back(0xA5);
                data.push_back(10);
                data.push_back(20);
                data.push_back(30);
                data.push_back(40);
                data.push_back(50);
                data.push_back(60);
                AppendUInt16(data, 0xAB55);

                for (int vertex_index = 0; vertex_index < 3; ++vertex_index){
                        AppendUInt16(data, static_cast<VGears::uint16>(-100 + vertex_index));
                        AppendUInt16(data, static_cast<VGears::uint16>(200 + vertex_index));
                        AppendUInt16(data, static_cast<VGears::uint16>(-300 + vertex_index));
                        AppendUInt16(data, 0);
                }
                for (int normal_index = 0; normal_index < 3; ++normal_index){
                        AppendUInt16(data, static_cast<VGears::uint16>(normal_index + 1));
                        AppendUInt16(data, static_cast<VGears::uint16>(normal_index + 2));
                        AppendUInt16(data, static_cast<VGears::uint16>(normal_index + 3));
                        AppendUInt16(data, 0);
                }
                return data;
        }

        std::vector<VGears::uint8> EncodeLzs(
            const std::vector<VGears::uint8>& data
        ){
                std::vector<VGears::uint8> payload;
                size_t data_offset = 0;
                while (data_offset < data.size()){
                        const size_t control_offset = payload.size();
                        payload.push_back(0);
                        VGears::uint8 control = 0;
                        for (unsigned int bit = 0; bit < 8 && data_offset < data.size(); ++bit){
                                control |= static_cast<VGears::uint8>(1u << bit);
                                payload.push_back(data[data_offset++]);
                        }
                        payload[control_offset] = control;
                }

                std::vector<VGears::uint8> compressed;
                AppendUInt32(compressed, static_cast<VGears::uint32>(payload.size()));
                compressed.insert(compressed.end(), payload.begin(), payload.end());
                return compressed;
        }

        std::vector<VGears::uint8> CreateMapFixture(
            bool truncated_mesh = false, bool invalid_vertex = false
        ){
                std::vector<VGears::uint8> data(WORLD_MAP_BLOCK_SIZE, 0);
                size_t write_offset = sizeof(VGears::uint32) * 16;
                for (size_t mesh_index = 0; mesh_index < 16; ++mesh_index){
                        const std::vector<VGears::uint8> mesh_data = mesh_index == 0
                            ? CreateMeshData(truncated_mesh, invalid_vertex)
                            : std::vector<VGears::uint8>{0, 0, 0, 0};
                        const std::vector<VGears::uint8> compressed = EncodeLzs(mesh_data);
                        write_offset = (write_offset + 3) & ~static_cast<size_t>(3);
                        WriteUInt32(data, mesh_index * sizeof(VGears::uint32),
                            static_cast<VGears::uint32>(write_offset));
                        std::copy(compressed.begin(), compressed.end(), data.begin() + write_offset);
                        write_offset += compressed.size();
                }
                return data;
        }

        void ImportFixture(std::vector<VGears::uint8>& data){
                VGears::WorldmapFileSerializer serializer;
                TestWorldmapFile file;
                Ogre::DataStreamPtr stream(OGRE_NEW Ogre::MemoryDataStream(
                    data.data(), data.size(), false, true
                ));
                serializer.ImportWorldmapFile(stream, file);
        }

}

BOOST_AUTO_TEST_CASE(TestMapFileSerializerReadsPackedTriangleFields){
        std::vector<VGears::uint8> data = CreateMapFixture();
        VGears::WorldmapFileSerializer serializer;
        TestWorldmapFile file;
        Ogre::DataStreamPtr stream(OGRE_NEW Ogre::MemoryDataStream(
            data.data(), data.size(), false, true
        ));

        serializer.ImportWorldmapFile(stream, file);

        BOOST_REQUIRE_EQUAL(serializer.blocks.size(), 1u);
        BOOST_REQUIRE_EQUAL(serializer.blocks[0].meshes.size(), 16u);
        const VGears::WorldmapFileSerializer::BlockPart& part
            = serializer.blocks[0].meshes[0];
        BOOST_CHECK_EQUAL(part.header.triengle_count, 1);
        BOOST_CHECK_EQUAL(part.header.vertex_count, 3);
        BOOST_REQUIRE_EQUAL(part.triangles.size(), 1u);
        const VGears::WorldmapFileSerializer::BlockTriangle& triangle
            = part.triangles[0];
        BOOST_CHECK_EQUAL(triangle.walkability_info_, 5);
                BOOST_CHECK_EQUAL(triangle.function_id, 5);
        BOOST_CHECK_EQUAL(triangle.vertex_0_u, 10);
        BOOST_CHECK_EQUAL(triangle.vertex_0_v, 20);
        BOOST_CHECK_EQUAL(triangle.vertex_1_u, 30);
        BOOST_CHECK_EQUAL(triangle.vertex_1_v, 40);
        BOOST_CHECK_EQUAL(triangle.vertex_2_u, 50);
        BOOST_CHECK_EQUAL(triangle.vertex_2_v, 60);
        BOOST_CHECK_EQUAL(triangle.TextureInfo, 0x155);
        BOOST_CHECK_EQUAL(triangle.Location, 0x55);
        BOOST_REQUIRE_EQUAL(part.vertices.size(), 3u);
        BOOST_CHECK_EQUAL(part.vertices[0].x, -100);
        BOOST_CHECK_EQUAL(part.vertices[0].y, 200);
        BOOST_CHECK_EQUAL(part.vertices[0].z, -300);
        BOOST_REQUIRE_EQUAL(part.normals.size(), 3u);
        BOOST_CHECK_EQUAL(part.normals[0].x, 1);
        BOOST_CHECK_EQUAL(part.normals[0].y, 2);
        BOOST_CHECK_EQUAL(part.normals[0].z, 3);
}

BOOST_AUTO_TEST_CASE(TestMapFileSerializerRejectsPartialBlocks){
        std::vector<VGears::uint8> data = CreateMapFixture();
        data.pop_back();
        BOOST_CHECK_THROW(ImportFixture(data), Ogre::Exception);
}

BOOST_AUTO_TEST_CASE(TestMapFileSerializerRejectsInvalidMeshOffsets){
        std::vector<VGears::uint8> data = CreateMapFixture();
        WriteUInt32(data, 0, static_cast<VGears::uint32>(WORLD_MAP_BLOCK_SIZE - 2));
        BOOST_CHECK_THROW(ImportFixture(data), Ogre::Exception);
}

BOOST_AUTO_TEST_CASE(TestMapFileSerializerRejectsMeshesOutsideBlock){
        std::vector<VGears::uint8> data = CreateMapFixture();
        const size_t mesh_offset = WORLD_MAP_BLOCK_SIZE - sizeof(VGears::uint32);
        WriteUInt32(data, 0, static_cast<VGears::uint32>(mesh_offset));
        WriteUInt32(data, mesh_offset, 1);
        BOOST_CHECK_THROW(ImportFixture(data), Ogre::Exception);
}

BOOST_AUTO_TEST_CASE(TestMapFileSerializerRejectsTruncatedLzsTokens){
        std::vector<VGears::uint8> data = CreateMapFixture();
        const size_t mesh_offset = ReadUInt32(data, 0);
        WriteUInt32(data, mesh_offset, 2);
        data[mesh_offset + sizeof(VGears::uint32)] = 0;
        data[mesh_offset + sizeof(VGears::uint32) + 1] = 0x10;
        BOOST_CHECK_THROW(ImportFixture(data), Ogre::Exception);
}

BOOST_AUTO_TEST_CASE(TestMapFileSerializerRejectsTruncatedMeshes){
        std::vector<VGears::uint8> data = CreateMapFixture(true);
        BOOST_CHECK_THROW(ImportFixture(data), Ogre::Exception);
}

BOOST_AUTO_TEST_CASE(TestMapFileSerializerRejectsInvalidVertexIndices){
        std::vector<VGears::uint8> data = CreateMapFixture(false, true);
        BOOST_CHECK_THROW(ImportFixture(data), Ogre::Exception);
}

