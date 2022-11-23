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
 * A party instruction
 */
class FieldPartyInstruction : public KernelCallInstruction{

    public:

        /**
         * Processes the instruction.
         *
         * @param[in] func Function to process.
         * @param[out] stack Function stack.
         * @param[in] engine The engine. Unused
         * @param[in,out] code_gen Code generator to append lines.
         */
        virtual void ProcessInst(
          Function& func, ValueStack&stack, Engine* engine, CodeGenerator* code_gen
        ) override;

    private:

        /**
         * Processes a GOLDU opcode.
         *
         * Opcode: 0x39
         * Short name: GOLDU
         * Long name: Gil Up
         * Memory layout (6 bytes)
         * |0x39|0|A|A|A|A|     or    |0x39|B/0|A|0|0|0|
         *
         * Arguments
         * - Increase by a constant amount:
         *   - const UByte 0: Zero.
         *   - const ULong A: Amount to increase.
         * - Increase by an amount found in memory:
         *   - const Bit[4] B: Source bank.
         *   - const UByte A: Source address.
         *   - const UByte[3] 0: Three zero bytes.
         *
         * Increases the amount of gil by a constant amount, or by an amount found in the source
         * bank B and address A. The total gil is capped above by 0xFFFFFFFF; attempts to increment
         * further will fail.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessGOLDU(CodeGenerator* code_gen);

        /**
         * Processes a GOLDD opcode.
         *
         * Opcode: 0x40
         * Short name: GOLDD
         * Long name: Gil Down
         * Memory layout (6 bytes)
         * |0x39|0|A|A|A|A|     or    |0x39|B/0|A|0|0|0|
         *
         * Arguments
         * - Decrease by a constant amount:
         *   - const UByte 0: Zero.
         *   - const ULong A: Amount to increase.
         * - Decrease by an amount found in memory:
         *   - const Bit[4] B: Source bank.
         *   - const UByte A: Source address.
         *   - const UByte[3] 0: Three zero bytes.
         *
         * Decreases the amount of gil by a constant amount, or by an amount found in the source
         * bank B and address A. The total gil can't get below 0.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessGOLDD(CodeGenerator* code_gen);

        /**
         * Processes a STITM opcode.
         *
         * Opcode: 0x58
         * Short name: STITM
         * Long name: Set item
         * Memory layout (4 bytes)
         * |0x58|B1/B2|T|T|A|
         *
         * Arguments
         * - const Bit[4] B1: Source bank 1, or zero if T is set as a constant value.
         * - const Bit[4] B2: Source bank 2, or zero if A is set as a constant value.
         * - const UShort T: Type of item to add, or source address to retrieve item type from.
         * - const UByte A: Amount of item to add, or source address to retrieve quantity from.
         *
         * Adds a new item to the inventory. Either the item is added explicitly with values, in
         * which case B1 and B2 are zero, or the item type and quantity are retrieved from memory.
         * In this case, bank B1 and address T retrieve the item type, whilst bank B2 and address A
         * retrieve the quantity to add.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessSTITM(CodeGenerator* code_gen);

        /**
         * Processes a DLITM opcode.
         *
         * Opcode: 0x59
         * Short name: STITM
         * Long name: Set item
         * Memory layout (4 bytes)
         * |0x59|B1/B2|T|T|A|
         *
         * Arguments
         * - const Bit[4] B1: Source bank 1, or zero if T is set as a constant value.
         * - const Bit[4] B2: Source bank 2, or zero if A is set as a constant value.
         * - const UShort T: Type of item to remove, or source address to retrieve item type from.
         * - const UByte A: Amount of item to remove, or source address to retrieve quantity from.
         *
         * Removes a quantity of an item from the inventory. Either the item is removed explicitly
         * with values, in which case B1 and B2 are zero, or the item type and quantity are
         * retrieved from memory. In this case, bank B1 and address T retrieve the item type,
         * whilst bank B2 and address A retrieve the quantity to remove.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessDLITM(CodeGenerator* code_gen);

        /**
         * Processes a CKITM opcode.
         *
         * Opcode: 0x5A
         * Short name: CKITM
         * Long name: Check item
         * Memory layout (3 bytes)
         * |0x5A|B|I|A|
         *
         * Arguments
         * - const UByte B: Bank to store result.
         * - const UShort I: Item ID to check.
         * - const UByte A: Address to store result.
         *
         * Copies the amount of item I the player has in their inventory, to the bank and address
         * specified.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessCKITM(CodeGenerator* code_gen);

        void ProcessPRTYE(CodeGenerator* code_gen);

        /**
         * Processes a CHGLD opcode.
         *
         * Opcode: 0x3B
         * Short name: CHGLD
         * Long name: Check gil
         * Memory layout (6 bytes)
         * |0x39|B1/B2|A1|A2|
         *
         * Arguments
         * - const Bit[4] B1: Destination bank 1.
         * - const Bit[4] B2: Destination bank 2.
         * - const UByte A1: Destination address 1.
         * - const UByte A2: Destination address 2.
         *
         * Copies the amount of gil the party has into the destination addresses. As the gil amount
         * is a four-byte value, the arguments require two destination addresses to place two
         * two-byte values into. Address 1 takes the lower two bytes of the gil amount, while
         * address 2 takes the higher two bytes.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessCHGLD(CodeGenerator* code_gen);
};
