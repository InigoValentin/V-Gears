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

#include "core/Logger.h"
#include "core/XmlMapsFile.h"

XmlMapsFile::XmlMapsFile(const Ogre::String& file): XmlFile(file){}

XmlMapsFile::~XmlMapsFile(){}

const Ogre::String XmlMapsFile::GetMapFileNameByName(const Ogre::String& name){
    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "maps"){
        LOG_ERROR(
          "Field XML Manager: " + file_.ValueStr() + " is not a valid maps file! No <maps> in root."
        );
        return "";
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "map")
            if (GetString(node, "name") == name) return GetString(node, "file_name");
        node = node->NextSibling();
    }
    LOG_WARNING("Can't find map \"" + name + "\" in " + file_.ValueStr() + ".");
    return "";
}

void XmlMapsFile::GetMapNames(Ogre::StringVector& complete_params){
    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "maps"){
        LOG_ERROR(
          "Field XML Manager: " + file_.ValueStr() + " is not a valid maps file! No <maps> in root."
        );
        return;
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "map")
            complete_params.push_back(GetString(node, "name"));
        node = node->NextSibling();
    }
}
