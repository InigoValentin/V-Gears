/*
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

#include "decompiler/instruction/Instruction.h"
#include "decompiler/field/FieldEngine.h"

/**
 * An instruction that does nothing.
 */
class FieldNoOperationInstruction : public Instruction{

    public:

        /**
         * Generates a instruction that does nothing.
         *
         * @return The generated instruction.
         */
        static InstPtr Create(){return new FieldNoOperationInstruction();}

        /**
         * Processes the instruction.
         *
         * It doesn't do anything.
         *
         * @param[in] func Function to process. Unused.
         * @param[out] stack Function stack. Unused.
         * @param[in] engine Engine. Unused.
         * @param[in,out] code_gen Code generator to append lines.
         */
        virtual void ProcessInst(
          Function& func, ValueStack& stack, Engine* engine, CodeGenerator* code_gen
        ) override;
};
