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

#include "decompiler/field/instruction/FieldBackgroundInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldBackgroundInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::BGPDH: WriteTodo(code_gen, md.GetEntityName(), "BGPDH");break;
        case OPCODE::BGSCR: WriteTodo(code_gen, md.GetEntityName(), "BGSCR"); break;
        case OPCODE::MPPAL: WriteTodo(code_gen, md.GetEntityName(), "MPPAL"); break;
        case OPCODE::BGON: ProcessBGON(code_gen); break;
        case OPCODE::BGOFF: ProcessBGOFF(code_gen); break;
        case OPCODE::BGROL: WriteTodo(code_gen, md.GetEntityName(), "BGROL"); break;
        case OPCODE::BGROL2: WriteTodo(code_gen, md.GetEntityName(), "BGROL2"); break;
        case OPCODE::BGCLR: ProcessBGCLR(code_gen); break;
        case OPCODE::STPAL: ProcessSTPAL(code_gen); break;
        case OPCODE::LDPAL: ProcessLDPAL(code_gen); break;
        case OPCODE::CPPAL: ProcessCPPAL(code_gen); break;
        case OPCODE::RTPAL: WriteTodo(code_gen, md.GetEntityName(), "RTPAL"); break;
        case OPCODE::ADPAL: ProcessADPAL(code_gen); break;
        case OPCODE::MPPAL2: ProcessMPPAL2(code_gen); break;
        case OPCODE::STPLS: ProcessSTPLS(code_gen); break;
        case OPCODE::LDPLS: ProcessLDPLS(code_gen); break;
        case OPCODE::CPPAL2: WriteTodo(code_gen, md.GetEntityName(), "CPPAL2"); break;
        case OPCODE::ADPAL2: WriteTodo(code_gen, md.GetEntityName(), "ADPAL2"); break;
        case OPCODE::RTPAL2: WriteTodo(code_gen, md.GetEntityName(), "RTPAL2"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FieldBackgroundInstruction::ProcessBGON(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& layer_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_on(%1%, %2%)") % background_id % layer_id).str()
    );
}

void FieldBackgroundInstruction::ProcessBGOFF(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& layer_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_off(%1%, %2%)") % background_id % layer_id).str()
    );
}

void FieldBackgroundInstruction::ProcessBGCLR(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& background_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[2]->getUnsigned()
    );
    code_gen->AddOutputLine(
      (boost::format("-- field:background_clear(%1%)") % background_id).str()
    );
}

void FieldBackgroundInstruction::ProcessSTPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- store palette %1% to position %2%, start CLUT index 0, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FieldBackgroundInstruction::ProcessLDPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- load palette %2% from position %1%, start CLUT index 0, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FieldBackgroundInstruction::ProcessCPPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    auto num_entries = _params[4]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- copy palette %1% to palette %2%, %3% entries")
      % source % destination % num_entries
    ).str());
}

void FieldBackgroundInstruction::ProcessADPAL(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[4]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
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

void FieldBackgroundInstruction::ProcessMPPAL2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[6]->getUnsigned()
    );
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[7]->getUnsigned()
    );
    const auto& r = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[4]->getUnsigned(), _params[10]->getUnsigned()
    );
    const auto& g = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[9]->getUnsigned()
    );
    const auto& b = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[8]->getUnsigned()
    );
    auto num_entries = _params[11]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format(
        "-- multiply RGB(%3%, %4%, %5%) by %6% entries of palette stored at position %1%, "
        "storing result in position %2%"
      ) % source % destination % r % g % b % num_entries).str());
}

void FieldBackgroundInstruction::ProcessSTPLS(CodeGenerator* code_gen){
    auto source = _params[0]->getUnsigned();
    auto destination = _params[1]->getUnsigned();
    auto start_clut = _params[2]->getUnsigned();
    auto num_entries = _params[3]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- store palette %1% to position %2%, start CLUT index %3%, %4% entries")
      % source % destination % start_clut % num_entries
    ).str());
}

void FieldBackgroundInstruction::ProcessLDPLS(CodeGenerator* code_gen){
    auto source = _params[0]->getUnsigned();
    auto destination = _params[1]->getUnsigned();
    auto startClut = _params[2]->getUnsigned();
    auto num_entries = _params[3]->getUnsigned() + 1;
    code_gen->AddOutputLine((
      boost::format("-- load palette %2% from position %1%, start CLUT index %3%, %4% entries")
      % source % destination % startClut % num_entries
    ).str());
}
