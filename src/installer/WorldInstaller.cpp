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

#include <iostream>
#include <fstream>
#include <OgreRoot.h>
#include <OgreMesh.h>
#include <OgreMeshSerializer.h>
#include <boost/filesystem.hpp>
#include "WorldInstaller.h"
#include "common/Lzs.h"

WorldInstaller::WorldInstaller(
  std::string input_dir, std::string output_dir, const bool keep_originals
):
  input_dir_(input_dir), output_dir_(output_dir), keep_originals_(keep_originals)
{}

WorldInstaller::~WorldInstaller(){}

unsigned int WorldInstaller::Initialize(){
    if (wm_map_.size() > 0) wm_map_.clear();
    processed_maps_ = 0;
    wm_map_.push_back(File(input_dir_ + "/data/wm/WM0.MAP"));
    wm_map_.push_back(File(input_dir_ + "/data/wm/WM2.MAP"));
    wm_map_.push_back(File(input_dir_ + "/data/wm/WM3.MAP"));
    return wm_map_.size();
}

bool WorldInstaller::ProcessMap(){
    //std::cout << "[WI] Processing map " << processed_maps_ << "/" << wm_map_.size() << std::endl;
    if (processed_maps_ >= wm_map_.size()) return true;
    // Read WM*.MAP file.
    Map map;
    Ogre::MeshSerializer mesh_serializer;
    boost::filesystem::path p(wm_map_[processed_maps_].GetFileName());
    std::string map_name = p.stem().native(); // w/o path or extension.

    for (int b = 0; b < wm_map_[processed_maps_].GetFileSize() / 0xB800; b ++){
        //std::cout << "[WI]    Block " << b << std::endl;
        Block block;
        for (int m = 0; m < 16; m ++){

            // Extract lzss compressed mesh to file.
            std::vector<unsigned char> lzss;
            wm_map_[processed_maps_].SetOffset(0xB800 * b + m * 4);
            u32 mesh_offset = 0xB800 * b + wm_map_[processed_maps_].readU32LE();
            wm_map_[processed_maps_].SetOffset(mesh_offset);
            u32 mesh_size = wm_map_[processed_maps_].readU32LE();

            // Add size to lzss
            wm_map_[processed_maps_].SetOffset(mesh_offset);
            for (int d = 0; d < 4; d ++)
                lzss.push_back(wm_map_[processed_maps_].readU8());

            //std::cout << "[WI]        Mesh " << m << " Size: " << mesh_size
            //  << " Offset: " << mesh_offset << " - 0x";
            //std::cout << std::hex << mesh_offset;
            //std::cout << std::dec << std::endl;

            for (int d = 0; d < mesh_size ; d ++) lzss.push_back(wm_map_[processed_maps_].readU8());

            //std::cout << "            LZSS size: " << lzss.size() << std::endl;

            // Extract the lzss file.
            std::vector<unsigned char> data = Lzs::Decompress(lzss);
            if (data.size() == 0) continue;
            std::string dat_name = output_dir_ + "temp/wm/" + map_name + "_"
              + std::to_string(b) + "_" + std::to_string(m) + ".dat";
            std::ofstream dat(dat_name, std::ios::out | std::ios::binary);
            if (!dat){
                std::cerr << "Cannot create temporary world map file " << dat_name << std::endl;
                processed_maps_ ++;
                if (processed_maps_ >= wm_map_.size()) return true;
                else return false;
            }
            for (int d = 0; d <= data.size(); d ++)
                dat.write((char *) &data[d], sizeof(unsigned char));
            dat.close();

            // Open and read the decompressed file.
            File mesh_data(dat_name);
            block.mesh[m].triangle_count = mesh_data.readU16LE();
            block.mesh[m].vertex_count = mesh_data.readU16LE();
            for (int tri = 0; tri < block.mesh[m].triangle_count; tri ++){
                Triangle t;
                t.vertex_index[0] = mesh_data.readU8();
                t.vertex_index[1] = mesh_data.readU8();
                t.vertex_index[2] = mesh_data.readU8();
                u8 walkability_function = mesh_data.readU8();
                t.walkability = walkability_function >> 5; // 5 bits
                t.function_id = walkability_function & 0x7; // 3 bits
                t.vertex_coord[0].u = mesh_data.readU8();
                t.vertex_coord[0].v = mesh_data.readU8();
                t.vertex_coord[1].u = mesh_data.readU8();
                t.vertex_coord[1].v = mesh_data.readU8();
                t.vertex_coord[2].u = mesh_data.readU8();
                t.vertex_coord[2].v = mesh_data.readU8();
                u16 texture_location = mesh_data.readU16LE();
                t.texture_info = texture_location >> 9; // 9 bytes
                t.location = texture_location & ((1 << 7) - 1); // 7 bytes.
                block.mesh[m].triangles.push_back(t);
            }
            for (int ver = 0; ver < block.mesh[m].vertex_count; ver ++){
                Vertex v;
                v.x = mesh_data.readU16LE();
                v.y = mesh_data.readU16LE();
                v.z = mesh_data.readU16LE();
                v.unknown = mesh_data.readU16LE();
                block.mesh[m].vertices.push_back(v);
            }
            for (int nor = 0; nor < block.mesh[m].vertex_count; nor ++){
                Vertex v;
                v.x = mesh_data.readU16LE();
                v.y = mesh_data.readU16LE();
                v.z = mesh_data.readU16LE();
                v.unknown = mesh_data.readU16LE();
                block.mesh[m].normals.push_back(v);
            }
        }
        map.blocks.push_back(block);
    }
    // TODO: Generate material.
    // Generate manual meshes.
    for (int b = 0; b < map.blocks.size(); b ++){
        //std::cout << "[MAN] Block " << b << "/" << map.blocks.size() << std::endl;

        std::string name = map_name + "_" + std::to_string(b);
        Ogre::ManualObject man = Ogre::ManualObject(name);
        //man.setBoundingBox(Ogre::AxisAlignedBox({-100,-100,0}, {100,100,0})); //TODO;
          //= Ogre::Root::getSingleton().getSceneManager("Scene")->createManualObject(name);
        man.begin(name, Ogre::RenderOperation::OT_TRIANGLE_LIST);
        for (int m = 0; m < 16; m ++){
            //std::cout << "[MAN]        Mesh " << m << " triangles: "
            //  << map.blocks[b].mesh[m].triangle_count << "\n";
            // Add vertices.
            for (int v = 0; v < map.blocks[b].mesh[m].vertex_count; v ++){
                man.position(
                  map.blocks[b].mesh[m].vertices[v].x, map.blocks[b].mesh[m].vertices[v].y,
                  map.blocks[b].mesh[m].vertices[v].z
                );
                man.normal(
                  map.blocks[b].mesh[m].normals[v].x, map.blocks[b].mesh[m].normals[v].y,
                  map.blocks[b].mesh[m].normals[v].z
                );
                // TODO: How to add texture coordinates at this point?
                //man->textureCoord(0, 0);
            }
            // Conform faces from vertices
            for (int t = 0; t < map.blocks[b].mesh[m].triangle_count; t ++){
                //std::cout << "Triangle " << t << " vertizes: "
                //  << (int) map.blocks[b].mesh[m].triangles[t].vertex_index[0] << ", "
                //  << (int) map.blocks[b].mesh[m].triangles[t].vertex_index[1] << ", "
                //  << (int) map.blocks[b].mesh[m].triangles[t].vertex_index[2] << std::endl;
                man.triangle(
                  map.blocks[b].mesh[m].triangles[t].vertex_index[0],
                  map.blocks[b].mesh[m].triangles[t].vertex_index[1],
                  map.blocks[b].mesh[m].triangles[t].vertex_index[2]
                );
            }
        }
        // Export the mesh.
        man.end();
        Ogre::MeshPtr mesh = man.convertToMesh(name);
        //std::cout << "[WI] Export mesh " << (output_dir_ + "wm/" + mesh->getName() + ".mesh")
        //  << std::endl;
        mesh_serializer.exportMesh(
          mesh.getPointer(), output_dir_ + "wm/" + mesh->getName() + ".mesh"
        );
    }
    // Export mesh file.
    processed_maps_ ++;
    if (processed_maps_ >= wm_map_.size()) return true;
    else return false;
}

