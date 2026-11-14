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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdint>
#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include "decompiler/field/instruction/FieldCondJumpInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldCondJumpInstruction::ProcessInst(
  Function& function, ValueStack &stack, Engine* engine, CodeGenerator* code_gen
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    std::string func_name;
    if (opcode_ == OPCODES::IFKEYON) func_name = "entity_manager:is_key_on";
    else if (opcode_ == OPCODES::IFKEYOFF) func_name = "entity_manager:is_key_off";
    else if (opcode_ == OPCODES::IFKEY) func_name = "entity_manager:is_key_on";
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
            // XOR only if the bit is set, to clear it. Testing with >= added them to the keys
            // that had a bigger value, such as 32768.
            if (param & 1024) param = param ^ 1024;
            if (param & 512) param = param ^ 512;
        }
        ValuePtr v = new UnquotedStringValue(func_name + "(" + std::to_string(param) + ")");
        stack.Push(v);
        return;
    }
    std::string op;
    uint32 type = params_[4]->GetUnsigned();
    // The values of the signed comparisons (IFSW) are signed words: 65504 is -32. The ones of the
    // unsigned comparisons, and the addresses of the variables, are not.
    const bool is_signed = opcode_ == OPCODES::IFSW || opcode_ == OPCODES::IFSWL;
    const auto& source = is_signed
      ? FieldCodeGenerator::FormatValueOrVariable(
          cg->GetFormatter(), params_[0]->GetUnsigned(),
          static_cast<int>(static_cast<int16_t>(params_[2]->GetUnsigned() & 0xFFFF))
        )
      : FieldCodeGenerator::FormatValueOrVariable(
          cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
        );
    const auto& destination = is_signed
      ? FieldCodeGenerator::FormatValueOrVariable(
          cg->GetFormatter(), params_[1]->GetUnsigned(),
          static_cast<int>(static_cast<int16_t>(params_[3]->GetUnsigned() & 0xFFFF))
        )
      : FieldCodeGenerator::FormatValueOrVariable(
          cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
        );

    switch (type){
        case 0: op = "=="; break;
        case 1: op = "~="; break;
        case 2: op = ">"; break;
        case 3: op = "<"; break;
        case 4: op = ">="; break;
        case 5: op = "<="; break;
        // The game runs on LuaJIT (Lua 5.1), where there are no operators for the bits, and a
        // number is always true, even if it is 0. These are true if the result isn't zero.
        case 6:
        case 7:
        case 8:
            {
                const std::string function
                  = (type == 6) ? "bit_and" : ((type == 7) ? "bit_xor" : "bit_or");
                ValuePtr v = new UnquotedStringValue(
                  function + "(" + source + ", " + destination + ") ~= 0"
                );
                stack.Push(v);
            }
            return;
        case 9:
            {
                op = "get_bank_bit(" + params_[0]->GetString() + ", " + params_[2]->GetString()
                  + ", " + destination + ") == 1";
                ValuePtr v = new UnquotedStringValue(op);
                stack.Push(v);
            }
            return;
        case 0xA:
            {
                op = "get_bank_bit(" + params_[0]->GetString() + ", " + params_[2]->GetString()
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

uint32 FieldCondJumpInstruction::GetDestAddress() const{
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

std::ostream& FieldCondJumpInstruction::Print(std::ostream &output) const{
    Instruction::Print(output);
    output << " (False target address: 0x" << std::hex << GetDestAddress() << std::dec << ")";
    return output;
}
