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

#include <iostream>
#include "data/VGearsLZSFLevelFile.h"
#include "data/VGearsLZSDataStream.h"

namespace VGears{

    const String LZSFLevelFile::RESOURCE_TYPE("VGearsLZSFLevelFile");


    LZSFLevelFile::LZSFLevelFile(
      Ogre::ResourceManager *creator, const String &name,
      Ogre::ResourceHandle handle, const String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader
    ) : FLevelFile(creator, name, handle, group, is_manual, loader){
        std::cout << "[LZSFLevelFile] " << name << std::endl;
    }

    LZSFLevelFile::~LZSFLevelFile(){}

    const String& LZSFLevelFile::getResourceType() const{return RESOURCE_TYPE;}

    Ogre::DataStreamPtr LZSFLevelFile::openResource(){
        Ogre::DataStreamPtr compressed(FLevelFile::openResource());
        return Ogre::DataStreamPtr(
          new VGears::LZSDataStream(compressed->getName(), compressed)
        );
    }

}
