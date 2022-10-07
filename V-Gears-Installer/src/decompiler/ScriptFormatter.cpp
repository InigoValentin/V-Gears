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

#include "decompiler/ScriptFormatter.h"

std::string ScriptFormatter::GetFriendlyVarName(unsigned int bank, unsigned int addr){return "";}

std::string ScriptFormatter::GetFriendlyEntityName(const std::string& entity_name){
    return entity_name;
}

std::string ScriptFormatter::GetFriendlyAnimationName(int animation_id, int id){
    return std::to_string(animation_id);
}

std::string ScriptFormatter::GetFriendlyCharName(int char_id){return std::to_string(char_id);}

std::string ScriptFormatter::GetFriendlyFunctionName(
  const std::string& entity_name, const std::string& function_name
){return function_name;}

std::string ScriptFormatter::GetFunctionComment(
  const std::string& entity_name, const std::string& function_name
){return "";}
