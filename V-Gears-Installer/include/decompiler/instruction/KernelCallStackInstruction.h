/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#pragma once

#include "Instruction.h"

/**
 * Default implementation for stack-based instruction performing a kernel function call.
 */
class KernelCallStackInstruction : public Instruction{

    public:

        /**
         * Process an instruction for code generation.
         *
         * @param[in] function The function the instruction is in.
         * @param[in] stack The current stack.
         * @param[in] engine The Engine used for code generation.
         * @param[in] code_gen The CodeGenerator used for code generation.
         */
        virtual void ProcessInst(
          Function& function, ValueStack& stack, Engine* engine, CodeGenerator* code_gen
        ) override;
};
