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

#include "map/VGearsBackground2DFileManager.h"

template<> VGears::Background2DFileManager
  *Ogre::Singleton<VGears::Background2DFileManager>::msSingleton = nullptr;

namespace VGears{

    Background2DFileManager *Background2DFileManager::GetSingletonPtr(){
        return msSingleton;
    }


    Background2DFileManager &Background2DFileManager::GetSingleton(){
        assert( msSingleton );
        return(*msSingleton );
    }
 
    Background2DFileManager::Background2DFileManager(){
        mResourceType = Background2DFile::RESOURCE_TYPE;
        // Low, because it will likely reference other resources.
        mLoadOrder = 30.0f;
        // this is how the ResourceManager registers with OGRE.
        Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(
          mResourceType, this
        );
    }

    Background2DFileManager::~Background2DFileManager(){
        Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(
          mResourceType
        );
    }

    Ogre::Resource *Background2DFileManager::createImpl(
      const Ogre::String &name, Ogre::ResourceHandle handle,
      const Ogre::String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader,
      const Ogre::NameValuePairList *createParams
    ){
        return new Background2DFile(
          this, name, handle, group, is_manual, loader
        );
    }

}
