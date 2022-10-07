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
#include "data/VGearsPaletteFile.h"
#include "data/VGearsPaletteFileSerializer.h"

namespace VGears{

    const String PaletteFile::RESOURCE_TYPE("VGearsPaletteFile");


    PaletteFile::PaletteFile(
      Ogre::ResourceManager* creator, const String &name,
      Ogre::ResourceHandle handle, const String& group, bool is_manual,
      Ogre::ManualResourceLoader* loader
    ) : Ogre::Resource(creator, name, handle, group, is_manual, loader){
        createParamDictionary(RESOURCE_TYPE);
    }

    PaletteFile::~PaletteFile(){unload();}

    void PaletteFile::loadImpl(){
        PaletteFileSerializer serializer;
        Ogre::DataStreamPtr stream(
          Ogre::ResourceGroupManager::getSingleton().openResource(
            mName, mGroup, true, this
          )
        );
        serializer.ImportPaletteFile(stream, this);
    }

    void PaletteFile::unloadImpl(){pages_.clear();}

    size_t PaletteFile::calculateSize() const{
        size_t data_size(0);
        for (
          PageList::const_iterator it(pages_.begin());
          it != pages_.end();
          ++ it
        ){
            data_size += it->size() * sizeof((*it)[0]);
        }
        return data_size;
    }

}
