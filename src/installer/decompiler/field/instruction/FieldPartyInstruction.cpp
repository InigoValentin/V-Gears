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
        case OPCODES::SPTYE: ProcessPartyVariables(code_gen, true); break;
        case OPCODES::GTPYE: ProcessPartyVariables(code_gen, false); break;
        case OPCODES::GOLDU: ProcessGOLDU(code_gen); break;
        case OPCODES::GOLDD: ProcessGOLDD(code_gen); break;
        case OPCODES::HMPMAX1: code_gen->WriteTodo(md.GetEntityName(), "HMPMAX1"); break;
        case OPCODES::HMPMAX2: code_gen->WriteTodo(md.GetEntityName(), "HMPMAX2"); break;
        case OPCODES::MHMMX:
            // Full HP and MP of all the characters, and no status.
            code_gen->AddOutputLine("restore_all_characters()");
            break;
        case OPCODES::HMPMAX3: code_gen->WriteTodo(md.GetEntityName(), "HMPMAX3"); break;
        case OPCODES::MPU: code_gen->WriteTodo(md.GetEntityName(), "MPU"); break;
        case OPCODES::MPD: code_gen->WriteTodo(md.GetEntityName(), "MPD"); break;
        case OPCODES::HPU: code_gen->WriteTodo(md.GetEntityName(), "HPU"); break;
        case OPCODES::HPD: code_gen->WriteTodo(md.GetEntityName(), "HPD"); break;
        case OPCODES::STITM: ProcessSTITM(code_gen); break;
        case OPCODES::DLITM: ProcessDLITM(code_gen); break;
        case OPCODES::CKITM: ProcessCKITM(code_gen); break;
        case OPCODES::SMTRA: ProcessSMTRA(code_gen, md.GetEntityName()); break;
        case OPCODES::DMTRA: code_gen->WriteTodo(md.GetEntityName(), "DMTRA"); break;
        case OPCODES::CMTRA: code_gen->WriteTodo(md.GetEntityName(), "CMTRA"); break;
        case OPCODES::GETPC: code_gen->WriteTodo(md.GetEntityName(), "GETPC"); break;
        case OPCODES::PRTYP:
            // Adds a character to the party, in the first free place.
            code_gen->AddOutputLine(
              (boost::format("add_party_character(%1%)") % params_[0]->GetString()).str()
            );
            break;
        case OPCODES::PRTYM:
            // Removes a character from the party.
            code_gen->AddOutputLine(
              (boost::format("remove_party_character(%1%)") % params_[0]->GetString()).str()
            );
            break;
        case OPCODES::PRTYE: ProcessPRTYE(code_gen); break;
        case OPCODES::MMBUD: ProcessMMBUD(code_gen); break;
        case OPCODES::MMBLK: ProcessMMBLK(code_gen); break;
        case OPCODES::MMBUK: ProcessMMBUK(code_gen); break;
        case OPCODES::CHGLD: ProcessCHGLD(code_gen); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

namespace{
    /**
     * Writes the amount of gil of a GOLDU or GOLDD.
     *
     * The opcode is read as a byte of two nibbles (two parameters, the bank is the first) followed
     * by four bytes. With a bank of zero, the four bytes are the amount, as a little endian dword.
     * Otherwise, the first of the four bytes is the address in the bank, and the rest are zeroes.
     */
    std::string GilAmount(const std::vector<ValuePtr>& params){
        if (params[0]->GetUnsigned() == 0){
            const long amount = params[2]->GetUnsigned() + params[3]->GetUnsigned() * 256
              + params[4]->GetUnsigned() * 256 * 256 + params[5]->GetUnsigned() * 256 * 256 * 256;
            return std::to_string(amount);
        }
        return (
          boost::format("Banks[%1%][%2%]") % params[0]->GetUnsigned() % params[2]->GetUnsigned()
        ).str();
    }
}

void FieldPartyInstruction::ProcessGOLDU(CodeGenerator* code_gen){
    code_gen->AddOutputLine("Inventory.add_money(" + GilAmount(params_) + ")");
}

