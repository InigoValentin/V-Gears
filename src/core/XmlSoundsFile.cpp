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

#include "core/AudioManager.h"
#include "core/Logger.h"
#include "core/XmlSoundsFile.h"

XmlSoundsFile::XmlSoundsFile(const Ogre::String& file): XmlFile(file){}

XmlSoundsFile::~XmlSoundsFile(){}

void XmlSoundsFile::LoadSounds(){
    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "sounds"){
        LOG_ERROR(file_.ValueStr() + " is not a valid sounds file! No <sounds> in root.");
        return;
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "sound"){
            AudioManager::Sound sound;
            sound.name = GetString(node, "name");
            sound.file = "./data/" + GetString(node, "file_name");
            AudioManager::getSingleton().AddSound(sound);
        }
        node = node->NextSibling();
    }
}
