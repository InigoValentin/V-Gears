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

#include "decompiler/instruction/KernelCallStackInstruction.h"
#include "decompiler/CodeGenerator.h"

void KernelCallStackInstruction::ProcessInst(
  Function& function, ValueStack &stack, Engine* engine, CodeGenerator *code_gen
){
    code_gen->GetArgList().clear();
    bool returns_value = (code_gen_data_.find("r") == 0);
    std::string metadata = (!returns_value ? code_gen_data_ : code_gen_data_.substr(1));
    for (size_t i = 0; i < metadata.length(); i ++)
        code_gen->ProcessSpecialMetadata(this, metadata[i], i);
    stack.Push(new CallValue(name_, code_gen->GetArgList()));
    if (!returns_value){
        std::stringstream stream;
        stream << stack.Pop() << ";";
        code_gen->AddOutputLine(stream.str());
    }
}
