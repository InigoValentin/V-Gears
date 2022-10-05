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

#include "decompiler/instruction/LoadInstruction.h"
#include "decompiler/world/WorldEngine.h"

/**
 * A bank loading instruction.
 */
class WorldLoadBankInstruction : public LoadInstruction{

    public:

        /**
         * Processes the instruction.
         *
         * @param function[in] Function to process. Unused.
         * @param stack[out] Function stack.
         * @param engine[in] Engine. Unused.
         * @param code_gen[in] Code generator. Unused.
         */
        virtual void ProcessInst(
          Function& function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
        ) override;

};
