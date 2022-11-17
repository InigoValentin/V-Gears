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
#include "decompiler/field/instruction/FieldWindowInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldWindowInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FieldEngine& eng = static_cast<FieldEngine&>(*engine);
    FunctionMetaData md(func.metadata);
    switch (opcode_){
        case OPCODES::TUTOR: code_gen->WriteTodo(md.GetEntityName(), "TUTOR"); break;
        case OPCODES::WCLS: code_gen->WriteTodo(md.GetEntityName(), "WCLS"); break;
        case OPCODES::WSIZW: code_gen->WriteTodo(md.GetEntityName(), "WSIZW"); break;
        case OPCODES::WSPCL: ProcessWSPCL(code_gen); break;
        case OPCODES::WNUMB: code_gen->WriteTodo(md.GetEntityName(), "WNUMB"); break;
        case OPCODES::STTIM: ProcessSTTIM(code_gen); break;
        case OPCODES::MESSAGE: ProcessMESSAGE(code_gen, eng.GetScriptName()); break;
        case OPCODES::MPARA: code_gen->WriteTodo(md.GetEntityName(), "MPARA"); break;
        case OPCODES::MPRA2: code_gen->WriteTodo(md.GetEntityName(), "MPRA2"); break;
        case OPCODES::MPNAM: ProcessMPNAM(code_gen, eng.GetScriptName()); break;
        case OPCODES::ASK: code_gen->WriteTodo(md.GetEntityName(), "ASK"); break;
        case OPCODES::MENU: ProcessMENU(code_gen); break;
        case OPCODES::MENU2: ProcessMENU2(code_gen); break;
        case OPCODES::WINDOW: ProcessWINDOW(code_gen); break;
        case OPCODES::WMOVE: code_gen->WriteTodo(md.GetEntityName(), "WMOVE"); break;
        case OPCODES::WMODE: ProcessWMODE(code_gen); break;
        case OPCODES::WREST: code_gen->WriteTodo(md.GetEntityName(), "WREST"); break;
        case OPCODES::WCLSE: ProcessWCLSE(code_gen); break;
        case OPCODES::WROW: code_gen->WriteTodo(md.GetEntityName(), "WROW"); break;
        case OPCODES::GWCOL: code_gen->WriteTodo(md.GetEntityName(), "GWCOL"); break;
        case OPCODES::SWCOL: code_gen->WriteTodo(md.GetEntityName(), "SWCOL"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

void FieldWindowInstruction::ProcessWSPCL(CodeGenerator* code_gen){
    auto window_id = params_[0]->GetUnsigned();
    std::string numeric = "false";
    std::string timer = "false";
    if (params_[1]->GetUnsigned() == 1){
        numeric = "true";
        timer = "true";
    }
    else if (params_[1]->GetUnsigned() == 2){
        numeric = "true";
        timer = "false";
    }
    code_gen->AddOutputLine((
      boost::format("dialog:set_numeric(\"%1%\", %2%, %3%) -- x: %4%,  y: %5%")
      % window_id % numeric % timer % params_[2]->GetUnsigned() % params_[3]->GetUnsigned()
    ).str());
}

void FieldWindowInstruction::ProcessWMODE(CodeGenerator* code_gen){
    auto window_id = params_[0]->GetUnsigned();
    std::string closeable = "true";
    if (params_[2]->GetUnsigned() == 1) closeable = "false";
    code_gen->AddOutputLine((
      boost::format("dialog:set_mode(\"%1%\", %2%, %3%)")
      % window_id % params_[1]->GetUnsigned() % closeable
    ).str());
}

void FieldWindowInstruction::ProcessWINDOW(CodeGenerator* code_gen){
    // Initializes a new window. It won't be displayed until MESSAGE is used.
    auto windowId = params_[0]->GetUnsigned();
    auto x = params_[1]->GetUnsigned();
    auto y = params_[2]->GetUnsigned();
    auto width = params_[3]->GetUnsigned();
    auto height = params_[4]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("dialog:dialog_open(\"%1%\", %2%, %3%, %4%, %5%)")
      % windowId % x % y % width % height
    ).str());
}

