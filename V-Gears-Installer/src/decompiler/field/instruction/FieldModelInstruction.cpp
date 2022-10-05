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
#include "decompiler/field/instruction/FieldModelInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldModelInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FieldEngine& eng = static_cast<FieldEngine&>(*engine);
    FunctionMetaData md(func.metadata);
    switch (opcode_){
        case OPCODES::JOIN: ProcessJOIN(code_gen); break;
        case OPCODES::SPLIT: ProcessSPLIT(code_gen); break;
        case OPCODES::BLINK: code_gen->WriteTodo(md.GetEntityName(), "BLINK"); break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::EYETX:
            code_gen->WriteTodo(md.GetEntityName(), "EYETX");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::TRNSP:
            code_gen->WriteTodo(md.GetEntityName(), "TRNSP");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::AMBNT:
            code_gen->WriteTodo(md.GetEntityName(), "AMBNT");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::Unknown03:
            code_gen->WriteTodo(md.GetEntityName(), "Unknown03");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::Unknown04:
            code_gen->WriteTodo(md.GetEntityName(), "Unknown04");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::Unknown05:
            code_gen->WriteTodo(md.GetEntityName(), "Unknown05");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::LIGHT:
            code_gen->WriteTodo(md.GetEntityName(), "LIGHT");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::Unknown07:
            code_gen->WriteTodo(md.GetEntityName(), "Unknown07");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::Unknown08:
            code_gen->WriteTodo(md.GetEntityName(), "Unknown08");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::Unknown09:
            code_gen->WriteTodo(md.GetEntityName(), "Unknown09");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::SBOBJ:
            code_gen->WriteTodo(md.GetEntityName(), "SBOBJ");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::Unknown0B:
            code_gen->WriteTodo(md.GetEntityName(), "Unknown0B");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::Unknown0C:
            code_gen->WriteTodo(md.GetEntityName(), "Unknown0C");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::SHINE:
            code_gen->WriteTodo(md.GetEntityName(), "SHINE");
            break;
        case (OPCODES::KAWAI << 8) | OPCODES_KAWAI::RESET:
            code_gen->WriteTodo(md.GetEntityName(), "RESET");
            break;
        case OPCODES::KAWIW: code_gen->WriteTodo(md.GetEntityName(), "KAWIW"); break;
        case OPCODES::PMOVA: code_gen->WriteTodo(md.GetEntityName(), "PMOVA"); break;
        case OPCODES::PDIRA: code_gen->WriteTodo(md.GetEntityName(), "PDIRA"); break;
        case OPCODES::PTURA: code_gen->WriteTodo(md.GetEntityName(), "PTURA"); break;
        case OPCODES::PGTDR: code_gen->WriteTodo(md.GetEntityName(), "PGTDR"); break;
        case OPCODES::PXYZI: code_gen->WriteTodo(md.GetEntityName(), "PXYZI"); break;
        case OPCODES::TLKON: ProcessTLKON(code_gen, md.GetEntityName()); break;
        case OPCODES::PC: ProcessPC(code_gen, md.GetEntityName()); break;
        case OPCODES::opCodeCHAR: ProcessCHAR(code_gen, md.GetEntityName()); break;
        case OPCODES::DFANM: ProcessDFANM(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODES::ANIME1:
            ProcessANIME1(code_gen, md.GetEntityName(), md.GetCharacterId());
            break;
        case OPCODES::VISI: ProcessVISI(code_gen, md.GetEntityName()); break;
        case OPCODES::XYZI: ProcessXYZI(code_gen, md.GetEntityName()); break;
        case OPCODES::XYI: code_gen->WriteTodo(md.GetEntityName(), "XYI"); break;
        case OPCODES::XYZ: code_gen->WriteTodo(md.GetEntityName(), "XYZ"); break;
        case OPCODES::MOVE: ProcessMOVE(code_gen, md.GetEntityName()); break;
        case OPCODES::CMOVE: code_gen->WriteTodo(md.GetEntityName(), "CMOVE"); break;
        case OPCODES::MOVA: code_gen->WriteTodo(md.GetEntityName(), "MOVA"); break;
        case OPCODES::TURA: code_gen->WriteTodo(md.GetEntityName(), "TURA"); break;
        case OPCODES::ANIMW: code_gen->WriteTodo(md.GetEntityName(), "ANIMW"); break;
        case OPCODES::FMOVE: code_gen->WriteTodo(md.GetEntityName(), "FMOVE"); break;
        case OPCODES::ANIME2: code_gen->WriteTodo(md.GetEntityName(), "ANIME2"); break;
        case OPCODES::ANIM_1: code_gen->WriteTodo(md.GetEntityName(), "ANIM_1"); break;
        case OPCODES::CANIM1: code_gen->WriteTodo(md.GetEntityName(), "CANIM1"); break;
        case OPCODES::CANM_1: code_gen->WriteTodo(md.GetEntityName(), "CANM_1"); break;
        case OPCODES::MSPED: ProcessMSPED(code_gen, md.GetEntityName()); break;
        case OPCODES::DIR: ProcessDIR(code_gen, md.GetEntityName()); break;
        case OPCODES::TURNGEN: ProcessTURNGEN(code_gen, md.GetEntityName()); break;
        case OPCODES::TURN: code_gen->WriteTodo(md.GetEntityName(), "TURN"); break;
        case OPCODES::DIRA: code_gen->WriteTodo(md.GetEntityName(), "DIRA"); break;
        case OPCODES::GETDIR: code_gen->WriteTodo(md.GetEntityName(), "GETDIR"); break;
        case OPCODES::GETAXY: code_gen->WriteTodo(md.GetEntityName(), "GETAXY"); break;
        case OPCODES::GETAI: ProcessGETAI(code_gen, eng); break;
        case OPCODES::ANIM_2:
            ProcessANIM_2(code_gen, md.GetEntityName(), md.GetCharacterId());
            break;
        case OPCODES::CANIM2:
            ProcessCANIM2(code_gen, md.GetEntityName(), md.GetCharacterId());
            break;
        case OPCODES::CANM_2:
            ProcessCANM_2(code_gen, md.GetEntityName(), md.GetCharacterId());
            break;
        case OPCODES::ASPED: code_gen->WriteTodo(md.GetEntityName(), "ASPED"); break;
        case OPCODES::CC: ProcessCC(code_gen, eng); break;
        case OPCODES::JUMP: ProcessJUMP(code_gen, md.GetEntityName()); break;
        case OPCODES::AXYZI: ProcessAXYZI(code_gen); break;
        case OPCODES::LADER: ProcessLADER(code_gen, md.GetEntityName()); break;
        case OPCODES::OFST: ProcessOFST(code_gen, md.GetEntityName()); break;
        case OPCODES::OFSTW:
            code_gen->AddOutputLine("self." + md.GetEntityName() + ":offset_sync()");
            break;
        case OPCODES::TALKR: code_gen->WriteTodo(md.GetEntityName(), "TALKR"); break;
        case OPCODES::SLIDR: code_gen->WriteTodo(md.GetEntityName(), "SLIDR"); break;
        case OPCODES::SOLID: ProcessSOLID(code_gen, md.GetEntityName()); break;
        case OPCODES::TLKR2: code_gen->WriteTodo(md.GetEntityName(), "TLKR2"); break;
        case OPCODES::SLDR2: code_gen->WriteTodo(md.GetEntityName(), "SLDR2"); break;
        case OPCODES::CCANM: code_gen->WriteTodo(md.GetEntityName(), "CCANM"); break;
        case OPCODES::FCFIX: code_gen->WriteTodo(md.GetEntityName(), "FCFIX"); break;
        case OPCODES::ANIMB: code_gen->WriteTodo(md.GetEntityName(), "ANIMB"); break;
        case OPCODES::TURNW: code_gen->WriteTodo(md.GetEntityName(), "TURNW"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), address_, opcode_
            ));
    }
}

