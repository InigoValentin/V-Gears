/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <vector>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include "../../../include/decompiler/field/FieldDisassembler.h"
#include "common/Lzs.h"
#include "decompiler/field/ff7_field_engine.h"
#include "decompiler/decompiler_engine.h"
#include "decompiler/field/FieldCodeGenerator.h"

const int FieldDisassembler::MAGIC(0x0502);

const int FieldDisassembler::NUM_SECTIONS(7);

FieldDisassembler::FieldDisassembler(
  SUDM::IScriptFormatter& formatter, FF7FieldEngine* engine, InstVec& insts,
  const std::vector<unsigned char>& raw_script_data
) : SimpleDisassembler(insts), engine_(engine), formatter_(formatter){
    loaded_from_raw_ = true;
    // If loading a raw section then we don't have a "sections header" to skip.
    section_pointers_size_ = 0;
    auto data_copy = raw_script_data;
    stream_ = std::make_unique<BinaryReader>(std::move(data_copy));
    ReadHeader();
}

FieldDisassembler::FieldDisassembler(
  SUDM::IScriptFormatter& formatter, FF7FieldEngine *engine, InstVec &insts
) : SimpleDisassembler(insts), engine_(engine), formatter_(formatter){
    section_pointers_size_ = (sizeof(uint32) * NUM_SECTIONS);
}

FieldDisassembler::~FieldDisassembler(){}

void FieldDisassembler::ReadHeader(){
    if (!loaded_from_raw_){
        // First read the file section pointers.
        for (int i = 0; i < NUM_SECTIONS; i ++) sections_[i] = stream_->ReadU32();
        // Now fix up from PSX RAM pointers to simple file offsets.
        const uint32 base_ptr = sections_[0];
        for (int i = 0; i < NUM_SECTIONS; i ++)
            sections_[i] = (sections_[i] - base_ptr) + section_pointers_size_;
        // Now seek to the script section.
        stream_->Seek(sections_[SCRIPT]);
    }

    // Read the script header
    header_.Read(*stream_);
    scale_factor_ = static_cast<float>(header_.scale) / 512.0f;
}

void FieldDisassembler::ReadHeader(BinaryReader& reader){
    header_.Read(reader);
    header_end_position_ = reader.GetPosition();
}

void FieldDisassembler::Open(const char *filename){
    // Read all of the file, decompress it, then stuff it into a stream.
    stream_ = std::make_unique<BinaryReader>(Lzs::Decompress(BinaryReader::ReadAll(filename)));
    ReadHeader();
}

uint32 FieldDisassembler::GetEndOfScriptOffset(
  uint16 cur_entry_point, size_t entity_index, size_t script_index
){
    uint16 next_entry_point = cur_entry_point;
    do{
        if (script_index + 1 >= 32){
            // If this is the very last script, so use the end of its data
            // which is the offset to strings.
            if (entity_index + 1 >= header_.entity_scripts.size()) return header_.offset_to_strings;
            else{
                // Wrap around to the next entity
                entity_index ++;
                script_index = 0;
            }
        }
        // Get the next script in the same entity:
        else script_index ++;
        next_entry_point = header_.entity_scripts[entity_index][script_index];
    } while (next_entry_point == cur_entry_point);
    return next_entry_point;
}

std::unique_ptr<Function> FieldDisassembler::StartFunction(size_t script_index){
    auto func = std::make_unique<Function>();
    func->_retVal = false;
    func->_args = 0;
    func->_name = "script_" + std::to_string(script_index);
    func->mStartAddr = address_;
    return func;
}

void FieldDisassembler::DoDisassemble(){
    // Loop through the scripts for each entity.
    for (size_t entity_number = 0; entity_number < header_.entity_scripts.size(); entity_number ++){
        std::string original_name = header_.field_entity_names[entity_number].data();
        // If the entity name was blank in the file then use a consistent generated name.
        if (original_name.empty()) original_name = "entity_" + std::to_string(entity_number);
        const std::string entity_name = formatter_.EntityName(original_name);
        // Only parse each script one.
        std::set<uint16> parsed_scripts;
        std::vector<ScriptInfo> script_info;
        // Collect the scripts to parse.
        for (
          size_t script_index = 0;
          script_index < header_.entity_scripts[entity_number].size();
          script_index ++
        ){
            uint16 script_entry_point = header_.entity_scripts[entity_number][script_index];
            // If this scripts entry point is already parsed, then don't do it again as it means
            // two scripts have the same entry which only seems to be true for "empty" scripts.
            if (parsed_scripts.find(script_entry_point) != std::end(parsed_scripts)) continue;
            parsed_scripts.insert(script_entry_point);
            const uint32 next_script_entry_point = GetEndOfScriptOffset(
              script_entry_point, entity_number, script_index
            );
            const uint32 script_size = next_script_entry_point - script_entry_point;
            if (script_size > 0){
                ScriptInfo info = { script_entry_point, next_script_entry_point, script_index };
                script_info.push_back(info);
            }
        }
        for (auto it = script_info.begin(); it != script_info.end(); it ++){
            const bool is_start = it == script_info.begin();
            const bool is_end = it == (--script_info.end());
            DisassembleIndivdualScript(
              entity_name, entity_number, it->index, it->entry_point,
              it->next_entry_point, is_start, is_end
            );
        }
    }
}

