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
#include "decompiler/field/instruction/FieldBackgroundInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FF7::FieldBackgroundInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
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
            code_gen->AddOutputLine(FF7::FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FF7::FieldBackgroundInstruction::ProcessBGON(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& layer_id = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_on(%1%, %2%)") % background_id % layer_id).str()
    );
}

void FF7::FieldBackgroundInstruction::ProcessBGOFF(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& layer_id = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_off(%1%, %2%)") % background_id % layer_id).str()
    );
}

void FF7::FieldBackgroundInstruction::ProcessBGCLR(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[2]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_clear(%1%)") % background_id).str()
    );
}

void FF7::FieldBackgroundInstruction::ProcessSTPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- store palette %1% to position %2%, start CLUT index 0, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FF7::FieldBackgroundInstruction::ProcessLDPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- load palette %2% from position %1%, start CLUT index 0, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FF7::FieldBackgroundInstruction::ProcessCPPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- copy palette %1% to palette %2%, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FF7::FieldBackgroundInstruction::ProcessADPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& destination = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& r = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[4]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& g = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& b = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[8]->getUnsigned()
    );
    auto num_entries = _params[11]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format(
        "-- add RGB(%3%, %4%, %5%) to %6% entries of palette stored at position %1%, "
        "storing result in position %2%"
      ) % source % destination % r % g % b % num_entries
    ).str());
}

void FF7::FieldBackgroundInstruction::ProcessMPPAL2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& destination = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& r = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[4]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& g = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& b = FF7::FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[8]->getUnsigned()
    );
    auto num_entries = _params[11]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format(
        "-- multiply RGB(%3%, %4%, %5%) by %6% entries of palette stored at position %1%, "
        "storing result in position %2%"
      ) % source % destination % r % g % b % num_entries).str());
}

void FF7::FieldBackgroundInstruction::ProcessSTPLS(CodeGenerator* code_gen){
    auto source = _params[0]->getUnsigned();
    auto destination = _params[1]->getUnsigned();
    auto start_clut = _params[2]->getUnsigned();
    auto num_entries = _params[3]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- store palette %1% to position %2%, start CLUT index %3%, %4% entries")
      % source % destination % start_clut % num_entries
    ).str());
}

void FF7::FieldBackgroundInstruction::ProcessLDPLS(CodeGenerator* code_gen){
    auto source = _params[0]->getUnsigned();
    auto destination = _params[1]->getUnsigned();
    auto startClut = _params[2]->getUnsigned();
    auto num_entries = _params[3]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- load palette %2% from position %1%, start CLUT index %3%, %4% entries")
      % source % destination % startClut % num_entries
    ).str());
}
