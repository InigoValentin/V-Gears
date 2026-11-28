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
#include <fstream>
#include <boost/filesystem.hpp>
#include <OgreMaterialManager.h>
#include <OgreMaterialSerializer.h>
#include <OgreMeshManager.h>
#include <OgreMeshSerializer.h>
#include <OgreSkeletonSerializer.h>
#include <OgreSubMesh.h>
#include <OgreTechnique.h>
#include "common/File.h"
#include "formats/FF7NameLookup.h"
#include "formats/lgp/LGPArchive.h"
#include "formats/model/AFileManager.h"
#include "formats/model/HRCFileManager.h"
#include "installer/formats/TexFile.h"
#include "installer/tables/ModelMaterials.h"
#include "FieldDataTypes.h"
#include "FieldModelInstaller.h"

FieldModelInstaller::FieldModelInstaller(
  const DiskImage& disk_image, const std::string& output_dir, const std::string& data_dir,
  ModelsAndAnimationsDb& used_models, std::function<void(std::string)> log_output_line
):
  disk_image_(disk_image), output_dir_(output_dir), data_dir_(data_dir), used_models_(used_models),
  write_output_line_(log_output_line)
{}

std::vector<std::string> FieldModelInstaller::Initialize(){
    ExtractModelFiles();
    // Ogre indexes a resource location when it is added, so files written afterwards are not
    // found until the location is registered again. On a clean install the folder was empty when
    // it was first added.
    Ogre::ResourceGroupManager& res_mgr = Ogre::ResourceGroupManager::getSingleton();
    res_mgr.removeResourceLocation(TempDir(), "FFVII");
    res_mgr.addResourceLocation(TempDir(), "FileSystem", "FFVII", true, true);
    std::vector<std::string> models;
    for (const auto& model : used_models_.map) models.push_back(model.first);
    return models;
}

void FieldModelInstaller::ConvertModel(const std::string& model_name){
    auto model = used_models_.map.find(model_name);
    if (model == used_models_.map.end()){
        write_output_line_("[ERROR] Model " + model_name + " is not used by any field.");
        return;
    }
    VGears::HRCFileManager& hrc_manager = VGears::HRCFileManager::GetSingleton();
    try{
        Ogre::ResourcePtr hrc = hrc_manager.load(model->first, "FFVII");
        Ogre::String base_name;
        VGears::StringUtil::splitBase(model->first, base_name);
        const auto mesh_name = VGears::NameLookup::model(base_name) + ".mesh";
        Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(mesh_name, "FFVII"));
        Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
        const Ogre::AxisAlignedBox bind_bounds = mesh->getBounds();
        Ogre::AxisAlignedBox translation_bounds(Ogre::Vector3::ZERO, Ogre::Vector3::ZERO);
        for (const auto& animation_name : model->second){
            VGears::AFilePtr animation = VGears::AFileManager::GetSingleton()
              .load(animation_name, "FFVII").staticCast<VGears::AFile>();
            // Convert the FF7 name to a more readable name set in the meta data.
            VGears::StringUtil::splitBase(animation_name, base_name);
            animation->AddTo(skeleton, VGears::NameLookup::Animation(base_name), true);
            translation_bounds.merge(animation->GetRootTranslationBounds(true));
        }
        ExtendBounds(mesh, bind_bounds, translation_bounds);
        ExportMesh(ModelsDir(), mesh);
        // Models can share a skeleton name with different bones. Drop the model (and its skeleton
        // and mesh) so the next one doesn't reuse them.
        hrc_manager.remove(model->first, "FFVII");
    }
    catch (const Ogre::Exception& ex){
        if (hrc_manager.resourceExists(model->first, "FFVII")) hrc_manager.remove(model->first, "FFVII");
        write_output_line_("[ERROR] Ogre exception converting model " + model->first + ": " + ex.what());
    }
    catch (const std::exception& ex){
        if (hrc_manager.resourceExists(model->first, "FFVII")) hrc_manager.remove(model->first, "FFVII");
        write_output_line_("[ERROR] Exception converting model " + model->first + ": " + ex.what());
    }
}

