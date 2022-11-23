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

#include "data/FinalFantasy7/FF7ModelListFile.h"
#include "data/FinalFantasy7/FF7ModelListFileSerializer.h"

namespace VGears{

    const String ModelListFile::RESOURCE_TYPE("FF7ModelListFile");

    ModelListFile::ModelListFile(
      Ogre::ResourceManager *creator, const String &name, Ogre::ResourceHandle handle,
      const String &group, bool is_manual, Ogre::ManualResourceLoader *loader
    ): Resource(creator, name, handle, group, is_manual, loader), scale_(0){}

    ModelListFile::~ModelListFile(){unload();}

    void ModelListFile::loadImpl(){
        ModelListFileSerializer serializer;
        Ogre::DataStreamPtr stream(openResource());
        serializer.ImportModelListFile(stream, this);
    }

    void ModelListFile::unloadImpl(){
        scale_ = 0;
        models_.clear();
    }

    size_t ModelListFile::calculateSize() const{return 0;}

    ModelListFile::ModelList&ModelListFile::GetModels(){return models_;}

    uint16 ModelListFile::GetScale() const{return scale_;}

    void ModelListFile::SetScale(uint16 scale){scale_ = scale;}

}
