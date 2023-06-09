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

#include "core/XmlFormationFile.h"
#include "core/BattleManager.h"
#include "core/Logger.h"

XmlFormationFile::XmlFormationFile(const Ogre::String& file): XmlFile(file){}

XmlFormationFile::~XmlFormationFile(){}

void XmlFormationFile::LoadFormation(){
    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "formation"){
        LOG_ERROR(file_.ValueStr() + " is not a valid fields map file! No <formation> in root.");
        return;
    }
    BattleManager::getSingleton().SetFormationId(GetInt(node, "id"));
    BattleManager::getSingleton().SetNextFormationId(GetInt(node, "next"));
    if (GetInt(node, "escapable") == 1)
        BattleManager::getSingleton().SetEscapeability(GetFloat(node, "escape_difficulty"));
    else BattleManager::getSingleton().SetEscapeability(-1.0f);
    BattleManager::getSingleton().SetSkipVictoryPose(GetInt(node, "skip_victory_pose") == 1);
    BattleManager::getSingleton().SetSkipSpoils(GetInt(node, "skip_spoils") == 1);
    node = node->FirstChild();
    while (node != nullptr){
        // Location data.
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "location"){
            int id = GetInt(node, "id");
            Ogre::String name(GetString(node, "name"));
            BattleManager::getSingleton().SetLocation(id, name);
        }
        else if (
          node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "arena"
        ){
            bool is_arena = GetInt(node, "is_arena") == 1;
            BattleManager::getSingleton().SetArenaBattle(is_arena);
            if (is_arena){
                // TODO: Read more info for arena battles
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "camera"){
            BattleManager::getSingleton().SetInitialCamera(GetInt(node, "initial"));
            // TODO: Loop cameras
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "enemies"){
            // TODO: Loop enemies
        }
        node = node->NextSibling();
    }
}

