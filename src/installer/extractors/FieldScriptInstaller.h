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

#include <functional>
#include <string>
#include <vector>
#include "decompiler/field/FieldDecompiler.h"
#include "formats/field/FLevelFile.h"
#include "FieldDataTypes.h"
#include "FieldTextWriter.h"

/**
 * Writes the script (script.lua) and the texts (text.xml) of a field.
 *
 * The script is the decompiled script of the original field, with the entities of the gateways
 * and of the background triggers added.
 */
class FieldScriptInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] data_dir The game data folder.
         * @param[in] map_list The names of the fields, by ID.
         * @param[in,out] spawn_points The spawn points of every field. The decompiler names them.
         * @param[in] log_output_line Function to write a line to the installation log.
         */
        FieldScriptInstaller(
          const std::string& data_dir, const MapList& map_list, FieldSpawnPointsMap& spawn_points,
          std::function<void(std::string)> log_output_line
        );

        /**
         * Decompiles the script of a field and writes it, with its texts.
         *
         * The script and the texts are not written if the script can't be decompiled. The error
         * is written to the log.
         *
         * @param[in] field The field.
         * @return The decompiled script. It's empty if the decompiler failed.
         */
        FieldDecompiler::DecompiledScript Install(const VGears::FLevelFilePtr& field);

        /**
         * Creates the header of a field script.
         *
         * It creates the entity container, records the IDs of this field and of the previous one
         * (for LSTMP), and sets the temporary memory banks to zero. In the original game a field
         * can't see what the previous one left in them, but here the banks live as long as the
         * game, so scripts that wait for a bank to change would see leftovers.
         *
         * @param[in] field_id ID of the field, or -1 to leave out the IDs.
         * @return The header.
         */
        static std::string FieldScriptHeader(const int field_id = -1);

        /**
         * Banks of variables that start at zero in every field.
         */
        static const std::vector<int> TEMPORARY_BANKS;

    private:

        /**
         * Creates the scripts of the entities of the gateways and of the background triggers.
         *
         * @param[in] field The field.
         * @return The scripts of the entities.
         */
        std::string CreateTriggerScripts(const VGears::FLevelFilePtr& field) const;

        /**
         * Creates the script of the entity of a gateway.
         *
         * @param[in] gateway_entity_name Name of the entity.
         * @param[in] target_map_name Name of the field the gateway leads to.
         * @param[in] source_spawn_point_name Spawn point of the destination field.
         * @return The script of the entity.
         */
        static std::string CreateGatewayScript(
          const std::string& gateway_entity_name, const std::string& target_map_name,
          const std::string& source_spawn_point_name
        );

        /**
         * Creates the script of the entity of a background trigger.
         *
         * Behaviors: 0 on, 1 off, 2 on and off when away, 3 off and on when away, 4 and 5 the
         * same as 2 and 3 (the original only reverts when the player leaves by the plus side of
         * the line; here it is when the player leaves the line on any side).
         *
         * @param[in] name Name of the entity.
         * @param[in] trigger The background trigger.
         * @return The script of the entity.
         */
        static std::string CreateBackgroundTriggerScript(
          const std::string& name, const VGears::TriggersFile::Trigger& trigger
        );

        /**
         * Writes the texts of a field.
         *
         * @param[in] field The field.
         * @param[in] raw_script The script section of the field, which has the texts.
         */
        void WriteTexts(const VGears::FLevelFilePtr& field, const std::vector<u8>& raw_script);

        /**
         * The game data folder.
         */
        std::string data_dir_;

        /**
         * The names of the fields, by ID.
         */
        const MapList& map_list_;

        /**
         * The spawn points of every field.
         */
        FieldSpawnPointsMap& spawn_points_;

        /**
         * Function to write a line to the installation log.
         */
        std::function<void(std::string)> write_output_line_;

        /**
         * Writes the texts of the fields.
         */
        FieldTextWriter text_writer_;
};
