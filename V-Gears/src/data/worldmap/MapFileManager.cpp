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

#include "data/worldmap/MapFileManager.h"
#include "data/worldmap/MapFile.h"

template<> VGears::MapFileManager*
  Ogre::Singleton<VGears::MapFileManager>::msSingleton = nullptr;

namespace VGears {

    MapFileManager *MapFileManager::GetSingletonPtr(){return msSingleton;}

    MapFileManager &MapFileManager::GetSingleton(){
        assert(msSingleton);
        return(*msSingleton);
    }

    MapFileManager::MapFileManager(){
        mResourceType = WorldMapFile::RESOURCE_TYPE;
        mLoadOrder = 30.0f;
        Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(
          mResourceType, this
        );
    }

    MapFileManager::~MapFileManager(){
        Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(
          mResourceType
        );
    }

    Ogre::Resource* MapFileManager::createImpl(
      const Ogre::String& name,
      Ogre::ResourceHandle handle,
      const Ogre::String& group,
      bool is_manual,
      Ogre::ManualResourceLoader* loader,
      const Ogre::NameValuePairList* create_params
    ){
        return new WorldMapFile(this, name, handle, group, is_manual, loader);
    }
}
