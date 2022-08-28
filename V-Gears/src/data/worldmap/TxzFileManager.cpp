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

#include "data/worldmap/TxzFileManager.h"
#include "data/worldmap/TxzFile.h"

template<> QGears::TxzFileManager*
  Ogre::Singleton<QGears::TxzFileManager>::msSingleton = nullptr;

namespace QGears {

    TxzFileManager* TxzFileManager::getSingletonPtr(){return msSingleton;}

    TxzFileManager& TxzFileManager::getSingleton(){
        assert( msSingleton );
        return(*msSingleton );
    }

    TxzFileManager::TxzFileManager(){
        mResourceType = TxzFile::RESOURCE_TYPE;
        mLoadOrder = 20.0f;
        Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(
          mResourceType, this
        );
    }

    TxzFileManager::~TxzFileManager(){
        Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(
          mResourceType
        );
    }

    Ogre::Resource* TxzFileManager::createImpl(
      const Ogre::String& name, Ogre::ResourceHandle handle,
      const Ogre::String& group, bool is_manual,
      Ogre::ManualResourceLoader* loader,
      const Ogre::NameValuePairList* create_params
    ){
        return new TxzFile(this, name, handle, group, is_manual, loader);
    }

}