float FieldDisassembler::GetScaleFactor() const{return scale_factor_;}

int FieldDisassembler::FindId(uint32 start_addr, uint32 end_addr, const InstVec& insts){
    for (const InstPtr& instruction : insts){
        if (instruction->_address >= start_addr && instruction->_address <= end_addr){
            if (instruction->_opcode == OPCODE::opCodeCHAR)
                return instruction->_params[0]->getSigned();
        }
    }
    return -1;
}

void FieldDisassembler::AddFunc(
  std::string entity_name, size_t entity_index, size_t script_index, uint32 next_script_entry_point,
  const bool is_start, bool is_end, bool to_return_only, std::string func_name
){
    bool is_line = false;
    std::vector<float> point_a = {0, 0, 0};
    std::vector<float> point_b = {0, 0, 0};
    const auto SCRIPT_ENTRY_POINT = stream_->GetPosition();
    // Read each block of opcodes up to a return.
    const size_t old_num_instructions = _insts.size();
    // Initialize the function.
    std::unique_ptr<Function> func = StartFunction(script_index);
    // Read.
    if (to_return_only){
        // Read opcodes to the end or bail at the first return.
        is_line = ReadOpCodesToPositionOrReturn(
          next_script_entry_point + section_pointers_size_, point_a, point_b
        );
        auto stream_pos = stream_->GetPosition();
        const size_t endPos = next_script_entry_point + section_pointers_size_;
        // Can't be the end if there is more data:
        if (stream_pos != endPos) is_end = false;
    }
    else{
        // Keep going till we have all of the script, i.e if we bail at a
        // return then call again till we have everything
        while (stream_->GetPosition() != next_script_entry_point + section_pointers_size_){
            is_line = ReadOpCodesToPositionOrReturn(
              next_script_entry_point + section_pointers_size_, point_a, point_b
            );
        }
    }
    // Read metadata, mark start the function as first and/or last of the class.
    std::string meta_data;
    if (is_start && is_end) meta_data = "start_end_";
    else if (is_start) meta_data = "start_";
    else if (is_end) meta_data = "end_";
    const size_t new_num_instructions = _insts.size();
    func->mNumInstructions = new_num_instructions - old_num_instructions;
    func->mEndAddr = _insts.back()->_address;
    if (!func_name.empty()) func->_name = func_name;
    if (engine_->EntityIsLine(entity_index)){
        switch (script_index){
            // main   - on_update
            case 0: break;
            // [OK]   - on_interact
            case 1: break;
            // Move - on_enter_line
            case 2: func->_name = "on_enter_line"; break;
            // Move - on_move_to_line
            case 3: func->_name = "on_move_to_line"; break;
            // Go - on_cross_line
            case 4: func->_name = "on_cross_line"; break;
            // Go1x - on_cross_line
            case 5: func->_name = "on_cross_line_once"; break;
            // GoAway - on_leave_line
            case 6: func->_name = "on_leave_line"; break;
        }
    }
    int id = FindId(func->mStartAddr, func->mEndAddr, _insts);
    // If there is no ID check if there was an ID for this entity in any of
    // its other functions and use that instead.
    if (id == -1){
        for (auto& func : engine_->_functions){
            FunctionMetaData func_meta_data(func.second._metadata);
            if (
              func_meta_data.GetEntityName() == entity_name && func_meta_data.GetCharacterId() != -1
            ){
                id = func_meta_data.GetCharacterId();
                break;
            }
        }
    }
    meta_data += std::to_string(id) + "_" + entity_name;
    func->_metadata = meta_data;
    engine_->_functions[SCRIPT_ENTRY_POINT] = *func;
    engine_->AddEntityFunction(entity_name, entity_index, func->_name, script_index);
    // If the entity is a line, mark it as so.
    if (is_line) engine_->MarkEntityAsLine(entity_index, true, point_a, point_b);
}

