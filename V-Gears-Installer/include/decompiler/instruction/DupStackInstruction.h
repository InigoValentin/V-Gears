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

#include "DupInstruction.h"

/**
 * Default implementation for stack-based instruction duplicating the topmost stack value.
 */
class DupStackInstruction: public DupInstruction{

    public:

        /**
         * Process an instruction for code generation.
         *
         * @param stack[in] The current stack.
         * @param engine[in] The Engine used for code generation.
         * @param code_gen[in] The CodeGenerator used for code generation.
         */
        virtual void ProcessInst(
          Function& function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
        ) override;
};
