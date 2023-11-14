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

#include <OgreLogManager.h>
#include <OgreMeshManager.h>
#include <OgreSkeletonManager.h>
#include "data/VGearsHRCFile.h"
#include "common/FinalFantasy7/FF7NameLookup.h"
#include "common/VGearsStringUtil.h"
#include "data/VGearsHRCFileSerializer.h"
#include "data/VGearsHRCMeshLoader.h"
#include "data/VGearsHRCSkeletonLoader.h"

namespace VGears{

    const String HRCFile::RESOURCE_TYPE("VGearsHRCFile");

    //33.0f; TODO fix unit tests before changing this.
    const float HRCFile::DOWN_SCALER = 1.0f;


    HRCFile::HRCFile(
      Ogre::ResourceManager *creator, const String &name, Ogre::ResourceHandle handle,
      const String &group, bool is_manual, Ogre::ManualResourceLoader *loader
    ) :
      Ogre::Resource(creator, name, handle, group, is_manual, loader),
       mesh_loader_(nullptr), skeleton_loader_(nullptr)
    {createParamDictionary(RESOURCE_TYPE);}

    HRCFile::~HRCFile(){
        if (skeleton_loader_){
            Ogre::SkeletonManager::getSingleton().remove(skeleton_->getHandle());
            delete skeleton_loader_;
            skeleton_loader_ = nullptr;
        }
        if (mesh_loader_){
            Ogre::MeshManager::getSingleton().remove(mesh_->getHandle());
            delete mesh_loader_;
            mesh_loader_ = nullptr;
        }
        skeleton_.reset();
        mesh_.reset();
        unload();
    }

    void HRCFile::loadImpl(){
        HRCFileSerializer serializer;
        Ogre::DataStreamPtr stream(
          Ogre::ResourceGroupManager::getSingleton().openResource(mName, mGroup, true, this)
        );
        serializer.ImportHRCFile(stream, this);
        const String skeletonfile_name_(GetSkeletonFileName());
        Ogre::SkeletonManager &skeleton_manager(Ogre::SkeletonManager::getSingleton());
        skeleton_ = skeleton_manager.getByName(skeletonfile_name_, mGroup);
        if (skeleton_ == nullptr){
            assert(skeleton_loader_ == nullptr);
            skeleton_loader_ = new HRCSkeletonLoader(*this);
            skeleton_ = skeleton_manager.create(skeletonfile_name_, mGroup, true, skeleton_loader_);
        }
        const String meshfile_name_(GetMeshFileName());
        Ogre::LogManager::getSingleton().stream()
          << "HRC::loadImpl(): meshfile_name_: " << meshfile_name_;
        Ogre::MeshManager &mesh_manager(Ogre::MeshManager::getSingleton());
        mesh_ = mesh_manager.getByName(meshfile_name_, mGroup);
        if (mesh_ == nullptr){
            assert(mesh_loader_ == nullptr);
            mesh_loader_ = new HRCMeshLoader(*this);
            mesh_ = mesh_manager.create(meshfile_name_, mGroup, true, mesh_loader_);
        }
    }

    void HRCFile::unloadImpl(){
        skeleton_name_.clear();
        bones_.clear();
    }

    size_t HRCFile::CalculateSize(const Bone &bone) const{
        size_t size_rsd_names(0);
        for(
          RSDNameList::const_iterator it(bone.rsd_names.begin()); it != bone.rsd_names.end(); ++ it
        ){size_rsd_names += it->size();}

        return bone.name.size() + bone.parent.size() + sizeof(bone.length) + size_rsd_names;
    }

    size_t HRCFile::CalculateSize() const{
        size_t size_bones(0);
        for(BoneList::const_iterator it(bones_.begin()); it != bones_.end(); ++ it)
            size_bones += CalculateSize(*it);
        return skeleton_name_.size() + size_bones;
    }

    void HRCFile::SetSkeletonName(const String &name){skeleton_name_ = name;}

    String HRCFile::GetSkeletonFileName(void) const{
        String path;
        StringUtil::splitPath(getName(), path);
        return path + skeleton_name_ + EXT_SKELETON;
    }

    String HRCFile::GetMeshFileName(void) const{
        String base_name;
        StringUtil::splitBase(getName(), base_name);
        return NameLookup::model(base_name) + EXT_MESH;
    }

    Ogre::SkeletonPtr HRCFile::GetSkeleton(void) const{
        skeleton_->load();
        return skeleton_;
    }

}
