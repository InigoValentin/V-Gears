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
#include "decompiler/world/instruction/WorldLoadBankInstruction.h"
#include "decompiler/world/WorldEngine.h"
#include "decompiler/world/WorldCodeGenerator.h"
#include "decompiler/world/WorldDisassembler.h"

void WorldLoadBankInstruction::ProcessInst(
  Function &function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
){
    // The game variables are at the savemap from 0xBA4 (the story counter is the word at 0): the
    // opcode says if the address is of a byte, a word (0x11c) or a bit (0x114). The second bank
    // (0x119, 0x11d) is the memory of the script and the "special" ones (0x11b, 0x117) are the
    // values that the engine gives to the script (how the world was entered...): they have the same
    // addresses as the game variables, but are not them.
    std::string function_name = "Read";
    switch (GetOpcode()){
        case 0x11c: function_name = "ReadWord"; break;
        case 0x114: function_name = "ReadBit"; break;
        case 0x119: case 0x11d: function_name = "ReadLocal"; break;
        case 0x11b: case 0x117: function_name = "ReadSpecial"; break;
        default: break;
    }
    stack.Push(new WorldEngine::BankValue(function_name + "(" + params_[0]->GetString() + ")"));
}
