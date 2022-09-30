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
#include "decompiler/field/instruction/FieldUncategorizedInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FF7::FieldUncategorizedInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODES::MPDSP: code_gen->WriteTodo(md.GetEntityName(), "MPDSP"); break;
        case OPCODES::SETX: code_gen->WriteTodo(md.GetEntityName(), "SETX"); break;
        case OPCODES::GETX: code_gen->WriteTodo(md.GetEntityName(), "GETX"); break;
        case OPCODES::SEARCHX: code_gen->WriteTodo(md.GetEntityName(), "SEARCHX"); break;
        default:
            code_gen->AddOutputLine(FF7::FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode)
            );
    }
}
