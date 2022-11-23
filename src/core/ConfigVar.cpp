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

#include <OgreStringConverter.h>
#include "core/Assert.h"
#include "core/ConfigVar.h"

ConfigVar* ConfigVar::static_config_var_list_ = nullptr;

ConfigVar::ConfigVar(
  const Ogre::String& name, const Ogre::String& description, const Ogre::String& default_value
):
  name_(name), description_(description), default_value_(default_value), value_s_(default_value)
{
    VGEARS_ASSERT(name != "", "name_ of ConfigVar can't be empty!");
    VGEARS_ASSERT(default_value != "", "default_value_ of ConfigVar can't be empty!");
    VGEARS_ASSERT(description != "", "description_ of ConfigVar can't be empty!");
    UpdateVariables();

    // TODO: Properly cast this
    if (reinterpret_cast<std::uintptr_t>(&static_config_var_list_) != 0xffffffff){
        previous_ = static_config_var_list_;
        static_config_var_list_ = this;
    }
    else VGEARS_ASSERT(false, "ConfigVar declared after RegisterCvars were called!");
}

int ConfigVar::GetI() const{return value_i_;}

float ConfigVar::GetF() const{return value_f_;}

bool ConfigVar::GetB() const{return value_b_;}

Ogre::String ConfigVar::GetS() const{return value_s_;}

void ConfigVar::SetI(int value){
    value_s_ = Ogre::StringConverter::toString(value);
    UpdateVariables();
}

void ConfigVar::SetF(float value){
    value_s_ = Ogre::StringConverter::toString(value);
    UpdateVariables();
}

void ConfigVar::SetB(bool value){
    value_s_ = Ogre::StringConverter::toString(value);
    UpdateVariables();
}

void ConfigVar::SetS(const Ogre::String& value){
    value_s_ = value;
    UpdateVariables();
}

const Ogre::String& ConfigVar::GetName() const{return name_;}

const Ogre::String& ConfigVar::GetDescription() const{return description_;}

const Ogre::String& ConfigVar::GetDefaultValue() const{return default_value_;}

void ConfigVar::UpdateVariables(){
    value_i_ = Ogre::StringConverter::parseInt(value_s_);
    value_f_ = Ogre::StringConverter::parseReal(value_s_);
    value_b_ = Ogre::StringConverter::parseBool(value_s_);
}
