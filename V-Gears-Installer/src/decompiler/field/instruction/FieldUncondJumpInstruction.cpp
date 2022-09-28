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

#include "decompiler/field/instruction/FieldUncondJumpInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

FieldUncondJumpInstruction::FieldUncondJumpInstruction() : call_(false) {}

bool FieldUncondJumpInstruction::IsFuncCall() const{return call_;}

bool FieldUncondJumpInstruction::IsUncondJump() const{return !call_;}

uint32 FieldUncondJumpInstruction::GetDestAddress() const{
    if (
      static_cast<OPCODE>(_opcode) == OPCODE::JMPF
      || static_cast<OPCODE>(_opcode) == OPCODE::JMPFL){
        // Short or long forward jump.
        return _address + _params[0]->getUnsigned() + 1;
    }
    // Backwards jump,  OPCODE::JMPB/L.
    return _address - _params[0]->getUnsigned();
}

void FieldUncondJumpInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator* code_gen
){
    switch (_opcode){
        case OPCODE::JMPB:
        case OPCODE::JMPBL:
            // HACK: Hard loop will hang the game, insert a wait to yield control.
            code_gen->AddOutputLine("-- Hack, yield control for possible inf loop");
            code_gen->AddOutputLine("script:wait(0)");
            break;
    }
}

std::ostream& FieldUncondJumpInstruction::Print(std::ostream &output) const{
    Instruction::Print(output);
    output << " (Jump target address: 0x" << std::hex << GetDestAddress() << std::dec << ")";
    return output;
}
