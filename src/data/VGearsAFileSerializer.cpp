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

#include <OgreException.h>
#include <OgreLogManager.h>
#include "data/VGearsAFileSerializer.h"
#include "data/VGearsHRCFile.h"

namespace VGears{

    AFileSerializer::AFileSerializer() : Serializer(){}

    AFileSerializer::~AFileSerializer(){}

    template<typename ValueType>void AFileSerializer::ReadVector(
      Ogre::DataStreamPtr &stream, std::vector<ValueType> &dest ,size_t count
   ){
        dest.clear();
        dest.reserve(count);
        for(size_t i(count); i --;){
            ValueType in_tmp;
            readObject(stream, in_tmp);
            dest.push_back(in_tmp);
        }
    }

    void AFileSerializer::ReadFileHeader(Ogre::DataStreamPtr &stream){
        size_t header_size(sizeof(header_));
        stream->read(&header_, header_size);
        flipFromLittleEndian(&header_, 4, header_size / 4);
    }

    void AFileSerializer::readObject(
      Ogre::DataStreamPtr &stream, AFile::Frame &dest
    ){
        readObject(stream, dest.root_rotation);
        readObject(stream, dest.root_translation);
        dest.root_translation = dest.root_translation / HRCFile::DOWN_SCALER;
        ReadVector(stream, dest.bone_rotations, header_.bone_count);
    }

    void AFileSerializer::ImportAFile(
      Ogre::DataStreamPtr &stream, AFile* dest
    ){
        ReadFileHeader(stream);

        if(header_.version != 1){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "Animation has unsupported version only version 1 supported",
              "AFileSerializer::importAFile"
            );
        }
        dest->SetBoneCount(header_.bone_count);
        ReadVector(stream, dest->GetFrames(), header_.frame_count);
    }

}
