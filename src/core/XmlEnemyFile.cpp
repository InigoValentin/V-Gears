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
    enemy.SetId(GetInt(node, "id"));
    enemy.SetName(GetString(node, "name"));
    enemy.SetLevel(GetInt(node, "level"));
    enemy.SetExp(GetInt(node, "exp"));
    enemy.SetAp(GetInt(node, "ap"));
    enemy.SetMoney(GetInt(node, "money"));
    enemy.SetMorph(GetInt(node, "morph"));
    enemy.SetBackDamage(GetFloat(node, "back_damage"));
    node = node->FirstChild();
    while (node != nullptr){
        // Location data.
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "stats"){
            TiXmlNode* node_stat = node->FirstChild();
            while (node_stat != nullptr){
                int value = GetInt(node_stat, "value");
                if (GetString(node_stat, "id") == "str") enemy.SetStr(value);
                else if (GetString(node_stat, "id") == "mag") enemy.SetMag(value);
                else if (GetString(node_stat, "id") == "def") enemy.SetDef(value);
                else if (GetString(node_stat, "id") == "mdef") enemy.SetSpr(value);
                else if (GetString(node_stat, "id") == "spd") enemy.SetDex(value);
                else if (GetString(node_stat, "id") == "lck") enemy.SetLck(value);
                else if (GetString(node_stat, "id") == "eva") enemy.SetEva(value);
                node_stat = node_stat->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "elements"){
            TiXmlNode* node_elm = node->FirstChild();
            while (node_elm != nullptr && node->ValueStr() == "element"){
                Enemy::Element elm;
                elm.id = GetInt(node_elm, "element");
                elm.factor = GetFloat(node_elm, "factor");
                enemy.AddElement(elm);
                node_elm = node_elm->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "immunities"){
            TiXmlNode* node_imm = node->FirstChild();
            while (node_imm != nullptr && node->ValueStr() == "element"){
                Enemy::Immunity imm;
                imm.status = GetInt(node_imm, "immunity");
                imm.rate = GetFloat(node_imm, "rate");
                enemy.AddImmunity(imm);
                node_imm = node_imm->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "attacks"){
            // TODO
        }
        else if (
          node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "manipulate"
        ){
            // TODO
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "steal"){
            // TODO
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "drops"){
            // TODO
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "animations"){
            // TODO
        }
        node = node->NextSibling();
    }
}

