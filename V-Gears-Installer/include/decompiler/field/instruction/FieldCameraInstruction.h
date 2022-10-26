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
 * A camera instruction.
 */
class FieldCameraInstruction : public KernelCallInstruction{

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
          Function& func, ValueStack &stack, Engine* engine, CodeGenerator* code_gen
        ) override;

    private:

        void ProcessNFADE(CodeGenerator* code_gen);

        void ProcessSCR2D(CodeGenerator* code_gen);

        void ProcessSCR2DC(CodeGenerator* code_gen);

        /**
         * Processes a VWOFT opcode.
         *
         * Opcode: 0x6A
         * Short name: VWOFT
         * Long name: View Offset
         *
         * Memory layout (5 bytes)
         * |0x6A|B1/B2|Y|X|S|
         *
         * Arguments
         * - const Bit[4] B1: Bank to retrieve the destination Y offset amount from, or zero if it
         * is given as a literal value.
         * - const Bit[4]: Bank to retrieve the destination X offset amount from, or zero if it
         * is given as a literal value.
         * - const Short Y: Amount to offset the camera in the Y coordinate, or the address to
         * retrieve it if B1 is non-zero.
         * - const Short X: Amount to offset the camera in the X coordinate, or the address to
         * retrieve it if B2 is non-zero.
         * - const UShort S: Movement speed. Lower values move the camera faster.
         *
         * Scrolls the current view by the values supplied in X and Y coordinates, instantly.
         *
         * @param[in,out] code_gen Code generator to append lines.
         * @todo Y is the Y coordinate for sure, but I dont't know about X and S.
         */
        void ProcessVWOFT(CodeGenerator* code_gen);

        void ProcessFADE(CodeGenerator* code_gen);
};

