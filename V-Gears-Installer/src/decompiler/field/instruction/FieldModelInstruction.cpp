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

#include "decompiler/field/instruction/FieldModelInstruction.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"

void FieldModelInstruction::ProcessInst(
  Function& func, ValueStack&, Engine* engine, CodeGenerator *code_gen
){
    FieldEngine& eng = static_cast<FieldEngine&>(*engine);
    FunctionMetaData md(func._metadata);
    switch (_opcode){
        case OPCODE::JOIN: ProcessJOIN(code_gen); break;
        case OPCODE::SPLIT: ProcessSPLIT(code_gen); break;
        case OPCODE::BLINK: WriteTodo(code_gen, md.GetEntityName(), "BLINK"); break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::EYETX:
            WriteTodo(code_gen, md.GetEntityName(), "EYETX");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::TRNSP:
            WriteTodo(code_gen, md.GetEntityName(), "TRNSP");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::AMBNT:
            WriteTodo(code_gen, md.GetEntityName(), "AMBNT");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown03:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown03");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown04:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown04");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown05:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown05");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::LIGHT:
            WriteTodo(code_gen, md.GetEntityName(), "LIGHT");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown07:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown07");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown08:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown08");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown09:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown09");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::SBOBJ:
            WriteTodo(code_gen, md.GetEntityName(), "SBOBJ");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown0B:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown0B");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::Unknown0C:
            WriteTodo(code_gen, md.GetEntityName(), "Unknown0C");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::SHINE:
            WriteTodo(code_gen, md.GetEntityName(), "SHINE");
            break;
        case (OPCODE::KAWAI << 8) | OPCODE_KAWAI::RESET:
            WriteTodo(code_gen, md.GetEntityName(), "RESET");
            break;
        case OPCODE::KAWIW: WriteTodo(code_gen, md.GetEntityName(), "KAWIW"); break;
        case OPCODE::PMOVA: WriteTodo(code_gen, md.GetEntityName(), "PMOVA"); break;
        case OPCODE::PDIRA: WriteTodo(code_gen, md.GetEntityName(), "PDIRA"); break;
        case OPCODE::PTURA: WriteTodo(code_gen, md.GetEntityName(), "PTURA"); break;
        case OPCODE::PGTDR: WriteTodo(code_gen, md.GetEntityName(), "PGTDR"); break;
        case OPCODE::PXYZI: WriteTodo(code_gen, md.GetEntityName(), "PXYZI"); break;
        case OPCODE::TLKON: ProcessTLKON(code_gen, md.GetEntityName()); break;
        case OPCODE::PC: ProcessPC(code_gen, md.GetEntityName()); break;
        case OPCODE::opCodeCHAR: ProcessCHAR(code_gen, md.GetEntityName()); break;
        case OPCODE::DFANM: ProcessDFANM(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODE::ANIME1: ProcessANIME1(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODE::VISI: ProcessVISI(code_gen, md.GetEntityName()); break;
        case OPCODE::XYZI: ProcessXYZI(code_gen, md.GetEntityName()); break;
        case OPCODE::XYI: WriteTodo(code_gen, md.GetEntityName(), "XYI"); break;
        case OPCODE::XYZ: WriteTodo(code_gen, md.GetEntityName(), "XYZ"); break;
        case OPCODE::MOVE: ProcessMOVE(code_gen, md.GetEntityName()); break;
        case OPCODE::CMOVE: WriteTodo(code_gen, md.GetEntityName(), "CMOVE"); break;
        case OPCODE::MOVA: WriteTodo(code_gen, md.GetEntityName(), "MOVA"); break;
        case OPCODE::TURA: WriteTodo(code_gen, md.GetEntityName(), "TURA"); break;
        case OPCODE::ANIMW: WriteTodo(code_gen, md.GetEntityName(), "ANIMW"); break;
        case OPCODE::FMOVE: WriteTodo(code_gen, md.GetEntityName(), "FMOVE"); break;
        case OPCODE::ANIME2: WriteTodo(code_gen, md.GetEntityName(), "ANIME2"); break;
        case OPCODE::ANIM_1: WriteTodo(code_gen, md.GetEntityName(), "ANIM_1"); break;
        case OPCODE::CANIM1: WriteTodo(code_gen, md.GetEntityName(), "CANIM1"); break;
        case OPCODE::CANM_1: WriteTodo(code_gen, md.GetEntityName(), "CANM_1"); break;
        case OPCODE::MSPED: ProcessMSPED(code_gen, md.GetEntityName()); break;
        case OPCODE::DIR: ProcessDIR(code_gen, md.GetEntityName()); break;
        case OPCODE::TURNGEN: ProcessTURNGEN(code_gen, md.GetEntityName()); break;
        case OPCODE::TURN: WriteTodo(code_gen, md.GetEntityName(), "TURN"); break;
        case OPCODE::DIRA: WriteTodo(code_gen, md.GetEntityName(), "DIRA"); break;
        case OPCODE::GETDIR: WriteTodo(code_gen, md.GetEntityName(), "GETDIR"); break;
        case OPCODE::GETAXY: WriteTodo(code_gen, md.GetEntityName(), "GETAXY"); break;
        case OPCODE::GETAI: ProcessGETAI(code_gen, eng); break;
        case OPCODE::ANIM_2: ProcessANIM_2(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODE::CANIM2: ProcessCANIM2(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODE::CANM_2: ProcessCANM_2(code_gen, md.GetEntityName(), md.GetCharacterId()); break;
        case OPCODE::ASPED: WriteTodo(code_gen, md.GetEntityName(), "ASPED"); break;
        case OPCODE::CC: ProcessCC(code_gen, eng); break;
        case OPCODE::JUMP: ProcessJUMP(code_gen, md.GetEntityName()); break;
        case OPCODE::AXYZI: ProcessAXYZI(code_gen); break;
        case OPCODE::LADER: ProcessLADER(code_gen, md.GetEntityName()); break;
        case OPCODE::OFST: ProcessOFST(code_gen, md.GetEntityName()); break;
        case OPCODE::OFSTW:
            code_gen->AddOutputLine("self." + md.GetEntityName() + ":offset_sync()");
            break;
        case OPCODE::TALKR: WriteTodo(code_gen, md.GetEntityName(), "TALKR"); break;
        case OPCODE::SLIDR: WriteTodo(code_gen, md.GetEntityName(), "SLIDR"); break;
        case OPCODE::SOLID: ProcessSOLID(code_gen, md.GetEntityName()); break;
        case OPCODE::TLKR2: WriteTodo(code_gen, md.GetEntityName(), "TLKR2"); break;
        case OPCODE::SLDR2: WriteTodo(code_gen, md.GetEntityName(), "SLDR2"); break;
        case OPCODE::CCANM: WriteTodo(code_gen, md.GetEntityName(), "CCANM"); break;
        case OPCODE::FCFIX: WriteTodo(code_gen, md.GetEntityName(), "FCFIX"); break;
        case OPCODE::ANIMB: WriteTodo(code_gen, md.GetEntityName(), "ANIMB"); break;
        case OPCODE::TURNW: WriteTodo(code_gen, md.GetEntityName(), "TURNW"); break;
        default:
            code_gen->AddOutputLine(FieldCodeGenerator::FormatInstructionNotImplemented(
              md.GetEntityName(), _address, _opcode
            ));
    }
}

void FieldModelInstruction::ProcessJOIN(CodeGenerator* code_gen){
    code_gen->AddOutputLine("join_party(" + std::to_string(_params[0]->getUnsigned()) + ")");
}

void FieldModelInstruction::ProcessSPLIT(CodeGenerator* code_gen){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& ax = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[6]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& ay = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[7]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& ar = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[8]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& bx = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[9]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& by = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[4]->getUnsigned(), _params[10]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& br = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[5]->getUnsigned(), _params[11]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& speed = _params[12]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("split_party(%1%, %2%, %3%, %4%, %5%, %6%, %7%)")
      % ax % ay % ar % bx % by % br % speed
    ).str());
}

