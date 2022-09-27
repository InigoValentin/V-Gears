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

#include "data/VGearsPFileManager.h"

template<> VGears::PFileManager
  *Ogre::Singleton<VGears::PFileManager>::msSingleton = nullptr;

namespace VGears {

    PFileManager *PFileManager::GetSingletonPtr(){return msSingleton;}

    PFileManager &PFileManager::GetSingleton(){
        assert( msSingleton );
        return(*msSingleton );
    }

    PFileManager::PFileManager(){
        mResourceType = PFile::RESOURCE_TYPE;
        // Low, because it will likely reference other resources.
        mLoadOrder = 30.0f;
        // This is how the ResourceManager registers with OGRE.
        Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(
          mResourceType, this
        );
    }

    PFileManager::~PFileManager(){
        Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(
          mResourceType
        );
    }

    Ogre::Resource *PFileManager::createImpl(
      const Ogre::String &name, Ogre::ResourceHandle handle,
      const Ogre::String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader,
      const Ogre::NameValuePairList *create_params
    ){
        return new PFile(this, name, handle, group, is_manual, loader);
    }

}
