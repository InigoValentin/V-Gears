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

#include "decompiler/instruction/CondJumpInstruction.h"

/**
 * A conditional map jump instruction.
 */
class FieldCondJumpInstruction : public CondJumpInstruction{

    public:

        /**
         * Processes a conditional jump instruction.
         *
         * Checks if the condition is a function or a comparison, and adds the
         * function to the stack.
         *
         * @param[in] function Function to process. Unused.
         * @param[out] stack Function stack. The instruction will be added
         * here.
         * @param[in] engine Engine. Unused.
         * @param[in,out] code_gen Code generator to append lines.
         */
        virtual void ProcessInst(
          Function& function, ValueStack &stack, Engine* engine, CodeGenerator* code_gen
        ) override;

        /**
         * Retrieves the destination address of the jump.
         *
         * @return The offset (number of bytes) to jump from the beginning of
         * the instruction.
         */
        virtual uint32 GetDestAddress() const override;

        /**
         * Prints the instruction
         *
         * @param output The stream to print the instruction to.
         */
        virtual std::ostream& Print(std::ostream &output) const override;
};
