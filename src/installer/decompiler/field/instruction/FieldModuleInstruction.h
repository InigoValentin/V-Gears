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

#include "decompiler/instruction/KernelCallInstruction.h"
#include "decompiler/field/FieldEngine.h"

/**
 * A module instruction.
 */
class FieldModuleInstruction : public KernelCallInstruction{

    public:

        /**
         * Processes the instruction.
         *
         * @param[in] func Function to process.
         * @param[out] stack Function stack.
         * @param[in] engine Engine. Unused.
         * @param[in] code_gen Code generator.
         */
        virtual void ProcessInst(
          Function& func, ValueStack& stack, Engine* engine, CodeGenerator* code_gen
        ) override;

    private:

        /**
         * Processes a BATTLE opcode.
         *
         * Opcode: 0x70
         * Short name: BATTLE
         * Long name: Start battle
         *
         * Memory layout (4 bytes)
         * |0x70|B|N|N|
         *
         * Arguments
         * - const UByte B: Bank (16-bit) to retrieve the address of the
         * battle ID, or zero if it is given as a literal value.
         * - const UWord N: Battle ID, or address to find ID if B is non-zero.
         *
         * This launches the battle module with whatever battle number is used
         * in the argument, or the value retrieved from memory location N if B
         * is non-zero. Battle 1, 2, and 999 (0x03E7) are debug battles.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessBATTLE(CodeGenerator* code_gen);

        /**
         * Processes a BTLON opcode.
         *
         * Opcode: 0x71
         * Short name: BTLON
         * Long name: Battle switch
         *
         * Memory layout (2 bytes)
         * |0x71|S|
         *
         * Arguments
         * - const UByte S: Switch battles on/off (0/1, respectively).
         *
         * Turns random encounters on or off for this field. Note that if a
         * field does not have any Encounter Data set in its field file,
         * battles will not occur regardless of the argument passed with this
         * opcode.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessBTLON(CodeGenerator* code_gen);

        /**
         * Processes a MAPJUMP opcode.
         *
         * Opcode: 0x60
         * Short name: BTLON
         * Long name: Change Field
         *
         * Memory layout
         * |0x60|I|I|X|X|Y|Y|Z|Z|D|
         *
         * Arguments
         * - const UShort I: Field ID of the map to jump to.
         * - const Short X: X-coordinate of the player on the next field.
         * - const Short Y: Y-coordinate of the player on the next field.
         * - const Short Z: Z-coordinate of the player on the next field.
         * - const UByte D: Direction the character will be facing on the next
         * field, in the standard game format.
         *
         * Switches fields to the one indicated by I, and places the character
         * at the coordinates and direction specified. This is an alternative
         * to using a gateway, and can complement their usage as it allows for
         * more than 12 gateways by simulating their behavior through a LINE
         * which, when crossed, executes a MAPJUMP.
         *
         * @param[in,out] code_gen Code generator to append lines.
         * @param[in] func Function to get the spawn point from.
         */
        void ProcessMAPJUMP(CodeGenerator* code_gen, Function& func);
};
