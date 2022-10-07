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

#include "core/ConfigCmd.h"

ConfigCmd::ConfigCmd(
  const Ogre::String& name, const Ogre::String& description,
  const Ogre::String& params_description, ConfigCmdHandler handler,
  ConfigCmdCompletion completion):
    name_(name),
    description_(description),
    params_description_(params_description),
    handler_(handler),
    completion_(completion)
{}

const Ogre::String& ConfigCmd::GetName() const{return name_;}

const Ogre::String& ConfigCmd::GetDescription() const{ return description_;}

const Ogre::String& ConfigCmd::GetParamsDescription() const{
    return params_description_;
}

ConfigCmdHandler ConfigCmd::GetHandler() const{return handler_;}

ConfigCmdCompletion ConfigCmd::GetCompletion() const{return completion_;}
