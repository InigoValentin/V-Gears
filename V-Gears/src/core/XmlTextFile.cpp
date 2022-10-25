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
#include "core/UiManager.h"
#include "core/XmlTextFile.h"
#include "core/TextManager.h"

XmlTextFile::XmlTextFile(const Ogre::String& file): XmlFile(file){}

XmlTextFile::~XmlTextFile(){}

void XmlTextFile::LoadTexts(){
    TiXmlNode* node = file_.RootElement();
    if(node == nullptr || node->ValueStr() != "texts"){
        LOG_ERROR(
          "Text Manager: " + file_.ValueStr() + " is not a valid text file! No <texts> in root."
        );
        return;
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "text"){
            Ogre::String name = GetString(node, "name");
            TextManager::getSingleton().AddText(name, node->Clone());
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "dialog"){
            Ogre::String name = GetString( node, "name" );
            float width = GetFloat( node, "width", 0.0f );
            float height = GetFloat( node, "height", 0.0f );
            TextManager::getSingleton().AddDialog(name, node->Clone(), width, height);
        }
        node = node->NextSibling();
    }
}
