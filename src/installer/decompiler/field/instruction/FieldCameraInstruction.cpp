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
    case OPCODES::SHAKE: ProcessSHAKE(code_gen); break;
    case OPCODES::SCRLO: code_gen->WriteTodo(md.GetEntityName(), "SCRLO"); break;
    case OPCODES::SCRLC: ProcessSCRLC(code_gen); break;
    case OPCODES::SCR2D: ProcessSCR2D(code_gen); break;
    case OPCODES::SCRCC: ProcessSCRCC(code_gen); break;
    case OPCODES::SCR2DC: ProcessSCR2DC(code_gen); break;
    case OPCODES::SCRLW: code_gen->AddOutputLine("background2d:scroll_sync()"); break;
    case OPCODES::SCR2DL: ProcessSCR2DL(code_gen); break;
    case OPCODES::VWOFT: ProcessVWOFT(code_gen); break;
    case OPCODES::FADE: ProcessFADE(code_gen); break;
    case OPCODES::FADEW: code_gen->AddOutputLine("screen_fade:wait()"); break;
    case OPCODES::SCRLP: code_gen->WriteTodo(md.GetEntityName(), "SCRLP"); break;
    case OPCODES::MVCAM: code_gen->WriteTodo(md.GetEntityName(), "MVCAM"); break;
    case OPCODES::SCRLA: ProcessSCRLA(code_gen, static_cast<FieldEngine&>(*engine)); break;
    default:
        code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
          md.GetEntityName(), address_, opcode_
        ));
    }
}

void FieldCameraInstruction::ProcessNFADE(CodeGenerator* code_gen){
    // Operands: the banks of R, G and B (the third nibble is always zero), the type, R, G, B
    // and the duration in frames.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[5]->GetUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[6]->GetUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[7]->GetUnsigned()
    );
    code_gen->AddOutputLine((
      boost::format("screen_fade:nfade(%1%, %2%, %3%, %4%, %5%)")
      % params_[4]->GetUnsigned() % r % g % b % params_[8]->GetUnsigned()
    ).str());
}

void FieldCameraInstruction::ProcessSHAKE(CodeGenerator* code_gen){
    // Operands: two banks for the amplitudes (always zero in the game data), the type, and the
    // amplitude and frames between bounces of each axis.
    if (params_[0]->GetUnsigned() != 0 || params_[1]->GetUnsigned() != 0){
        code_gen->WriteTodo("FieldCameraInstruction", "SHAKE with variable parameters");
        return;
    }
    code_gen->AddOutputLine((
      boost::format("background2d:shake(%1%, %2%, %3%, %4%, %5%)")
      % params_[2]->GetUnsigned() % params_[3]->GetUnsigned() % params_[4]->GetUnsigned()
      % params_[5]->GetUnsigned() % params_[6]->GetUnsigned()
    ).str());
}

void FieldCameraInstruction::ProcessSCRLC(CodeGenerator* code_gen){
    std::string mode;
    switch (params_[3]->GetUnsigned()){
        case 2: mode = "Background2D.LINEAR"; break;
        case 3: mode = "Background2D.SMOOTH"; break;
        default:
            // The other types don't move the camera, nor change what it is following.
            code_gen->AddOutputLine("-- SCRLC without effect");
            return;
    }
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& seconds = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[1]->GetUnsigned(),
      FieldCodeGenerator::ValueType::Float, 30.0f
    );
    code_gen->AddOutputLine("background2d:scroll_to_player(" + mode + ", " + seconds + ")");
}

void FieldCameraInstruction::ProcessSCRLA(CodeGenerator* code_gen, const FieldEngine& engine){
    std::string mode;
    switch (params_[4]->GetUnsigned()){
        case 0:
            code_gen->AddOutputLine("-- SCRLA without effect");
            return;
        case 1: mode = "Background2D.NONE"; break;
        case 2: mode = "Background2D.LINEAR"; break;
        case 3: mode = "Background2D.SMOOTH"; break;
        default:
            code_gen->WriteTodo("FieldCameraInstruction", "SCRLA with unsupported scroll type");
            return;
    }
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& seconds = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[2]->GetUnsigned(),
      FieldCodeGenerator::ValueType::Float, 30.0f
    );
    const auto& entity = engine.EntityByIndex(params_[3]->GetUnsigned());
    code_gen->AddOutputLine((
      boost::format("background2d:scroll_to_entity(entity_manager:get_entity(\"%1%\"), %2%, %3%)")
      % entity.GetName() % mode % seconds
    ).str());
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
    // Operands: the banks of R, G and B (the third nibble is always zero), R, G, B, the speed,
    // the type and the adjust, which doesn't change what is seen.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[4]->GetUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[5]->GetUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[6]->GetUnsigned()
    );
    code_gen->AddOutputLine((
      boost::format("screen_fade:fade(%1%, %2%, %3%, %4%, %5%)")
      % params_[8]->GetUnsigned() % r % g % b % params_[7]->GetUnsigned()
    ).str());
}
