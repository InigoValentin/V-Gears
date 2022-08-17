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

#include "core/TextManager.h"
#include "core/TextManagerCommands.h"
#include "core/XmlTextsFile.h"

template<>TextManager* Ogre::Singleton< TextManager >::msSingleton = NULL;

TextManager::TextManager():
  language_("")
{InitCmd();}

TextManager::~TextManager(){UnloadTexts();}

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
  const Ogre::String& name, TiXmlNode* node,
  const float width, const float height
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
    LOG_WARNING("Can't find text \"" + name + "\".");
    return NULL;
}

TiXmlNode* TextManager::GetDialog(
  const Ogre::String& name, float &width, float& height
) const{
    for (unsigned int i = 0; i < dialogs_.size(); ++ i){
        if (dialogs_[i].name == name){
            width = dialogs_[i].width;
            height = dialogs_[i].height;
            return dialogs_[i].node;
        }
    }
    return NULL;
}

void TextManager::UnloadTexts(){
    for (unsigned int i = 0; i < texts_.size(); ++ i)
        delete texts_[i].node;
    texts_.clear();
    for (unsigned int i = 0; i < dialogs_.size(); ++ i)
        delete dialogs_[i].node;
    dialogs_.clear();
}
