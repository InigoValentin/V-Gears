/*
 * V-Gears
 * Copyright (C) 2022 V-Gears Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include "decompiler/Decompiler.h"
#include "FieldScriptFormatter.h"

/**
 * Decompiler for field maps.
 */
class FieldDecompiler: public Decompiler{

    public:

        /**
         * A line.
         */
        struct Line{

            /**
             * Name of the line entity.
             */
            std::string name;

            /**
             * First point of the line.
             */
            std::vector<float> point_a;

            /**
             * Second point of the line.
             */
            std::vector<float> point_b;
        };

        /**
         * An entity.
         */
        struct FieldEntity{

            /**
             * Character identifier of the entity.
             */
            uint char_id;

            /**
             * Index of the entity in the field.
             */
            uint index;

            /**
             * Name of the entity.
             */
            std::string name;
        };

        /**
         * The field decompiled script.
         */
        struct DecompiledScript{

            /**
             * The LUA script for the field.
             */
            std::string luaScript;

            /**
             * The field entities.
             *
             * Lines are not included.
             */
            std::vector<FieldEntity> entities;

            /**
             * Lines in the field.
             */
            std::vector<Line> lines;
        };

        /**
         * Retrieves the scale factor of a field.
         *
         * @param script_bytes[in] Vector of raw byte data that makes up
         * the script.
         * @return The scale fctor.
         */
        static float ScaleFactor(const std::vector<unsigned char>& script_bytes);

        /**
         * Decompiles a field script.
         *
         * @param script_name[in] Name of the script to be converted,
         * should match file name.
         * @param script_bytes[in] Vector of raw byte data that makes up
         * the script.
         * @param formatter[in] Formatter used to rename variables, drop
         * functions...
         * @param text_after[in] Raw text that is added at to the end of
         * the decompiled output.
         * @param text_before[in] Raw text that is added at to the start of
         * the decompiled output.
         * @return A string with the decompiled script.
         * @throws DecompilerException on failure.
         */
        static DecompiledScript Decompile(
          std::string script_name, const std::vector<unsigned char>& script_bytes,
          FieldScriptFormatter& formatter, std::string text_after = "", std::string text_before = ""
        );
};
