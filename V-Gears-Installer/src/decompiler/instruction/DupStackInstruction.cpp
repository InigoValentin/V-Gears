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

#include "decompiler/instruction/DupStackInstruction.h"
#include "decompiler/CodeGenerator.h"

void DupStackInstruction::ProcessInst(
  Function& function, ValueStack &stack, Engine* engine, CodeGenerator *code_gen
){
    std::stringstream s;
    ValuePtr p = stack.Pop()->Dup(s);
    if (s.str().length() > 0) code_gen->AddOutputLine(s.str());
    stack.Push(p);
    stack.Push(p);
}
