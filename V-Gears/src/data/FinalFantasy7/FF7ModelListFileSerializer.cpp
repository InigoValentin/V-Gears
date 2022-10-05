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
#include <OgreException.h>
#include "data/FinalFantasy7/FF7ModelListFileSerializer.h"

namespace VGears{

    ModelListFileSerializer::ModelListFileSerializer(): Serializer(){}

    ModelListFileSerializer::~ModelListFileSerializer(){}

    template<typename ValueType> void ModelListFileSerializer::ReadVector(
      Ogre::DataStreamPtr &stream, std::vector<ValueType> &dest, size_t count
    ){
        dest.clear();
        dest.reserve(count);
        for (size_t i(count); i-- ;){
            ValueType in_tmp;
            readObject(stream, in_tmp);
            dest.push_back(in_tmp);
        }
    }

    void ModelListFileSerializer::ImportModelListFile(
      Ogre::DataStreamPtr &stream, ModelListFile *dest){
        stream->skip(2); // uint16 unused.
        uint16 model_count;
        ReadShort(stream, model_count);
        uint16 model_scale;
        ReadShort(stream, model_scale);
        dest->SetScale(model_scale);
        ReadVector(stream, dest->GetModels(), model_count);
    }

    void ModelListFileSerializer::readObject(
      Ogre::DataStreamPtr &stream, ModelListFileSerializer::ModelDescription &dest
    ){
        uint16 name_length;
        ReadShort(stream, name_length);
        dest.name = readString(stream, name_length);
        uint16 type;
        ReadShort(stream, type);
        dest.type = ModelListFile::UNKNOWN;
        if (type == ModelListFile::PLAYER) dest.type = ModelListFile::PLAYER;
        else if (type == ModelListFile::NPC) dest.type = ModelListFile::NPC;
        dest.hrc_name = readString(stream, HRC_NAME_LENGTH);
        dest.scale = readString(stream, SCALE_LENGTH);
        uint16 animation_count;
        ReadShort(stream, animation_count);
        // 3 *(3 uint8 + 3 uint16) + 3 uint8 light colors
        stream->skip(3 * (3 + 6) + 3);
        ReadVector(stream, dest.animations, animation_count);
    }

    void ModelListFileSerializer::readObject(
      Ogre::DataStreamPtr &stream, ModelListFileSerializer::AnimationDescription &dest
    ){
        uint16 name_length;
        ReadShort(stream, name_length);
        dest.name = readString(stream, name_length);
        ReadShort(stream, dest.unknown);
    }

}
