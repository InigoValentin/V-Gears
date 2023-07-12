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

#include "core/XmlBattleCharactersFile.h"
#include "core/Logger.h"

XmlBattleCharactersFile::XmlBattleCharactersFile(const Ogre::String& file): XmlFile(file){

    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "characters"){
        LOG_ERROR(
          file_.ValueStr() + " is not a valid battle characters file! No <characters> in root."
        );
        return;
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (node->ValueStr() != "character"){
            node = node->NextSibling();
            continue;
        }
        BattleCharacter character;
        character.id = GetInt(node, "id");
        TiXmlNode* model_node = node->FirstChild();
        while (model_node != nullptr){
            if (model_node->ValueStr() != "model"){
                model_node = model_node->NextSibling();
                continue;
            }
            BattleCharacterModel model;
            model.model = GetString(model_node, "file");
            model.name = GetString(model_node, "name");
            character.models.push_back(model);
            model_node = model_node->NextSibling();
        }
        list_.push_back(character);
        node = node->NextSibling();
    }
}

XmlBattleCharactersFile::~XmlBattleCharactersFile(){}

std::vector<XmlBattleCharactersFile::BattleCharacter> XmlBattleCharactersFile::GetCharacters(){
    return list_;
}
