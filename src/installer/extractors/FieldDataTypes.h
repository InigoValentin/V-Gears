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
#include <stdexcept>
#include <string>
#include <vector>
#include "formats/field/TriggersFile.h"
#include "installer/tables/SpawnPointDb.h"

/**
 * The names of the fields, by field ID.
 */
typedef std::vector<std::string> MapList;

/**
 * The spawn points of each field, by field ID.
 */
typedef std::map<u16, SpawnPointDb> FieldSpawnPointsMap;

/**
 * The scale factor of each field, by field ID.
 */
typedef std::map<u16, float> FieldScaleFactorMap;

/**
 * Data and helpers shared by the parts of the field installer.
 */
namespace FieldData{

    /**
     * Folder of the field models, relative to the game data folder.
     */
    const std::string MODELS_DIR("field/models");

    /**
     * Folder of the field maps, relative to the game data folder.
     */
    const std::string MAPS_DIR("field/maps");

    /**
     * Destination of the gateways that are not in use.
     */
    const int INACTIVE_GATEWAY_ID = 32767;

    /**
     * Finds the ID of a field.
     *
     * @param[in] name Name of the field.
     * @param[in] map_list The names of the fields, by ID.
     * @return The ID of the field.
     * @throws std::runtime_error If the field is not in the list.
     */
    inline size_t FieldId(const std::string& name, const MapList& map_list){
        for (size_t i = 0; i < map_list.size(); i ++){
            if (map_list[i] == name) return i;
        }
        throw std::runtime_error("No Id found for field name");
    }

    /**
     * Finds the scale factor of a field.
     *
     * @param[in] scale_factors The scale factors of the fields.
     * @param[in] field_id ID of the field.
     * @return The scale factor of the field.
     * @throws std::runtime_error If the field has no scale factor.
     */
    inline float ScaleFactor(const FieldScaleFactorMap& scale_factors, const size_t field_id){
        auto it = scale_factors.find(static_cast<u16>(field_id));
        if (it == std::end(scale_factors)) throw std::runtime_error("Scale factor not found for field id");
        return it->second;
    }

    /**
     * Checks if a background trigger is in use. Unused ones are all zeroes.
     *
     * @param[in] trigger The trigger to check.
     * @return True if the trigger is in use.
     */
    inline bool IsBackgroundTriggerUsed(const VGears::TriggersFile::Trigger& trigger){
        for (const auto& v : trigger.trigger_line){
            if (v.x != 0 || v.y != 0 || v.z != 0) return true;
        }
        return false;
    }

    /**
     * Adds an entrance to the spawn points of the field it leads to.
     *
     * @param[in,out] spawn_points The spawn points of every field.
     * @param[in] record The entrance. Its gateway has the destination field.
     */
    inline void AddSpawnPoint(FieldSpawnPointsMap& spawn_points, const SpawnPointDb::Record& record){
        const u16 target = record.gateway.destination_field_id;
        SpawnPointDb& db = spawn_points[target];
        db.target_field_id = target;
        db.gateways_to_this_field.push_back(record);
    }
}
