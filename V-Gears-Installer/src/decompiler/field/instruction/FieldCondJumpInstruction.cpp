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

#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include "decompiler/field/instruction/FieldCondJumpInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FF7::FieldCondJumpInstruction::ProcessInst(
  Function& function, ValueStack &stack, Engine* engine, CodeGenerator* code_gen
){
    FF7::FieldCodeGenerator* cg = static_cast<FF7::FieldCodeGenerator*>(code_gen);
    std::string func_name;
    if (opcode_ == OPCODES::IFKEYON) func_name = "entity_manager:is_key_on";
    else if (opcode_ == OPCODES::IFKEYOFF) func_name = "entity_manager:is_key_off";
    else if (opcode_ == OPCODES::IFKEY) func_name = "Key";
    else if (opcode_ == OPCODES::IFMEMBQ) func_name = "IFMEMBQ";
    else if (opcode_ == OPCODES::IFPRTYQ) func_name = "IFPRTYQ";

    // If condition is a function, add and stop.
    if (!func_name.empty()){
        uint32 param = params_[0]->GetUnsigned();
        // Special cases. The first parameter of IFKEY, IFKEYON and IFKEYOFF
        // can be ORed to get the individual keys, but there are two invalid
        // ones: 512 and 1024. They must be XORed.
        if (
          opcode_ == OPCODES::IFKEY
          || opcode_ == OPCODES::IFKEYON
          || opcode_ == OPCODES::IFKEYOFF
        ){
            if (param >= 1024) param = param ^ 1024;
            if (param >= 512) param = param ^ 512;
        }
        ValuePtr v = new UnquotedStringValue(func_name + "(" + std::to_string(param) + ")");
        stack.Push(v);
        return;
    }
    std::string op;
    uint32 type = params_[4]->GetUnsigned();
    const auto& source = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& destination = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );

    switch (type){
        case 0: op = "=="; break;
        case 1: op = "~="; break;
        case 2: op = ">"; break;
        case 3: op = "<"; break;
        case 4: op = ">="; break;
        case 5: op = "<="; break;
        case 6: op = "&"; break;
        case 7: op = "^"; break;
        case 8: op = "|"; break;
        case 9:
            {
                op = "bit(" + params_[0]->GetString() + ", " + params_[2]->GetString()
                  + ", " + destination + ") == 1";
                ValuePtr v = new UnquotedStringValue(op);
                stack.Push(v);
            }
            return;
        case 0xA:
            {
                op = "bit(" + params_[0]->GetString() + ", " + params_[2]->GetString()
                  + ", " + destination + ") == 0";
                ValuePtr v = new UnquotedStringValue(op);
                stack.Push(v);
            }
            return;
        default: throw UnknownConditionalOperatorException(address_, type);
    }
    ValuePtr v = new BinaryOpValue(new VarValue(source), new VarValue(destination), op);
    stack.Push(v);
}

uint32 FF7::FieldCondJumpInstruction::GetDestAddress() const{
    uint32 params_size = 0;
    uint32 jump_param_index = 5;
    switch (opcode_){
        case OPCODES::IFUB: params_size = 5; break;
        case OPCODES::IFUBL: params_size = 5; break;
        case OPCODES::IFSW: params_size = 7; break;
        case OPCODES::IFSWL: params_size = 7; break;
        case OPCODES::IFUW: params_size = 7; break;
        case OPCODES::IFUWL: params_size = 7; break;
        case OPCODES::IFKEYON:
        case OPCODES::IFKEYOFF:
        case OPCODES::IFKEY:
            params_size = 3;
            jump_param_index = 1;
            break;
        case OPCODES::IFPRTYQ:
        case OPCODES::IFMEMBQ:
            params_size = 2;
            jump_param_index = 1;
            break;
        default: throw UnknownJumpTypeException(address_, opcode_);
    }
    return address_ + params_[jump_param_index]->GetUnsigned() + params_size;
}

std::ostream& FF7::FieldCondJumpInstruction::Print(std::ostream &output) const{
    Instruction::Print(output);
    output << " (False target address: 0x" << std::hex << GetDestAddress() << std::dec << ")";
    return output;
}
