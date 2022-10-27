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
 * A window instruction.
 */
class FieldWindowInstruction : public KernelCallInstruction{

    public:

        /**
         * Processes the instruction.
         *
         * @param[in] func Function to process.
         * @param[out] stack Function stack. Unused.
         * @param[in] engine Engine.
         * @param[in,out] code_gen Code generator to append lines.
         */
        virtual void ProcessInst(
          Function& func, ValueStack& stack, Engine* engine, CodeGenerator* code_gen
        ) override;

    private:

        void ProcessMESSAGE(CodeGenerator* code_gen, const std::string& script_name);

        void ProcessMPNAM(CodeGenerator* code_gen);


        /**
         * Processes a MENU opcode.
         *
         * Opcode: 0x49
         * Short name: MENU
         * Long name: Menu
         *
         * Memory layout (4 bytes)
         * |0x49|B|T|E|
         *
         * Arguments
         * - const UByte B: Bank for parameter, or zero if P is specified as a literal value.
         * - const UByte T: Type of menu, or special event.
         * - const UByte P: Parameter to the menu, or address of parameter value, if B is non-zero.
         *
         * MENU has two uses. Its primary function is to display a menu or other special screen.
         * These menus range from the character name entry screen, to a shop, and even the staff
         * credit display. The other function is to provide a set of special events that would
         * normally be accomplished through a set of opcodes, but are instead coded directly into a
         * MENU call. Some types of menu are erroneous or produce erratic behaviour, and were most
         * likely used for testing. As such, they are not listed here.
         *
         * Standard Menu Types
         *
         * ID  Menu Type
         *  5  FF7 Credits
         *  6  Character Name Entry
         *  7  Party Select
         *  8  Shop
         *  9  Main Menu
         *  E  Save Screen
         *
         * Special Event Types
         * ID  Event Type
         *  F  Yuffie's Materia Steal (Remove All Materia)
         * 12  Remove Cloud's Materia
         * 13  Restore Cloud's Materia
         *
         * Parameters
         *
         * Character Name Entry: Parameter indicates the name of the character to edit, and follows
         * the standard Character IDs, as well as 0x64 to indicate the Chocobo naming screen.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessMENU(CodeGenerator* code_gen);

        void ProcessMENU2(CodeGenerator* code_gen);

        void ProcessWINDOW(CodeGenerator* code_gen);

        void ProcessWCLSE(CodeGenerator* code_gen);
};
