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

#include "decompiler/field/instruction/FieldWindowInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldWindowInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FieldEngine& eng = static_cast<FieldEngine&>(*engine);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::TUTOR: WriteTodo(code_gen, md.GetEntityName(), "TUTOR"); break;
        case OPCODE::WCLS: WriteTodo(code_gen, md.GetEntityName(), "WCLS"); break;
        case OPCODE::WSIZW: WriteTodo(code_gen, md.GetEntityName(), "WSIZW"); break;
        case OPCODE::WSPCL: WriteTodo(code_gen, md.GetEntityName(), "WSPCL"); break;
        case OPCODE::WNUMB: WriteTodo(code_gen, md.GetEntityName(), "WNUMB"); break;
        case OPCODE::STTIM: WriteTodo(code_gen, md.GetEntityName(), "STTIM"); break;
        case OPCODE::MESSAGE: ProcessMESSAGE(code_gen, eng.ScriptName()); break;
        case OPCODE::MPARA: WriteTodo(code_gen, md.GetEntityName(), "MPARA"); break;
        case OPCODE::MPRA2: WriteTodo(code_gen, md.GetEntityName(), "MPRA2"); break;
        case OPCODE::MPNAM: ProcessMPNAM(code_gen); break;
        case OPCODE::ASK: WriteTodo(code_gen, md.GetEntityName(), "ASK"); break;
        case OPCODE::MENU: WriteTodo(code_gen, md.GetEntityName(), "MENU"); break;
        case OPCODE::MENU2: ProcessMENU2(code_gen); break;
        case OPCODE::WINDOW: ProcessWINDOW(code_gen); break;
        case OPCODE::WMOVE: WriteTodo(code_gen, md.GetEntityName(), "WMOVE"); break;
        case OPCODE::WMODE: WriteTodo(code_gen, md.GetEntityName(), "WMODE"); break;
        case OPCODE::WREST: WriteTodo(code_gen, md.GetEntityName(), "WREST"); break;
        case OPCODE::WCLSE: ProcessWCLSE(code_gen); break;
        case OPCODE::WROW: WriteTodo(code_gen, md.GetEntityName(), "WROW"); break;
        case OPCODE::GWCOL: WriteTodo(code_gen, md.GetEntityName(), "GWCOL"); break;
        case OPCODE::SWCOL: WriteTodo(code_gen, md.GetEntityName(), "SWCOL"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FieldWindowInstruction::ProcessWINDOW(CodeGenerator* code_gen){
    // Init a new window. It won't be displayed until MESSAGE is used.
    auto windowId = _params[0]->getUnsigned();
    auto x = _params[1]->getUnsigned();
    auto y = _params[2]->getUnsigned();
    auto width = _params[3]->getUnsigned();
    auto height = _params[4]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("dialog:dialog_open(\"%1%\", %2%, %3%, %4%, %5%)")
      % windowId % x % y % width % height
    ).str());
}

void FieldWindowInstruction::ProcessMESSAGE(
  CodeGenerator* code_gen, const std::string& script_name
){
    // Displays a dialog in the WINDOW that has previously been initialized to display this dialog.
    auto window_id = _params[0]->getUnsigned();
    auto dialog_id = _params[1]->getUnsigned();
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
    auto windowId = _params[0]->getUnsigned();
    code_gen->AddOutputLine((boost::format("dialog:dialog_close(\"%1%\")") % windowId).str());
}

void FieldWindowInstruction::ProcessMPNAM(CodeGenerator* code_gen){
    code_gen->AddOutputLine(
      (boost::format("-- field:map_name(%1%)") % _params[0]->getUnsigned()).str()
    );
}

void FieldWindowInstruction::ProcessMENU2(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- field:menu_lock(%1%)")
      % FieldCodeGenerator::FormatBool(_params[0]->getUnsigned())
    ).str());
}
