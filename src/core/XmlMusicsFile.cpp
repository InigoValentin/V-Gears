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
#include "core/XmlMusicsFile.h"

XmlMusicsFile::XmlMusicsFile(const Ogre::String& file): XmlFile(file){}

XmlMusicsFile::~XmlMusicsFile(){}

void XmlMusicsFile::LoadMusics(){
    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "musics"){
        LOG_ERROR(file_.ValueStr() + " is not a valid musics file! No <musics> in root.");
        return;
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "music"){
            AudioManager::Music music;
            music.name = GetString(node, "name");
            music.file = "./data/" + GetString(node, "file_name");
            music.loop = GetFloat(node, "loop", -1);
            AudioManager::getSingleton().AddMusic(music);
        }
        node = node->NextSibling();
    }
}
