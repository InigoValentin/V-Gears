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
#include "core/XmlPrototypesFile.h"

XmlPrototypesFile::XmlPrototypesFile(const Ogre::String& file): XmlFile(file){}

XmlPrototypesFile::~XmlPrototypesFile(){}

void XmlPrototypesFile::LoadPrototypes(){
    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "prototypes"){
        LOG_ERROR(
          "UI Manager: " + file_.ValueStr()
          + " is not a valid prototypes file! No <prototypes> in root."
        );
        return;
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (
          node->Type() == TiXmlNode::TINYXML_ELEMENT
          && node->ValueStr() == "prototype"
        ){
            Ogre::String name = GetString(node, "name");
            UiManager::getSingleton().AddPrototype(name, node->Clone());
        }
        node = node->NextSibling();
    }
}
