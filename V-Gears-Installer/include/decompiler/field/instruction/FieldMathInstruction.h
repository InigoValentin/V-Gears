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
 * A math instruction.
 */
class FieldMathInstruction : public StoreInstruction{

    public:

        /**
         * Processes the instruction.
         *
         * @param func[in] Function to process.
         * @param stack[out] Function stack.
         * @param engine[in] Engine. Unused.
         * @param code_gen[in|out] Code generator.
         */
        virtual void ProcessInst(
          Function& func, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
        ) override;

    private:

        void ProcessSaturatedPLUS(CodeGenerator* code_gen);
        void ProcessSaturatedPLUS2(CodeGenerator* code_gen);
        void ProcessSaturatedMINUS(CodeGenerator* code_gen);
        void ProcessSaturatedMINUS2(CodeGenerator* code_gen);
        void ProcessSaturatedINC(CodeGenerator* code_gen);
        void ProcessSaturatedINC2(CodeGenerator* code_gen);
        void ProcessSaturatedDEC(CodeGenerator* code_gen);
        void ProcessSaturatedDEC2(CodeGenerator* code_gen);
        void ProcessRDMSD(CodeGenerator* code_gen);
        void ProcessSETBYTE_SETWORD(CodeGenerator* code_gen);

        /**
         * Processes a BITON opcode.
         *
         * Opcode: 0x82
         * Short name: BITON
         * Long name: Set Bit
         *
         * Memory layout (4 bytes)
         * |0x82|D/S|A|B|
         *
         * Arguments
         * - const Bit[4] D: Destination bank.
         * - const Bit[4] S: Source bank.
         * - const UByte A: Destination address.
         * - const UByte Bit: The number of the bit to turn on.
         *
         * Sets the nth bit in the "A" location, where n is a number between
         * 0-7 supplied in B. A value of zero in B will set the least
         * significant bit. If the Source Bank is 0 then the bit to be set is
         * taken from "Bit". If the Source Bank is an 8 bit bank, then the bit
         * is the address in that bank where the operand is.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         */
        void ProcessBITON(CodeGenerator* code_gen);

        /**
         * Processes a BITON opcode.
         *
         * Opcode: 0x83
         * Short name: BITOFF
         * Long name: Reset Bit
         *
         * Memory layout (4 bytes)
         * |0x83|D/S|A|B|
         *
         * Arguments
         * - const Bit[4] D: Destination bank.
         * - const Bit[4] S: Source bank.
         * - const UByte A: Destination address.
         * - const UByte Bit: The number of the bit to turn off.
         *
         * Sets the nth bit in the "A" location, where n is a number between
         * 0-7 supplied in B. A value of zero in B will reset the least
         * significant bit. If the Source Bank is 0 then the bit to be set is
         * taken from "Bit". If the Source Bank is an 8 bit bank, then the bit
         * is the address in that bank where the operand is.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         */
        void ProcessBITOFF(CodeGenerator* code_gen);
        void ProcessPLUSx_MINUSx(CodeGenerator* code_gen, const std::string& op);
        void ProcessINCx_DECx(CodeGenerator* code_gen, const std::string& op);
        void ProcessRANDOM(CodeGenerator* code_gen);
};
