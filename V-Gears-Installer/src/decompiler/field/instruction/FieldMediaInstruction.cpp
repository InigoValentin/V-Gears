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
#include "decompiler/field/instruction/FieldMediaInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FF7::FieldMediaInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func.metadata);
    switch (opcode_){
        case OPCODES::BGMOVIE: code_gen->WriteTodo(md.GetEntityName(), "BGMOVIE"); break;
        case OPCODES::AKAO2: ProcessAKAO2(code_gen); break;
        case OPCODES::MUSIC: ProcessMUSIC(code_gen); break;
        case OPCODES::SOUND: ProcessSOUND(code_gen); break;
        case OPCODES::AKAO: ProcessAKAO(code_gen); break;
        case OPCODES::MUSVT: code_gen->WriteTodo(md.GetEntityName(), "MUSVT"); break;
        case OPCODES::MUSVM: code_gen->WriteTodo(md.GetEntityName(), "MUSVM"); break;
        case OPCODES::MULCK: ProcessMULCK(code_gen); break;
        case OPCODES::BMUSC: code_gen->WriteTodo(md.GetEntityName(), "BMUSC"); break;
        case OPCODES::CHMPH: code_gen->WriteTodo(md.GetEntityName(), "CHMPH"); break;
        case OPCODES::PMVIE: ProcessPMVIE(code_gen); break;
        case OPCODES::MOVIE: ProcessMOVIE(code_gen); break;
        case OPCODES::MVIEF: ProcessMVIEF(code_gen); break;
        case OPCODES::FMUSC: code_gen->WriteTodo(md.GetEntityName(), "FMUSC"); break;
        case OPCODES::CMUSC: code_gen->WriteTodo(md.GetEntityName(), "CMUSC"); break;
        case OPCODES::CHMST: code_gen->WriteTodo(md.GetEntityName(), "CHMST"); break;
        default:
            code_gen->AddOutputLine(FF7::FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

void FF7::FieldMediaInstruction::ProcessAKAO2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& param1 = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[7]->GetUnsigned()
    );
    const auto& param2 = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[8]->GetUnsigned()
    );
    const auto& param3 = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[9]->GetUnsigned()
    );
    const auto& param4 = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[10]->GetUnsigned()
    );
    const auto& param5 = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[5]->GetUnsigned(), params_[11]->GetUnsigned()
    );
    auto op = params_[6]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- music:execute_akao(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
      % param1 % param2 % param3 % param4 % param5 % op
    ).str());
}

void FF7::FieldMediaInstruction::ProcessMUSIC(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- music:execute_akao(0x10, pointer_to_field_AKAO_%1%)")
      % params_[0]->GetUnsigned()
    ).str());
}

void FF7::FieldMediaInstruction::ProcessSOUND(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& soundId = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& panning = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- music:execute_akao(0x20, %1%, %2%)") % soundId % panning).str()
    );
}

void FF7::FieldMediaInstruction::ProcessAKAO(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& param1 = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[7]->GetUnsigned()
    );
    const auto& param2 = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[8]->GetUnsigned()
    );
    const auto& param3 = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[9]->GetUnsigned()
    );
    const auto& param4 = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[10]->GetUnsigned()
    );
    const auto& param5 = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[5]->GetUnsigned(), params_[11]->GetUnsigned()
    );
    auto op = params_[6]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- music:execute_akao(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
      % param1 % param2 % param3 % param4 % param5 % op
    ).str());
}

void FF7::FieldMediaInstruction::ProcessMULCK(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- music:lock(%1%)")
      % FF7::FieldCodeGenerator::FormatBool(params_[0]->GetUnsigned())
    ).str());
}

void FF7::FieldMediaInstruction::ProcessPMVIE(CodeGenerator* code_gen){
    code_gen->AddOutputLine(
      (boost::format("-- field:movie_set(%1%)") % params_[0]->GetUnsigned()
    ).str());
}

void FF7::FieldMediaInstruction::ProcessMOVIE(CodeGenerator* code_gen){
    code_gen->AddOutputLine("-- field:play_movie()");
}

void FF7::FieldMediaInstruction::ProcessMVIEF(CodeGenerator* code_gen){
    // TODO: Check for assignment to value.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& destination = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[2]->GetUnsigned());
    code_gen->AddOutputLine(
      (boost::format("-- %1% = field:get_movie_frame()") % destination).str()
    );
}
