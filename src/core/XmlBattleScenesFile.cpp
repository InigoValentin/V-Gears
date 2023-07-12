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

#include "core/XmlBattleScenesFile.h"
#include "core/Logger.h"

XmlBattleScenesFile::XmlBattleScenesFile(const Ogre::String& file): XmlFile(file){

    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "scenes"){
        LOG_ERROR(file_.ValueStr() + " is not a valid battle scenes file! No <scenes> in root.");
        return;
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (node->ValueStr() != "scene"){
            node = node->NextSibling();
            continue;
        }
        BattleScene scene;
        scene.id = GetInt(node, "id");
        scene.model = GetString(node, "file");
        scene.description = GetString(node, "description");
        list_.push_back(scene);
        node = node->NextSibling();
    }
}

XmlBattleScenesFile::~XmlBattleScenesFile(){}

std::vector<XmlBattleScenesFile::BattleScene> XmlBattleScenesFile::GetScenes(){return list_;}
