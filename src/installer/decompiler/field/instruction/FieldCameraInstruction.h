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

        /**
         * Prodesses a SCR2D opcode.
         *
         * Opcode: 0x64
         * Short name: SCR2D
         * Long name: Scroll 2D
         *
         * Memory layout (4 bytes)
         * |0x64|B1/B2|X|Y|
         *
         * Arguments
         *
         * - const Bit[4] B1: Source bank 1, or zero if X is set as a constant value.
         * - const Bit[4] B2: Source bank 2, or zero if Y is set as a constant value.
         * - const Short X: X-coordinate to scroll to, or address to find X if B1 is non-zero.
         * - const Short Y: Y-coordinate to scroll to, or address to find Y if B2 is non-zero.
         *
         * Instantaneously scrolls the current view to the coordinates found in the arguments (or
         * the values found at the addresses if memory banks and address are specified). The move
         * to the new coordinates is instant; variants exist for linear and smooth scrolling.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessSCR2D(CodeGenerator* code_gen);

        /**
         * Processes a SCRLC opcode.
         *
         * Opcode: 0x62
         * Short name: SCRLC
         * Long name: Scroll To Playable Character Stop Movement
         *
         * Memory layout (5 bytes)
         * |0x63|B|S|U|T|
         *
         * Arguments
         *
         * - const UByte B: PRESUME -> Bank for the scroll speed, or zero if it is specified as a
         * literal value. Always zero in game data
         * - const UShort S: Speed of the scroll, in frames, or the address to find the speed if B
         * is non-zero.
         * - const UByte U: Unknown -> Always zero in game data. Seems to affect the speed above.
         * - const UByte T: Type of scroll.
         *
         * Scrolls the current view so that the main playable character is in the center of the
         * view. The script executes asynchronously and does not block whilst the camera movement
         * is active but returns control to the next script operation. This is tweened over a
         * period of S frames. If the main playable character also moves during this time period,
         * the tween position is also updated to match the new playable character location. Once
         * the camera has completed its movement, the camera NO LONGER follows the main playable
         * character's movement. It stays in place.
         *
         * Scroll Types:
         * 2 - Linear.
         * 3 - Smooth (QuadraticInAndOut).
         */
        void ProcessSCRLC(CodeGenerator* code_gen);

        /**
         * Processes a SCRCC opcode.
         *
         * Opcode: 0x65
         * Short name: SCRCC
         * Long name: Scroll To Playable Character
         *
         * Memory layout (1 byte)
         * |0x65|
         *
         * Instantaneously scrolls the current view so that it is centered on the current playable
         * character.
         */
        void ProcessSCRCC(CodeGenerator* code_gen);

        /**
         * Processes a SCR2DC opcode.
         *
         * Opcode: 0x66
         * Short name: SCR2DC
         * Long name: Scroll to Coordinates (Smooth)
         *
         * Memory layout (6 bytes)
         * |0x66|B1/B2|0/B3|X|Y|S|
         *
         * Arguments
         *
         * - const Bit[4] B1: Source bank 1, or zero if X is set as a constant value.
         * - const Bit[4] B2: Source bank 2, or zero if Y is set as a constant value.
         * - const Bit[4] 0: Zero.
         * - const Bit[4] B4: Source bank 3, or zero if S is set as a constant value.
         * - const Short X: X-coordinate to scroll to.
         * - const Short Y: Y-coordinate to scroll to.
         * - const UShort S: Speed to scroll; higher values scroll more slowly.
         *
         * Similar to SCR2D, except the scroll is not instantaneous and is performed smoothly, with
         * a slower start and ending, with speed peaking in the center of the scroll. The overall
         * speed can be set with S.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessSCR2DC(CodeGenerator* code_gen);

        /**
         * Processes a SCR2DL opcode.
         *
         * Opcode: 0x68
         * Short name: SCR2DC
         * Long name: Scroll to Coordinates (Linear)
         *
         * Memory layout (6 bytes)
         * |0x68|B1/B2|0/B3|X|Y|S|
         *
         * Arguments
         *
         * - const Bit[4] B1: Source bank 1, or zero if X is set as a constant value.
         * - const Bit[4] B2: Source bank 2, or zero if Y is set as a constant value.
         * - const Bit[4] 0: Zero.
         * - const Bit[4] B4: Source bank 3, or zero if S is set as a constant value.
         * - const Short X: X-coordinate to scroll to.
         * - const Short Y: Y-coordinate to scroll to.
         * - const UShort S: Speed to scroll; higher values scroll more slowly.
         *
         * Similar to SCR2D, except the scroll to the coordinates is linear; that is, the speed is
         * constant throughout the duration of the screen scroll.
         *
         * @param[in,out] code_gen Code generator to append lines.
         */
        void ProcessSCR2DL(CodeGenerator* code_gen);

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

