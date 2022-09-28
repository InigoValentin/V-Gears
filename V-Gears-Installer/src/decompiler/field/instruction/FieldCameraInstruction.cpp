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

#include "decompiler/field/instruction/FieldCameraInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldCameraInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
    case OPCODE::NFADE: ProcessNFADE(code_gen); break;
    case OPCODE::SHAKE: WriteTodo(code_gen, md.GetEntityName(), "SHAKE"); break;
    case OPCODE::SCRLO: WriteTodo(code_gen, md.GetEntityName(), "SCRLO"); break;
    case OPCODE::SCRLC: WriteTodo(code_gen, md.GetEntityName(), "SCRLC"); break;
    case OPCODE::SCR2D: ProcessSCR2D(code_gen); break;
    case OPCODE::SCRCC: WriteTodo(code_gen, md.GetEntityName(), "SCRCC"); break;
    case OPCODE::SCR2DC: ProcessSCR2DC(code_gen); break;
    case OPCODE::SCRLW: WriteTodo(code_gen, md.GetEntityName(), "SCRLW"); break;
    case OPCODE::SCR2DL: WriteTodo(code_gen, md.GetEntityName(), "SCR2DL"); break;
    case OPCODE::VWOFT: WriteTodo(code_gen, md.GetEntityName(), "VWOFT"); break;
    case OPCODE::FADE: ProcessFADE(code_gen); break;
    case OPCODE::FADEW: WriteTodo(code_gen, md.GetEntityName(), "FADEW"); break;
    case OPCODE::SCRLP: WriteTodo(code_gen, md.GetEntityName(), "SCRLP"); break;
    case OPCODE::MVCAM: WriteTodo(code_gen, md.GetEntityName(), "MVCAM"); break;
    case OPCODE::SCRLA: WriteTodo(code_gen, md.GetEntityName(), "SCRLA"); break;
    default:
        code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
          md.GetEntityName(), _address, _opcode
        ));
    }
}

void FieldCameraInstruction::ProcessNFADE(CodeGenerator* code_gen){
    // TODO: Not fully reversed.
    auto raw_type = _params[4]->getUnsigned();
    if (raw_type == 0){
        code_gen->AddOutputLine("-- fade:clear()");
        return;
    }
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const std::string type = raw_type == 12 ? "Fade.SUBTRACT" : "Fade.ADD";
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[5]->getUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& unknown = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[8]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- fade:fade(%2%, %3%, %4%, %1%, %5%)") % type % r % g % b % unknown).str()
    );
}

void FieldCameraInstruction::ProcessSCR2D(CodeGenerator* code_gen){
    // kUpScaler.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getSigned()
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getSigned()
    );
    code_gen->AddOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.NONE, 0)")
      % x % y
    ).str());
}

void FieldCameraInstruction::ProcessSCR2DC(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[4]->getSigned()
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[5]->getSigned()
    );
    const auto& speed = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[6]->getUnsigned(),
      FieldCodeGenerator::ValueType::Float, 30.0f
    );
    code_gen->AddOutputLine((
      boost::format("background2d:scroll_to_position(%1% * 3, %2% * 3, Background2D.SMOOTH, %3%)")
      % x % y % speed
    ).str());
}

void FieldCameraInstruction::ProcessFADE(CodeGenerator* code_gen){
    // TODO: not fully reversed
    auto raw_type = _params[8]->getUnsigned();
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
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[4]->getUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[5]->getUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[6]->getUnsigned()
    );
    // TODO: needs to be divided by 30.0f?
    auto speed = _params[7]->getUnsigned();
    auto start = _params[9]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- fade:fade(%2%, %3%, %4%, %1%, %5%, %6%)") % type % r % g % b % speed % start
    ).str());
}
