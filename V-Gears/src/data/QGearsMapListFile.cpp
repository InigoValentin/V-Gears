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

#include <array>
#include "data/QGearsMapListFile.h"

template<> QGears::MapListFileManager
  *Ogre::Singleton<QGears::MapListFileManager>::msSingleton = nullptr;

namespace QGears{

    const int MAP_NAME_MAX_SIZE = 32;

    MapListFileManager::MapListFileManager(){
        mResourceType = MapListFile::RESOURCE_TYPE;
        // Low, because it will likely reference other resources.
        mLoadOrder = 30.0f;
        // This is how the ResourceManager registers with OGRE.
        Ogre::ResourceGroupManager::getSingleton()._registerResourceManager(
          mResourceType, this
        );
    }

    MapListFileManager::~MapListFileManager(){
        Ogre::ResourceGroupManager::getSingleton()._unregisterResourceManager(
          mResourceType
        );
    }

    MapListFileManager& MapListFileManager::GetSingleton(){
        assert(msSingleton);
        return(*msSingleton);
    }

    MapListFileManager* MapListFileManager::GetSingletonPtr(){
        return msSingleton;
    }

    MapListFile::MapListFile(
      Ogre::ResourceManager* creator, const String &name,
      Ogre::ResourceHandle handle, const String& group, bool is_manual,
      Ogre::ManualResourceLoader* loader
    )
      : Resource(creator, name, handle, group, is_manual, loader)
    {}

    MapListFile::~MapListFile(){unload();}

    void MapListFile::loadImpl(void){
        MapListFileSerializer serializer;
        Ogre::DataStreamPtr stream(openResource());
        serializer.ImportMapListFile(stream, *this);
    }

    void MapListFile::unloadImpl(void){map_list_.clear();}

    size_t MapListFile::calculateSize(void) const{
        return map_list_.size() * MAP_NAME_MAX_SIZE;
    }

    const String MapListFile::RESOURCE_TYPE("FF7FieldPCMapList");

    Ogre::Resource* MapListFileManager::createImpl(
      const Ogre::String &name, Ogre::ResourceHandle handle,
      const Ogre::String& group, bool is_manual,
      Ogre::ManualResourceLoader* loader,
      const Ogre::NameValuePairList* createParams
    ){
        return new MapListFile(this, name, handle, group, is_manual, loader);
    }

    void MapListFileSerializer::ImportMapListFile(
      Ogre::DataStreamPtr &stream, MapListFile& dest
    ){
        uint16 num_maps = 0;
        ReadShort(stream, num_maps);
        for (auto i = 0u; i < num_maps; i ++){
            // +1 to ensure nullptr termination.
            std::array<char, MAP_NAME_MAX_SIZE + 1> tmp_buffer = {};
            ReadChars(stream, tmp_buffer.data(), tmp_buffer.size() - 1);
            std::string tmp = tmp_buffer.data();
            if (!tmp.empty()) dest.map_list_.emplace_back(tmp);
        }
    }

}
