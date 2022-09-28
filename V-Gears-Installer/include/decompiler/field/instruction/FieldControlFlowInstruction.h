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
#include "decompiler/field/FieldEngine.h"

/**
 * A script flow control instruction.
 */
class FieldControlFlowInstruction : public KernelCallInstruction{

    public:

        /**
         * Create a FieldControlFlowInstruction.
         *
         * @return Pointer to the newly created instruction.
         */
        static InstPtr Create(){return new FieldControlFlowInstruction();}

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

        /**
         * Processes a REQ command.
         *
         * Opcode: 0x01
         * Short name: REQ
         * Long name: Request remote execution (asynchronous, non-guaranteed)
         *
         * Memory layout (3 bytes)
         * |0x01|E|P/F|
         *
         * Arguments
         * - const UByte E: The ID of the target entity.
         * - const Bit[3] P: The priority at which we want to execute the
         * remote script (high 3 bits of byte).
         * - const Bit[5] F: The ID of the specific member function of E to be
         * executed (low 5 bits of byte).
         *
         * Requests that a remote entity executes one of its member functions
         * at a specified priority. The request is asynchronous and returns
         * immediately without waiting for the remote execution to start or
         * finish. If the specified priority is already busy executing, the
         * request will fail silently.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         * @param engine[in] The engine.
         */
        void ProcessREQ(CodeGenerator* code_gen, const FieldEngine& engine);

        /**
         * Processes a REQSW command.
         *
         * Opcode: 0x02
         * Short name: REQSW
         * Long name: Request remote execution (asynchronous execution,
         * guaranteed)
         *
         * Memory layout (3 bytes)
         * |0x02|E|P/F|
         *
         * Arguments
         * - const UByte E: The ID of the target entity.
         * - const Bit[3] P: The priority at which we want to execute the
         * remote script (high 3 bits of byte).
         * - const Bit[5] F: The ID of the specific member function of E to be
         * executed (low 5 bits of byte).
         *
         * Requests that a remote entity executes one of its member functions
         * at a specified priority. If the specified priority is already busy
         * executing, the request will block until it becomes available and
         * only then return. The remote execution is still carried out
         * asynchronously, with no notification of completion.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         * @param engine[in] The engine.
         */
        void ProcessREQSW(CodeGenerator* code_gen, const FieldEngine& engine);

        /**
         * Processes a REQEW command.
         *
         * Opcode: 0x03
         * Short name: REQEW
         * Long name: Request remote execution (synchronous, guaranteed)
         *
         * Memory layout (3 bytes)
         * |0x03|E|P/F|
         *
         * Arguments
         * - const UByte E: The ID of the target entity.
         * - const Bit[3] P: The priority at which we want to execute the
         * remote script (high 3 bits of byte).
         * - const Bit[5] F: The ID of the specific member function of E to be
         * be executed (low 5 bits of byte).
         *
         * Requests that a remote entity executes one of its member functions
         * at a specified priority. The request will block until remote
         * execution has finished before returning.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         * @param engine[in] The engine.
         */
        void ProcessREQEW(CodeGenerator* code_gen, const FieldEngine& engine);

        /**
         * Processes a RETTO command.
         *
         * Opcode: 0x07
         * Short name: RETTO
         * Long name: Return To
         *
         * Memory layout (2 bytes)
         * |0x07|P/F|
         *
         * Arguments
         * - const Bit[3] P: The priority at which we want to execute the
         * remote script (high 3 bits of byte).
         * - const Bit[5] F: The ID of the specific member function of the
         * current entity to be executed to (low 5 bits of byte).
         *
         * Stops the active script loop for this entity and also any script
         * loops (except the main) that are queuing to be executed after the
         * current script. This is essentially the same as adding a RET onto
         * each of the active / queued scripts next execution position and
         * returning the current op index to index for each script. Then the
         * script control is passed to the script F within the current entity
         * with the priority P.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         */
        void ProcessRETTO(CodeGenerator* code_gen);

        /**
         * Processes a WAIT command.
         *
         * Opcode: 0x24
         * Short name: WAIT
         * Long name: Wait
         *
         * Memory layout (2 bytes)
         * |0x24|A|
         *
         * Arguments
         * - const UShort A: Amount (number of frames) to wait.
         *
         * Pauses current script execution for a specific amount of time.
         * Rather than a specific time value in milliseconds/seconds, the
         * amount specifies the number of frames that must be drawn before
         * execution resumes. Since the game runs at 30fps, WAIT(0x1E) (or
         * WAIT(30) in decimal) will pause script execution for 1 second,
         * WAIT(0x96) will pause for 5 seconds, and so on.
         *
         * @param codegen[in|out] Code generator. Output lines are appended.
         */
        void ProcessWAIT(CodeGenerator* code_gen);
};