void FieldDisassembler::DisassembleIndivdualScript(
    std::string entity_name, size_t entity_index, size_t script_entry_point,
    size_t script_index, uint32 next_script_entry_point, bool is_start, bool is_end
){
    script_entry_point += section_pointers_size_;
    stream_->Seek(script_entry_point);
    address_base_ = stream_->GetPosition();
    address_ = address_base_;
    // "Normal" script
    if (script_index > 0){
        if (script_index == 1){
            AddFunc(
              entity_name, entity_index, script_index, next_script_entry_point,
              is_start, is_end, false, "on_interact"
            );
        }
        else{
            AddFunc(
              entity_name, entity_index, script_index, next_script_entry_point,
              is_start, is_end, false, ""
            );
        }
    }
    else{
        const size_t end_pos = next_script_entry_point + section_pointers_size_;
        // Read the init script, which means stop at the first return
        AddFunc(
          entity_name, entity_index, script_index, next_script_entry_point,
          is_start, is_end, true, "on_start"
        );

        // Not at the end of this script? Then the remaining data is the "main" script
        auto stream_pos = stream_->GetPosition();
        if (stream_pos != end_pos){
            // The "main" script can have more than one return statement
            AddFunc(
              entity_name, entity_index, script_index, next_script_entry_point,
              false, is_end, false, "on_update"
            );
            stream_pos = stream_->GetPosition();

            // But should end exactly on the end pos
            if (stream_pos != end_pos) throw InternalDecompilerError();
        }

    }
}

/**
 * @todo Understand and document.
 * @todo Maybe add to the class.
 */
const InstructionRecord kOpcodes[] ={
    // Flow
    { 1, OPCODE::RET, "RET", "", FF7ControlFlowInstruction::Create },
    { 1, OPCODE::REQ, "REQ", "BU", FF7ControlFlowInstruction::Create },
    { 1, OPCODE::REQSW, "REQSW", "BU", FF7ControlFlowInstruction::Create },
    { 1, OPCODE::REQEW, "REQEW", "BU", FF7ControlFlowInstruction::Create },
    { 1, OPCODE::NOP, "NOP", "", FF7NoOperationInstruction::Create },
    { 1, OPCODE::IFUB, "IFUB", "NNBBBL", FF7NoOperationInstruction::Create }
};

/**
 * @todo Understand and document.
 * @todo Maybe add to the class.
 */
std::map<std::string, const InstructionRecord*> FieldInstructions(){
    // Convert the array to a map that we can query on by mnemonic
    std::map<std::string, const InstructionRecord*> name_to_instruction_records;
    for (size_t i = 0; i < boost::size(kOpcodes); i++)
        name_to_instruction_records[kOpcodes[i].opcode_name] = &kOpcodes[i];
    return name_to_instruction_records;
}

