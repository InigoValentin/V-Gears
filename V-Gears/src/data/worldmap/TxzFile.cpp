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

#include "data/worldmap/TxzFile.h"
#include "data/worldmap/TxzFileSerializer.h"

namespace QGears {

    const String TxzFile::RESOURCE_TYPE("QGearsWorldMapTxzFile");

    TxzFile::TxzFile(
      Ogre::ResourceManager* creator, const Ogre::String& name,
      Ogre::ResourceHandle handle, const Ogre::String& group,
      bool is_manual, Ogre::ManualResourceLoader* loader
    ) : Resource(creator, name, handle, group, is_manual, loader){}

    TxzFile::~TxzFile(){}

    void TxzFile::loadImpl(){
        TxzFileSerializer serializer;
        Ogre::DataStreamPtr stream(openResource());
        serializer.Import(stream, *this);
    }

    void TxzFile::unloadImpl(){}

}
