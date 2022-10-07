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

#include "decompiler/instruction/KernelCallInstruction.h"
#include "decompiler/field/FieldEngine.h"

/**
 * An audio or video (or both) instruction.
 */
class FieldMediaInstruction : public KernelCallInstruction{

    public:

        /**
         * Processes the instruction.
         *
         * @param[in] func Function to process.
         * @param[out] stack Function stack.
         * @param[in] engine Engine. Unused
         * @param[in,out] code_gen Code generator to append lines.
         */
        virtual void ProcessInst(
          Function& func, ValueStack& stack, Engine* engine, CodeGenerator* code_gen
        ) override;

    private:

        void ProcessAKAO2(CodeGenerator* code_gen);

        void ProcessMUSIC(CodeGenerator* code_gen);

        void ProcessSOUND(CodeGenerator* code_gen);

        void ProcessAKAO(CodeGenerator* code_gen);

        void ProcessMULCK(CodeGenerator* code_gen);

        void ProcessPMVIE(CodeGenerator* code_gen);

        void ProcessMOVIE(CodeGenerator* code_gen);

        void ProcessMVIEF(CodeGenerator* code_gen);
};

