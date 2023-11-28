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
#include <tinyxml.h>
#include <OgreRoot.h>
#include <OgreMesh.h>
#include <OgreMeshSerializer.h>
#include <boost/filesystem.hpp>
#include "WorldInstaller.h"
#include "common/Lzs.h"
#include "data/WorldMapWalkmesh.h"

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
    // Initialize the xml file.
    TiXmlDocument doc;
    std::unique_ptr<TiXmlElement> xml(new TiXmlElement("world_map"));
    // Script.
    std::unique_ptr<TiXmlElement> xml_script_element(new TiXmlElement("script"));
    xml_script_element->SetAttribute("file_name", std::to_string(processed_maps_) + "/script.lua");
    xml->LinkEndChild(xml_script_element.release());
    // Background.
    std::unique_ptr<TiXmlElement> xml_background_2d(new TiXmlElement("background2d"));
    xml_background_2d->SetAttribute("file_name", std::to_string(processed_maps_) + "/bg.xml");
    xml->LinkEndChild(xml_background_2d.release());
    // Texts.
    std::unique_ptr<TiXmlElement> xml_texts(new TiXmlElement("texts"));
    xml_texts->SetAttribute("file_name", std::to_string(processed_maps_) + "/text.xml");
    xml->LinkEndChild(xml_texts.release());
    // Walkmesh.
    std::unique_ptr<TiXmlElement> xml_walkmesh(new TiXmlElement("walkmesh"));
    xml_walkmesh->SetAttribute("file_name", std::to_string(processed_maps_) + "/wm.xml");
    xml->LinkEndChild(xml_walkmesh.release());
    // Terrain (don't close yet).
    std::unique_ptr<TiXmlElement> xml_terrain(new TiXmlElement("terrain"));

    // Initialize the walkmesh data.
    WorldMapWalkmesh walkmesh(processed_maps_, processed_maps_ == 0 ? true : false);

    // Read WM*.MAP file.
    Map map;
    Ogre::MeshSerializer mesh_serializer;
    boost::filesystem::path p(wm_map_[processed_maps_].GetFileName());
    std::string map_name = p.stem().native(); // w/o path or extension.
    for (int b = 0; b < wm_map_[processed_maps_].GetFileSize() / 0xB800; b ++){
        // Calculate offsets for walkmesh.
        // WM0 is 9x7 blocks, WM2 probably is too, WM3 no idea?
        int wm_offset_x = 32 * (b % 9);
        int wm_offset_y = 32 * (b / 9);
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
            for (int d = 0; d < mesh_size ; d ++) lzss.push_back(wm_map_[processed_maps_].readU8());
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
                v.z = mesh_data.readU16LE();
                v.y = mesh_data.readU16LE();
                v.unknown = mesh_data.readU16LE();
                block.mesh[m].vertices.push_back(v);
            }
            for (int nor = 0; nor < block.mesh[m].vertex_count; nor ++){
                Vertex v;
                v.x = mesh_data.readU16LE();
                v.z = mesh_data.readU16LE();
                v.y = mesh_data.readU16LE();
                v.unknown = mesh_data.readU16LE();
                block.mesh[m].normals.push_back(v);
            }

            // Add data to the walkmesh
            for (int tri = 0; tri < block.mesh[m].triangle_count; tri ++){
                Triangle t = block.mesh[m].triangles.at(tri);
                walkmesh.addTriangle(
                  Ogre::Vector3(
                    block.mesh[m].vertices.at(t.vertex_index[0]).x + wm_offset_x,
                    block.mesh[m].vertices.at(t.vertex_index[0]).y + wm_offset_y,
                    block.mesh[m].vertices.at(t.vertex_index[0]).z
                  ), // a
                  Ogre::Vector3(
                    block.mesh[m].vertices.at(t.vertex_index[1]).x + wm_offset_x,
                    block.mesh[m].vertices.at(t.vertex_index[1]).y + wm_offset_y,
                    block.mesh[m].vertices.at(t.vertex_index[1]).z
                  ), // b
                  Ogre::Vector3(
                    block.mesh[m].vertices.at(t.vertex_index[2]).x + wm_offset_x,
                    block.mesh[m].vertices.at(t.vertex_index[2]).y + wm_offset_y,
                    block.mesh[m].vertices.at(t.vertex_index[2]).z
                  ), // c
                  t.walkability,
                  t.function_id
                );
            }
        }
        map.blocks.push_back(block);
    }
    // TODO: Generate material.
    // Generate manual meshes.
    for (int b = 0; b < map.blocks.size(); b ++){
        int wm_offset_x = 32 * (b % 9);
        int wm_offset_y = 32 * (b / 9);
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
          mesh.getPointer(),
          output_dir_ + "models/world/terrain/" + std::to_string(processed_maps_)
            + "/" + mesh->getName() + ".mesh"
        );
        // WM0 is te main worldmap, its shape changes according to the history progress, it has
        // 68 blocks but only 63 are used at a time
        if (processed_maps_ > 0 || b < 63){
            std::unique_ptr<TiXmlElement> xml_block(new TiXmlElement("block"));
            xml_block->SetAttribute("index", std::to_string(b));
            xml_block->SetAttribute(
              "file_name",
              "terrain/" + std::to_string(processed_maps_) + "/" + mesh->getName() + ".mesh"
            );
            xml_block->SetAttribute(
              "position", std::to_string(wm_offset_x) + " " + std::to_string(wm_offset_y) + " 0"
            );
            if (processed_maps_ == 0){
                // TODO: Correct history points for each block.
                switch (b){
                    case 50: // Block 50 replaced by block 63
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_63.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                    case 41: // Block 41 replaced by block 64
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_64.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                    case 42: // Block 42 replaced by block 65
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_65.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                    case 60: // Block 60 replaced by block 66
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_66.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                    case 47: // Block 50 replaced by block 67
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_67.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                    case 48: // Block 50 replaced by block 68
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_68.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                }
            }
            xml_terrain->LinkEndChild(xml_block.release());
        }
    }
    // Add other data to the XML file.
    // Close and save XML file,
    xml->LinkEndChild(xml_terrain.release());
    doc.LinkEndChild(xml.release());
    doc.SaveFile(output_dir_ + "/world/" + std::to_string(processed_maps_) + "/world" + std::to_string(processed_maps_) + ".xml");
    // Ready for next map or next step.
    processed_maps_ ++;
    if (processed_maps_ >= wm_map_.size()) return true;
    else return false;
}

