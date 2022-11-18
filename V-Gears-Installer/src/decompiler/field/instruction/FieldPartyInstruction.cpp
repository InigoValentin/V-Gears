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
#include "decompiler/field/instruction/FieldPartyInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldPartyInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func.metadata);
    //std::cout << "Party opcode: " << opcode_ << "\n";
    switch (opcode_){
        case OPCODES::SPTYE: code_gen->WriteTodo(md.GetEntityName(), "SPTYE"); break;
        case OPCODES::GTPYE: code_gen->WriteTodo(md.GetEntityName(), "GTPYE"); break;
        case OPCODES::GOLDU: ProcessGOLDU(code_gen); break;
        case OPCODES::GOLDD: ProcessGOLDD(code_gen); break;
        case OPCODES::HMPMAX1: code_gen->WriteTodo(md.GetEntityName(), "HMPMAX1"); break;
        case OPCODES::HMPMAX2: code_gen->WriteTodo(md.GetEntityName(), "HMPMAX2"); break;
        case OPCODES::MHMMX: code_gen->WriteTodo(md.GetEntityName(), "MHMMX"); break;
        case OPCODES::HMPMAX3: code_gen->WriteTodo(md.GetEntityName(), "HMPMAX3"); break;
        case OPCODES::MPU: code_gen->WriteTodo(md.GetEntityName(), "MPU"); break;
        case OPCODES::MPD: code_gen->WriteTodo(md.GetEntityName(), "MPD"); break;
        case OPCODES::HPU: code_gen->WriteTodo(md.GetEntityName(), "HPU"); break;
        case OPCODES::HPD: code_gen->WriteTodo(md.GetEntityName(), "HPD"); break;
        case OPCODES::STITM: ProcessSTITM(code_gen); break;
        case OPCODES::DLITM: ProcessDLITM(code_gen); break;
        case OPCODES::CKITM: ProcessCKITM(code_gen); break;
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
        case OPCODES::CHGLD: ProcessCHGLD(code_gen); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

void FieldPartyInstruction::ProcessGOLDU(CodeGenerator* code_gen){
    if (params_[0]->GetUnsigned() == 0){
        // Add by constant amount, params 1-4 are a dword with the amount.
        // Quick and dirty hex operation:
        long amount = params_[1]->GetUnsigned() + params_[2]->GetUnsigned() * 256
          + params_[3]->GetUnsigned() * 256 * 256 + params_[4]->GetUnsigned() * 256 * 256 * 256;
        code_gen->AddOutputLine((boost::format("Inventory.add_money(%1%)") % amount).str());
    }
    else{
        // Add by variable amount, param 0 is bank and 1 is address. 2-4 are zeroes, ignored.
        code_gen->AddOutputLine((
          boost::format("Inventory.add_money(Banks[%1%][%2%])")
            % params_[0]->GetUnsigned() % params_[1]->GetUnsigned()
        ).str());
    }
}

void FieldPartyInstruction::ProcessGOLDD(CodeGenerator* code_gen){
    if (params_[0]->GetUnsigned() == 0){
        // Add by constant amount, params 1-4 are a dword with the amount.
        // Quick and dirty hex operation:
        long amount = params_[1]->GetUnsigned() + params_[2]->GetUnsigned() * 256
          + params_[3]->GetUnsigned() * 256 * 256 + params_[4]->GetUnsigned() * 256 * 256 * 256;
        code_gen->AddOutputLine((boost::format("Inventory.add_money(%1%)") % amount).str());
    }
    else{
        // Add by variable amount, param 0 is bank and 1 is address. 2-4 are zeroes, ignored.
        code_gen->AddOutputLine((
          boost::format("Inventory.add_money(Banks[%1%][%2%])")
            % params_[0]->GetUnsigned() % params_[1]->GetUnsigned()
        ).str());
    }
}

void FieldPartyInstruction::ProcessSTITM(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& item_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& amount = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("Inventory.add_item(%1%, %2%)") % item_id % amount).str()
    );
}

void FieldPartyInstruction::ProcessDLITM(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& item_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& amount = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("Inventory.remove_item(%1%, %2%)") % item_id % amount).str()
    );
}

void FieldPartyInstruction::ProcessCKITM(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("Banks[%1%][%2%] = Inventory.get_item_quantity(%3%)")
      % params_[0]->GetUnsigned() % params_[2]->GetUnsigned() % params_[1]->GetUnsigned()
    ).str());
}

void FieldPartyInstruction::ProcessPRTYE(CodeGenerator* code_gen){
    std::string chars[3];
    for (int i = 0; i < 3; i ++){
        if (params_[i]->GetUnsigned() < 254) chars[i] = params_[i]->GetString();
        else chars[i] = "nil";
    }
    code_gen->AddOutputLine((
      boost::format("set_party(%1%, %2%, %3%)") % chars[0] % chars[1] % chars[2]
    ).str());
}

void FieldPartyInstruction::ProcessCHGLD(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("Inventory.money_to_banks(%1%, %2%, %3%, %4%)")
        % params_[0]->GetUnsigned() % params_[1]->GetUnsigned()
        % params_[2]->GetUnsigned() % params_[3]->GetUnsigned()
    ).str());
}
