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
#include "decompiler/field/FieldEngine.h"

namespace FF7{

    /**
     * A background instruction.
     */
    class FieldBackgroundInstruction : public KernelCallInstruction{

        public:

            /**
             * Processes the instruction.
             *
             * @param func[in] Function to process.
             * @param stack[out] Function stack.
             * @param engine[in] Engine. Unused
             * @param codegen[in|out] Code generator to append lines.
             */
            virtual void ProcessInst(
              Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
            ) override;

        private:

            void ProcessBGON(CodeGenerator* code_gen);
            void ProcessBGOFF(CodeGenerator* code_gen);
            void ProcessBGCLR(CodeGenerator* code_gen);
            void ProcessSTPAL(CodeGenerator* code_gen);
            void ProcessLDPAL(CodeGenerator* code_gen);
            void ProcessCPPAL(CodeGenerator* code_gen);
            void ProcessADPAL(CodeGenerator* code_gen);
            void ProcessMPPAL2(CodeGenerator* code_gen);
            void ProcessSTPLS(CodeGenerator* code_gen);
            void ProcessLDPLS(CodeGenerator* code_gen);
    };

}
