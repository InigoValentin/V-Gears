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
#include "core/TextManager.h"
#include "core/TextManagerCommands.h"
#include "core/XmlTextsFile.h"
#include "core/XmlTextFile.h"

/**
 * Text manager singleton.
 */
template<>TextManager* Ogre::Singleton< TextManager >::msSingleton = NULL;

TextManager::TextManager(): language_(""){
    for (int i = 0; i < 3; i ++) current_party_[i] = -1;
    for (int i = 0; i < 11; i ++) character_names_[i] = std::string("CHAR_") + std::to_string(i);
    InitCmd();
}

TextManager::~TextManager(){UnloadTexts();}

void TextManager::LoadFieldText(const std::string file_name){
    XmlTextFile texts("./data/" + file_name);
    texts.LoadTexts();
}

void TextManager::SetLanguage(const Ogre::String& language){
    language_ = language;
    UnloadTexts();
    XmlTextsFile texts("./data/texts.xml");
    texts.LoadTexts();
}

const Ogre::String& TextManager::GetLanguage(){return language_;}

void TextManager::AddText(const Ogre::String& name, TiXmlNode* node){
    Text text;
    text.name = name;
    text.node = node;
    texts_.push_back(text);
}

void TextManager::AddDialog(
  const Ogre::String& name, TiXmlNode* node, const float width, const float height
){
    Dialog dialog;
    dialog.name = name;
    dialog.node = node;
    dialog.width = width;
    dialog.height = height;
    dialogs_.push_back(dialog);
}

TiXmlNode* TextManager::GetText(const Ogre::String& name) const{
    for (unsigned int i = 0; i < texts_.size(); ++ i)
        if (texts_[i].name == name) return texts_[i].node;
    LOG_WARNING("Can't find text '" + name + "'.");
    return NULL;
}

TiXmlNode* TextManager::GetDialog(const Ogre::String& name, float &width, float& height) const{
    for (unsigned int i = 0; i < dialogs_.size(); ++ i){
        if (dialogs_[i].name == name){
            width = dialogs_[i].width;
            height = dialogs_[i].height;
            return dialogs_[i].node;
        }
    }
    return NULL;
}

std::string TextManager::GetDialogText(const std::string name){
    std::string text = "";
    for (unsigned int i = 0; i < dialogs_.size(); ++ i){
        if (dialogs_[i].name == name){
            text = dialogs_[i].node->FirstChild()->ToText()->Value();
            break;
        }
    }
    return text;
}
void TextManager::UnloadTexts(){
    for (unsigned int i = 0; i < texts_.size(); ++ i) delete texts_[i].node;
    texts_.clear();
    for (unsigned int i = 0; i < dialogs_.size(); ++ i) delete dialogs_[i].node;
    dialogs_.clear();
}

std::string TextManager::GetCharacterName(int id){
    if (id >= 0 && id < 11) return character_names_[id];
    else return std::string("UNKNOWN_CHAR_" + id);
}

std::string TextManager::GetPartyCharacterName(int position){
    if (position >= 1 && position <= 3){
        if (current_party_[position - 1] < 0) return "";
        else return GetCharacterName(current_party_[position - 1]);
    }
    else return std::string("UNKNOWN_PARTY_POS_" + position);
}

void TextManager::SetCharacterName(int id, char* name){
    if (id >= 0 && id < 11) character_names_[id] = std::string(name);
    else LOG_WARNING("Not setting character name for character ID " + std::to_string(id) + ".");
}

void TextManager::SetParty(int char_1, int char_2, int char_3){
    current_party_[0] = char_1;
    current_party_[2] = char_2;
    current_party_[3] = char_3;
}

