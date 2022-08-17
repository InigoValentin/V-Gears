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
#include <OgreResourceGroupManager.h>
#include "map/QGearsBackground2DFile.h"
#include "map/QGearsBackground2DFileXMLSerializer.h"

namespace QGears{

    const String Background2DFile::RESOURCE_TYPE("QGearsBackground2DFile");


    Background2DFile::Background2DFile(
      Ogre::ResourceManager *creator, const String &name,
      Ogre::ResourceHandle handle, const String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader
    ) : Resource(creator, name, handle, group, is_manual, loader){}

    Background2DFile::~Background2DFile(){unload();}

    void Background2DFile::loadImpl(){
        Background2DFileXMLSerializer serializer;
        Ogre::DataStreamPtr stream(openResource());
        serializer.ImportBackground2DFile(stream, this);
    }

    void Background2DFile::unloadImpl(){
        texture_name_.clear();
        // GCC error:
        // error: no match for ‘operator=’ (operand types are ‘Ogre::Vector4’
        // {aka ‘Ogre::Vector<4, float>’} and ‘int’)
        //range_ = 0;
        position_ = Ogre::Vector3::ZERO;
        orientation_ = Ogre::Quaternion::IDENTITY;
        fov_ = 0;
        tiles_.clear();
    }

    size_t Background2DFile::calculateSize() const{return 0;}

    void Background2DFile::SetTextureName(const String &texture_name){
        texture_name_ = texture_name;
    }

    String Background2DFile::GetTextureName() const{ return texture_name_;}

    void Background2DFile::SetClip(const Ogre::Vector2 &clip){clip_ = clip;}

    Ogre::Vector2 Background2DFile::GetClip() const{return clip_;}

    void Background2DFile::SetRange(const Ogre::Vector4 &range){range_ = range;}

    Ogre::Vector4 Background2DFile::GetRange() const{return range_;}

    void Background2DFile::SetPosition(const Ogre::Vector3 &position){
        position_ = position;
    }

    Ogre::Vector3 Background2DFile::GetPosition() const{return position_;}

    void Background2DFile::SetOrientation(const Ogre::Quaternion &orientation){
        orientation_ = orientation;
    }

    Ogre::Quaternion Background2DFile::GetOrientation() const{
        return orientation_;
    }

    void Background2DFile::SetFov(const Ogre::Radian &fov){fov_ = fov;}

    Ogre::Radian Background2DFile::GetFov() const{return fov_;}

    Background2DFile::TileList& Background2DFile::GetTiles(){return tiles_;}

}
