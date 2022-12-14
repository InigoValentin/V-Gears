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
#include "decompiler/field/instruction/FieldCameraInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldCameraInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func.metadata);
    //std::cout << "Camera opcode: " << opcode_ << "\n";
    switch (opcode_){
    case OPCODES::NFADE: ProcessNFADE(code_gen); break;
    case OPCODES::SHAKE: code_gen->WriteTodo(md.GetEntityName(), "SHAKE"); break;
    case OPCODES::SCRLO: code_gen->WriteTodo(md.GetEntityName(), "SCRLO"); break;
    case OPCODES::SCRLC: ProcessSCRLC(code_gen); break;
    case OPCODES::SCR2D: ProcessSCR2D(code_gen); break;
    case OPCODES::SCRCC: ProcessSCRCC(code_gen); break;
    case OPCODES::SCR2DC: ProcessSCR2DC(code_gen); break;
    case OPCODES::SCRLW: code_gen->WriteTodo(md.GetEntityName(), "SCRLW"); break;
    case OPCODES::SCR2DL: ProcessSCR2DL(code_gen); break;
    case OPCODES::VWOFT: ProcessVWOFT(code_gen); break;
    case OPCODES::FADE: ProcessFADE(code_gen); break;
    case OPCODES::FADEW: code_gen->WriteTodo(md.GetEntityName(), "FADEW"); break;
    case OPCODES::SCRLP: code_gen->WriteTodo(md.GetEntityName(), "SCRLP"); break;
    case OPCODES::MVCAM: code_gen->WriteTodo(md.GetEntityName(), "MVCAM"); break;
    case OPCODES::SCRLA: code_gen->WriteTodo(md.GetEntityName(), "SCRLA"); break;
    default:
        code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
          md.GetEntityName(), address_, opcode_
        ));
    }
}

void FieldCameraInstruction::ProcessNFADE(CodeGenerator* code_gen){
    // TODO: Not fully reversed.
    auto raw_type = params_[4]->GetUnsigned();
    if (raw_type == 0){
        code_gen->AddOutputLine("-- fade:clear()");
        return;
    }
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const std::string type = raw_type == 12 ? "Fade.SUBTRACT" : "Fade.ADD";
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[5]->GetUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[6]->GetUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[7]->GetUnsigned()
    );
    const auto& unknown = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[8]->GetUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- fade:fade(%2%, %3%, %4%, %1%, %5%)") % type % r % g % b % unknown).str()
    );
}

void FieldCameraInstruction::ProcessSCRLC(CodeGenerator* code_gen){
    std::string mode = "Background2D.NONE";
    if (params_[3]->GetUnsigned() == 2) mode = "Background2D.LINEAR";
    else if (params_[3]->GetUnsigned() == 3) mode = "Background2D.SMOOTH";
    code_gen->AddOutputLine("background2d:scroll_to_player(" + mode + ", 0)");
}

void FieldCameraInstruction::ProcessSCR2D(CodeGenerator* code_gen){
    // kUpScaler.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetSigned()
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetSigned()
    );
    code_gen->AddOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.NONE, 0)")
      % x % y
    ).str());
}

void FieldCameraInstruction::ProcessSCRCC(CodeGenerator* code_gen){
    code_gen->AddOutputLine("background2d:scroll_to_player(Background2D.NONE, 0)");
}

void FieldCameraInstruction::ProcessSCR2DL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[4]->GetSigned()
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[5]->GetSigned()
    );
    const auto& speed = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[6]->GetUnsigned(),
      FieldCodeGenerator::ValueType::Float, 30.0f
    );
    code_gen->AddOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.LINEAR, %3%)")
      % x % y % speed
    ).str());
}

void FieldCameraInstruction::ProcessSCR2DC(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[4]->GetSigned()
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[5]->GetSigned()
    );
    const auto& speed = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[6]->GetUnsigned(),
      FieldCodeGenerator::ValueType::Float, 30.0f
    );
    code_gen->AddOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.SMOOTH, %3%)")
      % x % y % speed
    ).str());
}

void FieldCameraInstruction::ProcessVWOFT(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetSigned()
    );
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetSigned()
    );
    const auto& speed = params_[4]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("background2d:offset(%1%, %2%) -- Speed %3%")
      % x % y % speed
    ).str());
}

void FieldCameraInstruction::ProcessFADE(CodeGenerator* code_gen){
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
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[4]->GetUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[5]->GetUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[6]->GetUnsigned()
    );
    // TODO: needs to be divided by 30.0f?
    auto speed = params_[7]->GetUnsigned();
    auto start = params_[9]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- fade:fade(%2%, %3%, %4%, %1%, %5%, %6%)") % type % r % g % b % speed % start
    ).str());
}
