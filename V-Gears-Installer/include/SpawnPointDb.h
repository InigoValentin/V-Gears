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

/**
 * A database of spawn points.
 */
struct SpawnPointDb{

        /**
         * Field IDs.
         *
         * ID of the field the gateways records from N other number of fields
         * are linking to.
         */
        u16 target_field_id = 0;

        /**
         * A spawn point database record.
         */
        struct Record{

                /**
                 * Field that links to {@see SpawnPointDb::target_field_id}.
                 */
                u16 field_id = 0;

                /**
                 * Index of the gateway in {@see field_id}.
                 */
                u32 gateway_index_or_map_jump_address = 0;

                /**
                 * Gateway data.
                 */
                VGears::TriggersFile::Gateway gateway;

                /**
                 * Indicates if the gateway is a map jump from a script.
                 */
                bool from_script = false;

                /**
                 * @todo Understand and document.
                 *
                 * Only used from script calls.
                 */
                std::string entity_name;

                /**
                 * @todo Understand and document.
                 *
                 * Only used from script calls.
                 */
                std::string script_function_name;
        };

        /**
         * List of gateways to this field.
         */
        std::vector<Record> gateways_to_this_field;
};
