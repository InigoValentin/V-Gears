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
 * A window instruction.
 */
class FieldWindowInstruction : public KernelCallInstruction{

    public:

        /**
         * Processes the instruction.
         *
         * @param func[in] Function to process.
         * @param stack[out] Function stack.
         * @param engine[in] Engine.
         * @param code_gen[in|out] Code generator.
         */
        virtual void ProcessInst(
          Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
        ) override;

    private:

        void ProcessMESSAGE(CodeGenerator* code_gen, const std::string& script_name);
        void ProcessMPNAM(CodeGenerator* code_gen);
        void ProcessMENU2(CodeGenerator* code_gen);
        void ProcessWINDOW(CodeGenerator* code_gen);
        void ProcessWCLSE(CodeGenerator* code_gen);
};
