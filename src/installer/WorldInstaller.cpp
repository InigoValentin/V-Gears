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
#include "TexFile.h"
#include "common/Lzs.h"
#include "common/VGearsStringUtil.h"
#include "data/VGearsLGPArchive.h"
#include "data/WorldMapWalkmesh.h"
#include "data/VGearsHRCFileManager.h"
#include "data/FF7Data.h"

std::string WorldInstaller::ELEMENT_MODELS_DIR("models/world/element");

std::string WorldInstaller::TERRAIN_MODELS_DIR("models/world/terrain");

WorldInstaller::WorldInstaller(
  const std::string input_dir, const std::string output_dir,
  const bool keep_originals, Ogre::ResourceGroupManager* res_mgr
):
  input_dir_(input_dir), output_dir_(output_dir), keep_originals_(keep_originals), res_mgr_(res_mgr)
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

void WorldInstaller::ProcessModels(){
    // Open world_us.lgp
    File world_file(input_dir_ + "data/wm/world_us.lgp");
    VGears::LGPArchive world_lgp(input_dir_ + "data/wm/world_us.lgp", "LGP");
    world_lgp.open(input_dir_ + "data/wm/world_us.lgp", true);
    world_lgp.load();
    VGears::LGPArchive::FileList file_list = world_lgp.GetFiles();
    for (int i = 0; i < file_list.size(); i ++){
        VGears::LGPArchive::FileEntry f = file_list.at(i);
        if (f.data_offset + f.data_size <= world_file.GetFileSize()){
            File w_lgp_file(&world_file, f.data_offset, f.data_size);
            std::string file_name = f.file_name;
            w_lgp_file.WriteFile(output_dir_ + "temp/world_models/" + file_name);
            if (file_name.substr(4, 3) == "hrc"){

                std::string id = file_name.substr(0, 3);
                std::string model_name = FF7Data::GetWorldMapModelName(id);
                std::cout << "CONVERTING WORLD MODEL: " << file_name << " " << i
                  << "/" << file_list.size() << std::endl;



                // TODO: Can this be done with declare resource?
                // TODO: If not, do aloop to save all the hrc files, then call this once, then
                // another loop to process them.
                /*res_mgr_->declareResource(
                  output_dir_ + "temp/world_models/" + file_name, "Skeleton", "FFVII"
                );*/
                res_mgr_->removeResourceLocation(output_dir_ + "temp/world_models/", "FFVII");
                res_mgr_->addResourceLocation(
                  output_dir_ + "temp/world_models/", "FileSystem", "FFVII", true, true
                );



                // TODO: This needs work to assemble all the pieces. Similar to field models.


                Ogre::ResourcePtr hrc = VGears::HRCFileManager::GetSingleton().load(
                  file_name, "FFVII"
                );
                auto mesh_name = model_name + ".mesh";
                Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(mesh_name, "FFVII"));
                Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
                // TODO: a files??
                /*for (std::string anim : model.a){
                    VGears::AFileManager &afl_mgr(VGears::AFileManager::GetSingleton());
                    Ogre::String a_base_name;
                    VGears::StringUtil::splitBase(anim, a_base_name);
                    VGears::AFilePtr a
                      = afl_mgr.load(a_base_name + ".a", "FFVII").staticCast<VGears::AFile>();
                    // Convert the FF7 name to a more readable name set in the meta data.
                    VGears::StringUtil::splitBase(anim, base_name);
                    a->AddTo(skeleton, VGears::NameLookup::Animation(base_name));
                }*/
                ExportMesh(output_dir_ + "data/models/world/" + mesh_name, mesh);
            }
        }
    }
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
                //t.walkability = walkability_function >> 5; // 5 bits
                //t.function_id = walkability_function & 0x7; // 3 bits
                t.walkability =  walkability_function & 0x1F; // Lowest 5 bits
                t.function_id =  walkability_function >> 3; // 3 bits
                // FIXME: I think function_id is wrong. It must be 3 bits
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
          output_dir_ + TERRAIN_MODELS_DIR + "/" + std::to_string(processed_maps_)
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
    // TODO: Add models, entities...
    doc.SaveFile(
      output_dir_ + "/world/" + std::to_string(processed_maps_)
      + "/world" + std::to_string(processed_maps_) + ".xml"
    );
    // Generate the walkmesh xml file
    walkmesh.generate(output_dir_ + "/world/" + std::to_string(processed_maps_) + "/wm.xml");
    // TODO: Generate other files: background, script, texts...
    // Ready for next map or next step.
    processed_maps_ ++;
    if (processed_maps_ >= wm_map_.size()) return true;
    else return false;
}

