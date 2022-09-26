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
#include "unknown_opcode_exception.h"

namespace SUDM{

    class IScriptFormatter{

        public:

            virtual ~IScriptFormatter() = default;

            /**
             * Adds an spawn point.
             *
             * If the spawn is new, a new record will be added to the spawn
             * point database. If it was already there, update the record to
             * add the origin.
             *
             * @param map_id[in] The target map ID.
             * @param entity[in] The entity that acts as the spawn point.
             * @param function_name[in] The spawn function name.
             * @param address[in] @todo Understand and document.
             * @param x[in] X coordinate of the field at which to spawn.
             * @param y[in] Y coordinate of the field at which to spawn.
             * @param z[in] Z coordinate of the field at which to spawn.
             * @param angle[in] Orientation at which to spawn.
             */
            virtual void AddSpawnPoint(
              unsigned int map_id, const std::string& entity, const std::string& function_name,
              unsigned int address, int x, int y, int triangle_id, int angle
            ){}

            /**
             * Retrieves the name of a spawn point.
             *
             * @param map_id[in] The target map ID.
             * @param entity[in] The entity that acts as the spawn point.
             * @param function_name[in] The spawn function name.
             * @param address[in] @todo Understand and document.
             * @return The name of the spawn point.
             */
            virtual std::string SpawnPointName(
              unsigned int map_id, const std::string& entity,
              const std::string& function_name, unsigned int address
            ){
                return MapName(map_id)
                  + "_" + entity + "_" + function_name + "_" + std::to_string(address);
            }

            /**
             * Retrieves the name of a map.
             *
             * The name of a map is usually it's ID.
             *
             * @param map_id[in] The map ID.
             * @return The map name.
             */
            virtual std::string MapName(unsigned int map_id) { return std::to_string(map_id);}

            /**
             * Retrieves a friendly name for a variable.
             *
             * @param bank[in] Variable memory bank.
             * @param address[in] Variable memory address.
             * @return Friendly name assigned to the variable, or an empty
             * string if there is none.
             */
            virtual std::string VarName(unsigned int bank, unsigned int addr){return "";}

            /**
             * Retrieves a friendly name for an entity.
             *
             * @param entity_name[in] Name of the entity.
             * @return Friendly name assigned to the entity, or <entity_name>
             * if there is none.
             */
            virtual std::string EntityName(const std::string& entity_name){return entity_name;}

            /**
             * Retrieves a friendly name for an animation.
             *
             * @param animation_id[in] ID of the animation.
             * @return Friendly name assigned to the animation. If there is
             * no one, the ID in string format.
             */
            virtual std::string AnimationName(int animation_id, int id){
                return std::to_string(animation_id);
            }

            /**
             * Retrieves a friendly name for a character.
             *
             * @param char_id[in] ID of the character.
             * @return Friendly name assigned to the character. If there is
             * no one, the ID in string format.
             */
            virtual std::string CharName(int char_id){return std::to_string(char_id);}

            /**
             * Retrieves a friendly name for a function.
             *
             * @param entity_name[in] Name of the entity.
             * @param function_name[in] Name of the function.
             * @return Friendly name assigned to the entity, or
             * <function_name> if there is none.
             */
            virtual std::string FunctionName(
              const std::string& entity_name, const std::string& function_name
            ){return function_name;}

            /**
             * Retrieves the header comment for a function in an entity.
             *
             * @param entity_name[in] Name of the entity.
             * @param function_name[in] Name of the function.
             * @return The function comment. An empty string if the entity or
             * the function don't exist.
             */
            virtual std::string FunctionComment(
              const std::string& entity_name, const std::string& function_name
            ){return "";}
        };

    namespace FF7{

        namespace Field{

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
            float ScaleFactor(const std::vector<unsigned char>& script_bytes);

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
             * @throws InternalDecompilerError on failure.
             */
            DecompiledScript Decompile(
              std::string script_name, const std::vector<unsigned char>& script_bytes,
              IScriptFormatter& formatter, std::string text_after = "", std::string text_before = ""
            );
        }
    }
}
