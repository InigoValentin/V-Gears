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

#include "core/Logger.h"
#include "core/ScriptManager.h"
#include "core/XmlScriptsFile.h"

XmlScriptsFile::XmlScriptsFile(const Ogre::String& file): XmlFile(file){}

XmlScriptsFile::~XmlScriptsFile(){}

void XmlScriptsFile::LoadScripts(){
    TiXmlNode* node = file_.RootElement();
    if(node == nullptr || node->ValueStr() != "scripts"){
        LOG_ERROR(
          file_.ValueStr()
          + " is not a valid scripts file! No <scripts> in root."
        );
        return;
    }
    node = node->FirstChild();
    ScriptManager &script_manager(ScriptManager::getSingleton());
    while (node != nullptr){
        if (
          node->Type() == TiXmlNode::TINYXML_ELEMENT
          && node->ValueStr() == "script"
        ){
            script_manager.RunFile(GetString(node, "file"));
        }
        else if(
          node->Type() == TiXmlNode::TINYXML_ELEMENT
          && node->ValueStr() == "system_script"
        ){
            Ogre::String name = GetString(node, "name");
            if (name == ""){
                LOG_ERROR(
                  "There is no name specified for <system_script> tag."
                );
                continue;
            }
            script_manager.AddEntity(ScriptManager::SYSTEM, name, nullptr);
        }
        node = node->NextSibling();
    }
}
