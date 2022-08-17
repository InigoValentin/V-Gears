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

#include "map/QGearsMapFile.h"
#include "map/QGearsMapFileXMLSerializer.h"

namespace QGears{

    const String MapFile::RESOURCE_TYPE("QGearsMapFile");

    MapFile::MapFile(
      Ogre::ResourceManager *creator, const String &name,
      Ogre::ResourceHandle handle, const String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader
    ) : Resource(creator, name, handle, group, is_manual, loader){}

    MapFile::~MapFile(){unload();}

    void MapFile::loadImpl(){
        MapFileXMLSerializer serializer;
        Ogre::DataStreamPtr stream(openResource());
        serializer.ImportMapFile(stream, this);
    }

    void MapFile::unloadImpl(){
        script_name_.clear();
        background_2d_name.clear();
        walkmesh_name_.clear();
        forward_direction_ = 0;
    }

    size_t MapFile::calculateSize() const{
        return
          script_name_.size() + background_2d_name.size()
          + walkmesh_name_.size() + sizeof(forward_direction_);
    }

    const String& MapFile::GetScriptName() const{return script_name_;}

    const String& MapFile::GetBackground2dName() const{
        return background_2d_name;
    }

    const String& MapFile::GetWalkmeshName() const{return walkmesh_name_;}

    const Ogre::Real& MapFile::GetForwardDirection() const{
        return forward_direction_;
    }
    MapFile::PointList& MapFile::GetPoints(){return points_;}

    MapFile::TriggerList& MapFile::GetTriggers(){return triggers_;}

    void MapFile::SetScriptName(const String &script_name){
        script_name_ = script_name;
    }
    void MapFile::SetBackground2dName(const String &background2d_name){
        background_2d_name = background2d_name;
    }

    void MapFile::SetWalkmeshName(const String &walkmesh_name){
        walkmesh_name_ = walkmesh_name;
    }

    void MapFile::SetForwardDirection(const Ogre::Real &forward_direction){
        forward_direction_ = forward_direction;
    }
}

