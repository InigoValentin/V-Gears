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
#include <set>
#include <boost/filesystem.hpp>
#include <OgreMaterialManager.h>
#include <OgreMaterialSerializer.h>
#include <OgreMeshManager.h>
#include <OgreMeshSerializer.h>
#include <OgreSkeletonSerializer.h>
#include <OgreSubMesh.h>
#include <OgreTechnique.h>
#include "common/StringUtil.h"
#include "formats/FF7NameLookup.h"
#include "formats/model/AFileManager.h"
#include "formats/model/HRCFileManager.h"
#include "installer/formats/TexFile.h"
#include "installer/tables/ModelMaterials.h"
#include "installer/tables/WorldModelTable.h"
#include "WorldModelInstaller.h"

WorldModelInstaller::WorldModelInstaller(
  const std::string& output_dir, const std::string& data_dir, Ogre::ResourceGroupManager* res_mgr
): output_dir_(output_dir), data_dir_(data_dir), res_mgr_(res_mgr){}

void WorldModelInstaller::Install(WorldArchive& archive){
    boost::filesystem::create_directories(TempDir());
    boost::filesystem::create_directories(ModelsDir());
    // All the files are extracted first, so the ones a model depends on (.rsd, .p, .tex) exist
    // when it is converted.
    for (const std::string& file_name : ExtractFiles(archive)) ConvertModel(file_name);
}

std::vector<std::string> WorldModelInstaller::ExtractFiles(WorldArchive& archive){
    std::vector<std::string> models;
    for (const VGears::LGPArchive::FileEntry& entry : archive.Entries()){
        if (!archive.Contains(entry)) continue;
        const std::string name = WorldArchive::LowerName(entry);
        archive.Open(entry).WriteFile(TempDir() + name);
        if (name.size() >= 7 && name.substr(4, 3) == "hrc") models.push_back(name);
    }
    return models;
}

void WorldModelInstaller::ConvertModel(const std::string& file_name){
    // TODO: Can this be done with declare resource?
    // TODO: If not, declare all the HRC files first, and register the folder only once.
    res_mgr_->declareResource(TempDir() + file_name, "Skeleton", "FFVII");
    res_mgr_->removeResourceLocation(TempDir(), "FFVII");
    res_mgr_->addResourceLocation(TempDir(), "FileSystem", "FFVII", true, true);
    VGears::HRCFileManager& hrc_manager = VGears::HRCFileManager::GetSingleton();
    try{
        // TODO: This needs work to assemble all the pieces. Similar to field models.
        Ogre::ResourcePtr hrc = hrc_manager.load(file_name, "FFVII");
        Ogre::String base_name;
        VGears::StringUtil::splitBase(file_name, base_name);
        const auto mesh_name = VGears::NameLookup::model(base_name) + ".mesh";
        Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(mesh_name, "FFVII"));
        Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
        AddAnimations(base_name, skeleton);
        ExportMesh(mesh);
        // Models can share a skeleton name with different bones (also with field models), so drop
        // this one (with its skeleton and mesh) before converting the next.
        hrc_manager.remove(file_name, "FFVII");
    }
    catch (const std::exception& ex){
        std::cerr << "[ERROR] Exception converting world model " << file_name << ": " << ex.what()
          << std::endl;
        if (hrc_manager.resourceExists(file_name, "FFVII")) hrc_manager.remove(file_name, "FFVII");
    }
}

void WorldModelInstaller::AddAnimations(const std::string& model_name, Ogre::SkeletonPtr& skeleton){
    // The animations of the model, from the tables of ff7.exe (see WorldModelTable.h). Their
    // names are the ones of the A files, in lower case.
    for (const WorldModelTable::Model& model : WorldModelTable::MODELS){
        if (model_name != model.hrc) continue;
        for (unsigned int i = 0; i < model.animation_count; ++ i){
            const std::string animation_file = std::string(model.animations[i]) + ".a";
            try{
                VGears::AFilePtr animation = VGears::AFileManager::GetSingleton().load(
                  animation_file, "FFVII"
                ).staticCast<VGears::AFile>();
                animation->AddTo(skeleton, model.animations[i]);
            }
            catch (const std::exception& ex){
                std::cerr << "[ERROR] Exception converting animation " << animation_file
                  << " of world model " << model_name << ".hrc: " << ex.what() << std::endl;
            }
        }
        return;
    }
}

