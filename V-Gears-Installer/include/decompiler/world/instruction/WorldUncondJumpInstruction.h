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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "decompiler/instruction.h"
#include "decompiler/world/WorldEngine.h"

namespace FF7{

    /**
     * An unconditional jump instruction.
     */
    class WorldUncondJumpInstruction : public UncondJumpInstruction{

        public:

            /**
             * Constructor.
             */
            WorldUncondJumpInstruction();


            /**
             * Indicates if the instruction is a function call.
             *
             * @return true if the instruction is a function call, false if not.
             */
            virtual bool IsFuncCall() const;

            /**
             * Indicates if the instruction is an unconditional jump.
             *
             * @return true if the instruction is an unconditional jump, false
             * if it's not.
             */
            virtual bool IsUncondJump() const;

            /**
             * Retrieves the destination address of the jump.
             *
             * @return The offset (number of bytes) to jump from the beginning
             * of the instruction.
             */
            virtual uint32 GetDestAddress() const;

            /**
             * Prints the instruction
             *
             * @param[in] output The stream to print the instruction to.
             * @return The same output.
             */
            virtual std::ostream& Print(std::ostream &output) const override;

            /**
             * Processes the instruction.
             *
             * @param function[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine.
             * @param code_gen[in] Code generator.
             */
            virtual void ProcessInst(
              Function& function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            /**
             * Whether or not this is really a call to a script function.
             */
            bool is_call_;

    };

}
