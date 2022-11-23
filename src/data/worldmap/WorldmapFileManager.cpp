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

#include "WorldmapFileManager.h"

#include "data/worldmap/WorldmapFile.h"

/**
 * Map file manager singleton.
 */
template<> VGears::WorldmapFileManager*
  Ogre::Singleton<VGears::WorldmapFileManager>::msSingleton = nullptr;

namespace VGears {

    WorldmapFileManager *WorldmapFileManager::GetSingletonPtr(){return msSingleton;}

    WorldmapFileManager &WorldmapFileManager::GetSingleton(){
        assert(msSingleton);
        return(*msSingleton);
    }

    WorldmapFileManager::WorldmapFileManager(){
        mResourceType = WorldmapFile::RESOURCE_TYPE;
        mLoadOrder = 30.0f;
        Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }

    WorldmapFileManager::~WorldmapFileManager(){
        Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }

    Ogre::Resource* WorldmapFileManager::createImpl(
      const Ogre::String& name, Ogre::ResourceHandle handle, const Ogre::String& group,
      bool is_manual, Ogre::ManualResourceLoader* loader,
      const Ogre::NameValuePairList* create_params
    ){return new WorldmapFile(this, name, handle, group, is_manual, loader);}
}
