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

#include "core/ConfigVarManager.h"

/**
 * Configuration variable manager singleton.
 */
template<>ConfigVarManager *Ogre::Singleton<ConfigVarManager>::msSingleton = nullptr;

ConfigVarManager::ConfigVarManager(){
    // TODO: Properly cast this.
    if (reinterpret_cast<std::uintptr_t>(&ConfigVar::static_config_var_list_) != 0xffffffff){
        for (ConfigVar* cvar = ConfigVar::static_config_var_list_; cvar; cvar = cvar->previous_)
            config_vars_.push_back(cvar);
        // TODO: Properly cast this.
        ConfigVar::static_config_var_list_ = reinterpret_cast<ConfigVar*>(0xffffffff);
    }
}

ConfigVar* ConfigVarManager::Find(const Ogre::String& name) const{
    for (size_t i = 0; i < config_vars_.size(); ++ i)
        if (config_vars_[i]->GetName() == name) return config_vars_[i];
    return nullptr;
}

unsigned int ConfigVarManager::GetConfigVarNumber() const{return config_vars_.size();}

ConfigVar* ConfigVarManager::GetConfigVar(const unsigned int i) const{
    if (i < config_vars_.size()) return config_vars_[i]; return nullptr;
}
