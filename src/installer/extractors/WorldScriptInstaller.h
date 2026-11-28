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
#include <ostream>
#include <string>
#include <vector>
#include "common/TypeDefine.h"
#include "WorldArchive.h"

/**
 * Writes the scripts of the world maps (world/scripts/wm<N>.lua), decompiled from the event files
 * of the game, with the tables that tell when each function runs and where the field exits are.
 */
class WorldScriptInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] data_dir The game data folder.
         */
        explicit WorldScriptInstaller(const std::string& data_dir);

        /**
         * Writes the script of every world map.
         *
         * @param[in] archive The world map archive.
         * @param[in] field_names The names of the fields, by ID.
         * @param[in] field_scale_factors The scale factors of the fields.
         * @param[in] converted_fields The names of the installed fields.
         * @param[out] error_message Why the scripts could not be written.
         * @return True if all the scripts were written.
         */
        bool Install(
          WorldArchive& archive, const std::vector<std::string>& field_names,
          const std::map<u16, float>& field_scale_factors,
          const std::vector<std::string>& converted_fields, std::string& error_message
        );

        /**
         * Checks if a file is the event file of a world map (wm<N>.ev).
         *
         * @param[in] name Name of the file, in lower case.
         * @return True if it is an event file.
         */
        static bool IsEventFile(const std::string& name);

        /**
         * Writes the table of the functions that run when an entity walks on a mesh.
         *
         * @param[out] script The script.
         * @param[in] event_data The event file, which starts with the types of the functions.
         * @param[in] generated Which functions are in the script.
         */
        static void WriteMeshFunctions(
          std::ostream& script, const std::vector<u8>& event_data, const std::vector<bool>& generated
        );

        /**
         * Writes the table of the functions that run when an entity of a type is created, and for
         * the other events of the entities.
         *
         * @param[out] script The script.
         * @param[in] event_data The event file, which starts with the types of the functions.
         * @param[in] generated Which functions are in the script.
         */
        static void WriteEntityFunctions(
          std::ostream& script, const std::vector<u8>& event_data, const std::vector<bool>& generated
        );

        /**
         * Writes the table of the places where the exits to fields take the player.
         *
         * The exits to fields that are not installed are left out.
         *
         * @param[out] script The script.
         * @param[in] field_table The contents of FIELD.TBL, of {@see FIELD_TABLE_SIZE} bytes.
         * @param[in] field_names The names of the fields, by ID.
         * @param[in] field_scale_factors The scale factors of the fields.
         * @param[in] converted_fields The names of the installed fields.
         */
        static void WriteFieldTable(
          std::ostream& script, const std::vector<u8>& field_table,
          const std::vector<std::string>& field_names,
          const std::map<u16, float>& field_scale_factors,
          const std::vector<std::string>& converted_fields
        );

        /**
         * Size of FIELD.TBL: 64 records of 2 destinations of 12 bytes.
         */
        static const size_t FIELD_TABLE_SIZE;

    private:

        /**
         * Decompiles the functions of an event file and writes them.
         *
         * Functions that can't be decompiled are replaced by a comment.
         *
         * @param[out] script The script.
         * @param[in] event_data The event file.
         * @param[in] map_name Name of the map, for the warnings.
         * @return Which functions were written.
         */
        static std::vector<bool> WriteFunctions(
          std::ostream& script, const std::vector<u8>& event_data, const std::string& map_name
        );

        /**
         * Writes the script of a world map.
         *
         * @param[in] archive The world map archive.
         * @param[in] event_entry The event file of the map.
         * @param[in] field_table The contents of FIELD.TBL.
         * @param[in] field_names The names of the fields, by ID.
         * @param[in] field_scale_factors The scale factors of the fields.
         * @param[in] converted_fields The names of the installed fields.
         * @param[out] error_message Why the script could not be written.
         * @return True if the script was written.
         */
        bool WriteScript(
          WorldArchive& archive, const VGears::LGPArchive::FileEntry& event_entry,
          const std::vector<u8>& field_table, const std::vector<std::string>& field_names,
          const std::map<u16, float>& field_scale_factors,
          const std::vector<std::string>& converted_fields, std::string& error_message
        );

        /**
         * The folder of the scripts.
         */
        std::string script_dir_;
};
