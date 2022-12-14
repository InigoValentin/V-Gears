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
#include "decompiler/field/instruction/FieldControlFlowInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

InstPtr FieldControlFlowInstruction::Create(){return new FieldControlFlowInstruction();}

void FieldControlFlowInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FieldEngine& eng = static_cast<FieldEngine&>(*engine);
    FunctionMetaData md(func.metadata);
    switch (opcode_){
        case OPCODES::RET:
            // A few notes in RET.
            // - Lua requires all functions to end with a return.
            // - Lua doesn's like returns if not followed by an end.
            // - Don't add a return statement to on_start functions.
            //     Game 'init' and 'main' code is concatenated in V-Gears. If the
            //     'main' return is kept, the code of 'main' is never executed.
            //     There are safeguards forfunctions without a return in the end,
            //     so it's OK not to include it here.
            if (func.name != "on_start") code_gen->AddOutputLine("do return 0 end");
            break;
        case OPCODES::REQ: ProcessREQ(code_gen, eng); break;
        case OPCODES::REQSW: ProcessREQSW(code_gen, eng); break;
        case OPCODES::REQEW: ProcessREQEW(code_gen, eng); break;
        case OPCODES::PREQ: code_gen->WriteTodo(md.GetEntityName(), "PREQ"); break;
        case OPCODES::PRQSW: code_gen->WriteTodo(md.GetEntityName(), "PRQSW"); break;
        case OPCODES::PRQEW: code_gen->WriteTodo(md.GetEntityName(), "PRQEW"); break;
        case OPCODES::RETTO: ProcessRETTO(code_gen); break;
        case OPCODES::WAIT: ProcessWAIT(code_gen); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

void FieldControlFlowInstruction::ProcessREQ(
  CodeGenerator* code_gen, const FieldEngine& engine
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& entity = engine.EntityByIndex(params_[0]->GetSigned());
    const auto& script_name = cg->GetFormatter().GetFriendlyFunctionName(
      entity.GetName(), entity.FunctionByIndex(params_[2]->GetUnsigned())
    );
    auto priority = params_[1]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("script:request(Script.ENTITY, \"%1%\", \"%2%\", %3%)")
      % entity.GetName() % script_name % priority
    ).str());
}

void FieldControlFlowInstruction::ProcessREQSW(
  CodeGenerator* code_gen, const FieldEngine& engine
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& entity = engine.EntityByIndex(params_[0]->GetSigned());
    const auto& script_name = cg->GetFormatter().GetFriendlyFunctionName(
      entity.GetName(), entity.FunctionByIndex(params_[2]->GetUnsigned())
    );
    auto priority = params_[1]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("script:request_start_sync(Script.ENTITY, \"%1%\", \"%2%\", %3%)")
      % entity.GetName() % script_name % priority
    ).str());
}

void FieldControlFlowInstruction::ProcessREQEW(
  CodeGenerator* code_gen, const FieldEngine& engine
){
    try{
        FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
        const auto& entity = engine.EntityByIndex(params_[0]->GetSigned());
        const auto& script_name = cg->GetFormatter().GetFriendlyFunctionName(
          entity.GetName(), entity.FunctionByIndex(params_[2]->GetUnsigned())
        );
        auto priority = params_[1]->GetUnsigned();
        code_gen->AddOutputLine((
          boost::format("script:request_end_sync(Script.ENTITY, \"%1%\", \"%2%\", %3%)")
          % entity.GetName() % script_name % priority
        ).str());
    }
    catch (const DecompilerException&){
        code_gen->AddOutputLine((
          boost::format("-- ERROR call to non existing function index %1%")
          % params_[2]->GetUnsigned()
        ).str());
    }
}

void FieldControlFlowInstruction::ProcessRETTO(CodeGenerator* code_gen){
    auto entity_index = params_[0]->GetUnsigned();
    auto priority = params_[1]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("-- return_to(script_id_in_current_entity=%2%, priority=%1%)")
      % entity_index % priority
    ).str());
}

void FieldControlFlowInstruction::ProcessWAIT(CodeGenerator* code_gen){
    code_gen->AddOutputLine((
      boost::format("script:wait(%1%)") % (params_[0]->GetUnsigned() / 30.0f)
    ).str());
}
