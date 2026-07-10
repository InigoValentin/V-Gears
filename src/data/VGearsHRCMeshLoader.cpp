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

#include <OgreMesh.h>
#include <vector>
#include "data/VGearsHRCMeshLoader.h"
#include "common/VGearsStringUtil.h"
#include "data/VGearsPFile.h"
#include "data/VGearsPFileManager.h"
#include "data/VGearsHRCFile.h"
#include "data/VGearsRSDFileManager.h"

namespace VGears{

  namespace {

    template<typename ResourcePtrType, typename LoaderFunc>
    ResourcePtrType loadWithFallbacks(
      const Ogre::String& group,
      const std::vector<Ogre::String>& candidates,
      LoaderFunc loader
    ){
      Ogre::Exception last_exception(
        Ogre::Exception::ERR_ITEM_NOT_FOUND,
        "No candidate resource could be loaded.",
        "loadWithFallbacks"
      );
      for (const auto& candidate : candidates){
        if (candidate.empty()) continue;
        try{
          return loader(candidate);
        }
        catch (const Ogre::Exception& ex){
          last_exception = ex;
        }
      }
      throw last_exception;
    }

    std::vector<Ogre::String> rsdCandidates(const Ogre::String& rsd_base){
      std::vector<Ogre::String> names;
      names.push_back(rsd_base + EXT_RSD);
      names.push_back(rsd_base);
      names.push_back(rsd_base + "rsd");
      return names;
    }

    std::vector<Ogre::String> pCandidates(const Ogre::String& pfile_name){
      std::vector<Ogre::String> names;
      names.push_back(pfile_name);
      names.push_back(StringUtil::replaceAll(pfile_name, EXT_P, ""));
      names.push_back(pfile_name + "p");
      return names;
    }

  }

    typedef HRCFile::Bone Bone;

    typedef HRCFile::BoneList BoneList;

    typedef HRCFile::RSDNameList RSDNameList;

    HRCMeshLoader::HRCMeshLoader(HRCFile &hrc_file) : hrc_file_(hrc_file){}

    HRCMeshLoader::~HRCMeshLoader(){}

    void HRCMeshLoader::loadResource(Ogre::Resource *resource){
        Ogre::Mesh *mesh(static_cast<Ogre::Mesh *>(resource));
        assert(mesh);
        hrc_file_.load();
        mesh->_notifyOrigin(hrc_file_.getName());
        mesh->setSkeletonName(hrc_file_.GetSkeletonFileName());
        String path;
        StringUtil::splitPath(hrc_file_.getName(), path);
        BoneList::const_iterator it(hrc_file_.GetBones().begin());
        BoneList::const_iterator end(hrc_file_.GetBones().end());
        while(it != end) LoadBone(mesh, *(it++), path);
    }

    void HRCMeshLoader::LoadBone(
      Ogre::Mesh *mesh, const Bone &bone, const String &path
    ){
        const String &bone_name(bone.name);
        RSDNameList::const_iterator it(bone.rsd_names.begin());
        RSDNameList::const_iterator end(bone.rsd_names.end());
        String rsd_base, rsdfile_name_;
        while (it != end){
            rsd_base = path + *it;
            StringUtil::toLowerCase(rsd_base);
            rsdfile_name_ = rsd_base + EXT_RSD;
            RSDFilePtr rsd_file = loadWithFallbacks<RSDFilePtr>(
              hrc_file_.getGroup(),
              rsdCandidates(rsd_base),
              [this](const Ogre::String& candidate){
                  return RSDFileManager::getSingleton().load(
                    candidate, hrc_file_.getGroup()
                  ).staticCast<RSDFile>();
              }
            );
            assert(rsd_file != nullptr);
            String pfile_name_ = path + rsd_file->GetPolygonName();
            StringUtil::toLowerCase(pfile_name_);
            pfile_name_ = StringUtil::replaceAll(pfile_name_, EXT_PLY, EXT_P);
            PFilePtr p_file = loadWithFallbacks<PFilePtr>(
              hrc_file_.getGroup(),
              pCandidates(pfile_name_),
              [this](const Ogre::String& candidate){
                  return PFileManager::getSingleton().load(
                    candidate, hrc_file_.getGroup()
                  ).staticCast<PFile>();
              }
            );
            assert(p_file != nullptr);
            p_file->AddGroups(mesh, bone_name, rsd_file);
            ++ it;
        }
    }

}