void FieldModelInstruction::ProcessJOIN(CodeGenerator* code_gen){
    code_gen->AddOutputLine("join_party(" + std::to_string(params_[0]->GetUnsigned()) + ")");
}

void FieldModelInstruction::ProcessSPLIT(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& ax = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[6]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& ay = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[7]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& ar = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[8]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& bx = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[9]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& by = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[4]->GetUnsigned(), params_[10]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& br = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[5]->GetUnsigned(), params_[11]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& speed = params_[12]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("split_party(%1%, %2%, %3%, %4%, %5%, %6%, %7%)")
      % ax % ay % ar % bx % by % br % speed
    ).str());
}

void FieldModelInstruction::ProcessTLKON(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_talkable(%2%)")
      % entity % FieldCodeGenerator::FormatInvertedBool(params_[0]->GetUnsigned())
    ).str());
}

void FieldModelInstruction::ProcessPC(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine(
      (boost::format("set_entity_to_character(\"%1%\", \"%1%\")") % entity).str()
    );
}

void FieldModelInstruction::ProcessCHAR(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine(
      (boost::format("self.%1% = entity_manager:get_entity(\"%1%\")") % entity).str()
    );
}

void FieldModelInstruction::ProcessDFANM(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    // ID will be fixed-up downstream.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    auto animation_id = params_[0]->GetUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / params_[1]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_default_animation(\"%2%\") -- speed %3%")
      % entity % cg->GetFormatter().GetFriendlyAnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\")")
      % entity % cg->GetFormatter().GetFriendlyAnimationName(char_id, animation_id)
    ).str());
}

