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

#include "decompiler/field/instruction/FieldModuleInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldModuleInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::DSKCG: WriteTodo(code_gen, md.GetEntityName(), "DSKCG"); break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::ARROW:
            code_gen->AddOutputLine((
              boost::format("game:pointer_enable(%1%)")
              % (_params[0]->getUnsigned() ? "true" : "false")
            ).str());
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::PNAME:
            WriteTodo(code_gen, md.GetEntityName(), "PNAME");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::GMSPD:
            WriteTodo(code_gen, md.GetEntityName(), "GMSPD");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::SMSPD:
            WriteTodo(code_gen, md.GetEntityName(), "SMSPD");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::FLMAT:
            code_gen->AddOutputLine("game:fill_materia()");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::FLITM:
            WriteTodo(code_gen, md.GetEntityName(), "FLITM");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::BTLCK:
            code_gen->AddOutputLine((
              boost::format("game:battle_enable(%1%)")
              % (_params[0]->getUnsigned() ? "true" : "false")
            ).str());
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::MVLCK:
            code_gen->AddOutputLine((
              boost::format("game:movie_enable(%1%)")
              % (_params[0]->getUnsigned() ? "true" : "false")
            ).str());
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::SPCNM:
            WriteTodo(code_gen, md.GetEntityName(), "SPCNM");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::RSGLB:
            code_gen->AddOutputLine("game:global_reset()");
            break;
        case (OPCODE::SPECIAL << 8) | OPCODE_SPECIAL::CLITM:
            WriteTodo(code_gen, md.GetEntityName(), "CLITM");
            break;
        case OPCODE::MINIGAME: WriteTodo(code_gen, md.GetEntityName(), "MINIGAME"); break;
        case OPCODE::BTMD2: WriteTodo(code_gen, md.GetEntityName(), "BTMD2"); break;
        case OPCODE::BTRLD: WriteTodo(code_gen, md.GetEntityName(), "BTRLD"); break;
        case OPCODE::BTLTB: WriteTodo(code_gen, md.GetEntityName(), "BTLTB"); break;
        case OPCODE::MAPJUMP: ProcessMAPJUMP(code_gen, func); break;
        case OPCODE::LSTMP: WriteTodo(code_gen, md.GetEntityName(), "LSTMP"); break;
        case OPCODE::BATTLE: ProcessBATTLE(code_gen); break;
        case OPCODE::BTLON: ProcessBTLON(code_gen); break;
        case OPCODE::BTLMD: WriteTodo(code_gen, md.GetEntityName(), "BTLMD"); break;
        case OPCODE::MPJPO:
            // Gateway function will do nothing if this is set to true
            code_gen->AddOutputLine(
              std::string("FFVII.Data.DisableGateways=")
              + (_params[0]->getUnsigned() ? "true" : "false")
            );
            break;
        // Prepare to change map, don't need to output anything for this.
        case OPCODE::PMJMP: code_gen->AddOutputLine("-- Prepare map change"); break;
        // Prepare to change map, don't need to output anything for this,
        // seems to be the same thing as PMJMP
        case OPCODE::PMJMP2: code_gen->AddOutputLine("-- Prepare map change 2"); break;
        case OPCODE::GAMEOVER: WriteTodo(code_gen, md.GetEntityName(), "GAMEOVER"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FieldModuleInstruction::ProcessBATTLE(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& battle_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    code_gen->AddOutputLine((boost::format("entity_manager:battle_run(%1%)") % battle_id).str());
}

void FieldModuleInstruction::ProcessBTLON(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("entity_manager:random_encounters_on(%1%)")
      % FieldCodeGenerator::FormatInvertedBool(_params[0]->getUnsigned())
    ).str());
}

void FieldModuleInstruction::ProcessMAPJUMP(CodeGenerator* code_gen, Function& func){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto target_map_id = _params[0]->getUnsigned();
    FunctionMetaData md(func._metadata);
    const std::string source_spawn_point_name
      = cg->GetFormatter().SpawnPointName(target_map_id, md.GetEntityName(), func._name, _address);

    cg->GetFormatter().AddSpawnPoint(
      target_map_id, md.GetEntityName(), func._name, _address,
      _params[1]->getSigned(), // X
      _params[2]->getSigned(), // Y
      _params[3]->getSigned(), // Walk mesh triangle ID
      _params[4]->getSigned()  // Angle
    );

    const std::string target_map_name = cg->GetFormatter().MapName(target_map_id);
    code_gen->AddOutputLine(
      "load_field_map_request(\"" + target_map_name + "\", \"" + source_spawn_point_name + "\")"
    );
}
