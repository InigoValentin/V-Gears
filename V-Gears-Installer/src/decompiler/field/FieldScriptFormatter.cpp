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

#include "decompiler/field/FieldScriptFormatter.h"

void FieldScriptFormatter::AddSpawnPoint(
  unsigned int map_id, const std::string& entity, const std::string& function_name,
  unsigned int address, int x, int y, int triangle_id, int angle
){}

std::string FieldScriptFormatter::GetSpawnPointName(
  unsigned int map_id, const std::string& entity,
  const std::string& function_name, unsigned int address
){
    return GetMapName(map_id)
      + "_" + entity + "_" + function_name + "_" + std::to_string(address);
}

std::string FieldScriptFormatter::GetMapName(unsigned int map_id) { return std::to_string(map_id);}
