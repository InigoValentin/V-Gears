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

#include "data/VGearsLZSFLevelFileManager.h"

/**
 * LZS-compressed FLevel file manager singleton.
 */
template<> VGears::LZSFLevelFileManager
  *Ogre::Singleton<VGears::LZSFLevelFileManager>::msSingleton = nullptr;

namespace VGears{

    LZSFLevelFileManager *LZSFLevelFileManager::GetSingletonPtr(){
        return msSingleton;
    }

    LZSFLevelFileManager &LZSFLevelFileManager::GetSingleton(){
        assert(msSingleton);
        return(*msSingleton);
    }

    LZSFLevelFileManager::LZSFLevelFileManager(){
        mResourceType = LZSFLevelFile::RESOURCE_TYPE;
        // Low, because it will likely reference other resources.
        mLoadOrder = 30.0f;
        // This is how the ResourceManager registers with OGRE
        Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(
          mResourceType, this
        );
    }

    LZSFLevelFileManager::~LZSFLevelFileManager(){
        Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(
          mResourceType
        );
    }

    Ogre::Resource *LZSFLevelFileManager::createImpl(
      const Ogre::String &name, Ogre::ResourceHandle handle,
      const Ogre::String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader,
      const Ogre::NameValuePairList *create_params
    ){return new LZSFLevelFile(this, name, handle, group, is_manual, loader);}

}
