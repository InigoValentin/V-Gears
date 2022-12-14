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
#include "decompiler/field/instruction/FieldMathInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldMathInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    FunctionMetaData md(func.metadata);
    //std::cout << "Math opcode: " << opcode_ << "\n";
    switch (opcode_){
        case OPCODES::PLUS_: ProcessSaturatedPLUS(code_gen); break;
        case OPCODES::PLUS2_: ProcessSaturatedPLUS2(code_gen); break;
        case OPCODES::MINUS_: ProcessSaturatedMINUS(code_gen); break;
        case OPCODES::MINUS2_: ProcessSaturatedMINUS2(code_gen); break;
        case OPCODES::INC_: ProcessSaturatedINC(code_gen); break;
        case OPCODES::INC2_: ProcessSaturatedINC2(code_gen); break;
        case OPCODES::DEC_: ProcessSaturatedDEC(code_gen); break;
        case OPCODES::DEC2_: ProcessSaturatedDEC2(code_gen); break;
        case OPCODES::RDMSD: ProcessRDMSD(code_gen); break;
        case OPCODES::SETBYTE:
        case OPCODES::SETWORD:
            ProcessSETBYTE_SETWORD(code_gen);
            break;
        case OPCODES::BITON: ProcessBITON(code_gen); break;
        case OPCODES::BITOFF: ProcessBITOFF(code_gen); break;
        case OPCODES::BITXOR: code_gen->WriteTodo(md.GetEntityName(), "BITXOR"); break;
        case OPCODES::PLUS:
        case OPCODES::PLUS2:
            ProcessPLUSx_MINUSx(code_gen, "+");
            break;
        case OPCODES::MINUS:
        case OPCODES::MINUS2:
            ProcessPLUSx_MINUSx(code_gen, "-");
            break;
        case OPCODES::MUL: code_gen->WriteTodo(md.GetEntityName(), "MUL"); break;
        case OPCODES::MUL2: code_gen->WriteTodo(md.GetEntityName(), "MUL2"); break;
        case OPCODES::DIV: code_gen->WriteTodo(md.GetEntityName(), "DIV"); break;
        case OPCODES::DIV2: code_gen->WriteTodo(md.GetEntityName(), "DIV2"); break;
        case OPCODES::MOD:
            {
                const uint32 source_bank = params_[0]->GetUnsigned();
                const uint32 sourceaddress__or_value = params_[2]->GetUnsigned();
                auto source = FieldCodeGenerator::FormatValueOrVariable(
                  cg->GetFormatter(), source_bank, sourceaddress__or_value
                );
                const uint32 dest_bank = params_[1]->GetUnsigned();
                const uint32 destaddress_ = params_[3]->GetUnsigned();
                auto dest = FieldCodeGenerator::FormatValueOrVariable(
                  cg->GetFormatter(), dest_bank, destaddress_
                );
                code_gen->AddOutputLine(
                  source + " = " + source + " % " + dest + code_gen->GetLanguage().LineTerminator()
                );
            }
        break;
        case OPCODES::MOD2: code_gen->WriteTodo(md.GetEntityName(), "MOD2"); break;
        case OPCODES::AND: code_gen->WriteTodo(md.GetEntityName(), "AND"); break;
        case OPCODES::AND2: code_gen->WriteTodo(md.GetEntityName(), "AND2"); break;
        case OPCODES::OR: code_gen->WriteTodo(md.GetEntityName(), "OR"); break;
        case OPCODES::OR2: code_gen->WriteTodo(md.GetEntityName(), "OR2"); break;
        case OPCODES::XOR: code_gen->WriteTodo(md.GetEntityName(), "XOR"); break;
        case OPCODES::XOR2: code_gen->WriteTodo(md.GetEntityName(), "XOR2"); break;
        case OPCODES::INC:
        case OPCODES::INC2:
            ProcessINCx_DECx(code_gen, "+");
            break;
        case OPCODES::DEC:
        case OPCODES::DEC2:
            ProcessINCx_DECx(code_gen, "-");
            break;
        case OPCODES::RANDOM: ProcessRANDOM(code_gen); break;
        case OPCODES::LBYTE: code_gen->WriteTodo(md.GetEntityName(), "LBYTE"); break;
        case OPCODES::HBYTE: code_gen->WriteTodo(md.GetEntityName(), "HBYTE"); break;
        case OPCODES::TWOBYTE: code_gen->WriteTodo(md.GetEntityName(), "2BYTE"); break;
        case OPCODES::SIN: code_gen->WriteTodo(md.GetEntityName(), "SIN"); break;
        case OPCODES::COS: code_gen->WriteTodo(md.GetEntityName(), "COS"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

void FieldMathInstruction::ProcessSaturatedPLUS(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 255); %1% = 255; end") % lhs).str());
}

void FieldMathInstruction::ProcessSaturatedPLUS2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 32767); %1% = 32767; end") % lhs).str());
}

void FieldMathInstruction::ProcessSaturatedMINUS(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % lhs).str());
}

void FieldMathInstruction::ProcessSaturatedMINUS2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % lhs).str());
}

void FieldMathInstruction::ProcessSaturatedINC(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[1]->GetUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 255); %1% = 255; end") % dest).str());
}

void FieldMathInstruction::ProcessSaturatedINC2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[1]->GetUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 32767); %1% = 32767; end") % dest).str());
}

void FieldMathInstruction::ProcessSaturatedDEC(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[1]->GetUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % dest).str());
}

void FieldMathInstruction::ProcessSaturatedDEC2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[1]->GetUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % dest).str());
}

void FieldMathInstruction::ProcessRDMSD(CodeGenerator* code_gen){
    // TODO: we don't have os.time...
    // TODO: RNG emulation?
    code_gen->AddOutputLine("math.randomseed(os.time())");
}

void FieldMathInstruction::ProcessSETBYTE_SETWORD(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    // TODO: respect destination bank sizes (16-bit writes only affect low byte)
    code_gen->AddOutputLine((boost::format("%1% = %2%") % destination % source).str());
}

void FieldMathInstruction::ProcessBITON(CodeGenerator* code_gen){
    code_gen->AddOutputLine((boost::format("set_bank_bit(%1%, %2%, %3%, 1)")
      % params_[0]->GetUnsigned() % params_[2]->GetUnsigned() % params_[3]->GetUnsigned()
    ).str());
}

void FieldMathInstruction::ProcessBITOFF(CodeGenerator* code_gen){
    code_gen->AddOutputLine((boost::format("set_bank_bit(%1%, %2%, %3%, 0)")
      % params_[0]->GetUnsigned() % params_[2]->GetUnsigned() % params_[3]->GetUnsigned()
    ).str());
}

void FieldMathInstruction::ProcessPLUSx_MINUSx(CodeGenerator* code_gen, const std::string& op){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    // TODO: repect destination bank sizes and wraparound
    code_gen->AddOutputLine((boost::format("%1% = %1% %2% %3%") % lhs % op % rhs).str());
}

void FieldMathInstruction::ProcessINCx_DECx(CodeGenerator* code_gen, const std::string& op){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[1]->GetUnsigned()
    );
    // TODO: repect destination bank sizes and wraparound
    code_gen->AddOutputLine((boost::format("%1% = %1% %2% 1") % destination % op).str());
}

void FieldMathInstruction::ProcessRANDOM(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[1]->GetUnsigned()
    );
    // TODO: Respect destination bank sizes (16-bit writes only affect low byte).
    // TODO: RNG emulation?
    code_gen->AddOutputLine((boost::format("%1% = math.random(0, 255)") % destination).str());
}
