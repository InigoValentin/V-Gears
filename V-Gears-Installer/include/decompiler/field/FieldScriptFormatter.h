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

#pragma once

#include <map>
#include <vector>
#include <string>

#include "decompiler/ScriptFormatter.h"

/**
 * Formatter for field scripts.
 */
class FieldScriptFormatter: public ScriptFormatter{

    public:

        /**
         * Adds an spawn point.
         *
         * If the spawn is new, a new record will be added to the spawn point
         * database. If it was already there, update the record to add the
         * origin.
         *
         * @param[in] map_id The target map ID.
         * @param[in] entity The entity that acts as the spawn point.
         * @param[in] function_name The spawn function name.
         * @param[in] address @todo Understand and document.
         * @param[in] x X coordinate of the field at which to spawn.
         * @param[in] y Y coordinate of the field at which to spawn.
         * @param[in] triangle_id Triangle at which to spawn.
         * @param[in] angle Orientation at which to spawn.
         */
        virtual void AddSpawnPoint(
          unsigned int map_id, const std::string& entity, const std::string& function_name,
          unsigned int address, int x, int y, int triangle_id, int angle
        );

        /**
         * Retrieves the name of a spawn point.
         *
         * @param[in] map_id The target map ID.
         * @param[in] entity The entity that acts as the spawn point.
         * @param[in] function_name The spawn function name.
         * @param[in] address @todo Understand and document.
         * @return The name of the spawn point.
         */
        virtual std::string GetSpawnPointName(
          unsigned int map_id, const std::string& entity,
          const std::string& function_name, unsigned int address
        );

        /**
         * Retrieves the name of a map.
         *
         * The name of a map is usually it's ID.
         *
         * @param[in] map_id The map ID.
         * @return The map name.
         */
        virtual std::string GetMapName(unsigned int map_id);

};
