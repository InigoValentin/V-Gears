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
#include "decompiler/field/instruction/FieldWalkmeshInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldWalkmeshInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func.metadata);
    //std::cout << "WMesh opcode: " << opcode_ << "\n";
    switch (opcode_){
        case OPCODES::SLIP: code_gen->WriteTodo(md.GetEntityName(), "SLIP"); break;
        case OPCODES::UC: ProcessUC(code_gen); break;
        case OPCODES::IDLCK:
            // Triangle id, on or off
            code_gen->AddOutputLine(
                (boost::format("walkmesh:lock_walkmesh(%1%, %2%)")
                % params_[0]->GetUnsigned()
                % FieldCodeGenerator::FormatBool(params_[1]->GetUnsigned())).str());
            break;
        case OPCODES::LINE: ProcessLINE(code_gen, md.GetEntityName()); break;
        case OPCODES::LINON: code_gen->WriteTodo(md.GetEntityName(), "LINON"); break;
        case OPCODES::SLINE: code_gen->WriteTodo(md.GetEntityName(), "SLINE"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

void FieldWalkmeshInstruction::ProcessUC(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("entity_manager:player_lock(%1%)")
      % FieldCodeGenerator::FormatBool(params_[0]->GetUnsigned())
    ).str());
}

void FieldWalkmeshInstruction::ProcessLINE(CodeGenerator* code_gen, const std::string& entity){
    float xa = params_[0]->GetSigned();
    float ya = params_[1]->GetSigned();
    float za = params_[2]->GetSigned();
    float xb = params_[3]->GetSigned();
    float yb = params_[4]->GetSigned();
    float zb = params_[5]->GetSigned();
    // Scale down. TODO: Why this number?
    xa *= 0.00781249709639;
    ya *= 0.00781249709639;
    za *= 0.00781249709639;
    xb *= 0.00781249709639;
    yb *= 0.00781249709639;
    zb *= 0.00781249709639;
    // Just print as a comment.
    code_gen->AddOutputLine(
      "-- LINE (" + std::to_string(xa) + ", " + std::to_string(ya) + ", " + std::to_string(za)
      + ")-(" + std::to_string(xb) + ", " +std::to_string(yb) + ", " + std::to_string(zb) + ")"
    );
}
