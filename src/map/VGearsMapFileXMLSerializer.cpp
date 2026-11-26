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
#include "map/VGearsMapFileXMLSerializer.h"

namespace VGears{

    MapFileXMLSerializer::MapFileXMLSerializer() : XMLSerializer(){}
    
    MapFileXMLSerializer::~MapFileXMLSerializer(){}
    
    void MapFileXMLSerializer::ReadHeader(TiXmlNode *node){
        if (node == nullptr || node->ValueStr() != "map"){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "not a valid map file, no <map> in root",
              "MapFileXMLSerializer::readHeader"
            );
        }
    }

    void MapFileXMLSerializer::ImportMapFile(
      Ogre::DataStreamPtr &stream, MapFile *dest
    ){
        TiXmlDocument document;
        Parse(stream, document);
        TiXmlNode *root_node(document.RootElement());
        ReadHeader(root_node);
        TiXmlNode &node(*root_node);
        ReadScript(node, dest);
        ReadBackground2D(node, dest);
        ReadWalkmesh(node, dest);
        ReadForwardDirection(node, dest);
        ReadEntities(node, dest);
    }
    
    void MapFileXMLSerializer::ReadScript(TiXmlNode &node, MapFile *dest){
        TiXmlNode *child(FindChildNode(node, "script"));
        if (child != nullptr){
            String file_name;
            ReadAttribute(*child, "file_name", file_name);
            dest->SetScriptName(file_name);
        }
    }

    void MapFileXMLSerializer::ReadBackground2D(TiXmlNode &node, MapFile *dest){
        TiXmlNode *child(FindChildNode(node, "background2d"));
        if (child != nullptr){
            String file_name;
            ReadAttribute(*child, "file_name", file_name);
            dest->SetBackground2dName(file_name);
        }
    }

    void MapFileXMLSerializer::ReadWalkmesh(TiXmlNode &node, MapFile *dest){
        TiXmlNode *child(FindChildNode(node, "walkmesh"));
        if (child != nullptr){
            String file_name;
            ReadAttribute(*child, "file_name", file_name);
            dest->SetWalkmeshName(file_name);
        }
    }

    void MapFileXMLSerializer::ReadForwardDirection(
      TiXmlNode &node, MapFile *dest
    ){
        TiXmlNode *child(FindChildNode(node, "movement_rotation"));
        if (child != nullptr){
            Ogre::Real degree;
            ReadAttribute(*child, "degree", degree);
            dest->SetForwardDirection(degree);
        }
    }

    void MapFileXMLSerializer::ReadEntities(TiXmlNode &node, MapFile *dest){
        // TODO: loop through all entities once and create corresponding
        // via factory pattern.
        ReadEntities(node, "entity_point", dest->GetPoints());
        ReadEntities(node, "entity_trigger", dest->GetTriggers());
    }

    void MapFileXMLSerializer::readObject(
      TiXmlNode &node, MapFileXMLSerializer::Point &dest
    ){
        Ogre::Vector3  position;
        Ogre::Real rotation;
        ReadAttribute(node, "position", position);
        ReadAttribute(node, "rotation", rotation);
        dest.SetPosition(position);
        dest.SetRotation(rotation);
    }

    void MapFileXMLSerializer::readObject(
      TiXmlNode &node, MapFileXMLSerializer::Trigger &dest
    ){
        Ogre::Vector3 point1, point2;
        bool enabled;
        ReadAttribute(node, "point1", point1);
        ReadAttribute(node, "point2", point2);
        ReadAttribute(node, "enabled", enabled);
        dest.SetPoints(point1, point2);
        dest.SetEnabled(enabled);
    }

    template<typename ValueType> void MapFileXMLSerializer::ReadEntities(
      TiXmlNode &node, const String &tag, std::vector<ValueType> &dest
    ){
        dest.clear();
        TiXmlNode* child(node.FirstChild());
        while (child != nullptr){
            if (
              child->Type() == TiXmlNode::TINYXML_ELEMENT
              && child->ValueStr() == tag
            ){
                String name;
                ReadAttribute(*child, "name", name);
                ValueType in_tmp(name);
                readObject(*child, in_tmp);
                dest.push_back(in_tmp);
            }
            child = child->NextSibling();
        }
    }

}

