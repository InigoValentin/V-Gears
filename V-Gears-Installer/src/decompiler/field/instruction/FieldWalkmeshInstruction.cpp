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

#include "decompiler/field/instruction/FieldWalkmeshInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldWalkmeshInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::SLIP: WriteTodo(code_gen, md.GetEntityName(), "SLIP"); break;
        case OPCODE::UC: ProcessUC(code_gen); break;
        case OPCODE::IDLCK:
            // Triangle id, on or off
            code_gen->AddOutputLine(
                (boost::format("walkmesh:lock_walkmesh(%1%, %2%)")
                % _params[0]->getUnsigned()
                % FieldCodeGenerator::FormatBool(_params[1]->getUnsigned())).str());
            break;
        case OPCODE::LINE: ProcessLINE(code_gen, md.GetEntityName()); break;
        case OPCODE::LINON: WriteTodo(code_gen, md.GetEntityName(), "LINON"); break;
        case OPCODE::SLINE: WriteTodo(code_gen, md.GetEntityName(), "SLINE"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FieldWalkmeshInstruction::ProcessUC(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("entity_manager:player_lock(%1%)")
      % FieldCodeGenerator::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FieldWalkmeshInstruction::ProcessLINE(CodeGenerator* code_gen, const std::string& entity){
    float xa = _params[0]->getSigned();
    float ya = _params[1]->getSigned();
    float za = _params[2]->getSigned();
    float xb = _params[3]->getSigned();
    float yb = _params[4]->getSigned();
    float zb = _params[5]->getSigned();
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
