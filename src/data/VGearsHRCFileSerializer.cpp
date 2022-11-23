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
#include <OgreStringConverter.h>
#include "data/VGearsHRCFileSerializer.h"

namespace VGears{

    const String HRCFileSerializer::TAG_VERSION(":HEADER_BLOCK");

    const String HRCFileSerializer::TAG_NAME(":SKELETON");

    const String HRCFileSerializer::TAG_BONE_COUNT(":BONES");

    HRCFileSerializer::HRCFileSerializer(){}

    HRCFileSerializer::~HRCFileSerializer(){}

    template<typename ValueType> void HRCFileSerializer::ReadVector(
      Ogre::DataStreamPtr &stream, std::vector<ValueType> &dest, size_t count
    ){
        dest.clear();
        dest.reserve(count);
        for (size_t i(count); i --;){
            ValueType in_tmp;
            readObject(stream, in_tmp);
            dest.push_back(in_tmp);
        }
    }


    void HRCFileSerializer::ReadBlock(Ogre::DataStreamPtr &stream, Block& dest){
        String line;
        while (!stream->eof() && !(line = GetLine(stream)).empty())
            dest.push_back(line);
    }

    void HRCFileSerializer::ReadFileHeader(Ogre::DataStreamPtr &stream){
        header_.version    = 0;
        header_.bone_count = 0;
        header_.name       = "";
        if(stream->eof()){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "Empty file",
              "HRCFileSerializer::ReadFileHeader"
            );
        }
        Block header_block;
        ReadBlock(stream, header_block);
        for(
          Block::const_iterator it(header_block.begin());
          it != header_block.end();
          ++ it
        ){
            Ogre::StringVector parts(Ogre::StringUtil::split(*it));
            if(parts.size() != 2){
                Ogre::LogManager::getSingleton().stream() << *it;
                OGRE_EXCEPT(
                  Ogre::Exception::ERR_INVALIDPARAMS,
                  "Header directive does not have a value",
                  "HRCFileSerializer::ReadFileHeader"
                );
            }
            if(parts[0] == TAG_VERSION)
                header_.version = Ogre::StringConverter::parseLong(parts[1]);
            else if(parts[0] == TAG_NAME)
                header_.name = parts[1];
            else if(parts[0] == TAG_BONE_COUNT)
                header_.bone_count = Ogre::StringConverter::parseLong(parts[1]);
        }
    }

    void HRCFileSerializer::readObject(Ogre::DataStreamPtr &stream, Bone &dest){
        Block bone_block;
        ReadBlock(stream, bone_block);
        Block::const_iterator it(bone_block.begin());
        dest.name = *(it ++);
        dest.parent = *(it ++);
        dest.length
          = Ogre::StringConverter::parseReal(*(it++)) / HRCFile::DOWN_SCALER;
        Ogre::StringVector rsd_list(Ogre::StringUtil::split(*it));
        size_t rsd_count(
          Ogre::StringConverter::parseUnsignedInt(rsd_list.front())
        );
        String rsd_names("");
        for (
          Ogre::StringVector::const_iterator rsd_it(++ rsd_list.begin());
          rsd_it != rsd_list.end();
          ++ rsd_it
        ){
            dest.rsd_names.push_back(*rsd_it);
            rsd_names += *rsd_it + " ";
        }

        if(rsd_count != dest.rsd_names.size()){
            Ogre::LogManager::getSingleton().stream()
              << "Warning: rsd_names should have " << rsd_count
              << " entries but got " << dest.rsd_names.size() << " instead"
              << "\n name     : " << dest.name
              << "\n parent   : " << dest.parent
              << "\n length   : " << dest.length
              << "\n rsd_count: " << rsd_names << "\n";
        }
    }

    void HRCFileSerializer::ImportHRCFile(
      Ogre::DataStreamPtr &stream, HRCFile* dest
    ){
        ReadFileHeader(stream);
        if(header_.bone_count == 0) header_.bone_count = 1;
        dest->SetSkeletonName(header_.name);
        ReadVector(stream, dest->GetBones(), header_.bone_count);
    }

}
