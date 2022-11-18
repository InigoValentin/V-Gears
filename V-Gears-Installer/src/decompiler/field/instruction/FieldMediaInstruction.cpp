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
#include "decompiler/field/instruction/FieldMediaInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldMediaInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func.metadata);
    //std::cout << "Media opcode: " << opcode_ << "\n";
    switch (opcode_){
        case OPCODES::BGMOVIE: code_gen->WriteTodo(md.GetEntityName(), "BGMOVIE"); break;
        case OPCODES::AKAO2: ProcessAKAO(code_gen); break;
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
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

void FieldMediaInstruction::ProcessAKAO2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& param1 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[7]->GetUnsigned()
    );
    const auto& param2 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[8]->GetUnsigned()
    );
    const auto& param3 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[9]->GetUnsigned()
    );
    const auto& param4 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[10]->GetUnsigned()
    );
    const auto& param5 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[5]->GetUnsigned(), params_[11]->GetUnsigned()
    );
    auto op = params_[6]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- music:execute_akao(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
      % param1 % param2 % param3 % param4 % param5 % op
    ).str());
}

void FieldMediaInstruction::ProcessMUSIC(CodeGenerator* code_gen){
    code_gen->AddOutputLine(
      (boost::format("play_map_music(%1%)") % params_[0]->GetUnsigned()).str()
    );
}

void FieldMediaInstruction::ProcessSOUND(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& soundId = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& panning = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    code_gen->AddOutputLine((
      boost::format("audio_manager:play_sound(\"%1%\") -- Direction: %2%)") % soundId % panning
    ).str());
}

void FieldMediaInstruction::ProcessAKAO(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& param1 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[7]->GetUnsigned()
    );
    const auto& param2 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[8]->GetUnsigned()
    );
    const auto& param3 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[9]->GetUnsigned()
    );
    const auto& param4 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[10]->GetUnsigned()
    );
    const auto& param5 = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[5]->GetUnsigned(), params_[11]->GetUnsigned()
    );
    int op = static_cast<int>(params_[6]->GetUnsigned());
    switch (op){
        /*code_gen->AddOutputLine((
            boost::format("-- music:execute_akao(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
            % param1 % param2 % param3 % param4 % param5 % op
        ).str());*/
        case 0x10: // Play music.
        case 0x14: // Play music.
            code_gen->AddOutputLine(
              (boost::format("audio_manager:play_music(\"%1%\")") % param1).str()
            );
            break;
        case 0x18: // Resume music.
        case 0x19: // Resume music.
            code_gen->AddOutputLine(
              (boost::format("audio_manager:play_music(\"%1%\")") % param1).str()
            );
            break;
        case 0x20: // Play 1 sound, channel 1.
        case 0x24: // Play 1 sound, channel 1.
            code_gen->AddOutputLine((
              boost::format("audio_manager:play_sounds(\"%1%\", nil, nil, nil) -- Panning %2%")
              % param2 % param1
            ).str());
            break;
        case 0x21: // Play 2 sounds, channels 1 and 2.
        case 0x25: // Play 2 sounds, channels 1 and 2.
            code_gen->AddOutputLine((
              boost::format("audio_manager:play_sounds(\"%1%\", \"%2%\", nil, nil) -- Panning %3%")
              % param2 % param3 % param1
            ).str());
            break;
        case 0x22: // Play 3 sounds, channels 1, 2 and 3.
        case 0x26: // Play 3 sounds, channels 1, 2 and 3.
            code_gen->AddOutputLine((
              boost::format(
                "audio_manager:play_sounds(\"%1%\", \"%2%\", \"%3%\", nil) -- Panning %4%"
              ) % param2 % param3 % param4 % param1
            ).str());
            break;
        case 0x23: // Play 4 sounds, channels 1, 2, 3 and 4.
        case 0x27: // Play 4 sounds, channels 1, 2, 3 and 4.
            code_gen->AddOutputLine((
              boost::format(
                "audio_manager:play_sounds(\"%1%\", \"%2%\", \"%3%\", \"%4%\") -- Panning %5%"
              ) % param2 % param3 % param4 % param5 % param1
            ).str());
            break;
        case 0x28: // Play sound, channel 1.
            code_gen->AddOutputLine(
              (boost::format("audio_manager:play_sound(\"%1%\", 1)") % param1).str()
            );
            break;
        case 0x29: // Play sound, channel 2.
            code_gen->AddOutputLine(
              (boost::format("audio_manager:play_sound(\"%1%\", 2)") % param1).str()
            );
            break;
        case 0x2A: // Play sound, channel 3.
            code_gen->AddOutputLine(
              (boost::format("audio_manager:play_sound(\"%1%\", 3)") % param1).str()
            );
            break;
        case 0x2B: // Play sound, channel 4.
            code_gen->AddOutputLine(
              (boost::format("audio_manager:play_sound(\"%1%\", 4)") % param1).str()
            );
            break;
        case 0x30: // Play sound, channel 5.
            code_gen->AddOutputLine(
              (boost::format("audio_manager:play_sound(\"%1%\", 5)") % param1).str()
            );
            break;
        case 0x15: // Unused.
        default:
            code_gen->AddOutputLine((
                boost::format("-- Unknown akao operation AKAO(0x%6$02x, %1%, %2%, %3%, %4%, %5%)")
                % param1 % param2 % param3 % param4 % param5 % op
            ).str());
    }
}

void FieldMediaInstruction::ProcessMULCK(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("-- music:lock(%1%)")
      % FieldCodeGenerator::FormatBool(params_[0]->GetUnsigned())
    ).str());
}

void FieldMediaInstruction::ProcessPMVIE(CodeGenerator* code_gen){
    code_gen->AddOutputLine(
      (boost::format("-- field:movie_set(%1%)") % params_[0]->GetUnsigned()
    ).str());
}

void FieldMediaInstruction::ProcessMOVIE(CodeGenerator* code_gen){
    code_gen->AddOutputLine("-- field:play_movie()");
}

void FieldMediaInstruction::ProcessMVIEF(CodeGenerator* code_gen){
    // TODO: Check for assignment to value.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[2]->GetUnsigned());
    code_gen->AddOutputLine(
      (boost::format("-- %1% = field:get_movie_frame()") % destination).str()
    );
}