void WorldInstaller::ExportMesh(const std::string outdir, const Ogre::MeshPtr &mesh){

    // TODO: Share function with pc model exporter
    VGears::String base_mesh_name;
    VGears::StringUtil::splitFull(mesh->getName(), base_mesh_name);
    Ogre::MeshSerializer mesh_serializer;
    Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
    Ogre::SkeletonSerializer skeleton_serializer;
    skeleton_serializer.exportSkeleton(
      skeleton.getPointer(), outdir + base_mesh_name + ".skeleton"
    );
    mesh->setSkeletonName(base_mesh_name + ".skeleton");
    mesh_serializer.exportMesh(mesh.getPointer(), outdir + mesh->getName());
    Ogre::Mesh::SubMeshIterator it(mesh->getSubMeshIterator());
    Ogre::MaterialSerializer mat_ser;
    size_t i(0);
    std::set<std::string> textures;
    while (it.hasMoreElements()){
        Ogre::SubMesh *sub_mesh(it.getNext());
        Ogre::MaterialPtr mat(Ogre::MaterialManager::getSingleton().getByName(
          sub_mesh->getMaterialName())
        );
        if (mat != nullptr){
            for (size_t techs = 0; techs < mat->getNumTechniques(); techs ++){
                Ogre::Technique* tech = mat->getTechnique(techs);
                if (tech){
                    for (size_t pass_num = 0; pass_num < tech->getNumPasses(); pass_num ++){
                        Ogre::Pass* pass = tech->getPass(pass_num);
                        if (pass){
                            for (
                              size_t texture_unit_num = 0;
                              texture_unit_num < pass->getNumTextureUnitStates();
                              texture_unit_num ++
                            ){
                                Ogre::TextureUnitState* unit
                                  = pass->getTextureUnitState(texture_unit_num);
                                if (unit && unit->getTextureName().empty() == false){
                                    // Convert the texture from .tex to .png.
                                    TexFile tex(
                                      output_dir_ + "temp/wm/" + unit->getTextureName()
                                    );

                                    // Ensure the output material script references png files
                                    // rather than tex files.
                                    Ogre::String base_name;
                                    VGears::StringUtil::splitBase(
                                      unit->getTextureName(), base_name
                                    );
                                    unit->setTextureName(base_mesh_name + "_" + base_name + ".png");

                                    tex.SavePng(
                                      output_dir_ + ELEMENT_MODELS_DIR + "/" + base_name + ".png", 0
                                    );
                                    // Copy subtexture (xxxx.png) to model_xxxx.png
                                    // TODO: obtain the "data" folder
                                    // programatically.
                                    boost::filesystem::copy_file(
                                        output_dir_ + ELEMENT_MODELS_DIR + "/" + base_name + ".png",
                                        output_dir_ + ELEMENT_MODELS_DIR + "/"
                                        + base_mesh_name + "_" + base_name + ".png",
                                      boost::filesystem::copy_option::overwrite_if_exists
                                    );
                                    textures.insert(unit->getTextureName());
                                }
                            }
                        }
                    }
                }
            }
            // TODO: Check what to do with materials
            /**
            if (std::count(materials_.begin(), materials_.end(), sub_mesh->getMaterialName()) == 0){
                mat_ser.queueForExport(mat);
                materials_.push_back(sub_mesh->getMaterialName());
            }*/

        }
        ++ i;
    }
    mat_ser.exportQueued(outdir + base_mesh_name + VGears::EXT_MATERIAL);
    for (auto& texture_name : textures){
        std::string tex_name = texture_name.c_str();
        try{
            Ogre::TexturePtr texture_ptr
              = Ogre::TextureManager::getSingleton().load(tex_name, "FFVIITextures" /*"FFVII"*/);
            Ogre::Image image;
            texture_ptr->convertToImage(image);
            Ogre::String base_name;
            VGears::StringUtil::splitBase(texture_name, base_name);
            image.save(outdir + base_mesh_name + "_" + base_name + ".png");
        }
        catch (std::exception const& ex){
            std::cerr << "[ERROR] Exception: " << ex.what() << std::endl;
        }
    }
}

