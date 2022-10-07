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

#include "decompiler/instruction/BinaryOpStackInstruction.h"
#include "decompiler/CodeGenerator.h"

void BinaryOpStackInstruction::ProcessInst(
  Function& function, ValueStack &stack, Engine* engine, CodeGenerator *code_gen
){
    ValuePtr op1 = stack.Pop();
    ValuePtr op2 = stack.Pop();
    if (code_gen->GetBinaryOrder() == FIFO_ARGUMENT_ORDER)
        stack.Push(new BinaryOpValue(op2, op1, code_gen_data_));
    else if (code_gen->GetBinaryOrder() == LIFO_ARGUMENT_ORDER)
        stack.Push(new BinaryOpValue(op1, op2, code_gen_data_));
}
