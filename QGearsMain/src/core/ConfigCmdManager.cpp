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

#include "common/make_unique.h"
#include "core/Assert.h"
#include "core/ConfigCmdManager.h"
#include "core/ConfigCmdManagerCommands.h"

template<>ConfigCmdManager *Ogre::Singleton<ConfigCmdManager>::msSingleton
  = nullptr;

ConfigCmdManager::ConfigCmdManager(){InitCmd();}

ConfigCmdManager::~ConfigCmdManager(){}

void ConfigCmdManager::AddCommand(
  const Ogre::String& name, const Ogre::String& description,
  const Ogre::String& params_description, ConfigCmdHandler handler,
  ConfigCmdCompletion completion
){
    QGEARS_ASSERT(name != "", "Command name shouldn't be empty.");
    QGEARS_ASSERT(handler, "Null command handler.");
    // Check if command already added
    for (unsigned int i = 0; i < commands_.size(); ++ i)
        QGEARS_ASSERT(
          commands_[i]->GetName() != name, "Command already exist."
        );
    commands_.emplace_back(std::make_unique<ConfigCmd>(
      name, description, params_description, handler, completion
    ));
}

void ConfigCmdManager::ExecuteString(const Ogre::String& cmd_string){}

ConfigCmd* ConfigCmdManager::Find(const Ogre::String& name) const{
    for (unsigned int i = 0; i < commands_.size(); ++ i){
        if (commands_[i]->GetName() == name) return commands_[i].get();
    }
    return nullptr;
}

int ConfigCmdManager::GetConfigCmdNumber(){return commands_.size();}

ConfigCmd* ConfigCmdManager::GetConfigCmd(unsigned int i) const{
    if (i < commands_.size()) return commands_[i].get();
    return nullptr;
}