void FieldModelInstruction::ProcessTLKON(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_talkable(%2%)")
      % entity % FieldCodeGenerator::FormatInvertedBool(_params[0]->getUnsigned())
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
    auto animation_id = _params[0]->getUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / _params[1]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_default_animation(\"%2%\") -- speed %3%")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\")")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id)
    ).str());
}

void FieldModelInstruction::ProcessANIME1(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    // ID will be fixed-up downstream.
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    auto animation_id = _params[0]->getUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / _params[1]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\") -- speed %3%")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FieldModelInstruction::ProcessVISI(CodeGenerator* code_gen, const std::string& entity){
    code_gen->AddOutputLine((
      boost::format("self.%1%:set_visible(%2%)")
      % entity % FieldCodeGenerator::FormatBool(_params[0]->getUnsigned())
    ).str());
}

void FieldModelInstruction::ProcessXYZI(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[4]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[5]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& z = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[6]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& triangle_id = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[7]->getUnsigned()
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
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[2]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getSigned(),
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
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[2]->getUnsigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f * scale / 30.0f
    );
    code_gen->AddOutputLine(
      (boost::format("self.%1%:set_move_auto_speed(%2%)") % entity % speed).str()
    );
}

void FieldModelInstruction::ProcessDIR(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& degrees = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[1]->getUnsigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f / 360.0f
    );
    code_gen->AddOutputLine((boost::format("self.%1%:set_rotation(%2%)") % entity % degrees).str());
}

