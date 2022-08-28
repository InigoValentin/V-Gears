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
#include "data/QGearsRSDFileSerializer.h"

namespace QGears{

    typedef Ogre::StringConverter StringConverter;

    const String RSDFileSerializer::TAG_HEADER("@RSD");

    const String RSDFileSerializer::TAG_POLYGON("PLY");

    const String RSDFileSerializer::TAG_MATERIAL("MAT");

    const String RSDFileSerializer::TAG_GROUP("GRP");

    const String RSDFileSerializer::TAG_TEXTURE_COUNT("NTEX");

    const String RSDFileSerializer::TAG_TEXTURE_NAME("TEX");

    const String RSDFileSerializer::PARSE_DELIMITER("[]=");

    RSDFileSerializer::RSDFileSerializer(){}

    RSDFileSerializer::~RSDFileSerializer(){}

    void RSDFileSerializer::ReadFileHeader(Ogre::DataStreamPtr &stream){
        String line(GetLine(stream));
        if (!Ogre::StringUtil::startsWith(line, TAG_HEADER, false)){
            Ogre::LogManager::getSingleton().stream() << line;
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "First line in RSD File should start with " + TAG_HEADER,
              "RSDFileSerializer::readFileHeader"
            );
        }
    }

    void RSDFileSerializer::ParseLine(const String &line, RSDFile* dest){
        if (line.empty()) return;
        Ogre::StringVector parts(
          Ogre::StringUtil::split(line, PARSE_DELIMITER)
        );
        if (parts.size() < 2){
            Ogre::LogManager::getSingleton().stream()
              << "line is: '" << line << "'";
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "Lines should contain a variable name and an assignment",
              "RSDFileSerializer::parseLine"
            );
        }

        if (parts[0] == TAG_POLYGON) dest->SetPolygonName(parts[1]);
        else if (parts[0] == TAG_MATERIAL) dest->SetMaterialName(parts[1]);
        else if (parts[0] == TAG_GROUP) dest->SetGroupName(parts[1]);
        else if (parts[0] == TAG_TEXTURE_COUNT){
            if (has_texture_count_){
                Ogre::LogManager::getSingleton().stream() << line;
                OGRE_EXCEPT(
                  Ogre::Exception::ERR_INVALIDPARAMS,
                  "texture count already set",
                  "RSDFileSerializer::parseLine"
                );
            }
            texture_count_ = StringConverter::parseUnsignedInt(parts.back());
            dest->GetTextureNames().resize(texture_count_, "");
            has_texture_count_ = true;
        }
        else if (parts[0] == TAG_TEXTURE_NAME){
            if (!has_texture_count_){
                Ogre::LogManager::getSingleton().stream() << line;
                OGRE_EXCEPT(
                  Ogre::Exception::ERR_INVALIDPARAMS,
                  "can't set texture names if texture count is not set",
                  "RSDFileSerializer::parseLine"
                );
            }
            if (parts.size() != 3){
                Ogre::LogManager::getSingleton().stream() << line;
                OGRE_EXCEPT(
                  Ogre::Exception::ERR_INVALIDPARAMS,
                  "can't set texture name unknown format, array assignment "
                    "should look like " + TAG_TEXTURE_NAME + "[0]=filename.ext",
                  "RSDFileSerializer::parseLine"
                );
            }
            size_t texture_index(StringConverter::parseUnsignedInt(parts[1]));
            String texture_name(parts[2]);
            if (texture_index >= texture_count_){
                Ogre::LogManager::getSingleton().stream()
                  << "Warning: Index Out of Bounds in texture name array '"
                  << line << "' RSDFileSerializer::parseLine";
            }
            dest->GetTextureNames()[texture_index] = texture_name;
        }
    }

    void RSDFileSerializer::ImportRSDFile(
      Ogre::DataStreamPtr &stream, RSDFile* dest
    ){
        texture_count_ = 0;
        has_texture_count_ = false;
        readFileHeader(stream);
        while (!stream->eof()) ParseLine(GetLine(stream), dest);
        if (!has_texture_count_){
            Ogre::LogManager::getSingleton().stream()
              << "Warning: no texture count was read"
              << " RSDFileSerializer::importRSDFile";
        }
    }


}
