
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
#include "decompiler/world/instruction/WorldStoreInstruction.h"
#include "decompiler/world/WorldEngine.h"
#include "decompiler/world/WorldCodeGenerator.h"
#include "decompiler/world/WorldDisassembler.h"

void FF7::WorldStoreInstruction::ProcessInst(
  Function& function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
){
    std::string value = stack.Pop()->getString();
    // If the bank address is from a load bank instruction, then only
    // the bank address is needed, not whats *at* the bank address.
    ValuePtr bank_addr = stack.Pop();
    code_gen->AddOutputLine("Write(" + bank_addr->getString() + ", " + value + ");");
}
