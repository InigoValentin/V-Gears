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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "decompiler/decompiler_engine.h"

/**
 * An unconditional map jump instruction.
 */
class FieldUncondJumpInstruction : public UncondJumpInstruction{

    public:

        /**
         * Constructor.
         */
        FieldUncondJumpInstruction();

        /**
         * Indicates if the instruction is a function call.
         *
         * @return true if the instruction is a function call, false if not.
         */
        virtual bool IsFuncCall() const;

        /**
         * Indicates if the instruction is an unconditional jump.
         *
         * @return true if the instruction is an unconditional jump, false if
         * it's not.
         */
        virtual bool IsUncondJump() const;

        /**
         * Retrieves the destination address of the jump.
         *
         * @return The offset (number of bytes) to jump from the beginning of
         * the instruction.
         */
        virtual uint32 GetDestAddress() const;

        /**
         * Processes the instruction.
         *
         * @param func[in] Function to process. Unused.
         * @param stack[out] Function stack.
         * @param engine[in] Engine. Unused.
         * @param code_gen[in|out] Code generator.
         */
        virtual void ProcessInst(
          Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
        ) override;

        /**
         * Prints the instruction
         *
         * @param output The stream to print the instruction to.
         * @todo Understand and document properly.
         */
        virtual std::ostream& Print(std::ostream &output) const override;

    private:

        /**
         * Whether or not this is really a call to a script function.
         */
        bool call_;
};
