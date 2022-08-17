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

#include "core/XmlWalkmeshFile.h"
#include "core/EntityManager.h"
#include "core/Logger.h"
#include "core/Walkmesh.h"

XmlWalkmeshFile::XmlWalkmeshFile(const Ogre::String& file): XmlFile(file){}

XmlWalkmeshFile::~XmlWalkmeshFile(){}

void XmlWalkmeshFile::Load(){
    TiXmlNode* node = file_.RootElement();
    Walkmesh* walkmesh = EntityManager::getSingleton().GetWalkmesh();
    if (node == nullptr || node->ValueStr() != "walkmesh"){
        LOG_ERROR(
          "Field XML Manager: " + file_.ValueStr()
          + " is not a valid walkmesh file! No <walkmesh> in root."
        );
        return;
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (
          node->Type() == TiXmlNode::TINYXML_ELEMENT
          && node->ValueStr() == "triangle"
        ){
            WalkmeshTriangle triangle;
            triangle.a = GetVector3(node, "a");
            triangle.b = GetVector3(node, "b");
            triangle.c = GetVector3(node, "c");
            triangle.access_side[ 0 ] = GetInt(node, "a_b");
            triangle.access_side[ 1 ] = GetInt(node, "b_c");
            triangle.access_side[ 2 ] = GetInt(node, "c_a");
            walkmesh->AddTriangle(triangle);
        }
        node = node->NextSibling();
    }
}
