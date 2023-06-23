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

#include "TextHandler.h"

#include <iostream>
#include "core/TextHandlerCommands.h"
#include "core/XmlTextsFile.h"
#include "core/XmlTextFile.h"

/**
 * Text handler singleton.
 */
template<>TextHandler* Ogre::Singleton<TextHandler>::msSingleton = NULL;

TextHandler::TextHandler(): language_(""){
    for (int i = 0; i < 3; i ++) current_party_[i] = -1;
    for (int i = 0; i < 11; i ++) character_names_[i] = std::string("CHAR_") + std::to_string(i);
    InitCmd();
}

TextHandler::~TextHandler(){UnloadTexts();}

void TextHandler::LoadFieldText(const std::string file_name){
    XmlTextFile texts("./data/fields/" + file_name);
    texts.LoadTexts();
}

void TextHandler::SetLanguage(const Ogre::String& language){
    language_ = language;
    UnloadTexts();
    XmlTextsFile texts("./data/texts/_texts.xml");
    texts.LoadTexts();
}

const Ogre::String& TextHandler::GetLanguage(){return language_;}

void TextHandler::AddText(const Ogre::String& name, TiXmlNode* node){
    Text text;
    text.name = name;
    text.node = node;
    texts_.push_back(text);
}

void TextHandler::AddDialog(
  const Ogre::String& name, TiXmlNode* node, const float width, const float height
){
    Dialog dialog;
    dialog.name = name;
    dialog.node = node;
    dialog.width = width;
    dialog.height = height;
    dialogs_.push_back(dialog);
}

TiXmlNode* TextHandler::GetText(const Ogre::String& name) const{
    for (unsigned int i = 0; i < texts_.size(); ++ i)
        if (texts_[i].name == name) return texts_[i].node;
    LOG_WARNING("Can't find text '" + name + "'.");
    return NULL;
}

TiXmlNode* TextHandler::GetDialog(const Ogre::String& name, float &width, float& height) const{
    for (unsigned int i = 0; i < dialogs_.size(); ++ i){
        if (dialogs_[i].name == name){
            width = dialogs_[i].width;
            height = dialogs_[i].height;
            return dialogs_[i].node;
        }
    }
    return NULL;
}

std::string TextHandler::GetDialogText(const std::string name){
    std::string text = "";
    for (unsigned int i = 0; i < dialogs_.size(); ++ i){
        if (dialogs_[i].name == name){
            text = dialogs_[i].node->FirstChild()->ToText()->Value();
            break;
        }
    }
    return text;
}
void TextHandler::UnloadTexts(){
    for (unsigned int i = 0; i < texts_.size(); ++ i) delete texts_[i].node;
    texts_.clear();
    for (unsigned int i = 0; i < dialogs_.size(); ++ i) delete dialogs_[i].node;
    dialogs_.clear();
}

std::string TextHandler::GetCharacterName(int id){
    if (id >= 0 && id < 11) return character_names_[id];
    else return std::string("UNKNOWN_CHAR_" + id);
}

std::string TextHandler::GetPartyCharacterName(int position){
    if (position >= 1 && position <= 3){
        if (current_party_[position - 1] < 0) return "";
        else return GetCharacterName(current_party_[position - 1]);
    }
    else return std::string("UNKNOWN_PARTY_POS_" + position);
}

void TextHandler::SetCharacterName(int id, char* name){
    if (id >= 0 && id < 11) character_names_[id] = std::string(name);
    else LOG_WARNING("Not setting character name for character ID " + std::to_string(id) + ".");
}

void TextHandler::SetParty(int char_1, int char_2, int char_3){
    current_party_[0] = char_1;
    current_party_[1] = char_2;
    current_party_[2] = char_3;
}

std::vector<int> TextHandler::GetParty(){
    std::vector<int> party = {current_party_[0], current_party_[1], current_party_[2]};
    return party;
}