void WorldModelInstaller::ExportMesh(const Ogre::MeshPtr& mesh){
    if (mesh.isNull()){
        std::cerr << "[ERROR] ExportMesh called with null mesh." << std::endl;
        return;
    }
    VGears::String base_mesh_name;
    VGears::StringUtil::splitFull(mesh->getName(), base_mesh_name);
    MakeMaterialsUnique(mesh, base_mesh_name);
    Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
    if (!skeleton.isNull()){
        Ogre::SkeletonSerializer skeleton_serializer;
        skeleton_serializer.exportSkeleton(
          skeleton.getPointer(), ModelsDir() + base_mesh_name + ".skeleton"
        );
        mesh->setSkeletonName(base_mesh_name + ".skeleton");
    }
    Ogre::MeshSerializer mesh_serializer;
    mesh_serializer.exportMesh(mesh.getPointer(), ModelsDir() + mesh->getName());
    Ogre::MaterialSerializer material_serializer;
    std::set<std::string> queued_materials;
    Ogre::Mesh::SubMeshIterator it(mesh->getSubMeshIterator());
    while (it.hasMoreElements()){
        Ogre::SubMesh* sub_mesh(it.getNext());
        Ogre::MaterialPtr material(
          Ogre::MaterialManager::getSingleton().getByName(sub_mesh->getMaterialName())
        );
        if (material == nullptr) continue;
        for (size_t t = 0; t < material->getNumTechniques(); t ++){
            Ogre::Technique* technique = material->getTechnique(t);
            if (technique == nullptr) continue;
            for (size_t p = 0; p < technique->getNumPasses(); p ++){
                Ogre::Pass* pass = technique->getPass(p);
                if (pass == nullptr) continue;
                for (size_t u = 0; u < pass->getNumTextureUnitStates(); u ++){
                    Ogre::TextureUnitState* unit = pass->getTextureUnitState(u);
                    if (unit != nullptr && !unit->getTextureName().empty())
                        ConvertTexture(*unit, base_mesh_name);
                }
            }
        }
        // Each material is queued once.
        if (queued_materials.insert(sub_mesh->getMaterialName()).second)
            material_serializer.queueForExport(material);
    }
    // Exporting with an empty queue throws in Ogre.
    if (!queued_materials.empty())
        material_serializer.exportQueued(ModelsDir() + base_mesh_name + VGears::EXT_MATERIAL);
}

void WorldModelInstaller::ConvertTexture(
  Ogre::TextureUnitState& unit, const std::string& base_mesh_name
){
    // Materials are shared by name across all model types, so this one may have already been
    // converted for a field or battle model (its texture is then a png).
    if (!boost::filesystem::exists(TempDir() + unit.getTextureName())){
        std::cerr << "[WARNING] Texture " << unit.getTextureName()
          << " not found in temp/world_models; skipping it." << std::endl;
        return;
    }
    try{
        TexFile tex(TempDir() + unit.getTextureName());
        // The material uses the png, under the name of the model.
        Ogre::String base_name;
        VGears::StringUtil::splitBase(unit.getTextureName(), base_name);
        const std::string png_name = base_mesh_name + "_" + base_name + ".png";
        unit.setTextureName(png_name);
        tex.SavePng(ModelsDir() + png_name, 0);
    }
    catch (const std::exception& ex){
        std::cerr << "[ERROR] Texture export failed for " << unit.getTextureName() << ": "
          << ex.what() << std::endl;
    }
}

std::string WorldModelInstaller::TempDir() const{return output_dir_ + "temp/world_models/";}

std::string WorldModelInstaller::ModelsDir() const{return data_dir_ + "world/models/";}
