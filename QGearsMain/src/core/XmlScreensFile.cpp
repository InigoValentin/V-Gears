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
#include "core/XmlScreenFile.h"
#include "core/XmlScreensFile.h"
#include "core/XmlPrototypesFile.h"

XmlScreensFile::XmlScreensFile(const Ogre::String& file): XmlFile(file){}

XmlScreensFile::~XmlScreensFile(){}

void XmlScreensFile::LoadScreens(){
    TiXmlNode* node = file_.RootElement();

    if (node == nullptr || node->ValueStr() != "ui"){
        LOG_ERROR(
          "UI XML Manager: " + file_.ValueStr()
          + " is not a valid ui file! No <ui> in root."
        );
        return;
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (
          node->Type() == TiXmlNode::TINYXML_ELEMENT
          && node->ValueStr() == "prototype"
        ){
            XmlPrototypesFile prototypes(
              "./data/" + GetString(node, "file_name")
            );
            prototypes.LoadPrototypes();
        }
        node = node->NextSibling();
    }
    node = file_.RootElement();
    node = node->FirstChild();
    while (node != NULL){
        if (
          node->Type() == TiXmlNode::TINYXML_ELEMENT
          && node->ValueStr() == "screen"
        ){
            XmlScreenFile screen("./data/" + GetString(node, "file_name"));
            screen.LoadScreen();
        }
        node = node->NextSibling();
    }
}
