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
#include "decompiler/field/instruction/FieldBackgroundInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldBackgroundInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func.metadata);
    //std::cout << "Background opcode: " << opcode_ << "\n";
    switch (opcode_){
        case OPCODES::BGPDH: code_gen->WriteTodo(md.GetEntityName(), "BGPDH");break;
        case OPCODES::BGSCR: code_gen->WriteTodo(md.GetEntityName(), "BGSCR"); break;
        case OPCODES::MPPAL: code_gen->WriteTodo(md.GetEntityName(), "MPPAL"); break;
        case OPCODES::BGON: ProcessBGON(code_gen); break;
        case OPCODES::BGOFF: ProcessBGOFF(code_gen); break;
        case OPCODES::BGROL: code_gen->WriteTodo(md.GetEntityName(), "BGROL"); break;
        case OPCODES::BGROL2: code_gen->WriteTodo(md.GetEntityName(), "BGROL2"); break;
        case OPCODES::BGCLR: ProcessBGCLR(code_gen); break;
        case OPCODES::STPAL: ProcessSTPAL(code_gen); break;
        case OPCODES::LDPAL: ProcessLDPAL(code_gen); break;
        case OPCODES::CPPAL: ProcessCPPAL(code_gen); break;
        case OPCODES::RTPAL: code_gen->WriteTodo(md.GetEntityName(), "RTPAL"); break;
        case OPCODES::ADPAL: ProcessADPAL(code_gen); break;
        case OPCODES::MPPAL2: ProcessMPPAL2(code_gen); break;
        case OPCODES::STPLS: ProcessSTPLS(code_gen); break;
        case OPCODES::LDPLS: ProcessLDPLS(code_gen); break;
        case OPCODES::CPPAL2: code_gen->WriteTodo(md.GetEntityName(), "CPPAL2"); break;
        case OPCODES::ADPAL2: code_gen->WriteTodo(md.GetEntityName(), "ADPAL2"); break;
        case OPCODES::RTPAL2: code_gen->WriteTodo(md.GetEntityName(), "RTPAL2"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

void FieldBackgroundInstruction::ProcessBGON(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& layer_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_on(%1%, %2%)") % background_id % layer_id).str()
    );
}

void FieldBackgroundInstruction::ProcessBGOFF(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& layer_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_off(%1%, %2%)") % background_id % layer_id).str()
    );
}

void FieldBackgroundInstruction::ProcessBGCLR(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_clear(%1%)") % background_id).str()
    );
}

void FieldBackgroundInstruction::ProcessSTPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    auto num_entries = params_[4]->GetUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- store palette %1% to position %2%, start CLUT index 0, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FieldBackgroundInstruction::ProcessLDPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    auto num_entries = params_[4]->GetUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- load palette %2% from position %1%, start CLUT index 0, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FieldBackgroundInstruction::ProcessCPPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    auto num_entries = params_[4]->GetUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- copy palette %1% to palette %2%, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FieldBackgroundInstruction::ProcessADPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[6]->GetUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[7]->GetUnsigned()
    );
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[4]->GetUnsigned(), params_[10]->GetUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[9]->GetUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[8]->GetUnsigned()
    );
    auto num_entries = params_[11]->GetUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format(
        "-- add RGB(%3%, %4%, %5%) to %6% entries of palette stored at position %1%, "
        "storing result in position %2%"
      ) % source % destination % r % g % b % num_entries
    ).str());
}

void FieldBackgroundInstruction::ProcessMPPAL2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[6]->GetUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[7]->GetUnsigned()
    );
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[4]->GetUnsigned(), params_[10]->GetUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[9]->GetUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[8]->GetUnsigned()
    );
    auto num_entries = params_[11]->GetUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format(
        "-- multiply RGB(%3%, %4%, %5%) by %6% entries of palette stored at position %1%, "
        "storing result in position %2%"
      ) % source % destination % r % g % b % num_entries).str());
}

void FieldBackgroundInstruction::ProcessSTPLS(CodeGenerator* code_gen){
    auto source = params_[0]->GetUnsigned();
    auto destination = params_[1]->GetUnsigned();
    auto start_clut = params_[2]->GetUnsigned();
    auto num_entries = params_[3]->GetUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- store palette %1% to position %2%, start CLUT index %3%, %4% entries")
      % source % destination % start_clut % num_entries
    ).str());
}

void FieldBackgroundInstruction::ProcessLDPLS(CodeGenerator* code_gen){
    auto source = params_[0]->GetUnsigned();
    auto destination = params_[1]->GetUnsigned();
    auto startClut = params_[2]->GetUnsigned();
    auto num_entries = params_[3]->GetUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- load palette %2% from position %1%, start CLUT index %3%, %4% entries")
      % source % destination % startClut % num_entries
    ).str());
}