void FieldPartyInstruction::ProcessGOLDD(CodeGenerator* code_gen){
    code_gen->AddOutputLine("Inventory.remove_money(" + GilAmount(params_) + ")");
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
    // Operands: the bank of the destination (in the two nibbles of the first byte), the ID of the
    // item and the address, in that bank, that receives how many of the item there are.
    code_gen->AddOutputLine((
      boost::format("Banks[%1%][%2%] = Inventory.get_item_quantity(%3%)")
      % (params_[0]->GetUnsigned() * 16 + params_[1]->GetUnsigned()) % params_[3]->GetUnsigned()
      % params_[2]->GetUnsigned()
    ).str());
}

void FieldPartyInstruction::ProcessSMTRA(CodeGenerator* code_gen, const std::string& entity){
    // Operands: four bank nibbles (all of them zero in the original fields: the values are
    // literal), the ID of the materia and its AP, in three bytes (255, 255, 255 is the most).
    for (int i = 0; i < 4; i ++){
        if (params_[i]->GetUnsigned() != 0){
            code_gen->WriteTodo(entity, "SMTRA");
            return;
        }
    }
    const long ap = params_[5]->GetUnsigned() + params_[6]->GetUnsigned() * 256
      + params_[7]->GetUnsigned() * 256 * 256;
    code_gen->AddOutputLine((
      boost::format("Materia.add(%1%, %2%)") % params_[4]->GetUnsigned() % ap
    ).str());
}

void FieldPartyInstruction::ProcessPartyVariables(CodeGenerator* code_gen, const bool set){
    // Operands: the banks of three variables (a nibble for each, plus an unused one) and their
    // addresses. GTPYE stores the characters of the party in them (255 for an empty place) and
    // SPTYE sets the party from them. The scenes that change the party (a flashback) keep it that
    // way and restore it.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    std::string variables[3];
    for (int i = 0; i < 3; i ++){
        variables[i] = FieldCodeGenerator::FormatValueOrVariable(
          cg->GetFormatter(), params_[i]->GetUnsigned(), params_[4 + i]->GetUnsigned()
        );
    }
    if (set){
        code_gen->AddOutputLine((
          boost::format("set_party_from_values(%1%, %2%, %3%)")
          % variables[0] % variables[1] % variables[2]
        ).str());
    }
    else{
        for (int i = 0; i < 3; i ++){
            code_gen->AddOutputLine(
              (boost::format("%1% = Party[%2%] or 255") % variables[i] % (i + 1)).str()
            );
        }
    }
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

void FieldPartyInstruction::ProcessMMBUD(CodeGenerator* code_gen){
    // Param 0 is 0 to hide the character in the PHS and anything else to show it, param 1 is the
    // character (checked in the opcode handler of ff7.exe, 0x61c782).
    code_gen->AddOutputLine((
      boost::format("set_character_enable(%1%, %2%)") % params_[1]->GetString()
        % (params_[0]->GetUnsigned() != 0 ? "true" : "false")
    ).str());
}

void FieldPartyInstruction::ProcessMMBLK(CodeGenerator* code_gen){
    // Locks the character in the party: it can't be swapped in the PHS.
    code_gen->AddOutputLine((
      boost::format("set_party_lock(%1%, true)") % params_[0]->GetString()
    ).str());
}

void FieldPartyInstruction::ProcessMMBUK(CodeGenerator* code_gen){
    // Unlocks the character, so it can be swapped in the PHS again.
    code_gen->AddOutputLine((
      boost::format("set_party_lock(%1%, false)") % params_[0]->GetString()
    ).str());
}

void FieldPartyInstruction::ProcessCHGLD(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("Inventory.money_to_banks(%1%, %2%, %3%, %4%)")
        % params_[0]->GetUnsigned() % params_[1]->GetUnsigned()
        % params_[2]->GetUnsigned() % params_[3]->GetUnsigned()
    ).str());
}
