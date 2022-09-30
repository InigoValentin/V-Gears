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
#include "decompiler/field/instruction/FieldWindowInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FF7::FieldWindowInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FF7::FieldEngine& eng = static_cast<FF7::FieldEngine&>(*engine);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODES::TUTOR: code_gen->WriteTodo(md.GetEntityName(), "TUTOR"); break;
        case OPCODES::WCLS: code_gen->WriteTodo(md.GetEntityName(), "WCLS"); break;
        case OPCODES::WSIZW: code_gen->WriteTodo(md.GetEntityName(), "WSIZW"); break;
        case OPCODES::WSPCL: code_gen->WriteTodo(md.GetEntityName(), "WSPCL"); break;
        case OPCODES::WNUMB: code_gen->WriteTodo(md.GetEntityName(), "WNUMB"); break;
        case OPCODES::STTIM: code_gen->WriteTodo(md.GetEntityName(), "STTIM"); break;
        case OPCODES::MESSAGE: ProcessMESSAGE(code_gen, eng.GetScriptName()); break;
        case OPCODES::MPARA: code_gen->WriteTodo(md.GetEntityName(), "MPARA"); break;
        case OPCODES::MPRA2: code_gen->WriteTodo(md.GetEntityName(), "MPRA2"); break;
        case OPCODES::MPNAM: ProcessMPNAM(code_gen); break;
        case OPCODES::ASK: code_gen->WriteTodo(md.GetEntityName(), "ASK"); break;
        case OPCODES::MENU: code_gen->WriteTodo(md.GetEntityName(), "MENU"); break;
        case OPCODES::MENU2: ProcessMENU2(code_gen); break;
        case OPCODES::WINDOW: ProcessWINDOW(code_gen); break;
        case OPCODES::WMOVE: code_gen->WriteTodo(md.GetEntityName(), "WMOVE"); break;
        case OPCODES::WMODE: code_gen->WriteTodo(md.GetEntityName(), "WMODE"); break;
        case OPCODES::WREST: code_gen->WriteTodo(md.GetEntityName(), "WREST"); break;
        case OPCODES::WCLSE: ProcessWCLSE(code_gen); break;
        case OPCODES::WROW: code_gen->WriteTodo(md.GetEntityName(), "WROW"); break;
        case OPCODES::GWCOL: code_gen->WriteTodo(md.GetEntityName(), "GWCOL"); break;
        case OPCODES::SWCOL: code_gen->WriteTodo(md.GetEntityName(), "SWCOL"); break;
        default:
            code_gen->AddOutputLine(FF7::FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7::FieldWindowInstruction::ProcessWINDOW(CodeGenerator* code_gen){
    // Initializes a new window. It won't be displayed until MESSAGE is used.
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

void FF7::FieldWindowInstruction::ProcessMESSAGE(
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

void FF7::FieldWindowInstruction::ProcessWCLSE(CodeGenerator* code_gen){
    // Close a dialog.
    auto windowId = _params[0]->getUnsigned();
    code_gen->AddOutputLine((boost::format("dialog:dialog_close(\"%1%\")") % windowId).str());
}

void FF7::FieldWindowInstruction::ProcessMPNAM(CodeGenerator* code_gen){
    code_gen->AddOutputLine(
      (boost::format("-- field:map_name(%1%)") % _params[0]->getUnsigned()).str()
    );
}

void FF7::FieldWindowInstruction::ProcessMENU2(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- field:menu_lock(%1%)")
      % FF7::FieldCodeGenerator::FormatBool(_params[0]->getUnsigned())
    ).str());
}
