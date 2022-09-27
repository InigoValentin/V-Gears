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
#include "data/VGearsHRCFileManager.h"

template<> VGears::HRCFileManager
  *Ogre::Singleton<VGears::HRCFileManager>::msSingleton = nullptr;

namespace VGears{

    HRCFileManager* HRCFileManager::GetSingletonPtr(){return msSingleton;}

    HRCFileManager& HRCFileManager::GetSingleton(){
        assert( msSingleton );
        return(*msSingleton );
    }

    HRCFileManager::HRCFileManager(){
        mResourceType = HRCFile::RESOURCE_TYPE;
        // Low, because it will likely reference other resources.
        mLoadOrder = 30.0f;
        // this is how the ResourceManager registers with OGRE.
        Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(
          mResourceType, this
        );
    }

    HRCFileManager::~HRCFileManager(){
        Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(
          mResourceType
        );
    }

    void HRCFileManager::ParseScript(
      Ogre::DataStreamPtr &stream, const String &group_name
    ){
        HRCFilePtr  hrc
          = createResource(stream->getName(), group_name).staticCast<HRCFile>();
        hrc->load();
    }

    Ogre::Resource* HRCFileManager::createImpl(
      const String &name, Ogre::ResourceHandle handle, const String &group,
      bool is_manual, Ogre::ManualResourceLoader *loader,
      const Ogre::NameValuePairList *create_params
    ){
        return new HRCFile(this, name, handle, group, is_manual, loader);
    }

}