void FieldModelInstruction::ProcessANIME1(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    // ID will be fixed-up downstream.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    auto animation_id = params_[0]->GetUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / params_[1]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\") -- speed %3%")
      % entity % cg->GetFormatter().GetFriendlyAnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FieldModelInstruction::ProcessVISI(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_visible(%2%)")
      % entity % FieldCodeGenerator::FormatBool(params_[0]->GetUnsigned())
    ).str());
}

void FieldModelInstruction::ProcessXYZI(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[4]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[5]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& z = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[6]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& triangle_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[7]->GetUnsigned()
    );
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_position(%2%, %3%, %4%) -- triangle ID %5%")
      % entity % x % y % z % triangle_id
    ).str());
}

void FieldModelInstruction::ProcessMOVE(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[2]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    code_gen->AddOutputLine(
      (boost::format("self.%1%:move_to_position(%2%, %3%)") % entity % x % y).str()
    );
    code_gen->AddOutputLine((boost::format("self.%1%:move_sync()") % entity).str());
}

void FieldModelInstruction::ProcessMSPED(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& speed = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[2]->GetUnsigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f * scale / 30.0f
    );
    code_gen->AddOutputLine(
      (boost::format("self.%1%:set_move_auto_speed(%2%)") % entity % speed).str()
    );
}

void FieldModelInstruction::ProcessDIR(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& degrees = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[1]->GetUnsigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f / 360.0f
    );
    code_gen->AddOutputLine((boost::format("self.%1%:set_rotation(%2%)") % entity % degrees).str());
}

void FieldModelInstruction::ProcessTURNGEN(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& degrees = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[2]->GetUnsigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f / 360.0f
    );
    std::string direction;
    switch (params_[3]->GetUnsigned()){
        case 0: direction = "Entity.CLOCKWISE"; break;
        case 1: direction = "Entity.ANTICLOCKWISE"; break;
        case 2: direction = "Entity.CLOSEST"; break;
        // Default to closest:
        default: direction = "Entity.CLOSEST"; break;
    }
    auto steps = params_[4]->GetUnsigned();
    std::string step_type;
    switch (params_[5]->GetUnsigned()){
        case 1: step_type = "Entity.LINEAR"; break;
        case 2: step_type = "Entity.SMOOTH"; break;
        // Default to smooth
        default: step_type = "Entity.SMOOTH"; break;
    }
    const float scaled_steps = static_cast<float>(steps) / 30.0f;
    code_gen->AddOutputLine((
      boost::format("self.%1%:turn_to_direction(%2%, %3%, %4%, %5%)")
      % entity % degrees % direction % step_type % scaled_steps
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:turn_sync()") % entity).str());
}

void FieldModelInstruction::ProcessGETAI(CodeGenerator* code_gen, const FieldEngine& engine){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // TODO: check for assignment to literal.
    const auto& variable = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[3]->GetUnsigned()
    );
    const auto& entity = engine.EntityByIndex(params_[2]->GetUnsigned());
    code_gen->AddOutputLine(
      (boost::format("%1% = entity_manager:get_entity(\"%2%\"):get_move_triangle_id()")
      % variable % entity.GetName()
    ).str());
}