bool FieldDisassembler::ReadOpCodesToPositionOrReturn(
  size_t end_pos, std::vector<float>& point_a, std::vector<float>& point_b
){
    bool is_line = false;
    std::vector<unsigned int> exitAddrs;
    while (stream_->GetPosition() < end_pos){
        uint8 opcode = stream_->ReadU8();
        uint32 full_opcode = 0;
        std::string opcodePrefix;
        switch (opcode){
            // Flow
            CASE_OPCODE(OPCODE::IFUB, "IFUB", FF7CondJumpInstruction, 0, "NBBBB");
            CASE_OPCODE(OPCODE::RET, "RET", FF7ControlFlowInstruction, 0, "");
            CASE_OPCODE(OPCODE::REQ, "REQ", FF7ControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::REQSW, "REQSW", FF7ControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::REQEW, "REQEW", FF7ControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::PREQ, "PREQ", FF7ControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::PRQSW, "PRQSW", FF7ControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::PRQEW, "PRQEW", FF7ControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::RETTO, "RETTO", FF7ControlFlowInstruction, 0, "U");
            CASE_OPCODE(OPCODE::JMPF, "JMPF", FF7UncondJumpInstruction, 0, "B");
            CASE_OPCODE(OPCODE::JMPFL, "JMPFL", FF7UncondJumpInstruction, 0, "w");
            CASE_OPCODE(OPCODE::JMPB, "JMPB", FF7UncondJumpInstruction, 0, "B");
            CASE_OPCODE(OPCODE::JMPBL, "JMPBL", FF7UncondJumpInstruction, 0, "w");
            CASE_OPCODE(OPCODE::IFUBL, "IFUBL", FF7CondJumpInstruction, 0, "NBBBw");
            CASE_OPCODE(OPCODE::IFSW, "IFSW", FF7CondJumpInstruction, 0, "NwwBB");
            CASE_OPCODE(OPCODE::IFSWL, "IFSWL", FF7CondJumpInstruction, 0, "NwwBw");
            CASE_OPCODE(OPCODE::IFUW, "IFUW", FF7CondJumpInstruction, 0, "NwwBB");
            CASE_OPCODE(OPCODE::IFUWL, "IFUWL", FF7CondJumpInstruction, 0, "NwwBw");
            CASE_OPCODE(OPCODE::WAIT, "WAIT", FF7ControlFlowInstruction, 0, "w");
            CASE_OPCODE(OPCODE::IFKEY, "IFKEY", FF7CondJumpInstruction, 0, "wB");
            CASE_OPCODE(OPCODE::IFKEYON, "IFKEYON", FF7CondJumpInstruction, 0, "wB");
            CASE_OPCODE(OPCODE::IFKEYOFF, "IFKEYOFF", FF7CondJumpInstruction, 0, "wB");
            CASE_OPCODE(OPCODE::NOP, "NOP", FF7NoOperationInstruction, 0, "");
            CASE_OPCODE(OPCODE::IFPRTYQ, "IFPRTYQ", FF7CondJumpInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::IFMEMBQ, "IFMEMBQ", FF7CondJumpInstruction, 0, "BB");

            // Module
            CASE_OPCODE(OPCODE::DSKCG, "DSKCG", FF7ModuleInstruction, 0, "B");
            START_SUBOPCODE(OPCODE::SPECIAL)
                CASE_OPCODE(OPCODE_SPECIAL::ARROW, "ARROW", FF7ModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE_SPECIAL::PNAME, "PNAME", FF7ModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE_SPECIAL::GMSPD, "GMSPD", FF7ModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE_SPECIAL::SMSPD, "SMSPD", FF7ModuleInstruction, 0, "BB");
            CASE_OPCODE(OPCODE_SPECIAL::FLMAT, "FLMAT", FF7ModuleInstruction, 0, "");
            CASE_OPCODE(OPCODE_SPECIAL::FLITM, "FLITM", FF7ModuleInstruction, 0, "");
            CASE_OPCODE(OPCODE_SPECIAL::BTLCK, "BTLCK", FF7ModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE_SPECIAL::MVLCK, "MVLCK", FF7ModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE_SPECIAL::SPCNM, "SPCNM", FF7ModuleInstruction, 0, "BB");
            CASE_OPCODE(OPCODE_SPECIAL::RSGLB, "RSGLB", FF7ModuleInstruction, 0, "");
            CASE_OPCODE(OPCODE_SPECIAL::CLITM, "CLITM", FF7ModuleInstruction, 0, "");
            END_SUBOPCODE
                CASE_OPCODE(OPCODE::MINIGAME, "MINIGAME", FF7ModuleInstruction, 0, "wsswBB");
            CASE_OPCODE(OPCODE::BTMD2, "BTMD2", FF7ModuleInstruction, 0, "d");
            CASE_OPCODE(OPCODE::BTRLD, "BTRLD", FF7ModuleInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::BTLTB, "BTLTB", FF7ModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE::MAPJUMP, "MAPJUMP", FF7ModuleInstruction, 0, "wsswB");
            CASE_OPCODE(OPCODE::LSTMP, "LSTMP", FF7ModuleInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::BATTLE, "BATTLE", FF7ModuleInstruction, 0, "Nw");
            CASE_OPCODE(OPCODE::BTLON, "BTLON", FF7ModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE::BTLMD, "BTLMD", FF7ModuleInstruction, 0, "w");
            CASE_OPCODE(OPCODE::MPJPO, "MPJPO", FF7ModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PMJMP, "PMJMP", FF7ModuleInstruction, 0, "w");
            CASE_OPCODE(OPCODE::PMJMP2, "PMJMP2", FF7ModuleInstruction, 0, "");
            CASE_OPCODE(OPCODE::GAMEOVER, "GAMEOVER", FF7ModuleInstruction, 0, "");

            // Math
            CASE_OPCODE(OPCODE::PLUS_, "PLUS!", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::PLUS2_, "PLUS2!", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::MINUS_, "MINUS!", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::MINUS2_, "MINUS2!", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::INC_, "INC!", FF7MathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::INC2_, "INC2!", FF7MathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::DEC_, "DEC!", FF7MathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::DEC2_, "DEC2!", FF7MathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::RDMSD, "RDMSD", FF7MathInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::SETBYTE, "SETBYTE", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::SETWORD, "SETWORD", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::BITON, "BITON", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::BITOFF, "BITOFF", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::BITXOR, "BITXOR", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::PLUS, "PLUS", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::PLUS2, "PLUS2", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::MINUS, "MINUS", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::MINUS2, "MINUS2", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::MUL, "MUL", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::MUL2, "MUL2", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::DIV, "DIV", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::DIV2, "DIV2", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::MOD, "MOD", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::MOD2, "MOD2", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::AND, "AND", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::AND2, "AND2", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::OR, "OR", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::OR2, "OR2", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::XOR, "XOR", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::XOR2, "XOR2", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::INC, "INC", FF7MathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::INC2, "INC2", FF7MathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::DEC, "DEC", FF7MathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::DEC2, "DEC2", FF7MathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::RANDOM, "RANDOM", FF7MathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::LBYTE, "LBYTE", FF7MathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::HBYTE, "HBYTE", FF7MathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::TWOBYTE, "2BYTE", FF7MathInstruction, 0, "NNBBB");
            CASE_OPCODE(OPCODE::SIN, "SIN", FF7MathInstruction, 0, "NNwwwB");
            CASE_OPCODE(OPCODE::COS, "COS", FF7MathInstruction, 0, "NNwwwB");

            // Window
            CASE_OPCODE(OPCODE::TUTOR, "TUTOR", FF7WindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::WCLS, "WCLS", FF7WindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::WSIZW, "WSIZW", FF7WindowInstruction, 0, "Bwwww");
            CASE_OPCODE(OPCODE::WSPCL, "WSPCL", FF7WindowInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::WNUMB, "WNUMB", FF7WindowInstruction, 0, "NBwwB"); // NBdB when N == 0
            CASE_OPCODE(OPCODE::STTIM, "STTIM", FF7WindowInstruction, 0, "NNBBB");
            CASE_OPCODE(OPCODE::MESSAGE, "MESSAGE", FF7WindowInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::MPARA, "MPARA", FF7WindowInstruction, 0, "NBBB");
            CASE_OPCODE(OPCODE::MPRA2, "MPRA2", FF7WindowInstruction, 0, "NBBw");
            CASE_OPCODE(OPCODE::MPNAM, "MPNAM", FF7WindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::ASK, "ASK", FF7WindowInstruction, 0, "NBBBBB");
            CASE_OPCODE(OPCODE::MENU, "MENU", FF7WindowInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::MENU2, "MENU2", FF7WindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::WINDOW, "WINDOW", FF7WindowInstruction, 0, "Bwwww");
            CASE_OPCODE(OPCODE::WMOVE, "WMOVE", FF7WindowInstruction, 0, "Bss");
            CASE_OPCODE(OPCODE::WMODE, "WMODE", FF7WindowInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::WREST, "WREST", FF7WindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::WCLSE, "WCLSE", FF7WindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::WROW, "WROW", FF7WindowInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::GWCOL, "GWCOL", FF7WindowInstruction, 0, "NNBBBB");
            CASE_OPCODE(OPCODE::SWCOL, "SWCOL", FF7WindowInstruction, 0, "NNBBBB");

            // Party
            CASE_OPCODE(OPCODE::SPTYE, "SPTYE", FF7PartyInstruction, 0, "NNBBB");
            CASE_OPCODE(OPCODE::GTPYE, "GTPYE", FF7PartyInstruction, 0, "NNBBB");
            CASE_OPCODE(OPCODE::GOLDU, "GOLDU", FF7PartyInstruction, 0, "Nww"); // Nd when N == 0
            CASE_OPCODE(OPCODE::GOLDD, "GOLDD", FF7PartyInstruction, 0, "Nww"); // Nd when N == 0
            CASE_OPCODE(OPCODE::CHGLD, "CHGLD", FF7PartyInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::HMPMAX1, "HMPMAX1", FF7PartyInstruction, 0, "");
            CASE_OPCODE(OPCODE::HMPMAX2, "HMPMAX2", FF7PartyInstruction, 0, "");
            CASE_OPCODE(OPCODE::MHMMX, "MHMMX", FF7PartyInstruction, 0, "");
            CASE_OPCODE(OPCODE::HMPMAX3, "HMPMAX3", FF7PartyInstruction, 0, "");
            CASE_OPCODE(OPCODE::MPU, "MPU", FF7PartyInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::MPD, "MPD", FF7PartyInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::HPU, "HPU", FF7PartyInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::HPD, "HPD", FF7PartyInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::STITM, "STITM", FF7PartyInstruction, 0, "NwB");
            CASE_OPCODE(OPCODE::DLITM, "DLITM", FF7PartyInstruction, 0, "NwB");
            CASE_OPCODE(OPCODE::CKITM, "CKITM", FF7PartyInstruction, 0, "NwB");
            CASE_OPCODE(OPCODE::SMTRA, "SMTRA", FF7PartyInstruction, 0, "NNBBBB");
            CASE_OPCODE(OPCODE::DMTRA, "DMTRA", FF7PartyInstruction, 0, "NNBBBBB");
            CASE_OPCODE(OPCODE::CMTRA, "CMTRA", FF7PartyInstruction, 0, "NNNBBBBBB");
            CASE_OPCODE(OPCODE::GETPC, "GETPC", FF7PartyInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::PRTYP, "PRTYP", FF7PartyInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PRTYM, "PRTYM", FF7PartyInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PRTYE, "PRTYE", FF7PartyInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::MMBUD, "MMBUD", FF7PartyInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::MMBLK, "MMBLK", FF7PartyInstruction, 0, "B");
            CASE_OPCODE(OPCODE::MMBUK, "MMBUK", FF7PartyInstruction, 0, "B");

            // Model
            CASE_OPCODE(OPCODE::JOIN, "JOIN", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::SPLIT, "SPLIT", FF7ModelInstruction, 0, "NNNssBssBB");
            CASE_OPCODE(OPCODE::BLINK, "BLINK", FF7ModelInstruction, 0, "B");
            OPCODE_BASE(OPCODE::KAWAI){
                int length = this->stream_->ReadU8();
                assert(length >= 3);
                std::ostringstream paramStream;
                for (int i = 3; i < length; ++ i) paramStream << "B";
                auto parameters = paramStream.str();

                opcode = this->stream_->ReadU8();
                switch (opcode){
                    CASE_OPCODE(
                        OPCODE_KAWAI::EYETX, "EYETX", FF7ModelInstruction, 0, parameters.c_str()
                    ); // was BBBB
                    CASE_OPCODE(
                        OPCODE_KAWAI::TRNSP, "TRNSP", FF7ModelInstruction, 0, parameters.c_str()
                    ); // was B
                    CASE_OPCODE(
                        OPCODE_KAWAI::AMBNT, "AMBNT", FF7ModelInstruction, 0, parameters.c_str()
                    ); // was BBBBBBB
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown03, "Unknown03",
                        FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown04, "Unknown04",
                        FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown05, "Unknown05",
                        FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::LIGHT, "LIGHT",FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown07, "Unknown07",
                        FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown08, "Unknown08",
                        FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown09, "Unknown09",
                        FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::SBOBJ, "SBOBJ",
                        FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                      OPCODE_KAWAI::Unknown0B, "Unknown0B",
                      FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown0C, "Unknown0C",
                        FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::SHINE, "SHINE", FF7ModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::RESET, "RESET", FF7ModelInstruction, 0, parameters.c_str()
                    );
                default:
                    throw UnknownSubOpcodeException(this->address_, opcode);
                }
                INC_ADDR;
                INC_ADDR;
            }
            OPCODE_END
            CASE_OPCODE(OPCODE::KAWIW, "KAWIW", FF7ModelInstruction, 0, "");
            CASE_OPCODE(OPCODE::PMOVA, "PMOVA", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PDIRA, "PDIRA", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PTURA, "PTURA", FF7ModelInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::PGTDR, "PGTDR", FF7ModelInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::PXYZI, "PXYZI", FF7ModelInstruction, 0, "NNBBBBB");
            CASE_OPCODE(OPCODE::TLKON, "TLKON", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PC, "PC", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::opCodeCHAR, "CHAR", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::DFANM, "DFANM", FF7ModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::ANIME1, "ANIME1", FF7ModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::VISI, "VISI", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::XYZI, "XYZI", FF7ModelInstruction, 0, "NNsssw");
            CASE_OPCODE(OPCODE::XYI, "XYI", FF7ModelInstruction, 0, "NNssw");
            CASE_OPCODE(OPCODE::XYZ, "XYZ", FF7ModelInstruction, 0, "NNsss");
            CASE_OPCODE(OPCODE::MOVE, "MOVE", FF7ModelInstruction, 0, "Nss");
            CASE_OPCODE(OPCODE::CMOVE, "CMOVE", FF7ModelInstruction, 0, "Nss");
            CASE_OPCODE(OPCODE::MOVA, "MOVA", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::TURA, "TURA", FF7ModelInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::ANIMW, "ANIMW", FF7ModelInstruction, 0, "");
            CASE_OPCODE(OPCODE::FMOVE, "FMOVE", FF7ModelInstruction, 0, "Nss");
            CASE_OPCODE(OPCODE::ANIME2, "ANIME2", FF7ModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::ANIM_1, "ANIM!1", FF7ModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::CANIM1, "CANIM1", FF7ModelInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::CANM_1, "CANM!1", FF7ModelInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::MSPED, "MSPED", FF7ModelInstruction, 0, "Nw");
            CASE_OPCODE(OPCODE::DIR, "DIR", FF7ModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::TURNGEN, "TURNGEN", FF7ModelInstruction, 0, "NBBBB");
            CASE_OPCODE(OPCODE::TURN, "TURN", FF7ModelInstruction, 0, "NBBBB");
            CASE_OPCODE(OPCODE::DIRA, "DIRA", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::GETDIR, "GETDIR", FF7ModelInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::GETAXY, "GETAXY", FF7ModelInstruction, 0, "NBBB");
            CASE_OPCODE(OPCODE::GETAI, "GETAI", FF7ModelInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::ANIM_2, "ANIM!2", FF7ModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::CANIM2, "CANIM2", FF7ModelInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::CANM_2, "CANM!2", FF7ModelInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::ASPED, "ASPED", FF7ModelInstruction, 0, "Nw");
            CASE_OPCODE(OPCODE::CC, "CC", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::JUMP, "JUMP", FF7ModelInstruction, 0, "NNssww");
            CASE_OPCODE(OPCODE::AXYZI, "AXYZI", FF7ModelInstruction, 0, "NNBBBBB");
            CASE_OPCODE(OPCODE::LADER, "LADER", FF7ModelInstruction, 0, "NNssswBBBB");
            CASE_OPCODE(OPCODE::OFST, "OFST", FF7ModelInstruction, 0, "NNBsssw");
            CASE_OPCODE(OPCODE::OFSTW, "OFSTW", FF7ModelInstruction, 0, "");
            CASE_OPCODE(OPCODE::TALKR, "TALKR", FF7ModelInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::SLIDR, "SLIDR", FF7ModelInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::SOLID, "SOLID", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::TLKR2, "TLKR2", FF7ModelInstruction, 0, "Nw");
            CASE_OPCODE(OPCODE::SLDR2, "SLDR2", FF7ModelInstruction, 0, "Nw");
            CASE_OPCODE(OPCODE::CCANM, "CCANM", FF7ModelInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::FCFIX, "FCFIX", FF7ModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::ANIMB, "ANIMB", FF7ModelInstruction, 0, "");
            CASE_OPCODE(OPCODE::TURNW, "TURNW", FF7ModelInstruction, 0, "");

            // Walkmesh
            CASE_OPCODE(OPCODE::SLIP, "SLIP", FF7WalkmeshInstruction, 0, "B");
            CASE_OPCODE(OPCODE::UC, "UC", FF7WalkmeshInstruction, 0, "B");
            CASE_OPCODE(OPCODE::IDLCK, "IDLCK", FF7WalkmeshInstruction, 0, "wB");
            CASE_OPCODE(OPCODE::LINON, "LINON", FF7WalkmeshInstruction, 0, "B");
            CASE_OPCODE(OPCODE::SLINE, "SLINE", FF7WalkmeshInstruction, 0, "NNNssssss");
            //CASE_OPCODE(OPCODE::LINE, "LINE", FF7WalkmeshInstruction, 0, "ssssss");
            // LINE is done like this to save opcode params to the function out params.
            case OPCODE::LINE:
                is_line = true;
                full_opcode = (full_opcode << 8) + OPCODE::LINE;
                this->_insts.push_back(new FF7WalkmeshInstruction());
                (this->_insts.back())->_opcode = full_opcode;
                (this->_insts.back())->_address = this->address_;
                (this->_insts.back())->_stackChange = 0;
                (this->_insts.back())->_name = opcodePrefix + std::string("LINE");
                (this->_insts.back())->_codeGenData = "";
                this->readParams((this->_insts.back()), "ssssss");
                point_a[0] = this->_insts.back()->_params[0]->getSigned();
                point_a[1] = this->_insts.back()->_params[1]->getSigned();
                point_a[2] = this->_insts.back()->_params[2]->getSigned();
                point_b[0] = this->_insts.back()->_params[3]->getSigned();
                point_b[1] = this->_insts.back()->_params[4]->getSigned();
                point_b[2] = this->_insts.back()->_params[5]->getSigned();
                break;

            // Backgnd
            CASE_OPCODE(OPCODE::BGPDH, "BGPDH", FF7BackgroundInstruction, 0, "NBs");
            CASE_OPCODE(OPCODE::BGSCR, "BGSCR", FF7BackgroundInstruction, 0, "NBss");
            CASE_OPCODE(OPCODE::MPPAL, "MPPAL", FF7BackgroundInstruction, 0, "NNNBBBBBBB");
            CASE_OPCODE(OPCODE::BGON, "BGON", FF7BackgroundInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::BGOFF, "BGOFF", FF7BackgroundInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::BGROL, "BGROL", FF7BackgroundInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::BGROL2, "BGROL2", FF7BackgroundInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::BGCLR, "BGCLR", FF7BackgroundInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::STPAL, "STPAL", FF7BackgroundInstruction, 0, "NBBB");
            CASE_OPCODE(OPCODE::LDPAL, "LDPAL", FF7BackgroundInstruction, 0, "NBBB");
            CASE_OPCODE(OPCODE::CPPAL, "CPPAL", FF7BackgroundInstruction, 0, "NBBB");
            CASE_OPCODE(OPCODE::RTPAL, "RTPAL", FF7BackgroundInstruction, 0, "NNBBBB");
            CASE_OPCODE(OPCODE::ADPAL, "ADPAL", FF7BackgroundInstruction, 0, "NNNBBBBBB");
            CASE_OPCODE(OPCODE::MPPAL2, "MPPAL2", FF7BackgroundInstruction, 0, "NNNBBBBBB");
            CASE_OPCODE(OPCODE::STPLS, "STPLS", FF7BackgroundInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::LDPLS, "LDPLS", FF7BackgroundInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::CPPAL2, "CPPAL2", FF7BackgroundInstruction, 0, "BBBBBBB");
            CASE_OPCODE(OPCODE::RTPAL2, "RTPAL2", FF7BackgroundInstruction, 0, "BBBBBBB");
            CASE_OPCODE(OPCODE::ADPAL2, "ADPAL2", FF7BackgroundInstruction, 0, "BBBBBBBBBB");

            // Camera
            CASE_OPCODE(OPCODE::NFADE, "NFADE", FF7CameraInstruction, 0, "NNBBBBBB");
            CASE_OPCODE(OPCODE::SHAKE, "SHAKE", FF7CameraInstruction, 0, "BBBBBBB");
            CASE_OPCODE(OPCODE::SCRLO, "SCRLO", FF7CameraInstruction, 0, "B");
            CASE_OPCODE(OPCODE::SCRLC, "SCRLC", FF7CameraInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::SCRLA, "SCRLA", FF7CameraInstruction, 0, "NwBB");
            CASE_OPCODE(OPCODE::SCR2D, "SCR2D", FF7CameraInstruction, 0, "Nss");
            CASE_OPCODE(OPCODE::SCRCC, "SCRCC", FF7CameraInstruction, 0, "");
            CASE_OPCODE(OPCODE::SCR2DC, "SCR2DC", FF7CameraInstruction, 0, "NNssw");
            CASE_OPCODE(OPCODE::SCRLW, "SCRLW", FF7CameraInstruction, 0, "");
            CASE_OPCODE(OPCODE::SCR2DL, "SCR2DL", FF7CameraInstruction, 0, "NNssw");
            CASE_OPCODE(OPCODE::VWOFT, "VWOFT", FF7CameraInstruction, 0, "NssB");
            CASE_OPCODE(OPCODE::FADE, "FADE", FF7CameraInstruction, 0, "NNBBBBBB");
            CASE_OPCODE(OPCODE::FADEW, "FADEW", FF7CameraInstruction, 0, "");
            CASE_OPCODE(OPCODE::SCRLP, "SCRLP", FF7CameraInstruction, 0, "NwBB");
            CASE_OPCODE(OPCODE::MVCAM, "MVCAM", FF7CameraInstruction, 0, "B");

            // AV
            CASE_OPCODE(OPCODE::BGMOVIE, "BGMOVIE", FF7AudioVideoInstruction, 0, "B");
            CASE_OPCODE(OPCODE::AKAO2, "AKAO2", FF7AudioVideoInstruction, 0, "NNNBwwwww");
            CASE_OPCODE(OPCODE::MUSIC, "MUSIC", FF7AudioVideoInstruction, 0, "B");
            CASE_OPCODE(OPCODE::SOUND, "SOUND", FF7AudioVideoInstruction, 0, "NwB");
            CASE_OPCODE(OPCODE::AKAO, "AKAO", FF7AudioVideoInstruction, 0, "NNNBBwwww");
            CASE_OPCODE(OPCODE::MUSVT, "MUSVT", FF7AudioVideoInstruction, 0, "B");
            CASE_OPCODE(OPCODE::MUSVM, "MUSVM", FF7AudioVideoInstruction, 0, "B");
            CASE_OPCODE(OPCODE::MULCK, "MULCK", FF7AudioVideoInstruction, 0, "B");
            CASE_OPCODE(OPCODE::BMUSC, "BMUSC", FF7AudioVideoInstruction, 0, "B");
            CASE_OPCODE(OPCODE::CHMPH, "CHMPH", FF7AudioVideoInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::PMVIE, "PMVIE", FF7AudioVideoInstruction, 0, "B");
            CASE_OPCODE(OPCODE::MOVIE, "MOVIE", FF7AudioVideoInstruction, 0, "");
            CASE_OPCODE(OPCODE::MVIEF, "MVIEF", FF7AudioVideoInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::FMUSC, "FMUSC", FF7AudioVideoInstruction, 0, "B");
            CASE_OPCODE(OPCODE::CMUSC, "CMUSC", FF7AudioVideoInstruction, 0, "BBBBBBB");
            CASE_OPCODE(OPCODE::CHMST, "CHMST", FF7AudioVideoInstruction, 0, "NB");

            // Uncat
            CASE_OPCODE(OPCODE::MPDSP, "MPDSP", FF7UncategorizedInstruction, 0, "B");
            CASE_OPCODE(OPCODE::SETX, "SETX", FF7UncategorizedInstruction, 0, "BBBBBB");
            CASE_OPCODE(OPCODE::GETX, "GETX", FF7UncategorizedInstruction, 0, "BBBBBB");
            CASE_OPCODE(OPCODE::SEARCHX, "SEARCHX", FF7UncategorizedInstruction, 0, "BBBBBBBBBB");

            default:
                throw UnknownOpcodeException(this->address_, opcode);
        }
        INC_ADDR;

        // Is it within an "if" statement tracking?
        InstPtr i = this->_insts.back();
        if (i->isCondJump()) exitAddrs.push_back(i->getDestAddress());
        if (!exitAddrs.empty())
            if (i->_address == exitAddrs.back()) exitAddrs.pop_back();

        // Only bail if its the first RET that isn't within an "if" block.
        if (full_opcode == OPCODE::RET && exitAddrs.empty()) return is_line;
    }
    return is_line;
}
