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
#include "decompiler/field/instruction/FieldPartyInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FF7::FieldPartyInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func.metadata);
    switch (_opcode){
        case OPCODES::SPTYE: code_gen->WriteTodo(md.GetEntityName(), "SPTYE"); break;
        case OPCODES::GTPYE: code_gen->WriteTodo(md.GetEntityName(), "GTPYE"); break;
        case OPCODES::GOLDU: code_gen->WriteTodo(md.GetEntityName(), "GOLDU"); break;
        case OPCODES::GOLDD: code_gen->WriteTodo(md.GetEntityName(), "GOLDD"); break;
        case OPCODES::HMPMAX1: code_gen->WriteTodo(md.GetEntityName(), "HMPMAX1"); break;
        case OPCODES::HMPMAX2: code_gen->WriteTodo(md.GetEntityName(), "HMPMAX2"); break;
        case OPCODES::MHMMX: code_gen->WriteTodo(md.GetEntityName(), "MHMMX"); break;
        case OPCODES::HMPMAX3: code_gen->WriteTodo(md.GetEntityName(), "HMPMAX3"); break;
        case OPCODES::MPU: code_gen->WriteTodo(md.GetEntityName(), "MPU"); break;
        case OPCODES::MPD: code_gen->WriteTodo(md.GetEntityName(), "MPD"); break;
        case OPCODES::HPU: code_gen->WriteTodo(md.GetEntityName(), "HPU"); break;
        case OPCODES::HPD: code_gen->WriteTodo(md.GetEntityName(), "HPD"); break;
        case OPCODES::STITM: ProcessSTITM(code_gen); break;
        case OPCODES::DLITM: code_gen->WriteTodo(md.GetEntityName(), "DLITM"); break;
        case OPCODES::CKITM: code_gen->WriteTodo(md.GetEntityName(), "CKITM"); break;
        case OPCODES::SMTRA: code_gen->WriteTodo(md.GetEntityName(), "SMTRA"); break;
        case OPCODES::DMTRA: code_gen->WriteTodo(md.GetEntityName(), "DMTRA"); break;
        case OPCODES::CMTRA: code_gen->WriteTodo(md.GetEntityName(), "CMTRA"); break;
        case OPCODES::GETPC: code_gen->WriteTodo(md.GetEntityName(), "GETPC"); break;
        case OPCODES::PRTYP: code_gen->WriteTodo(md.GetEntityName(), "PRTYP"); break;
        case OPCODES::PRTYM: code_gen->WriteTodo(md.GetEntityName(), "PRTYM"); break;
        case OPCODES::PRTYE: ProcessPRTYE(code_gen); break;
        case OPCODES::MMBUD: code_gen->WriteTodo(md.GetEntityName(), "MMBUD"); break;
        case OPCODES::MMBLK: code_gen->WriteTodo(md.GetEntityName(), "MMBLK"); break;
        case OPCODES::MMBUK: code_gen->WriteTodo(md.GetEntityName(), "MMBUK"); break;
        case OPCODES::CHGLD: code_gen->WriteTodo(md.GetEntityName(), "CHGLD"); break;
        default:
            code_gen->AddOutputLine(FF7::FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7::FieldPartyInstruction::ProcessSTITM(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& item_id = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& amount = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine((boost::format("FFVII.add_item(%1%, %2%)") % item_id % amount).str());
}

void FF7::FieldPartyInstruction::ProcessPRTYE(CodeGenerator* code_gen){
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