void FieldModelInstruction::ProcessANIM_2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    // ID will be fixed-up downstream.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    auto animation_id = params_[0]->GetUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / params_[1]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation_stop(\"%2%\") -- speed %3%")
      % entity % cg->GetFormatter().GetFriendlyAnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FieldModelInstruction::ProcessCANIM2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    auto animation_id = params_[0]->GetUnsigned();
    auto start_frame = params_[1]->GetUnsigned() / 30.0f;
    auto end_frame = params_[2]->GetUnsigned() / 30.0f;
    // TODO: check for zero.
    auto speed = 1.0f / params_[3]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\", %3%, %4%) -- speed %5%")
      % entity % cg->GetFormatter().GetFriendlyAnimationName(char_id, animation_id)
      % start_frame % end_frame % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FieldModelInstruction::ProcessCANM_2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // ID will be fixed-up downstream.
    auto animation_id = params_[0]->GetUnsigned();
    auto start_frame = params_[1]->GetUnsigned() / 30.0f;
    auto end_frame = params_[2]->GetUnsigned() / 30.0f;
    // TODO: check for zero.
    auto speed = 1.0f / params_[3]->GetUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation_stop(\"%2%\", %3%, %4%) -- speed %5%")
      % entity % cg->GetFormatter().GetFriendlyAnimationName(char_id, animation_id)
      % start_frame % end_frame % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FieldModelInstruction::ProcessCC(CodeGenerator* code_gen, const FieldEngine& engine){
    const auto& entity = engine.EntityByIndex(params_[0]->GetUnsigned());
    code_gen->AddOutputLine(
      (boost::format("entity_manager:set_player_entity(\"%1%\")") % entity.GetName()).str()
    );
}

void FieldModelInstruction::ProcessJUMP(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    float x = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[4]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float y = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[5]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    int i = atoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[6]->GetSigned()
    ).c_str());
    int steps = atoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[7]->GetSigned()
    ).c_str());
    //x *= 0.00781250273224;
    //y *= 0.00781250273224;
    // Always force reduce steps and bound it between 1 and 5.
    steps /= 6;
    if (steps < 1) steps = 1;
    if (steps > 5) steps = 5;
    // TODO: Z hardcoded as -1, handled in Entity::ScriptJumpToPosition.
    // TODO: Hardcoded 0.5 in seconds. Calculate using distance.
    code_gen->AddOutputLine((
      boost::format("self.%1%:jump_to_position(%2%, %3%, -1, 0.5, %4%) -- %5% steps.")
      % entity % x % y % i % steps
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:jump_sync()") % entity).str());
}

void FieldModelInstruction::ProcessAXYZI(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    code_gen->AddOutputLine((
      boost::format("axyzi(%1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%, %9%, %10%)")
      % params_[0]->GetSigned() % params_[1]->GetSigned() % params_[2]->GetSigned()
      % params_[3]->GetSigned() % params_[4]->GetSigned() % params_[5]->GetSigned()
      % params_[6]->GetSigned() % params_[7]->GetSigned() % params_[8]->GetSigned()
      % scale
    ).str());
}

void FieldModelInstruction::ProcessLADER(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[4]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(), params_[5]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& z = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[6]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    uint end_triangle = atoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[7]->GetUnsigned()
    ).c_str());
    uint keys = params_[8]->GetUnsigned();
    uint animation = params_[9]->GetUnsigned();
    //float orientation = params_[10]->GetUnsigned() / (256.0f / 360.0f);
    const auto& orientation = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), 0, params_[10]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f / 360.0f
    );
    uint speed = params_[11]->GetUnsigned();
    // TODO: Animation hardcoded as "btce".
    // TODO: Orientation and speed not set.
    code_gen->AddOutputLine((
      boost::format(
        "self.%1%:linear_to_position(%2%, %3%, %4%, %5%, \"Climb\", %6%, %7%) "
        "-- Animation %8% -- Speed %9%"
      ) % entity % x % y % z % keys % orientation % end_triangle % animation % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:linear_sync()") % entity).str());
}

void FieldModelInstruction::ProcessSOLID(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_solid(%2%)")
      % entity % FieldCodeGenerator::FormatInvertedBool(params_[0]->GetUnsigned())
    ).str());
}

void FieldModelInstruction::ProcessOFST(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    float x = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[0]->GetUnsigned(), params_[5]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float y = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[1]->GetUnsigned(),params_[6]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float z = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[2]->GetUnsigned(), params_[7]->GetSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float speed = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), params_[3]->GetUnsigned(), params_[8]->GetUnsigned()
    ));
    // Speed needs to be scaled down by the frame rate.
    speed /= 30.0f;
    code_gen->AddOutputLine((
      boost::format("self.%1%:offset_to_position(%2%, %3%, %4%, %5%, %6%)")
      % entity % x % y % z
      % (params_[4]->GetUnsigned() ? "Entity.SMOOTH" : "Entity.LINEAR")% speed
    ).str());
}