void FieldModelInstruction::ProcessTURNGEN(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const auto& degrees = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[2]->getUnsigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f / 360.0f
    );
    std::string direction;
    switch (_params[3]->getUnsigned()){
        case 0: direction = "Entity.CLOCKWISE"; break;
        case 1: direction = "Entity.ANTICLOCKWISE"; break;
        case 2: direction = "Entity.CLOSEST"; break;
        // Default to closest:
        default: direction = "Entity.CLOSEST"; break;
    }
    auto steps = _params[4]->getUnsigned();
    std::string step_type;
    switch (_params[5]->getUnsigned()){
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
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[3]->getUnsigned()
    );
    const auto& entity = engine.EntityByIndex(_params[2]->getUnsigned());
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
    auto animation_id = _params[0]->getUnsigned();
    // TODO: check for zero.
    auto speed = 1.0f / _params[1]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation_stop(\"%2%\") -- speed %3%")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id) % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FieldModelInstruction::ProcessCANIM2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    auto animation_id = _params[0]->getUnsigned();
    auto start_frame = _params[1]->getUnsigned() / 30.0f;
    auto end_frame = _params[2]->getUnsigned() / 30.0f;
    // TODO: check for zero.
    auto speed = 1.0f / _params[3]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation(\"%2%\", %3%, %4%) -- speed %5%")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id)
      % start_frame % end_frame % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FieldModelInstruction::ProcessCANM_2(
  CodeGenerator* code_gen, const std::string& entity, int char_id
){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    // ID will be fixed-up downstream.
    auto animation_id = _params[0]->getUnsigned();
    auto start_frame = _params[1]->getUnsigned() / 30.0f;
    auto end_frame = _params[2]->getUnsigned() / 30.0f;
    // TODO: check for zero.
    auto speed = 1.0f / _params[3]->getUnsigned();
    code_gen->AddOutputLine((
      boost::format("self.%1%:play_animation_stop(\"%2%\", %3%, %4%) -- speed %5%")
      % entity % cg->GetFormatter().AnimationName(char_id, animation_id)
      % start_frame % end_frame % speed
    ).str());
    code_gen->AddOutputLine((boost::format("self.%1%:animation_sync()") % entity).str());
}

void FieldModelInstruction::ProcessCC(CodeGenerator* code_gen, const FieldEngine& engine){
    const auto& entity = engine.EntityByIndex(_params[0]->getUnsigned());
    code_gen->AddOutputLine(
      (boost::format("entity_manager:set_player_entity(\"%1%\")") % entity.GetName()).str()
    );
}

void FieldModelInstruction::ProcessJUMP(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    float x = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[4]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float y = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[5]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    int i = atoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[6]->getSigned()
    ).c_str());
    int steps = atoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[7]->getSigned()
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
      % _params[0]->getSigned() % _params[1]->getSigned() % _params[2]->getSigned()
      % _params[3]->getSigned() % _params[4]->getSigned() % _params[5]->getSigned()
      % _params[6]->getSigned() % _params[7]->getSigned() % _params[8]->getSigned()
      % scale
    ).str());
}

void FieldModelInstruction::ProcessLADER(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    const auto& x = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[4]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& y = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(), _params[5]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    const auto& z = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[6]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    );
    uint end_triangle = atoi(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[7]->getUnsigned()
    ).c_str());
    uint keys = _params[8]->getUnsigned();
    uint animation = _params[9]->getUnsigned();
    //float orientation = _params[10]->getUnsigned() / (256.0f / 360.0f);
    const auto& orientation = FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), 0, _params[10]->getSigned(),
      FieldCodeGenerator::ValueType::Float, 256.0f / 360.0f
    );
    uint speed = _params[11]->getUnsigned();
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
      % entity % FieldCodeGenerator::FormatInvertedBool(_params[0]->getUnsigned())
    ).str());
}

void FieldModelInstruction::ProcessOFST(CodeGenerator* code_gen, const std::string& entity){
    FieldCodeGenerator* cg = static_cast<FieldCodeGenerator*>(code_gen);
    const float scale = 128.0f * cg->GetScaleFactor();
    float x = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[0]->getUnsigned(), _params[5]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float y = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[1]->getUnsigned(),_params[6]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float z = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[2]->getUnsigned(), _params[7]->getSigned(),
      FieldCodeGenerator::ValueType::Float, scale
    ));
    float speed = std::stof(FieldCodeGenerator::FormatValueOrVariable(
      cg->GetFormatter(), _params[3]->getUnsigned(), _params[8]->getUnsigned()
    ));
    // Spatial coordinates need to be scaled down.
    // TODO: This number is empirically deducted. Why this number?
    //x *= 0.00390f;
    //y *= 0.00390f;
    //z *= 0.00390f;
    // Speed needs to be scaled down by the frame rate.
    speed /= 30.0f;
    code_gen->AddOutputLine((
      boost::format("self.%1%:offset_to_position(%2%, %3%, %4%, %5%, %6%)")
      % entity % x % y % z
      % (_params[4]->getUnsigned() ? "Entity.SMOOTH" : "Entity.LINEAR")% speed
    ).str());
}
