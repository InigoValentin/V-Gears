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

#include "DecompilerException.h"

class ScriptFormatter{

    public:

        virtual ~ScriptFormatter() = default;

        /**
         * Retrieves a friendly name for a variable.
         *
         * @param bank[in] Variable memory bank.
         * @param address[in] Variable memory address.
         * @return Friendly name assigned to the variable, or an empty
         * string if there is none.
         */
        virtual std::string GetFriendlyVarName(unsigned int bank, unsigned int addr);

        /**
         * Retrieves a friendly name for an entity.
         *
         * @param entity_name[in] Name of the entity.
         * @return Friendly name assigned to the entity, or <entity_name>
         * if there is none.
         */
        virtual std::string GetFriendlyEntityName(const std::string& entity_name);

        /**
         * Retrieves a friendly name for an animation.
         *
         * @param animation_id[in] ID of the animation.
         * @return Friendly name assigned to the animation. If there is
         * no one, the ID in string format.
         */
        virtual std::string GetFriendlyAnimationName(int animation_id, int id);

        /**
         * Retrieves a friendly name for a character.
         *
         * @param char_id[in] ID of the character.
         * @return Friendly name assigned to the character. If there is
         * no one, the ID in string format.
         */
        virtual std::string GetFriendlyCharName(int char_id);

        /**
         * Retrieves a friendly name for a function.
         *
         * @param entity_name[in] Name of the entity.
         * @param function_name[in] Name of the function.
         * @return Friendly name assigned to the entity, or
         * <function_name> if there is none.
         */
        virtual std::string GetFriendlyFunctionName(
          const std::string& entity_name, const std::string& function_name
        );

        /**
         * Retrieves the header comment for a function in an entity.
         *
         * @param entity_name[in] Name of the entity.
         * @param function_name[in] Name of the function.
         * @return The function comment. An empty string if the entity or
         * the function don't exist.
         */
        virtual std::string GetFunctionComment(
          const std::string& entity_name, const std::string& function_name
        );
};
