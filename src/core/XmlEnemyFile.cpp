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
    enemy.SetModel(GetString(node, "model"));
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
            for (TiXmlNode* node_elm = node->FirstChild(); node_elm != nullptr; node_elm = node_elm->NextSibling()){
                if (node_elm->Type() != TiXmlNode::TINYXML_ELEMENT || node_elm->ValueStr() != "element") continue;
                Enemy::Element elm;
                elm.id = GetInt(node_elm, "id");
                elm.factor = GetFloat(node_elm, "factor");
                enemy.AddElement(elm);
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "immunities"){
            for (TiXmlNode* node_imm = node->FirstChild(); node_imm != nullptr; node_imm = node_imm->NextSibling()){
                if (node_imm->Type() != TiXmlNode::TINYXML_ELEMENT || node_imm->ValueStr() != "immunity") continue;
                Enemy::Immunity imm;
                imm.status = GetInt(node_imm, "status");
                imm.rate = GetFloat(node_imm, "rate");
                enemy.AddImmunity(imm);
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "attacks"){
            for (TiXmlNode* node_atk = node->FirstChild(); node_atk != nullptr; node_atk = node_atk->NextSibling()){
                if (node_atk->Type() != TiXmlNode::TINYXML_ELEMENT || node_atk->ValueStr() != "attack") continue;
                Enemy::Attack atk;
                atk.id = GetInt(node_atk, "id");
                atk.animation = GetInt(node_atk, "animation", -1);
                atk.camera = GetInt(node_atk, "camera", -1);
                enemy.AddAttack(atk);
            }
        }
        else if (
          node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "manipulate"
        ){
            // TODO
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "ai"){
            // AI scripts, as hexadecimal bytecode.
            for (TiXmlNode* script = node->FirstChild(); script != nullptr; script = script->NextSibling()){
                if (script->Type() != TiXmlNode::TINYXML_ELEMENT || script->ValueStr() != "script") continue;
                const std::string hex = GetString(script, "data");
                std::string code;
                for (size_t i = 0; i + 1 < hex.size(); i += 2){
                    code.push_back(static_cast<char>(std::stoi(hex.substr(i, 2), nullptr, 16)));
                }
                enemy.SetAiScript(GetInt(script, "slot"), code);
            }
        }
        else if (
          node->Type() == TiXmlNode::TINYXML_ELEMENT
          && (node->ValueStr() == "steal" || node->ValueStr() == "drop" || node->ValueStr() == "drops")
        ){
            const bool is_steal = node->ValueStr() == "steal";
            TiXmlNode* node_item = node->FirstChild();
            while (node_item != nullptr){
                if (node_item->Type() == TiXmlNode::TINYXML_ELEMENT && node_item->ValueStr() == "item"){
                    Enemy::Item item;
                    item.id = GetInt(node_item, "id");
                    item.rate = GetFloat(node_item, "rate");
                    if (is_steal) enemy.AddSteal(item);
                    else enemy.AddDrop(item);
                }
                node_item = node_item->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "animations"){
            TiXmlNode* node_anim = node->FirstChild();
            while (node_anim != nullptr){
                if (node_anim->Type() == TiXmlNode::TINYXML_ELEMENT){
                    enemy.AddAnimation(GetInt(node_anim, "id"));
                }
                node_anim = node_anim->NextSibling();
            }
        }
        node = node->NextSibling();
    }
}

