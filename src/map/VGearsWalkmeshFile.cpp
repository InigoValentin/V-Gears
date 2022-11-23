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
#include "map/VGearsWalkmeshFile.h"
#include "map/VGearsWalkmeshFileXMLSerializer.h"

namespace VGears{

    const String WalkmeshFile::RESOURCE_TYPE("VGearsWalkmeshFile");

    WalkmeshFile::WalkmeshFile(
      Ogre::ResourceManager *creator, const String &name,
      Ogre::ResourceHandle handle, const String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader
    ) : Resource(creator, name, handle, group, is_manual, loader){}

    WalkmeshFile::~WalkmeshFile(){unload();}

    void WalkmeshFile::loadImpl(){
        WalkmeshFileXMLSerializer serializer;
        Ogre::DataStreamPtr stream(openResource());
        serializer.ImportWalkmeshFile(stream, this);
    }

    void WalkmeshFile::unloadImpl(){
        triangles_.clear();
    }

    size_t WalkmeshFile::calculateSize() const{
        return sizeof(Triangle) * triangles_.size();
    }

    WalkmeshFile::TriangleList& WalkmeshFile::GetTriangles(){return triangles_;}

}
