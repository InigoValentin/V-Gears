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
#include "decompiler/world/instruction/WorldUncondJumpInstruction.h"
#include "decompiler/world/WorldEngine.h"
#include "decompiler/world/WorldCodeGenerator.h"
#include "decompiler/world/WorldDisassembler.h"

FF7::WorldUncondJumpInstruction::WorldUncondJumpInstruction() : is_call_(false) { }

bool FF7::WorldUncondJumpInstruction::IsFuncCall() const{return is_call_;}

bool FF7::WorldUncondJumpInstruction::IsUncondJump() const{return !is_call_;}

uint32 FF7::WorldUncondJumpInstruction::GetDestAddress() const{
    // The world map while loops are incorrect without +1'in this, but
    // doing that will break some if elses...
    return 0x200 + params_[0]->getUnsigned();// +1; // TODO: +1 to skip param?
}

std::ostream& FF7::WorldUncondJumpInstruction::Print(std::ostream &output) const{
    Instruction::Print(output);
    // output << " (Jump target address: 0x" << std::hex << GetDestAddress() << std::dec << ")";
    return output;
}


void FF7::WorldUncondJumpInstruction::ProcessInst(
    Function& function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
){
    // TODO
}
