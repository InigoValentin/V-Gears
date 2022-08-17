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
#include "data/QGearsXMLSerializer.h"

namespace QGears{
    XMLSerializer::XMLSerializer(){}

    XMLSerializer::~XMLSerializer(){}

    void XMLSerializer::Parse(Ogre::DataStreamPtr &stream, TiXmlDocument &dest){
        dest.Parse(stream->getAsString().c_str());
    }

    const String* XMLSerializer::ReadAttribute(
      TiXmlNode &node, const String &attribute
    ){
        assertElement(node);
        return node.ToElement()->Attribute(attribute);
    }

    bool XMLSerializer::ReadAttribute(
      TiXmlNode &node, const String &attribute, bool &dest, const bool &def
    ){
        const String *value(ReadAttribute(node, attribute));
        if (value == nullptr){
            dest = def;
            return false;
        }
        dest = Ogre::StringConverter::parseBool(*value);
        return true;
    }

    bool XMLSerializer::ReadAttribute(
      TiXmlNode &node, const String &attribute, int &dest, const int &def
    ){
        const String *value(ReadAttribute(node, attribute));
        if (value == nullptr){
            dest = def;
            return false;
        }
        dest = Ogre::StringConverter::parseInt(*value);
        return true;
    }

    bool XMLSerializer::ReadAttribute(TiXmlNode &node, const String &attribute
                                , String &dest, const String &def){
        const String *value(ReadAttribute(node, attribute));
        if (value == nullptr){
            dest = def;
            return false;
        }
        dest = *value;
        return true;
    }

    bool XMLSerializer::ReadAttribute(
      TiXmlNode &node, const String &attribute, Ogre::Real &dest,
      const Ogre::Real &def
    ){
        const String *value(ReadAttribute(node, attribute));
        if (value == nullptr){
            dest = def;
            return false;
        }
        dest = Ogre::StringConverter::parseReal(*value);
        return true;
    }

    bool XMLSerializer::ReadAttribute(
      TiXmlNode &node, const String &attribute, Ogre::Vector2 &dest,
      const Ogre::Vector2 &def
    ){
        const String *value(ReadAttribute(node, attribute));
        if (value == nullptr){
            dest = def;
            return false;
        }
        dest = Ogre::StringConverter::parseVector2(*value);
        return true;
    }

    bool XMLSerializer::ReadAttribute(
      TiXmlNode &node, const String &attribute, Ogre::Vector3 &dest,
      const Ogre::Vector3 &def
    ){
        const String *value(ReadAttribute(node, attribute));
        if (value == nullptr){
            dest = def;
            return false;
        }
        dest = Ogre::StringConverter::parseVector3(*value);
        return true;
    }

    bool XMLSerializer::ReadAttribute(
      TiXmlNode &node, const String &attribute, Ogre::Vector4 &dest,
      const Ogre::Vector4 &def
    ){
        const String *value(ReadAttribute(node, attribute));
        if (value == nullptr){
            dest = def;
            return false;
        }
        dest = Ogre::StringConverter::parseVector4(*value);
        return true;
    }

    bool XMLSerializer::ReadAttribute(
      TiXmlNode &node, const String &attribute, Ogre::Quaternion &dest,
      const Ogre::Quaternion &def
    ){
        const String *value(ReadAttribute(node, attribute));
        if (value == nullptr){
            dest = def;
            return false;
        }
        dest = Ogre::StringConverter::parseQuaternion(*value);
        return true;
    }

    TiXmlNode* XMLSerializer::FindChildNode(TiXmlNode &node, const String &tag){
        TiXmlNode* child(node.FirstChild());
        while (child != nullptr && child->ValueStr() != tag)
            child = child->NextSibling();
        return child;
    }

}
