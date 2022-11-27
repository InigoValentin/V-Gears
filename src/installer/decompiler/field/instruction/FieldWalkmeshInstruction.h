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
 * A walkmesh instruction.
 */
class FieldWalkmeshInstruction : public KernelCallInstruction{

    public:

        /**
         * Processes the instruction.
         *
         * @param[in] func Function to process.
         * @param[out] stack Function stack.
         * @param[in] engine Engine. Unused.
         * @param[in,out] code_gen Code generator to append lines.
         */
        virtual void ProcessInst(
          Function& func, ValueStack& stack, Engine* engine, CodeGenerator* code_gen
        ) override;

    private:

        void ProcessUC(CodeGenerator* code_gen);

        /**
         * Processes a LINE opcode.
         *
         * Opcode: 0xD0
         * Short name: LINE
         * Long name: Line definition
         *
         * Memory layout (7 bytes)
         * |0xD0|XA|YA|ZA|XB|YB|ZB|
         *
         * Arguments:
         * - const Short XA: X-coordinate of the first point of the line.
         * - const Short YA: Y-coordinate of the first point of the line.
         * - const Short ZA: Z-coordinate of the first point of the line.
         * - const Short XB: X-coordinate of the second point of the line.
         * - const Short YB: Y-coordinate of the second point of the line.
         * - const Short ZB: Z-coordinate of the second point of the line.
         *
         * Defines a line on the walkmesh that, when crossed by a playable
         * character, causes one of the entity's scripts to be executed. These
         * are similar to the triggers in Section 8. All the lines in the
         * current field can be turned on or off by using the LINON opcode.
         *
         * There are generally 6 scripts (other than the init and main) if the
         * entity is a LINE.
         * - script index 2 -> S1 - [OK].
         * - script index 3 -> S2 - Move.
         * - script index 4 -> S3 - Move.
         * - script index 5 -> S4 - Go.
         * - script index 6 -> S5 - Go 1x.
         * - script index 7 -> S6 - Go away.
         *
         * @param[in,out] code_gen Code generator to append lines.
         * @param[in] entity The entity name.
         */
        void ProcessLINE(CodeGenerator* code_gen, const std::string& entity);

        /**
         * Processes a LINON opcode.
         *
         * Opcode: 0xD1
         * Short name: LINON
         * Long name: Line Switch
         *
         * Memory layout (2 bytes):
         * |0xD1|S|
         *
         * const UByte S: Switch on/off (1/0, respectively).
         *
         * Turns on or off the LINE that was registered by this entity in the current field. If set
         * to off, the line will not be triggered by the character walking through them.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessLINON(CodeGenerator* code_gen);
};
