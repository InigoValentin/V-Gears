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

#include <iostream>
#include "core/Logger.h"
#include "core/UiManager.h"
#include "core/UiFont.h"
#include "core/XmlFontFile.h"


XmlFontFile::XmlFontFile(const Ogre::String& file): XmlFile(file){}

XmlFontFile::~XmlFontFile(){}

void XmlFontFile::LoadFont(){
    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "font"){
        LOG_ERROR(file_.ValueStr() + " is not a valid font file! No <font> in root.");
        return;
    }
    Ogre::String name = GetString(node, "name", "");
    Ogre::String language = GetString( node, "language", "");
    Ogre::String image = GetString(node, "image", "");
    Ogre::Vector2 size = GetVector2(node, "image_size", Ogre::Vector2::ZERO);
    int height = GetInt(node, "height", 0);
    if (name != "" && image != "" && size.x != 0 && size.y != 0){
        UiFont* font = new UiFont(name, language);
        font->SetImage(image, static_cast<int>(size.x), static_cast<int>(size.y));
        font->SetHeight(height);
        node = node->FirstChild();
        while (node != nullptr){
            if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "char"){
                UiCharData data;
                Ogre::UTFString node_name = GetUTFString(node, "name", "");
                if (node_name != "") data.char_code = *(node_name.c_str());
                data.x = GetInt(node, "x", 0);
                data.y = GetInt(node, "y", 0);
                data.width = GetInt(node, "width", 0);
                data.height = GetInt(node, "height", 0);
                data.pre = GetInt(node, "pre", 0);
                data.post = GetInt(node, "post", 0);
                font->AddCharData(data);
            }
            node = node->NextSibling();
        }
        UiManager::getSingleton().AddFont(font);
    }
}