void FieldModelInstaller::ExtractModelFiles(){
    const std::string path = *disk_image_.filePath("data/field/char.lgp");
    // Read the list of files as an archive, and their contents as a file.
    VGears::LGPArchive char_lgp(path, "LGP");
    char_lgp.open(path, true);
    char_lgp.load();
    File char_file(path);
    const Ogre::StringVectorPtr file_list = char_lgp.list(true, true);
    const VGears::LGPArchive::FileList files = char_lgp.GetFiles();
    for (size_t i = 0; i < file_list->size(); i ++){
        const VGears::LGPArchive::FileEntry& entry = files.at(i);
        std::fstream out;
        out.open(TempDir() + entry.file_name, std::ios::out);
        char_file.SetOffset(entry.data_offset);
        for (int j = 0; j < entry.data_size; j ++) out << char_file.readU8();
        out.close();
    }
}

void FieldModelInstaller::ExtendBounds(
  const Ogre::MeshPtr& mesh, const Ogre::AxisAlignedBox& bind_bounds,
  const Ogre::AxisAlignedBox& translation_bounds
){
    if (!bind_bounds.isFinite()) return;
    const Ogre::AxisAlignedBox animated_bounds(
      bind_bounds.getMinimum() + translation_bounds.getMinimum(),
      bind_bounds.getMaximum() + translation_bounds.getMaximum()
    );
    mesh->_setBounds(animated_bounds, false);
    mesh->_setBoundingSphereRadius(std::max(
      mesh->getBoundingSphereRadius(),
      animated_bounds.getCenter().length() + animated_bounds.getHalfSize().length()
    ));
}

void FieldModelInstaller::ExportMesh(const std::string& outdir, const Ogre::MeshPtr& mesh){
    VGears::String base_mesh_name;
    VGears::StringUtil::splitFull(mesh->getName(), base_mesh_name);
    MakeMaterialsUnique(mesh, base_mesh_name);
    Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
    Ogre::SkeletonSerializer skeleton_serializer;
    skeleton_serializer.exportSkeleton(skeleton.getPointer(), outdir + base_mesh_name + ".skeleton");
    mesh->setSkeletonName(base_mesh_name + ".skeleton");
    Ogre::MeshSerializer mesh_serializer;
    mesh_serializer.exportMesh(mesh.getPointer(), outdir + mesh->getName());
    Ogre::MaterialSerializer material_serializer;
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
        if (std::count(materials_.begin(), materials_.end(), sub_mesh->getMaterialName()) == 0){
            material_serializer.queueForExport(material);
            materials_.push_back(sub_mesh->getMaterialName());
        }
    }
    material_serializer.exportQueued(outdir + base_mesh_name + VGears::EXT_MATERIAL);
}

void FieldModelInstaller::ConvertTexture(Ogre::TextureUnitState& unit, const std::string& base_mesh_name){
    // Materials are shared by name across all model types, so this one may have already been
    // converted for a battle model (its texture is then a png in another folder). There is
    // nothing to convert in that case.
    if (!boost::filesystem::exists(TempDir() + unit.getTextureName())){
        write_output_line_(
          "[WARNING] Texture " + unit.getTextureName() + " of model " + base_mesh_name
          + " not found; skipping it."
        );
        return;
    }
    TexFile tex(TempDir() + unit.getTextureName());
    // The material uses the png, under the name of the model.
    Ogre::String base_name;
    VGears::StringUtil::splitBase(unit.getTextureName(), base_name);
    const std::string png_name = base_mesh_name + "_" + base_name + ".png";
    unit.setTextureName(png_name);
    tex.SavePng(ModelsDir() + png_name, 0);
}

std::string FieldModelInstaller::TempDir() const{return output_dir_ + "temp/char/";}

std::string FieldModelInstaller::ModelsDir() const{return data_dir_ + FieldData::MODELS_DIR + "/";}
