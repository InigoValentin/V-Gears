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

#include "decompiler/field/instruction/FieldMathInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldMathInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::PLUS_: ProcessSaturatedPLUS(code_gen); break;
        case OPCODE::PLUS2_: ProcessSaturatedPLUS2(code_gen); break;
        case OPCODE::MINUS_: ProcessSaturatedMINUS(code_gen); break;
        case OPCODE::MINUS2_: ProcessSaturatedMINUS2(code_gen); break;
        case OPCODE::INC_: ProcessSaturatedINC(code_gen); break;
        case OPCODE::INC2_: ProcessSaturatedINC2(code_gen); break;
        case OPCODE::DEC_: ProcessSaturatedDEC(code_gen); break;
        case OPCODE::DEC2_: ProcessSaturatedDEC2(code_gen); break;
        case OPCODE::RDMSD: ProcessRDMSD(code_gen); break;
        case OPCODE::SETBYTE:
        case OPCODE::SETWORD:
            ProcessSETBYTE_SETWORD(code_gen);
            break;
        case OPCODE::BITON: ProcessBITON(code_gen); break;
        case OPCODE::BITOFF: ProcessBITOFF(code_gen); break;
        case OPCODE::BITXOR: WriteTodo(code_gen, md.GetEntityName(), "BITXOR"); break;
        case OPCODE::PLUS:
        case OPCODE::PLUS2:
            ProcessPLUSx_MINUSx(code_gen, "+");
            break;
        case OPCODE::MINUS:
        case OPCODE::MINUS2:
            ProcessPLUSx_MINUSx(code_gen, "-");
            break;
        case OPCODE::MUL: WriteTodo(code_gen, md.GetEntityName(), "MUL"); break;
        case OPCODE::MUL2: WriteTodo(code_gen, md.GetEntityName(), "MUL2"); break;
        case OPCODE::DIV: WriteTodo(code_gen, md.GetEntityName(), "DIV"); break;
        case OPCODE::DIV2: WriteTodo(code_gen, md.GetEntityName(), "DIV2"); break;
        case OPCODE::MOD:
            {
                const uint32 source_bank = _params[0]->getUnsigned();
                const uint32 source_address_or_value = _params[2]->getUnsigned();
                auto source = FieldCodeGenerator::FormatValueOrVariable(
                  cg->GetFormatter(), source_bank, source_address_or_value
                );
                const uint32 dest_bank = _params[1]->getUnsigned();
                const uint32 dest_address = _params[3]->getUnsigned();
                auto dest = FieldCodeGenerator::FormatValueOrVariable(
                  cg->GetFormatter(), dest_bank, dest_address
                );
                code_gen->AddOutputLine(
                  source + " = " + source + " % " + dest + code_gen->TargetLang().LineTerminator()
                );
            }
        break;
        case OPCODE::MOD2: WriteTodo(code_gen, md.GetEntityName(), "MOD2"); break;
        case OPCODE::AND: WriteTodo(code_gen, md.GetEntityName(), "AND"); break;
        case OPCODE::AND2: WriteTodo(code_gen, md.GetEntityName(), "AND2"); break;
        case OPCODE::OR: WriteTodo(code_gen, md.GetEntityName(), "OR"); break;
        case OPCODE::OR2: WriteTodo(code_gen, md.GetEntityName(), "OR2"); break;
        case OPCODE::XOR: WriteTodo(code_gen, md.GetEntityName(), "XOR"); break;
        case OPCODE::XOR2: WriteTodo(code_gen, md.GetEntityName(), "XOR2"); break;
        case OPCODE::INC:
        case OPCODE::INC2:
            ProcessINCx_DECx(code_gen, "+");
            break;
        case OPCODE::DEC:
        case OPCODE::DEC2:
            ProcessINCx_DECx(code_gen, "-");
            break;
        case OPCODE::RANDOM: ProcessRANDOM(code_gen); break;
        case OPCODE::LBYTE: WriteTodo(code_gen, md.GetEntityName(), "LBYTE"); break;
        case OPCODE::HBYTE: WriteTodo(code_gen, md.GetEntityName(), "HBYTE"); break;
        case OPCODE::TWOBYTE: WriteTodo(code_gen, md.GetEntityName(), "2BYTE"); break;
        case OPCODE::SIN: WriteTodo(code_gen, md.GetEntityName(), "SIN"); break;
        case OPCODE::COS: WriteTodo(code_gen, md.GetEntityName(), "COS"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FieldMathInstruction::ProcessSaturatedPLUS(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 255); %1% = 255; end") % lhs).str());
}

void FieldMathInstruction::ProcessSaturatedPLUS2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 32767); %1% = 32767; end") % lhs).str());
}

void FieldMathInstruction::ProcessSaturatedMINUS(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % lhs).str());
}

void FieldMathInstruction::ProcessSaturatedMINUS2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - %2%") % lhs % rhs).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % lhs).str());
}

void FieldMathInstruction::ProcessSaturatedINC(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 255); %1% = 255; end") % dest).str());
}

void FieldMathInstruction::ProcessSaturatedINC2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and negative wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% + 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% > 32767); %1% = 32767; end") % dest).str());
}

void FieldMathInstruction::ProcessSaturatedDEC(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes and positive wrap-around.
    code_gen->AddOutputLine((boost::format("%1% = %1% - 1") % dest).str());
    code_gen->AddOutputLine((boost::format("--if (%1% < 0); %1% = 0; end") % dest).str());
}

void FieldMathInstruction::ProcessSaturatedDEC2(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& dest = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
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
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& source = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: respect destination bank sizes (16-bit writes only affect low byte)
    code_gen->AddOutputLine((boost::format("%1% = %2%") % destination % source).str());
}

void FieldMathInstruction::ProcessBITON(CodeGenerator* code_gen){
    code_gen->AddOutputLine((boost::format("bit_on(%1%, %2%, %3%)")
      % _params[0]->getUnsigned() % _params[2]->getUnsigned() % _params[3]->getUnsigned()
    ).str());
}

void FieldMathInstruction::ProcessBITOFF(CodeGenerator* code_gen){
    code_gen->AddOutputLine((boost::format("bit_off(%1%, %2%, %3%)")
      % _params[0]->getUnsigned() % _params[2]->getUnsigned() % _params[3]->getUnsigned()
    ).str());
}

void FieldMathInstruction::ProcessPLUSx_MINUSx(CodeGenerator* code_gen, const std::string& op){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& lhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getUnsigned()
    );
    const auto& rhs = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    // TODO: repect destination bank sizes and wraparound
    code_gen->AddOutputLine((boost::format("%1% = %1% %2% %3%") % lhs % op % rhs).str());
}

void FieldMathInstruction::ProcessINCx_DECx(CodeGenerator* code_gen, const std::string& op){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: repect destination bank sizes and wraparound
    code_gen->AddOutputLine((boost::format("%1% = %1% %2% 1") % destination % op).str());
}

void FieldMathInstruction::ProcessRANDOM(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: Check for assignment to value.
    const auto& destination = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned()
    );
    // TODO: Respect destination bank sizes (16-bit writes only affect low byte).
    // TODO: RNG emulation?
    code_gen->AddOutputLine((boost::format("%1% = math.random(0, 255)") % destination).str());
}
