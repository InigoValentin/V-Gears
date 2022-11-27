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

        /**
         * Processes a STTIM opcode.
         *
         * Opcode: 0x38
         * Short name: STTIM
         * Long name: Set Timer
         *
         * Memory layout (6 bytes)
         * |0x38|B1/B2|0/B3|H|M|S|
         *
         * Arguments
         * - const Bit[4] B1: Bank to find hours value, or zero if hours (B1) is passed as a value.
         * - const Bit[4] B2: Bank to find minutes value, or zero if minutes (B2) is passed as a
         * value.
         * - const Bit[4] 0: Zero.
         * - const Bit[4] B3: Bank to find seconds value, or zero if seconds (B3) is passed as a
         * value.
         * - const UByte H: Hours, or address to find hours value, if B1 is non-zero.
         * - const UByte M: Minutes, or address to find minutes value, if B2 is non-zero.
         * - const UByte S: Seconds, or address to find seconds value, if B3 is non-zero.
         *
         * Sets the clock, as found in the WSPCL opcode. If the hours, minutes or seconds are
         * specified in the argument, the corresponding B nybble is zero. Otherwise, the value for
         * the time component is retrieved from the bank and address specified. The separate time
         * components can be retrieved from memory or specified as a value, in the same argument
         * list. Hours are not directly visible on the clock, as it only displays minutes and
         * seconds. Hours are translated into minutes.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessSTTIM(CodeGenerator* code_gen);

        void ProcessMESSAGE(CodeGenerator* code_gen, const std::string& script_name);

        /**
         * Processes an ASK opcode.
         *
         * Opcode: 0x48
         * Short name: ASK
         * Long name: Ask Question
         *
         * Memory layout (7 bytes)
         * |0x48|Bank|Win|Mess|First|Last|Addr|
         *
         * Arguments
         *
         * const UByte Bank: Bank to put line number selected.
         * const UByte Win: Window ID to place the question in. (Initialized with WINDOW)
         * const UByte Mess: Which dialog to display from dialog table.
         * const UByte First: Line from dialog where the first question is.
         * const UByte Last: Line from dialog where the last question is.
         * const UByte Addr: Address in bank where line selected is written.
         *
         * The ASK command opens a window with a set of choices to be picked with the "selector
         * finger". If ASK is called on an open window ID, the window will shrink closed and
         * re-open with the new data.
         */
        void ProcessASK(CodeGenerator* code_gen, const std::string& script_name);

        void ProcessMPNAM(CodeGenerator* code_gen, const std::string& script_name);


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

        /**
         * Processes a WSIZW opcode.
         *
         * Opcode: 0x2F
         * Short name: WSIZW
         * Long name: Window Resize
         *
         * Memory layout (6 bytes).
         * |0x2F|I|X|Y|W|H|
         *
         * Arguments:
         *
         * - const UByte I: WINDOW ID to resize.
         * - const UShort X: X-coordinate of the window.
         * - const UShort Y: Y-coordinate of the window.
         * - const UShort W: Width of the window.
         * - const UShort H: Height of the window.
         *
         * Resizes and/or repositions the window, after it has been created with the WINDOW opcode.
         * On the next MESSAGE or ASK, the window will be positioned and sized with the new
         * properties.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessWSIZW(CodeGenerator* code_gen);

        /**
         * Processes a WSPCL opcode.
         *
         * Opcode: 0x36
         * Short name: WSPCL
         * Long name: Window Special (Numerical Display)
         *
         * Memory layout (5 bytes.)
         * |0x36|W|T|X|Y|
         *
         * Arguments
         *
         * - const UByte W: WINDOW ID to apply the change to.
         * - const UByte T: Type of display.
         * - const UByte X: X-coordinate of the numerical display, relative to the top-left of the
         * window.
         * - const UByte Y: Y-coordinate of the numerical display, relative to the top-left of the
         * window.
         *
         * Creates a numerical display inside the given window. The display can be either in the
         * form of a clock, or a scoreboard with six digits. This only creates the numerical
         * display; to actually show it, a MESSAGE or ASK command needs to be issued. Using a blank
         * line of dialog will allow you to create a numerical display in the top-left of the
         * window without field dialog hidden behind it. Alternatively, dialog can be shown along
         * with the display by placing the display in an appropriate area of the window. To set the
         * time for the clock variant, STTIM is used. To set the number for the numerical display,
         * WNUMB is used.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessWSPCL(CodeGenerator* code_gen);

        /**
         * Processes a WMODE opcode.
         *
         * Opcode: 0x52
         * Short name: WMODE
         * Long name: Window Mode
         *
         * Memory layout (4 bytes)
         * |0x52|N|M|C|
         *
         * Arguments
         *
         * - const UByte N: The ID of the window whose mode will be set.
         * - const UByte M: Mode of the window.
         * - const UByte C: Window permanency.
         *
         * Changes properties associated with the WINDOW whose ID is specified. The mode byte sets
         * the style of the window, as detailed below. If the final byte is set to 1, the window
         * cannot be closed by the player pushing [OK]. The mode of the window should be changed
         * before it is displayed with MESSAGE or ASK, or the changes will not be visible unless
         * the window is closed and reopened.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessWMODE(CodeGenerator* code_gen);
};
