
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
#include "decompiler/world/instruction/WorldStoreInstruction.h"
#include "decompiler/world/WorldEngine.h"
#include "decompiler/world/WorldCodeGenerator.h"
#include "decompiler/world/WorldDisassembler.h"

void WorldStoreInstruction::ProcessInst(
  Function& function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
){
    std::string value = stack.Pop()->GetString();
    // If the bank address is from a load bank instruction, then only
    // the bank address is needed, not what's *at* the bank address.
    ValuePtr bank_addr = stack.Pop();
    // The destination is pushed with a load instruction, but what is written is the variable that it
    // names, not the one whose address is the value of that variable: Read(12) is Write(12, ...).
    std::string address = bank_addr->GetString();
    std::string function_name = "Write";
    if (dynamic_cast<WorldEngine::BankValue*>(bank_addr.get()) != nullptr && address.back() == ')'){
        // ReadX(n) is the variable that is written, WriteX(n, value).
        static const char* const kinds[] = {"Word", "Bit", "Local", "Special", ""};
        for (const char* kind : kinds){
            const std::string prefix = std::string("Read") + kind + "(";
            if (address.compare(0, prefix.size(), prefix) == 0){
                function_name = std::string("Write") + kind;
                address = address.substr(prefix.size(), address.size() - prefix.size() - 1);
                break;
            }
        }
    }
    code_gen->AddOutputLine(function_name + "(" + address + ", " + value + ");");
}
