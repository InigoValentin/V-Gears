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
#include "decompiler/world/instruction/WorldSubStackInstruction.h"
#include "decompiler/world/WorldEngine.h"
#include "decompiler/world/WorldCodeGenerator.h"
#include "decompiler/world/WorldDisassembler.h"

void FF7::WorldSubStackInstruction::ProcessInst(
    Function& function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
){
    std::string op;
    switch (opcode_){
        case 0x41: op = "-"; break;
        case 0x40: op = "+"; break;
        case 0x80: op = "&&"; break;
        case 0xa0: op = "||"; break;
        case 0xc0: op = "|"; break;
        case 0x15: // neg
        case 0x17: // not
            stack.Push(stack.Pop()->negate());
            return;
            break;
        case 0x30: op = "*";break;
        case 0x60: op = "<"; break;
        case 0x61: op = ">"; break;
        case 0x62: op = "<="; break;
        case 0x63: op = ">="; break;
        case 0x0b0: op = "&"; break; // TODO: Review op
        case 0x51: op = "<<"; break;
        default:op = "unknown_operation";
    }
    std::string value = stack.Pop()->getString() + " " + op + " " + stack.Pop()->getString();
    stack.Push(new VarValue(value));
}
