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

#include "decompiler/field/instruction/FieldMediaInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldMediaInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::BGMOVIE: WriteTodo(code_gen, md.GetEntityName(), "BGMOVIE"); break;
        case OPCODE::AKAO2: ProcessAKAO2(code_gen); break;
        case OPCODE::MUSIC: ProcessMUSIC(code_gen); break;
        case OPCODE::SOUND: ProcessSOUND(code_gen); break;
        case OPCODE::AKAO: ProcessAKAO(code_gen); break;
        case OPCODE::MUSVT: WriteTodo(code_gen, md.GetEntityName(), "MUSVT"); break;
        case OPCODE::MUSVM: WriteTodo(code_gen, md.GetEntityName(), "MUSVM"); break;
        case OPCODE::MULCK: ProcessMULCK(code_gen); break;
        case OPCODE::BMUSC: WriteTodo(code_gen, md.GetEntityName(), "BMUSC"); break;
        case OPCODE::CHMPH: WriteTodo(code_gen, md.GetEntityName(), "CHMPH"); break;
        case OPCODE::PMVIE: ProcessPMVIE(code_gen); break;
        case OPCODE::MOVIE: ProcessMOVIE(code_gen); break;
        case OPCODE::MVIEF: ProcessMVIEF(code_gen); break;
        case OPCODE::FMUSC: WriteTodo(code_gen, md.GetEntityName(), "FMUSC"); break;
        case OPCODE::CMUSC: WriteTodo(code_gen, md.GetEntityName(), "CMUSC"); break;
        case OPCODE::CHMST: WriteTodo(code_gen, md.GetEntityName(), "CHMST"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FieldMediaInstruction::ProcessAKAO2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& param1 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& param2 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[8]->getUnsigned()
    );
    const auto& param3 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& param4 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& param5 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[5]->getUnsigned(), _params[11]->getUnsigned()
    );
    auto op = _params[6]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- music:execute_akao(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
      % param1 % param2 % param3 % param4 % param5 % op
    ).str());
}

void FieldMediaInstruction::ProcessMUSIC(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- music:execute_akao(0x10, pointer_to_field_AKAO_%1%)")
      % _params[0]->getUnsigned()
    ).str());
}

void FieldMediaInstruction::ProcessSOUND(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& soundId = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& panning = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- music:execute_akao(0x20, %1%, %2%)") % soundId % panning).str()
    );
}

void FieldMediaInstruction::ProcessAKAO(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& param1 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& param2 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[8]->getUnsigned()
    );
    const auto& param3 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& param4 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& param5 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[5]->getUnsigned(), _params[11]->getUnsigned()
    );
    auto op = _params[6]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- music:execute_akao(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
      % param1 % param2 % param3 % param4 % param5 % op
    ).str());
}

void FieldMediaInstruction::ProcessMULCK(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- music:lock(%1%)")
      % FieldCodeGenerator::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FieldMediaInstruction::ProcessPMVIE(CodeGenerator* code_gen){
    code_gen->AddOutputLine(
      (boost::format("-- field:movie_set(%1%)") % _params[0]->getUnsigned()
    ).str());
}

void FieldMediaInstruction::ProcessMOVIE(CodeGenerator* code_gen){
    code_gen->AddOutputLine("-- field:play_movie()");
}

void FieldMediaInstruction::ProcessMVIEF(CodeGenerator* code_gen){
    // TODO: Check for assignment to value.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[2]->getUnsigned());
    code_gen->AddOutputLine(
      (boost::format("-- %1% = field:get_movie_frame()") % destination).str()
    );
}
