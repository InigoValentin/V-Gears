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
#include "decompiler/field/instruction/FieldModuleInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldModuleInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func.metadata);
    //std::cout << "Module opcode: " << opcode_ << "\n";
    switch (opcode_){
        case OPCODES::DSKCG: code_gen->WriteTodo(md.GetEntityName(), "DSKCG"); break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::ARROW:
            code_gen->AddOutputLine((
              boost::format("game:pointer_enable(%1%)")
              % (params_[0]->GetUnsigned() ? "true" : "false")
            ).str());
            break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::PNAME:
            code_gen->WriteTodo(md.GetEntityName(), "PNAME");
            break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::GMSPD:
            code_gen->WriteTodo(md.GetEntityName(), "GMSPD");
            break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::SMSPD:
            code_gen->WriteTodo(md.GetEntityName(), "SMSPD");
            break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::FLMAT:
            code_gen->AddOutputLine("game:fill_materia()");
            break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::FLITM:
            code_gen->WriteTodo(md.GetEntityName(), "FLITM");
            break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::BTLCK:
            code_gen->AddOutputLine((
              boost::format("game:battle_enable(%1%)")
              % (params_[0]->GetUnsigned() ? "true" : "false")
            ).str());
            break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::MVLCK:
            code_gen->AddOutputLine((
              boost::format("game:movie_enable(%1%)")
              % (params_[0]->GetUnsigned() ? "true" : "false")
            ).str());
            break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::SPCNM:
            code_gen->WriteTodo(md.GetEntityName(), "SPCNM");
            break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::RSGLB:
            code_gen->AddOutputLine("game:global_reset()");
            break;
        case (OPCODES::SPECIAL << 8) | OPCODES_SPECIAL::CLITM:
            code_gen->WriteTodo(md.GetEntityName(), "CLITM");
            break;
        case OPCODES::MINIGAME: code_gen->WriteTodo(md.GetEntityName(), "MINIGAME"); break;
        case OPCODES::BTMD2: code_gen->WriteTodo(md.GetEntityName(), "BTMD2"); break;
        case OPCODES::BTRLD: code_gen->WriteTodo(md.GetEntityName(), "BTRLD"); break;
        case OPCODES::BTLTB: code_gen->WriteTodo(md.GetEntityName(), "BTLTB"); break;
        case OPCODES::MAPJUMP: ProcessMAPJUMP(code_gen, func); break;
        case OPCODES::LSTMP: code_gen->WriteTodo(md.GetEntityName(), "LSTMP"); break;
        case OPCODES::BATTLE: ProcessBATTLE(code_gen); break;
        case OPCODES::BTLON: ProcessBTLON(code_gen); break;
        case OPCODES::BTLMD: code_gen->WriteTodo(md.GetEntityName(), "BTLMD"); break;
        case OPCODES::MPJPO:
            // Gateway function will do nothing if this is set to true
            code_gen->AddOutputLine(
              std::string("Data.DisableGateways=")
              + (params_[0]->GetUnsigned() ? "true" : "false")
            );
            break;
        // Prepare to change map, don't need to output anything for this.
        case OPCODES::PMJMP: code_gen->AddOutputLine("-- Prepare map change"); break;
        // Prepare to change map, don't need to output anything for this,
        // seems to be the same thing as PMJMP.
        case OPCODES::PMJMP2: code_gen->AddOutputLine("-- Prepare map change 2"); break;
        case OPCODES::GAMEOVER: code_gen->WriteTodo(md.GetEntityName(), "GAMEOVER"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

void FieldModuleInstruction::ProcessBATTLE(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& battle_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[1]->GetUnsigned()
    );
    code_gen->AddOutputLine((boost::format("attle_manager:start_battle(%1%)") % battle_id).str());
}

void FieldModuleInstruction::ProcessBTLON(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("entity_manager:random_encounters_on(%1%)")
      % FieldCodeGenerator::FormatInvertedBool(params_[0]->GetUnsigned())
    ).str());
}

void FieldModuleInstruction::ProcessMAPJUMP(CodeGenerator* code_gen, Function& func){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto target_map_id = params_[0]->GetUnsigned();
    FunctionMetaData md(func.metadata);
    const std::string source_spawn_point_name = cg->GetFormatter().GetSpawnPointName(
      target_map_id, md.GetEntityName(), func.name, address_
    );
    cg->GetFormatter().AddSpawnPoint(
      target_map_id, md.GetEntityName(), func.name, address_,
      params_[1]->GetSigned(), // X
      params_[2]->GetSigned(), // Y
      params_[3]->GetSigned(), // Walk mesh triangle ID
      params_[4]->GetSigned()  // Angle
    );
    const std::string target_map_name = cg->GetFormatter().GetMapName(target_map_id);
    code_gen->AddOutputLine(
      "load_field_map_request(\"" + target_map_name + "\", \"" + source_spawn_point_name + "\")"
    );
}
