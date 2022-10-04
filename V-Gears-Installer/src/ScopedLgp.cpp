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

#include "ScopedLgp.h"

ScopedLgp::ScopedLgp(Ogre::Root* root, std::string full_path, std::string type, std::string group):
  root_(root), full_path_(full_path), group_(group)
{
    if (root_){
        //std::cout << "[RESOURCE] " << full_path_ << ", " << type << ", " << group_ << std::endl;
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(full_path_, type, group_);
    }
}

ScopedLgp::~ScopedLgp(){
     if (root_)
         Ogre::ResourceGroupManager::getSingleton().removeResourceLocation(full_path_, group_);
 }
