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

#include "core/XmlEnemyFile.h"
#include "core/Enemy.h"
#include "core/Logger.h"

XmlEnemyFile::XmlEnemyFile(const Ogre::String& file): XmlFile(file){}

XmlEnemyFile::~XmlEnemyFile(){}

void XmlEnemyFile::LoadEnemy(Enemy& enemy){
    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "enemy"){
        LOG_ERROR(file_.ValueStr() + " is not a valid enemy map file! No <enemy> in root.");
        return;
    }
    enemy.SetEnemyId(GetInt(node, "id"));
    enemy.SetName(GetString(node, "name"));
    enemy.SetLevel(GetInt(node, "level"));
    enemy.SetExp(GetInt(node, "exp"));
    enemy.SetAp(GetInt(node, "ap"));
    enemy.SetMoney(GetInt(node, "money"));
    enemy.SetMorph(GetInt(node, "morph"));
    enemy.SetBackDamage(GetFloat(node, "back_damage"));
    node = node->FirstChild();
    while (node != nullptr){
        /*// Location data.
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
        }*/
        node = node->NextSibling();
    }
}

