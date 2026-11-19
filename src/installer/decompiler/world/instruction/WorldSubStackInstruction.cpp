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
#include "decompiler/world/instruction/WorldSubStackInstruction.h"
#include "decompiler/world/WorldEngine.h"
#include "decompiler/world/WorldCodeGenerator.h"
#include "decompiler/world/WorldDisassembler.h"

void WorldSubStackInstruction::ProcessInst(
    Function& function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
){
    // The operands are pushed in order, so the first one is the second from the top.
    if (opcode_ == 0x15){
        // Negative number.
        const std::string operand = stack.Pop()->GetString();
        stack.Push(new VarValue("(-" + operand + ")"));
        return;
    }
    if (opcode_ == 0x1b){
        // A function of one value that is not known (the emerald weapon uses it on a variable).
        const std::string operand = stack.Pop()->GetString();
        stack.Push(new VarValue("UnknownFunction1B(" + operand + ")"));
        return;
    }
    if (opcode_ == 0x17){
        // Logic not.
        const std::string condition = WorldEngine::AsCondition(stack.Pop());
        // not not x is x.
        if (condition.compare(0, 4, "not ") == 0) stack.Push(new WorldEngine::BoolValue(condition.substr(4)));
        else stack.Push(new WorldEngine::BoolValue("not " + condition));
        return;
    }
    const ValuePtr right = stack.Pop();
    const ValuePtr left = stack.Pop();
    const std::string l = left->GetString();
    const std::string r = right->GetString();
    switch (opcode_){
        case 0x40: stack.Push(new VarValue("(" + l + " + " + r + ")")); break;
        case 0x41: stack.Push(new VarValue("(" + l + " - " + r + ")")); break;
        case 0x30: stack.Push(new VarValue("(" + l + " * " + r + ")")); break;
        // Shift right.
        case 0x51: stack.Push(new VarValue("math.floor(" + l + " / 2 ^ " + r + ")")); break;
        // Bitwise and, or (see script_stubs.lua).
        case 0x80: stack.Push(new VarValue("BitAnd(" + l + ", " + r + ")")); break;
        case 0xa0: stack.Push(new VarValue("BitOr(" + l + ", " + r + ")")); break;
        case 0x60: stack.Push(new WorldEngine::BoolValue("(" + l + " < " + r + ")")); break;
        case 0x61: stack.Push(new WorldEngine::BoolValue("(" + l + " > " + r + ")")); break;
        case 0x62: stack.Push(new WorldEngine::BoolValue("(" + l + " <= " + r + ")")); break;
        case 0x63: stack.Push(new WorldEngine::BoolValue("(" + l + " >= " + r + ")")); break;
        case 0xb0:
            stack.Push(new WorldEngine::BoolValue(
              "(" + WorldEngine::AsCondition(left) + " and " + WorldEngine::AsCondition(right) + ")"
            ));
            break;
        case 0xc0:
            stack.Push(new WorldEngine::BoolValue(
              "(" + WorldEngine::AsCondition(left) + " or " + WorldEngine::AsCondition(right) + ")"
            ));
            break;
        default: stack.Push(new VarValue("unknown_operation(" + l + ", " + r + ")")); break;
    }
}
