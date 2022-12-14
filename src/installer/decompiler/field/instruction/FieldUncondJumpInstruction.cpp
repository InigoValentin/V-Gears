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

#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include "decompiler/field/instruction/FieldUncondJumpInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

FieldUncondJumpInstruction::FieldUncondJumpInstruction() : is_func_call_(false) {}

bool FieldUncondJumpInstruction::IsFuncCall() const{return is_func_call_;}

bool FieldUncondJumpInstruction::IsUncondJump() const{return !is_func_call_;}

uint32 FieldUncondJumpInstruction::GetDestAddress() const{
    if (
      static_cast<OPCODES>(opcode_) == OPCODES::JMPF
      || static_cast<OPCODES>(opcode_) == OPCODES::JMPFL
    ){
        // Short or long forward jump.
        return address_ + params_[0]->GetUnsigned() + 1;
    }
    // Backwards jump,  eOpcodes::JMPB/L.
    return address_ - params_[0]->GetUnsigned();
}

std::ostream& FieldUncondJumpInstruction::Print(std::ostream &output) const{
    Instruction::Print(output);
    output << " (Jump target address: 0x" << std::hex << GetDestAddress() << std::dec << ")";
    return output;
}


void FieldUncondJumpInstruction::ProcessInst(
  Function& func, ValueStack& stack, Engine* engine, CodeGenerator* code_gen
){
    switch (opcode_){
        case OPCODES::JMPB:
        case OPCODES::JMPBL:
            // HACK: Hard loop will hang the game, insert a wait to yield control.
            code_gen->AddOutputLine("-- HACK: Yield control for possible infinite loop");
            code_gen->AddOutputLine("script:wait(0)");
            break;
    }
}