void FieldWindowInstruction::ProcessMENU(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    int menu_id = std::stoi(FieldCodeGenerator::FormatValueOrVariable(
        cg->GetFormatter(), params_[1]->GetUnsigned(), params_[2]->GetSigned(),
        FieldCodeGenerator::ValueType::Integer
      ));
    int param = std::stoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetSigned(),
      FieldCodeGenerator::ValueType::Integer
    ));
    switch(menu_id){
        case 0x05: // Ending credits.
            code_gen->AddOutputLine("ending_credits()");
            break;
        case 0x06: // Name menu
            if (param == 64) // ID 64 is for chocobos.
                code_gen->AddOutputLine("name_chocobo()");
            else
                code_gen->AddOutputLine((boost::format("name_character(%1%)") % param).str());
            break;
        case 0x07: // Form party.
            switch (param){
                case 0x01: // Split in 3 teams (last battle).
                    code_gen->AddOutputLine("Party.split_teams(3)");
                    break;
                case 0x02: // Split in 2 teams (last battle).
                    code_gen->AddOutputLine("Party.split_teams(2)");
                    break;
                default: // 0x00: Normal, form party of three.
                    code_gen->AddOutputLine("Party.choose_party()");
            }
            break;
        case 0x08: //Shop
            code_gen->AddOutputLine((boost::format("open_shop(%1%)") % param).str());
            break;
        case 0x09: // Party menu
            code_gen->AddOutputLine("open_menu(\"party\")");
            break;
        case 0x0E: // Party menu
            code_gen->AddOutputLine("open_menu(\"save\")");
            break;
        case 0x0F: // Steal materia event.
            code_gen->AddOutputLine("Party.steal_materia()");
            break;
        case 0x12: // Remove materia from character.
            code_gen->AddOutputLine(
              (boost::format("Characters.remove_materia(%1%)") % param).str()
            );
            break;
        case 0x13: // Restore character materia.
            code_gen->AddOutputLine(
              (boost::format("Characters.restore_materia(%1%)") % param).str()
            );
            break;
        default:
            code_gen->AddOutputLine(
              (boost::format("-- Unknown menu %1% (%2%)") % menu_id % param).str()
            );
    }
}

void FieldWindowInstruction::ProcessSTTIM(CodeGenerator* code_gen){
    std::cout << "STTIM params:\n";
    for (int i = 0; i < params_.size(); i ++) std::cout << "    " << params_[i] << "\n";
    std::cout << "STTIM params END\n";
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    int h = std::stoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[4]->GetSigned(),
      FieldCodeGenerator::ValueType::Integer
    ));
    int m = std::stoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[5]->GetSigned(),
      FieldCodeGenerator::ValueType::Integer
    ));
    int s = std::stoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[6]->GetSigned(),
      FieldCodeGenerator::ValueType::Integer
    ));
    code_gen->AddOutputLine((
      boost::format("Timer.set(%1%) -- %2%:%3%:%4%") % (3600 * h + 60 * m + s) % h % m % s
    ).str());
}

void FieldWindowInstruction::ProcessMESSAGE(
  CodeGenerator* code_gen, const std::string& script_name
){
    // Displays a dialog in the WINDOW that has previously been initialized to display this dialog.
    auto window_id = params_[0]->GetUnsigned();
    auto dialog_id = params_[1]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("dialog:dialog_set_text(\"%1%\", \"%2%_%3%\")")
      % window_id % script_name % dialog_id
    ).str());
    code_gen->AddOutputLine(
      (boost::format("dialog:dialog_wait_for_close(\"%1%\")") % window_id).str()
    );
}

void FieldWindowInstruction::ProcessWCLSE(CodeGenerator* code_gen){
    // Close a dialog.
    auto windowId = params_[0]->GetUnsigned();
    code_gen->AddOutputLine((boost::format("dialog:dialog_close(\"%1%\")") % windowId).str());
}

void FieldWindowInstruction::ProcessMPNAM(CodeGenerator* code_gen, const std::string& script_name){
    code_gen->AddOutputLine((
      boost::format("set_map_name(\"%1%_%2%\")") % script_name % params_[0]->GetUnsigned()
    ).str());
}

void FieldWindowInstruction::ProcessMENU2(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- field:menu_lock(%1%)")
      % FieldCodeGenerator::FormatBool(params_[0]->GetUnsigned())
    ).str());
}
