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

#include "decompiler/field/instruction/FieldPartyInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldPartyInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::SPTYE: WriteTodo(code_gen, md.GetEntityName(), "SPTYE"); break;
        case OPCODE::GTPYE: WriteTodo(code_gen, md.GetEntityName(), "GTPYE"); break;
        case OPCODE::GOLDU: WriteTodo(code_gen, md.GetEntityName(), "GOLDU"); break;
        case OPCODE::GOLDD: WriteTodo(code_gen, md.GetEntityName(), "GOLDD"); break;
        case OPCODE::HMPMAX1: WriteTodo(code_gen, md.GetEntityName(), "HMPMAX1"); break;
        case OPCODE::HMPMAX2: WriteTodo(code_gen, md.GetEntityName(), "HMPMAX2"); break;
        case OPCODE::MHMMX: WriteTodo(code_gen, md.GetEntityName(), "MHMMX"); break;
        case OPCODE::HMPMAX3: WriteTodo(code_gen, md.GetEntityName(), "HMPMAX3"); break;
        case OPCODE::MPU: WriteTodo(code_gen, md.GetEntityName(), "MPU"); break;
        case OPCODE::MPD: WriteTodo(code_gen, md.GetEntityName(), "MPD"); break;
        case OPCODE::HPU: WriteTodo(code_gen, md.GetEntityName(), "HPU"); break;
        case OPCODE::HPD: WriteTodo(code_gen, md.GetEntityName(), "HPD"); break;
        case OPCODE::STITM: ProcessSTITM(code_gen); break;
        case OPCODE::DLITM: WriteTodo(code_gen, md.GetEntityName(), "DLITM"); break;
        case OPCODE::CKITM: WriteTodo(code_gen, md.GetEntityName(), "CKITM"); break;
        case OPCODE::SMTRA: WriteTodo(code_gen, md.GetEntityName(), "SMTRA"); break;
        case OPCODE::DMTRA: WriteTodo(code_gen, md.GetEntityName(), "DMTRA"); break;
        case OPCODE::CMTRA: WriteTodo(code_gen, md.GetEntityName(), "CMTRA"); break;
        case OPCODE::GETPC: WriteTodo(code_gen, md.GetEntityName(), "GETPC"); break;
        case OPCODE::PRTYP: WriteTodo(code_gen, md.GetEntityName(), "PRTYP"); break;
        case OPCODE::PRTYM: WriteTodo(code_gen, md.GetEntityName(), "PRTYM"); break;
        case OPCODE::PRTYE: ProcessPRTYE(code_gen); break;
        case OPCODE::MMBUD: WriteTodo(code_gen, md.GetEntityName(), "MMBUD"); break;
        case OPCODE::MMBLK: WriteTodo(code_gen, md.GetEntityName(), "MMBLK"); break;
        case OPCODE::MMBUK: WriteTodo(code_gen, md.GetEntityName(), "MMBUK"); break;
        case OPCODE::CHGLD: WriteTodo(code_gen, md.GetEntityName(), "CHGLD"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FieldPartyInstruction::ProcessSTITM(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& item_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& amount = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine((boost::format("FFVII.add_item(%1%, %2%)") % item_id % amount).str());
}

void FieldPartyInstruction::ProcessPRTYE(CodeGenerator* code_gen){
    FieldCodeGenerator* gc = static_cast<FieldCodeGenerator*>(code_gen);
    auto char_id_1 = gc->GetFormatter().CharName(_params[0]->getUnsigned());
    char_id_1 = (char_id_1 == "") ? "nil" : ("\"" + char_id_1 + "\"");
    auto char_id_2 = gc->GetFormatter().CharName(_params[1]->getUnsigned());
    char_id_2 = (char_id_2 == "") ? "nil" : ("\"" + char_id_2 + "\"");
    auto char_id_3 = gc->GetFormatter().CharName(_params[2]->getUnsigned());
    char_id_3 = (char_id_3 == "") ? "nil" : ("\"" + char_id_3 + "\"");
    code_gen->AddOutputLine((
      boost::format("FFVII.set_party(%1%, %2%, %3%)") % char_id_1 % char_id_2 % char_id_3
    ).str());
}
