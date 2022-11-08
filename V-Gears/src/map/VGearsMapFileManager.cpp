/*
Copyright (c) 27.10.2013 Tobias Peters <tobias.peters@kreativeffekt.at>

This file is part of V-Gears

V-Gears is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 2.0 (GPLv2) of the License.

V-Gears is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
*/

#include "map/VGearsMapFileManager.h"

/**
 * Map file manager singleton.
 */
template<> VGears::MapFileManager
  *Ogre::Singleton<VGears::MapFileManager>::msSingleton = nullptr;

namespace VGears{
    MapFileManager *MapFileManager::GetSingletonPtr(){return msSingleton;}
    
    MapFileManager &MapFileManager::GetSingleton(){
        assert(msSingleton);
        return(*msSingleton);
    }

    MapFileManager::MapFileManager(){
        mResourceType = MapFile::RESOURCE_TYPE;
        // Low, because it will likely reference other resources.
        mLoadOrder = 30.0f;
        // This is how the ResourceManager registers with OGRE.
        Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(
          mResourceType, this
       );
    }
    
    MapFileManager::~MapFileManager(){
        Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(
          mResourceType
        );
    }

    Ogre::Resource *MapFileManager::createImpl(
      const Ogre::String &name, Ogre::ResourceHandle handle,
      const Ogre::String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader,
      const Ogre::NameValuePairList *create_params
    ){return new MapFile(this, name, handle, group, is_manual, loader);}
    
}

