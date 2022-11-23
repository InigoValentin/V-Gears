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
#include "data/VGearsSerializer.h"

namespace VGears{

    typedef Ogre::StringUtil StringUtil;

    const String Serializer::TAG_COMMENT("#");

    Serializer::Serializer(){
        determineEndianness(Ogre::Serializer::ENDIAN_LITTLE);
    }

    Serializer::~Serializer(){}

    void Serializer::readObject(
      Ogre::DataStreamPtr &stream, Ogre::Vector2 &dest
    ){
        float v[2];
        readFloats(stream, v, 2);
        dest.x = v[0];
        dest.y = v[1];
    }

    void Serializer::readObject(
      Ogre::DataStreamPtr &stream, Ogre::Vector3 &dest
    ){
        float tmp[3];
        readFloats(stream, tmp, 3);
        dest.x = tmp[0];
        dest.y = tmp[1];
        dest.z = tmp[2];
    }

    void Serializer::readObject(
      Ogre::DataStreamPtr &stream, Ogre::AxisAlignedBox &dest
    ){
        Ogre::Vector3 tmp;
        readObject(stream, tmp);
        dest.setMaximum(tmp);
        readObject(stream, tmp);
        dest.setMinimum(tmp);
    }

    void Serializer::readObject(Ogre::DataStreamPtr &stream, Pixel &dest){
        stream->read(&dest, sizeof(dest));
        flipFromLittleEndian(&dest, 2, 2);
    }

    void Serializer::ReadChars(
      Ogre::DataStreamPtr& stream, char* dest, size_t count
    ){stream->read(dest, count);}

    void Serializer::Read1ByteBool(Ogre::DataStreamPtr &stream, bool &dest){
        uint8 enabled;
        stream->read(&enabled, 1);
        dest = enabled > 0;
    }

    void Serializer::Read2ByteBool(Ogre::DataStreamPtr &stream, bool &dest){
        uint16 enabled;
        ReadShort(stream, enabled);
        dest = enabled > 0;
    }

    void Serializer::ReadShort(Ogre::DataStreamPtr &stream, uint16 &dest){
        readShorts(stream, &dest, 1);
    }

    void Serializer::ReadInt16(Ogre::DataStream &stream, sint16 &dest){
        stream.read(&dest, sizeof(sint16));
    }

    void Serializer::ReadUInt16(Ogre::DataStream &stream, uint16 &dest){
        stream.read(&dest, sizeof(uint16));
    }

    void Serializer::ReadInt16(Ogre::DataStreamPtr &stream, sint16 &dest){
        stream->read(&dest, sizeof(sint16));
    }

    void Serializer::ReadUInt16(Ogre::DataStreamPtr &stream, uint16 &dest){
        readShorts(stream, &dest, 1);
    }

    void Serializer::ReadUInt32(Ogre::DataStreamPtr &stream, uint32 &dest){
        readInts(stream, &dest, 1);
    }

    void Serializer::ReadSInt32(Ogre::DataStreamPtr &stream, sint32 &dest){
        stream->read(&dest, sizeof(sint32));
    }

    void Serializer::ReadUInt32(Ogre::DataStream &stream, uint32 &dest){
        stream.read(&dest,sizeof(uint32));
    }

    void Serializer::ReadUInt8(Ogre::DataStream& stream, uint8 &dest){
        stream.read(&dest,1);
    }

    void Serializer::ReadUInt8(Ogre::DataStreamPtr &stream, uint8 &dest){
        stream->read(&dest,1);
    }

    void Serializer::ReadFloat(Ogre::DataStreamPtr &stream, float &dest){
        readFloats(stream, &dest, 1);
    }

    String Serializer::GetLine(Ogre::DataStreamPtr &stream) const{
        String line("");
        do{
            line = stream->getLine();
        } while(StringUtil::startsWith(line, TAG_COMMENT));
        return line;
    }

    void Serializer::ReadEndString(
      Ogre::DataStreamPtr &stream, const String &end_text
    ){
        String actual(readString(stream, end_text.size()));
        if (actual != end_text){
            Ogre::LogManager::getSingleton().stream()
              << "Warning: File didn't end with"
              << " expected String '" << end_text << "'"
              << " actual was '" << actual << "'";
        }
    }

}
