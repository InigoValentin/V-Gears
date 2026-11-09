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

#include "core/XmlAttackFile.h"
#include "core/Logger.h"

XmlAttackFile::XmlAttackFile(const Ogre::String& file): XmlFile(file){}

XmlAttackFile::~XmlAttackFile(){}

void XmlAttackFile::ReadParameters(TiXmlNode* node, std::map<std::string, float>& parameters) const{
    for (TiXmlNode* child = node->FirstChild(); child != nullptr; child = child->NextSibling()){
        if (child->Type() != TiXmlNode::TINYXML_ELEMENT || child->ValueStr() != "Parameter") continue;
        parameters[GetString(child, "id")] = GetFloat(child, "value");
    }
}

bool XmlAttackFile::LoadAttack(AttackData& attack){
    TiXmlNode* root = file_.RootElement();
    if (root == nullptr || root->ValueStr() != "Attack"){
        LOG_ERROR(file_.ValueStr() + " is not a valid attack file! No <Attack> in root.");
        return false;
    }
    attack.id = GetInt(root, "id");
    attack.name = GetString(root, "name");
    attack.mp = GetInt(root, "mp");
    attack.reflectable = GetInt(root, "reflectable") == 1;
    for (TiXmlNode* node = root->FirstChild(); node != nullptr; node = node->NextSibling()){
        if (node->Type() != TiXmlNode::TINYXML_ELEMENT) continue;
        if (node->ValueStr() == "Damage") ReadParameters(node, attack.damage);
        else if (node->ValueStr() == "Target") ReadParameters(node, attack.target);
        else if (node->ValueStr() == "Animation") ReadParameters(node, attack.animation);
        else if (node->ValueStr() == "Effects"){
            for (TiXmlNode* e = node->FirstChild(); e != nullptr; e = e->NextSibling()){
                if (e->Type() != TiXmlNode::TINYXML_ELEMENT || e->ValueStr() != "Effect") continue;
                AttackData::Effect effect;
                effect.id = GetInt(e, "id");
                effect.chance = GetFloat(e, "chance");
                effect.mode = GetString(e, "mode");
                attack.effects.push_back(effect);
            }
        }
        else if (node->ValueStr() == "Elements"){
            for (TiXmlNode* e = node->FirstChild(); e != nullptr; e = e->NextSibling()){
                if (e->Type() != TiXmlNode::TINYXML_ELEMENT || e->ValueStr() != "Element") continue;
                attack.elements.push_back(GetInt(e, "id"));
            }
        }
    }
    return true;
}

std::map<int, std::string> XmlAttackFile::LoadIndex(const Ogre::String& file){
    std::map<int, std::string> index;
    XmlAttackFile xml(file);
    TiXmlNode* root = xml.file_.RootElement();
    if (root == nullptr || root->ValueStr() != "attacks"){
        LOG_ERROR(file + " is not a valid attack index! No <attacks> in root.");
        return index;
    }
    for (TiXmlNode* node = root->FirstChild(); node != nullptr; node = node->NextSibling()){
        if (node->Type() != TiXmlNode::TINYXML_ELEMENT || node->ValueStr() != "attack") continue;
        index[xml.GetInt(node, "id")] = xml.GetString(node, "file");
    }
    return index;
}
