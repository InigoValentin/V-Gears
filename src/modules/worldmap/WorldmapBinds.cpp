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

#include "modules/worldmap/WorldmapBinds.h"
#include "engine/script/LuaIncludes.h"
#include "modules/worldmap/WorldmapModule.h"

namespace VGears{

    void BindWorldmapModule(lua_State* state){
        sol::state_view lua_view(state);
        sol::table lua = lua_view.globals();

        lua.new_usertype<WorldmapModule>(
          "world_map_module",
          "init", sol::overload(
            [](WorldmapModule& module){module.Init(0);},
            [](WorldmapModule& module, const int map_id){module.Init(map_id);}
          ),
          "suspend", &WorldmapModule::Suspend,
          "clear_vehicle_terrains", &WorldmapModule::ClearVehicleTerrains,
          "add_vehicle_terrain", &WorldmapModule::AddVehicleTerrain,
          "get_terrain_type_under_entity", &WorldmapModule::GetTerrainTypeUnderEntity,
          "get_terrain_triangle_under_entity", &WorldmapModule::GetTerrainTriangleUnderEntity,
          "unboard_vehicle", &WorldmapModule::UnboardVehicle,
          "set_encounters_locked", &WorldmapModule::SetEncountersLocked,
          "set_model_orientation", &WorldmapModule::SetModelOrientation,
          "set_model_heading", &WorldmapModule::SetModelHeading,
          "get_model_heading", &WorldmapModule::GetModelHeading,
          "set_model_offset", &WorldmapModule::SetModelOffset,
          "set_entity_altitude", &WorldmapModule::SetEntityAltitude,
          "enter", &WorldmapModule::Enter,
          "set_spawn", &WorldmapModule::SetSpawn,
          "set_speed", &WorldmapModule::SetSpeed,
          "set_camera", &WorldmapModule::SetCamera,
          "set_vehicle_hidden", &WorldmapModule::SetVehicleHidden,
          "add_world_model", &WorldmapModule::AddWorldModelFromScript
        );
        lua["world_map_module"] = WorldmapModule::getSingletonPtr();
    }
}
