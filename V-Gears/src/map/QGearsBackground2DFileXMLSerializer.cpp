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
#include "map/QGearsBackground2DFileXMLSerializer.h"

namespace QGears{

    const String Background2DFileXMLSerializer::BLENDING_ALPHA("alpha");

    const String Background2DFileXMLSerializer::BLENDING_ADD("add");

    const String Background2DFileXMLSerializer::BLENDING_SUBTRACT("subtract");

    Background2DFileXMLSerializer::Background2DFileXMLSerializer() :
        XMLSerializer(){
    }

    Background2DFileXMLSerializer::~Background2DFileXMLSerializer(){}

    void Background2DFileXMLSerializer::ReadHeader(TiXmlNode *node){
        if(node == nullptr || node->ValueStr() != "background2d"){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "not a valid background2d file, no <background2d> in root",
              "Background2DFileXMLSerializer::readHeader"
            );
        }
    }

    bool Background2DFileXMLSerializer::ReadAttribute(
      TiXmlNode &node, const String &attribute, Blending &dest,
      const Blending &def
    ){
        const String *value(ReadAttribute(node, attribute));
        if (value == nullptr){
            dest = def;
            return false;
        }
        if (*value == BLENDING_ADD){
            dest = B_ADD;
            return true;
        }
        if (*value == BLENDING_ALPHA){
            dest = B_ALPHA;
            return true;
        }
        if (*value == BLENDING_SUBTRACT){
            dest = B_SUBTRACT;
            return true;
        }
        dest = def;
        return false;
    }

    void Background2DFileXMLSerializer::ImportBackground2DFile(
      Ogre::DataStreamPtr &stream, Background2DFile *dest
    ){
        TiXmlDocument document;
        Parse(stream, document);
        TiXmlNode *root_node(document.RootElement());
        ReadHeader(root_node);
        TiXmlNode &node(*root_node);
        String texture_name;
        ReadAttribute(node, "image", texture_name);
        if (texture_name.empty()){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "image attribute not set",
              "Background2DFileXMLSerializer::ImportBackground2DFile"
            );
        }
        dest->SetTextureName(texture_name);
        Ogre::Vector2 clip;
        ReadAttribute(node, "clip", clip, Ogre::Vector2(320, 240));
        dest->SetClip(clip);
        Ogre::Vector4 range;
        ReadAttribute(node, "range", range, Ogre::Vector4(
          -100000, -100000, 100000, 100000
        ));
        dest->SetRange(range);
        Ogre::Vector3 position;
        ReadAttribute(node, "position", position);
        dest->SetPosition(position);
        Ogre::Quaternion orientation;
        ReadAttribute(node, "orientation", orientation);
        dest->SetOrientation(orientation);
        Ogre::Real fov;
        ReadAttribute(node, "fov", fov, Ogre::Real(45));
        dest->SetFov(Ogre::Radian(Ogre::Degree(fov)));
        ReadVector(node, "tile", dest->GetTiles());
    }


    void Background2DFileXMLSerializer::readObject(TiXmlNode& node, Tile& dest){
        ReadAttribute(node, "width", dest.width);
        ReadAttribute(node, "height", dest.height);
        ReadAttribute(node, "destination", dest.destination);
        ReadAttribute(node, "uv", dest.uv);
        ReadAttribute(node, "depth", dest.depth);
        ReadAttribute(node, "blending", dest.blending, B_ALPHA);
        ReadMap(node, "animation", "name", dest.animations);
    }

    void Background2DFileXMLSerializer::readObject(
      TiXmlNode& node, Animation& dest
    ){
        ReadAttribute(node, "length", dest.length);
        ReadVector(node, "keyframe", dest.key_frames);
    }

    void Background2DFileXMLSerializer::readObject(
      TiXmlNode& node, KeyFrame& dest
    ){
        ReadAttribute(node, "time", dest.time);
        ReadAttribute(node, "uv", dest.uv);
    }

}
