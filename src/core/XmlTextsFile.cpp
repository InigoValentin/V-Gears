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
#include "core/XmlTextFile.h"
#include "core/XmlTextsFile.h"
#include "core/TextManager.h"

XmlTextsFile::XmlTextsFile(const Ogre::String& file): XmlFile(file){}

XmlTextsFile::~XmlTextsFile(){}

void XmlTextsFile::GetAvailableLanguages(Ogre::StringVector& languages){
    TiXmlNode* node = file_.RootElement();
    if (node == NULL || node->ValueStr() != "texts"){
        LOG_ERROR(
          "UI Text Manager: " + file_.ValueStr() + " is not a valid texts file! No <texts> in root."
        );
        return;
    }
    node = node->FirstChild();
    while (node != NULL){
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "language")
            languages.push_back(GetString(node, "name"));
        node = node->NextSibling();
    }
}

void XmlTextsFile::LoadTexts(){
    TiXmlNode* node = file_.RootElement();
    if (node == NULL || node->ValueStr() != "texts"){
        LOG_ERROR(
          "Text Manager: " + file_.ValueStr() + " is not a valid texts file! No <texts> in root."
        );
        return;
    }
    Ogre::String language = TextManager::getSingleton().GetLanguage();
    node = node->FirstChild();
    while (node != nullptr){
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "language"){
            if (GetString(node, "name") == language){
                TiXmlNode* node2 = node->FirstChild();
                while (node2 != nullptr){
                    if (node2->Type() == TiXmlNode::TINYXML_ELEMENT && node2->ValueStr() == "text"){
                        Ogre::String file = GetString(node2, "file");
                        if (file != ""){
                            XmlTextFile text("./data/" + file);
                            text.LoadTexts();
                        }
                        else LOG_ERROR("Empty filename in language " + language + ".");
                    }
                    node2 = node2->NextSibling();
                }
            }
        }
        node = node->NextSibling();
    }
}
