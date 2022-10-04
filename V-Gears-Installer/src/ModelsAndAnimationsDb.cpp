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

#include "ModelsAndAnimationsDb.h"

#include <set>
#include <string>
#include <Ogre.h>
#include "common/FinalFantasy7/FF7NameLookup.h"

std::string ModelsAndAnimationsDb::NormalizeAnimationName(const std::string& name){
    Ogre::String base_name;
    VGears::StringUtil::splitBase(name, base_name);
    std::transform(base_name.begin(), base_name.end(), base_name.begin(), ::tolower);
    return base_name + ".a";
}


std::set<std::string>& ModelsAndAnimationsDb::ModelAnimations(const std::string model){
    // HACK FIX LGP READING.
    std::string model_lower = model;
    std::transform(model_lower.begin(), model_lower.end(), model_lower.begin(), ::tolower);
    return map[model_lower];
}

std::string ModelsAndAnimationsDb::ModelMetaDataName(const std::string& model_name){
    // If not in meta data then just replace .hrc with .mesh.
    Ogre::String base_name;
    VGears::StringUtil::splitBase(model_name, base_name);
    return VGears::FF7::NameLookup::model(base_name) + ".mesh";
}

