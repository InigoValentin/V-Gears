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
         * @param[in] bank Variable memory bank.
         * @param[in] addr Variable memory address.
         * @return Friendly name assigned to the variable, or an empty
         * string if there is none.
         */
        virtual std::string GetFriendlyVarName(unsigned int bank, unsigned int addr);

        /**
         * Retrieves a friendly name for an entity.
         *
         * @param[in] entity_name Name of the entity.
         * @return Friendly name assigned to the entity, or entity_name
         * if there is none.
         */
        virtual std::string GetFriendlyEntityName(const std::string& entity_name);

        /**
         * Retrieves a friendly name for an animation.
         *
         * @param[in] animation_id ID of the animation.
         * @param[in] id Unused.
         * @return Friendly name assigned to the animation. If there is
         * no one, the ID in string format.
         */
        virtual std::string GetFriendlyAnimationName(int animation_id, int id);

        /**
         * Retrieves the animation a model must play when its script doesn't set a default one.
         *
         * The engine plays the animation called "Idle" by default. Models that don't have one
         * show no animation at all, and they are not drawn.
         *
         * @param[in] char_id ID of the model, as given to the CHAR opcode.
         * @return Name of the animation to play, or an empty string if the model already has an
         * animation called "Idle", or there is no animation to choose.
         */
        virtual std::string GetDefaultAnimationName(int char_id);

        /**
         * Retrieves the animation a model plays when it walks or runs on its own.
         *
         * In the original game the second animation of a model is its walk, and the third its
         * run. Only some models have those called "Walk" and "Run".
         *
         * @param[in] char_id ID of the model, as given to the CHAR opcode.
         * @param[in] index 1 for the walk animation, 2 for the run animation.
         * @return Name of the animation, or an empty string if the model hasn't got it.
         */
        virtual std::string GetMoveAnimationName(int char_id, int index);

        /**
         * Retrieves a friendly name for a character.
         *
         * @param[in] char_id ID of the character.
         * @return Friendly name assigned to the character. If there is
         * no one, the ID in string format.
         */
        virtual std::string GetFriendlyCharName(int char_id);

        /**
         * Retrieves a friendly name for a function.
         *
         * @param[in] entity_name Name of the entity.
         * @param[in] function_name Name of the function.
         * @return Friendly name assigned to the entity, or
         * function_name if there is none.
         */
        virtual std::string GetFriendlyFunctionName(const std::string& entity_name, const std::string& function_name);

        /**
         * Retrieves the header comment for a function in an entity.
         *
         * @param[in] entity_name Name of the entity.
         * @param[in] function_name Name of the function.
         * @return The function comment. An empty string if the entity or
         * the function don't exist.
         */
        virtual std::string GetFunctionComment(const std::string& entity_name, const std::string& function_name);
};
