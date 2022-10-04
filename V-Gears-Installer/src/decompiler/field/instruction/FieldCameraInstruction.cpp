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
#include "decompiler/field/instruction/FieldCameraInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FF7::FieldCameraInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func.metadata);
    switch (opcode_){
    case OPCODES::NFADE: ProcessNFADE(code_gen); break;
    case OPCODES::SHAKE: code_gen->WriteTodo(md.GetEntityName(), "SHAKE"); break;
    case OPCODES::SCRLO: code_gen->WriteTodo(md.GetEntityName(), "SCRLO"); break;
    case OPCODES::SCRLC: code_gen->WriteTodo(md.GetEntityName(), "SCRLC"); break;
    case OPCODES::SCR2D: ProcessSCR2D(code_gen); break;
    case OPCODES::SCRCC: code_gen->WriteTodo(md.GetEntityName(), "SCRCC"); break;
    case OPCODES::SCR2DC: ProcessSCR2DC(code_gen); break;
    case OPCODES::SCRLW: code_gen->WriteTodo(md.GetEntityName(), "SCRLW"); break;
    case OPCODES::SCR2DL: code_gen->WriteTodo(md.GetEntityName(), "SCR2DL"); break;
    case OPCODES::VWOFT: code_gen->WriteTodo(md.GetEntityName(), "VWOFT"); break;
    case OPCODES::FADE: ProcessFADE(code_gen); break;
    case OPCODES::FADEW: code_gen->WriteTodo(md.GetEntityName(), "FADEW"); break;
    case OPCODES::SCRLP: code_gen->WriteTodo(md.GetEntityName(), "SCRLP"); break;
    case OPCODES::MVCAM: code_gen->WriteTodo(md.GetEntityName(), "MVCAM"); break;
    case OPCODES::SCRLA: code_gen->WriteTodo(md.GetEntityName(), "SCRLA"); break;
    default:
        code_gen->AddOutputLine(FF7::FieldCodeGenerator::FormatInstructionNotImplemented(
          md.GetEntityName(), address_, opcode_
        ));
    }
}

void FF7::FieldCameraInstruction::ProcessNFADE(CodeGenerator* code_gen){
    // TODO: Not fully reversed.
    auto raw_type = params_[4]->GetUnsigned();
    if (raw_type == 0){
        code_gen->AddOutputLine("-- fade:clear()");
        return;
    }
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const std::string type = raw_type == 12 ? "Fade.SUBTRACT" : "Fade.ADD";
    const auto& r = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[5]->GetUnsigned()
    );
    const auto& g = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[6]->GetUnsigned()
    );
    const auto& b = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[7]->GetUnsigned()
    );
    const auto& unknown = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[8]->GetUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- fade:fade(%2%, %3%, %4%, %1%, %5%)") % type % r % g % b % unknown).str()
    );
}

void FF7::FieldCameraInstruction::ProcessSCR2D(CodeGenerator* code_gen){
    // kUpScaler.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& x = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetSigned()
    );
    const auto& y = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetSigned()
    );
    code_gen->AddOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.NONE, 0)")
      % x % y
    ).str());
}

void FF7::FieldCameraInstruction::ProcessSCR2DC(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& x = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[4]->GetSigned()
    );
    const auto& y = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[5]->GetSigned()
    );
    const auto& speed = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[6]->GetUnsigned(),
      FF7::FieldCodeGenerator::ValueType::Float, 30.0f
    );
    code_gen->AddOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.SMOOTH, %3%)")
      % x % y % speed
    ).str());
}

void FF7::FieldCameraInstruction::ProcessFADE(CodeGenerator* code_gen){
    // TODO: not fully reversed
    auto raw_type = params_[8]->GetUnsigned();
    std::string type;
    switch (raw_type) {
        case 1:
        case 2:
        case 7:
        case 8:
            type = "Fade.SUBTRACT";
            break;
        case 4: code_gen->AddOutputLine("-- fade:black()"); return;
        default: type = "Fade.ADD"; break;
    }

    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& r = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[4]->GetUnsigned()
    );
    const auto& g = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[5]->GetUnsigned()
    );
    const auto& b = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[6]->GetUnsigned()
    );
    // TODO: needs to be divided by 30.0f?
    auto speed = params_[7]->GetUnsigned();
    auto start = params_[9]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- fade:fade(%2%, %3%, %4%, %1%, %5%, %6%)") % type % r % g % b % speed % start
    ).str());
}
