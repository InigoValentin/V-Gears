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
#include "common/Lzs.h"
#include "decompiler/decompiler_engine.h"
#include "decompiler/field/FieldDisassembler.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/instruction/FieldBackgroundInstruction.h"
#include "decompiler/field/instruction/FieldCameraInstruction.h"
#include "decompiler/field/instruction/FieldCondJumpInstruction.h"
#include "decompiler/field/instruction/FieldControlFlowInstruction.h"
#include "decompiler/field/instruction/FieldMathInstruction.h"
#include "decompiler/field/instruction/FieldMediaInstruction.h"
#include "decompiler/field/instruction/FieldModelInstruction.h"
#include "decompiler/field/instruction/FieldModuleInstruction.h"
#include "decompiler/field/instruction/FieldNoOperationInstruction.h"
#include "decompiler/field/instruction/FieldPartyInstruction.h"
#include "decompiler/field/instruction/FieldUncategorizedInstruction.h"
#include "decompiler/field/instruction/FieldUncondJumpInstruction.h"
#include "decompiler/field/instruction/FieldWalkmeshInstruction.h"
#include "decompiler/field/instruction/FieldWindowInstruction.h"

const int FieldDisassembler::MAGIC(0x0502);

const int FieldDisassembler::NUM_SECTIONS(7);

FieldDisassembler::FieldDisassembler(
  SUDM::IScriptFormatter& formatter, FieldEngine* engine, InstVec& insts,
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
  SUDM::IScriptFormatter& formatter, FieldEngine *engine, InstVec &insts
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
    { 1, OPCODE::RET, "RET", "", FieldControlFlowInstruction::Create },
    { 1, OPCODE::REQ, "REQ", "BU", FieldControlFlowInstruction::Create },
    { 1, OPCODE::REQSW, "REQSW", "BU", FieldControlFlowInstruction::Create },
    { 1, OPCODE::REQEW, "REQEW", "BU", FieldControlFlowInstruction::Create },
    { 1, OPCODE::NOP, "NOP", "", FieldNoOperationInstruction::Create },
    { 1, OPCODE::IFUB, "IFUB", "NNBBBL", FieldNoOperationInstruction::Create }
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
            CASE_OPCODE(OPCODE::IFUB, "IFUB", FieldCondJumpInstruction, 0, "NBBBB");
            /*case OPCODE::IFUB:
                full_opcode = (full_opcode << 8) + OPCODE::IFUB;
                this->_insts.push_back(new FieldCondJumpInstruction());
                (this->_insts.back())->_opcode = full_opcode;
                (this->_insts.back())->_address = this->address_;
                (this->_insts.back())->_stackChange = 0;
                (this->_insts.back())->_name = opcodePrefix + std::string("IFUB");
                (this->_insts.back())->_codeGenData = "";
                this->readParams((this->_insts.back()), "NBBBB");
                break;*/
            CASE_OPCODE(OPCODE::RET, "RET", FieldControlFlowInstruction, 0, "");
            CASE_OPCODE(OPCODE::REQ, "REQ", FieldControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::REQSW, "REQSW", FieldControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::REQEW, "REQEW", FieldControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::PREQ, "PREQ", FieldControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::PRQSW, "PRQSW", FieldControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::PRQEW, "PRQEW", FieldControlFlowInstruction, 0, "BU");
            CASE_OPCODE(OPCODE::RETTO, "RETTO", FieldControlFlowInstruction, 0, "U");
            CASE_OPCODE(OPCODE::JMPF, "JMPF", FieldUncondJumpInstruction, 0, "B");
            CASE_OPCODE(OPCODE::JMPFL, "JMPFL", FieldUncondJumpInstruction, 0, "w");
            CASE_OPCODE(OPCODE::JMPB, "JMPB", FieldUncondJumpInstruction, 0, "B");
            CASE_OPCODE(OPCODE::JMPBL, "JMPBL", FieldUncondJumpInstruction, 0, "w");
            CASE_OPCODE(OPCODE::IFUBL, "IFUBL", FieldCondJumpInstruction, 0, "NBBBw");
            CASE_OPCODE(OPCODE::IFSW, "IFSW", FieldCondJumpInstruction, 0, "NwwBB");
            CASE_OPCODE(OPCODE::IFSWL, "IFSWL", FieldCondJumpInstruction, 0, "NwwBw");
            CASE_OPCODE(OPCODE::IFUW, "IFUW", FieldCondJumpInstruction, 0, "NwwBB");
            CASE_OPCODE(OPCODE::IFUWL, "IFUWL", FieldCondJumpInstruction, 0, "NwwBw");
            CASE_OPCODE(OPCODE::WAIT, "WAIT", FieldControlFlowInstruction, 0, "w");
            CASE_OPCODE(OPCODE::IFKEY, "IFKEY", FieldCondJumpInstruction, 0, "wB");
            CASE_OPCODE(OPCODE::IFKEYON, "IFKEYON", FieldCondJumpInstruction, 0, "wB");
            CASE_OPCODE(OPCODE::IFKEYOFF, "IFKEYOFF", FieldCondJumpInstruction, 0, "wB");
            CASE_OPCODE(OPCODE::NOP, "NOP", FieldNoOperationInstruction, 0, "");
            CASE_OPCODE(OPCODE::IFPRTYQ, "IFPRTYQ", FieldCondJumpInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::IFMEMBQ, "IFMEMBQ", FieldCondJumpInstruction, 0, "BB");

            // Module
            CASE_OPCODE(OPCODE::DSKCG, "DSKCG", FieldModuleInstruction, 0, "B");
            START_SUBOPCODE(OPCODE::SPECIAL)
                CASE_OPCODE(OPCODE_SPECIAL::ARROW, "ARROW", FieldModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE_SPECIAL::PNAME, "PNAME", FieldModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE_SPECIAL::GMSPD, "GMSPD", FieldModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE_SPECIAL::SMSPD, "SMSPD", FieldModuleInstruction, 0, "BB");
            CASE_OPCODE(OPCODE_SPECIAL::FLMAT, "FLMAT", FieldModuleInstruction, 0, "");
            CASE_OPCODE(OPCODE_SPECIAL::FLITM, "FLITM", FieldModuleInstruction, 0, "");
            CASE_OPCODE(OPCODE_SPECIAL::BTLCK, "BTLCK", FieldModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE_SPECIAL::MVLCK, "MVLCK", FieldModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE_SPECIAL::SPCNM, "SPCNM", FieldModuleInstruction, 0, "BB");
            CASE_OPCODE(OPCODE_SPECIAL::RSGLB, "RSGLB", FieldModuleInstruction, 0, "");
            CASE_OPCODE(OPCODE_SPECIAL::CLITM, "CLITM", FieldModuleInstruction, 0, "");
            END_SUBOPCODE
                CASE_OPCODE(OPCODE::MINIGAME, "MINIGAME", FieldModuleInstruction, 0, "wsswBB");
            CASE_OPCODE(OPCODE::BTMD2, "BTMD2", FieldModuleInstruction, 0, "d");
            CASE_OPCODE(OPCODE::BTRLD, "BTRLD", FieldModuleInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::BTLTB, "BTLTB", FieldModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE::MAPJUMP, "MAPJUMP", FieldModuleInstruction, 0, "wsswB");
            CASE_OPCODE(OPCODE::LSTMP, "LSTMP", FieldModuleInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::BATTLE, "BATTLE", FieldModuleInstruction, 0, "Nw");
            CASE_OPCODE(OPCODE::BTLON, "BTLON", FieldModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE::BTLMD, "BTLMD", FieldModuleInstruction, 0, "w");
            CASE_OPCODE(OPCODE::MPJPO, "MPJPO", FieldModuleInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PMJMP, "PMJMP", FieldModuleInstruction, 0, "w");
            CASE_OPCODE(OPCODE::PMJMP2, "PMJMP2", FieldModuleInstruction, 0, "");
            CASE_OPCODE(OPCODE::GAMEOVER, "GAMEOVER", FieldModuleInstruction, 0, "");

            // Math
            CASE_OPCODE(OPCODE::PLUS_, "PLUS!", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::PLUS2_, "PLUS2!", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::MINUS_, "MINUS!", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::MINUS2_, "MINUS2!", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::INC_, "INC!", FieldMathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::INC2_, "INC2!", FieldMathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::DEC_, "DEC!", FieldMathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::DEC2_, "DEC2!", FieldMathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::RDMSD, "RDMSD", FieldMathInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::SETBYTE, "SETBYTE", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::SETWORD, "SETWORD", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::BITON, "BITON", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::BITOFF, "BITOFF", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::BITXOR, "BITXOR", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::PLUS, "PLUS", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::PLUS2, "PLUS2", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::MINUS, "MINUS", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::MINUS2, "MINUS2", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::MUL, "MUL", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::MUL2, "MUL2", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::DIV, "DIV", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::DIV2, "DIV2", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::MOD, "MOD", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::MOD2, "MOD2", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::AND, "AND", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::AND2, "AND2", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::OR, "OR", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::OR2, "OR2", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::XOR, "XOR", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::XOR2, "XOR2", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::INC, "INC", FieldMathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::INC2, "INC2", FieldMathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::DEC, "DEC", FieldMathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::DEC2, "DEC2", FieldMathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::RANDOM, "RANDOM", FieldMathInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::LBYTE, "LBYTE", FieldMathInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::HBYTE, "HBYTE", FieldMathInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::TWOBYTE, "2BYTE", FieldMathInstruction, 0, "NNBBB");
            CASE_OPCODE(OPCODE::SIN, "SIN", FieldMathInstruction, 0, "NNwwwB");
            CASE_OPCODE(OPCODE::COS, "COS", FieldMathInstruction, 0, "NNwwwB");

            // Window
            CASE_OPCODE(OPCODE::TUTOR, "TUTOR", FieldWindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::WCLS, "WCLS", FieldWindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::WSIZW, "WSIZW", FieldWindowInstruction, 0, "Bwwww");
            CASE_OPCODE(OPCODE::WSPCL, "WSPCL", FieldWindowInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::WNUMB, "WNUMB", FieldWindowInstruction, 0, "NBwwB"); // NBdB when N == 0
            CASE_OPCODE(OPCODE::STTIM, "STTIM", FieldWindowInstruction, 0, "NNBBB");
            CASE_OPCODE(OPCODE::MESSAGE, "MESSAGE", FieldWindowInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::MPARA, "MPARA", FieldWindowInstruction, 0, "NBBB");
            CASE_OPCODE(OPCODE::MPRA2, "MPRA2", FieldWindowInstruction, 0, "NBBw");
            CASE_OPCODE(OPCODE::MPNAM, "MPNAM", FieldWindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::ASK, "ASK", FieldWindowInstruction, 0, "NBBBBB");
            CASE_OPCODE(OPCODE::MENU, "MENU", FieldWindowInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::MENU2, "MENU2", FieldWindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::WINDOW, "WINDOW", FieldWindowInstruction, 0, "Bwwww");
            CASE_OPCODE(OPCODE::WMOVE, "WMOVE", FieldWindowInstruction, 0, "Bss");
            CASE_OPCODE(OPCODE::WMODE, "WMODE", FieldWindowInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::WREST, "WREST", FieldWindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::WCLSE, "WCLSE", FieldWindowInstruction, 0, "B");
            CASE_OPCODE(OPCODE::WROW, "WROW", FieldWindowInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::GWCOL, "GWCOL", FieldWindowInstruction, 0, "NNBBBB");
            CASE_OPCODE(OPCODE::SWCOL, "SWCOL", FieldWindowInstruction, 0, "NNBBBB");

            // Party
            CASE_OPCODE(OPCODE::SPTYE, "SPTYE", FieldPartyInstruction, 0, "NNBBB");
            CASE_OPCODE(OPCODE::GTPYE, "GTPYE", FieldPartyInstruction, 0, "NNBBB");
            CASE_OPCODE(OPCODE::GOLDU, "GOLDU", FieldPartyInstruction, 0, "Nww"); // Nd when N == 0
            CASE_OPCODE(OPCODE::GOLDD, "GOLDD", FieldPartyInstruction, 0, "Nww"); // Nd when N == 0
            CASE_OPCODE(OPCODE::CHGLD, "CHGLD", FieldPartyInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::HMPMAX1, "HMPMAX1", FieldPartyInstruction, 0, "");
            CASE_OPCODE(OPCODE::HMPMAX2, "HMPMAX2", FieldPartyInstruction, 0, "");
            CASE_OPCODE(OPCODE::MHMMX, "MHMMX", FieldPartyInstruction, 0, "");
            CASE_OPCODE(OPCODE::HMPMAX3, "HMPMAX3", FieldPartyInstruction, 0, "");
            CASE_OPCODE(OPCODE::MPU, "MPU", FieldPartyInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::MPD, "MPD", FieldPartyInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::HPU, "HPU", FieldPartyInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::HPD, "HPD", FieldPartyInstruction, 0, "NBw");
            CASE_OPCODE(OPCODE::STITM, "STITM", FieldPartyInstruction, 0, "NwB");
            CASE_OPCODE(OPCODE::DLITM, "DLITM", FieldPartyInstruction, 0, "NwB");
            CASE_OPCODE(OPCODE::CKITM, "CKITM", FieldPartyInstruction, 0, "NwB");
            CASE_OPCODE(OPCODE::SMTRA, "SMTRA", FieldPartyInstruction, 0, "NNBBBB");
            CASE_OPCODE(OPCODE::DMTRA, "DMTRA", FieldPartyInstruction, 0, "NNBBBBB");
            CASE_OPCODE(OPCODE::CMTRA, "CMTRA", FieldPartyInstruction, 0, "NNNBBBBBB");
            CASE_OPCODE(OPCODE::GETPC, "GETPC", FieldPartyInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::PRTYP, "PRTYP", FieldPartyInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PRTYM, "PRTYM", FieldPartyInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PRTYE, "PRTYE", FieldPartyInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::MMBUD, "MMBUD", FieldPartyInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::MMBLK, "MMBLK", FieldPartyInstruction, 0, "B");
            CASE_OPCODE(OPCODE::MMBUK, "MMBUK", FieldPartyInstruction, 0, "B");

            // Model
            CASE_OPCODE(OPCODE::JOIN, "JOIN", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::SPLIT, "SPLIT", FieldModelInstruction, 0, "NNNssBssBB");
            CASE_OPCODE(OPCODE::BLINK, "BLINK", FieldModelInstruction, 0, "B");
            OPCODE_BASE(OPCODE::KAWAI){
                int length = this->stream_->ReadU8();
                assert(length >= 3);
                std::ostringstream paramStream;
                for (int i = 3; i < length; ++ i) paramStream << "B";
                auto parameters = paramStream.str();

                opcode = this->stream_->ReadU8();
                switch (opcode){
                    CASE_OPCODE(
                        OPCODE_KAWAI::EYETX, "EYETX", FieldModelInstruction, 0, parameters.c_str()
                    ); // was BBBB
                    CASE_OPCODE(
                        OPCODE_KAWAI::TRNSP, "TRNSP", FieldModelInstruction, 0, parameters.c_str()
                    ); // was B
                    CASE_OPCODE(
                        OPCODE_KAWAI::AMBNT, "AMBNT", FieldModelInstruction, 0, parameters.c_str()
                    ); // was BBBBBBB
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown03, "Unknown03",
                        FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown04, "Unknown04",
                        FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown05, "Unknown05",
                        FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::LIGHT, "LIGHT",FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown07, "Unknown07",
                        FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown08, "Unknown08",
                        FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown09, "Unknown09",
                        FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::SBOBJ, "SBOBJ",
                        FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                      OPCODE_KAWAI::Unknown0B, "Unknown0B",
                      FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::Unknown0C, "Unknown0C",
                        FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::SHINE, "SHINE", FieldModelInstruction, 0, parameters.c_str()
                    );
                    CASE_OPCODE(
                        OPCODE_KAWAI::RESET, "RESET", FieldModelInstruction, 0, parameters.c_str()
                    );
                default:
                    throw UnknownSubOpcodeException(this->address_, opcode);
                }
                INC_ADDR;
                INC_ADDR;
            }
            OPCODE_END
            CASE_OPCODE(OPCODE::KAWIW, "KAWIW", FieldModelInstruction, 0, "");
            CASE_OPCODE(OPCODE::PMOVA, "PMOVA", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PDIRA, "PDIRA", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PTURA, "PTURA", FieldModelInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::PGTDR, "PGTDR", FieldModelInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::PXYZI, "PXYZI", FieldModelInstruction, 0, "NNBBBBB");
            CASE_OPCODE(OPCODE::TLKON, "TLKON", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::PC, "PC", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::opCodeCHAR, "CHAR", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::DFANM, "DFANM", FieldModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::ANIME1, "ANIME1", FieldModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::VISI, "VISI", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::XYZI, "XYZI", FieldModelInstruction, 0, "NNsssw");
            CASE_OPCODE(OPCODE::XYI, "XYI", FieldModelInstruction, 0, "NNssw");
            CASE_OPCODE(OPCODE::XYZ, "XYZ", FieldModelInstruction, 0, "NNsss");
            CASE_OPCODE(OPCODE::MOVE, "MOVE", FieldModelInstruction, 0, "Nss");
            CASE_OPCODE(OPCODE::CMOVE, "CMOVE", FieldModelInstruction, 0, "Nss");
            CASE_OPCODE(OPCODE::MOVA, "MOVA", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::TURA, "TURA", FieldModelInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::ANIMW, "ANIMW", FieldModelInstruction, 0, "");
            CASE_OPCODE(OPCODE::FMOVE, "FMOVE", FieldModelInstruction, 0, "Nss");
            CASE_OPCODE(OPCODE::ANIME2, "ANIME2", FieldModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::ANIM_1, "ANIM!1", FieldModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::CANIM1, "CANIM1", FieldModelInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::CANM_1, "CANM!1", FieldModelInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::MSPED, "MSPED", FieldModelInstruction, 0, "Nw");
            CASE_OPCODE(OPCODE::DIR, "DIR", FieldModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::TURNGEN, "TURNGEN", FieldModelInstruction, 0, "NBBBB");
            CASE_OPCODE(OPCODE::TURN, "TURN", FieldModelInstruction, 0, "NBBBB");
            CASE_OPCODE(OPCODE::DIRA, "DIRA", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::GETDIR, "GETDIR", FieldModelInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::GETAXY, "GETAXY", FieldModelInstruction, 0, "NBBB");
            CASE_OPCODE(OPCODE::GETAI, "GETAI", FieldModelInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::ANIM_2, "ANIM!2", FieldModelInstruction, 0, "BB");
            CASE_OPCODE(OPCODE::CANIM2, "CANIM2", FieldModelInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::CANM_2, "CANM!2", FieldModelInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::ASPED, "ASPED", FieldModelInstruction, 0, "Nw");
            CASE_OPCODE(OPCODE::CC, "CC", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::JUMP, "JUMP", FieldModelInstruction, 0, "NNssww");
            CASE_OPCODE(OPCODE::AXYZI, "AXYZI", FieldModelInstruction, 0, "NNBBBBB");
            CASE_OPCODE(OPCODE::LADER, "LADER", FieldModelInstruction, 0, "NNssswBBBB");
            CASE_OPCODE(OPCODE::OFST, "OFST", FieldModelInstruction, 0, "NNBsssw");
            CASE_OPCODE(OPCODE::OFSTW, "OFSTW", FieldModelInstruction, 0, "");
            CASE_OPCODE(OPCODE::TALKR, "TALKR", FieldModelInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::SLIDR, "SLIDR", FieldModelInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::SOLID, "SOLID", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::TLKR2, "TLKR2", FieldModelInstruction, 0, "Nw");
            CASE_OPCODE(OPCODE::SLDR2, "SLDR2", FieldModelInstruction, 0, "Nw");
            CASE_OPCODE(OPCODE::CCANM, "CCANM", FieldModelInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::FCFIX, "FCFIX", FieldModelInstruction, 0, "B");
            CASE_OPCODE(OPCODE::ANIMB, "ANIMB", FieldModelInstruction, 0, "");
            CASE_OPCODE(OPCODE::TURNW, "TURNW", FieldModelInstruction, 0, "");

            // Walkmesh
            CASE_OPCODE(OPCODE::SLIP, "SLIP", FieldWalkmeshInstruction, 0, "B");
            CASE_OPCODE(OPCODE::UC, "UC", FieldWalkmeshInstruction, 0, "B");
            CASE_OPCODE(OPCODE::IDLCK, "IDLCK", FieldWalkmeshInstruction, 0, "wB");
            CASE_OPCODE(OPCODE::LINON, "LINON", FieldWalkmeshInstruction, 0, "B");
            CASE_OPCODE(OPCODE::SLINE, "SLINE", FieldWalkmeshInstruction, 0, "NNNssssss");
            //CASE_OPCODE(OPCODE::LINE, "LINE", FieldWalkmeshInstruction, 0, "ssssss");
            // LINE is done like this to save opcode params to the function out params.
            case OPCODE::LINE:
                is_line = true;
                full_opcode = (full_opcode << 8) + OPCODE::LINE;
                this->_insts.push_back(new FieldWalkmeshInstruction());
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
            CASE_OPCODE(OPCODE::BGPDH, "BGPDH", FieldBackgroundInstruction, 0, "NBs");
            CASE_OPCODE(OPCODE::BGSCR, "BGSCR", FieldBackgroundInstruction, 0, "NBss");
            CASE_OPCODE(OPCODE::MPPAL, "MPPAL", FieldBackgroundInstruction, 0, "NNNBBBBBBB");
            CASE_OPCODE(OPCODE::BGON, "BGON", FieldBackgroundInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::BGOFF, "BGOFF", FieldBackgroundInstruction, 0, "NBB");
            CASE_OPCODE(OPCODE::BGROL, "BGROL", FieldBackgroundInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::BGROL2, "BGROL2", FieldBackgroundInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::BGCLR, "BGCLR", FieldBackgroundInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::STPAL, "STPAL", FieldBackgroundInstruction, 0, "NBBB");
            CASE_OPCODE(OPCODE::LDPAL, "LDPAL", FieldBackgroundInstruction, 0, "NBBB");
            CASE_OPCODE(OPCODE::CPPAL, "CPPAL", FieldBackgroundInstruction, 0, "NBBB");
            CASE_OPCODE(OPCODE::RTPAL, "RTPAL", FieldBackgroundInstruction, 0, "NNBBBB");
            CASE_OPCODE(OPCODE::ADPAL, "ADPAL", FieldBackgroundInstruction, 0, "NNNBBBBBB");
            CASE_OPCODE(OPCODE::MPPAL2, "MPPAL2", FieldBackgroundInstruction, 0, "NNNBBBBBB");
            CASE_OPCODE(OPCODE::STPLS, "STPLS", FieldBackgroundInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::LDPLS, "LDPLS", FieldBackgroundInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::CPPAL2, "CPPAL2", FieldBackgroundInstruction, 0, "BBBBBBB");
            CASE_OPCODE(OPCODE::RTPAL2, "RTPAL2", FieldBackgroundInstruction, 0, "BBBBBBB");
            CASE_OPCODE(OPCODE::ADPAL2, "ADPAL2", FieldBackgroundInstruction, 0, "BBBBBBBBBB");

            // Camera
            CASE_OPCODE(OPCODE::NFADE, "NFADE", FieldCameraInstruction, 0, "NNBBBBBB");
            CASE_OPCODE(OPCODE::SHAKE, "SHAKE", FieldCameraInstruction, 0, "BBBBBBB");
            CASE_OPCODE(OPCODE::SCRLO, "SCRLO", FieldCameraInstruction, 0, "B");
            CASE_OPCODE(OPCODE::SCRLC, "SCRLC", FieldCameraInstruction, 0, "BBBB");
            CASE_OPCODE(OPCODE::SCRLA, "SCRLA", FieldCameraInstruction, 0, "NwBB");
            CASE_OPCODE(OPCODE::SCR2D, "SCR2D", FieldCameraInstruction, 0, "Nss");
            CASE_OPCODE(OPCODE::SCRCC, "SCRCC", FieldCameraInstruction, 0, "");
            CASE_OPCODE(OPCODE::SCR2DC, "SCR2DC", FieldCameraInstruction, 0, "NNssw");
            CASE_OPCODE(OPCODE::SCRLW, "SCRLW", FieldCameraInstruction, 0, "");
            CASE_OPCODE(OPCODE::SCR2DL, "SCR2DL", FieldCameraInstruction, 0, "NNssw");
            CASE_OPCODE(OPCODE::VWOFT, "VWOFT", FieldCameraInstruction, 0, "NssB");
            CASE_OPCODE(OPCODE::FADE, "FADE", FieldCameraInstruction, 0, "NNBBBBBB");
            CASE_OPCODE(OPCODE::FADEW, "FADEW", FieldCameraInstruction, 0, "");
            CASE_OPCODE(OPCODE::SCRLP, "SCRLP", FieldCameraInstruction, 0, "NwBB");
            CASE_OPCODE(OPCODE::MVCAM, "MVCAM", FieldCameraInstruction, 0, "B");

            // AV
            CASE_OPCODE(OPCODE::BGMOVIE, "BGMOVIE", FieldMediaInstruction, 0, "B");
            CASE_OPCODE(OPCODE::AKAO2, "AKAO2", FieldMediaInstruction, 0, "NNNBwwwww");
            CASE_OPCODE(OPCODE::MUSIC, "MUSIC", FieldMediaInstruction, 0, "B");
            CASE_OPCODE(OPCODE::SOUND, "SOUND", FieldMediaInstruction, 0, "NwB");
            CASE_OPCODE(OPCODE::AKAO, "AKAO", FieldMediaInstruction, 0, "NNNBBwwww");
            CASE_OPCODE(OPCODE::MUSVT, "MUSVT", FieldMediaInstruction, 0, "B");
            CASE_OPCODE(OPCODE::MUSVM, "MUSVM", FieldMediaInstruction, 0, "B");
            CASE_OPCODE(OPCODE::MULCK, "MULCK", FieldMediaInstruction, 0, "B");
            CASE_OPCODE(OPCODE::BMUSC, "BMUSC", FieldMediaInstruction, 0, "B");
            CASE_OPCODE(OPCODE::CHMPH, "CHMPH", FieldMediaInstruction, 0, "BBB");
            CASE_OPCODE(OPCODE::PMVIE, "PMVIE", FieldMediaInstruction, 0, "B");
            CASE_OPCODE(OPCODE::MOVIE, "MOVIE", FieldMediaInstruction, 0, "");
            CASE_OPCODE(OPCODE::MVIEF, "MVIEF", FieldMediaInstruction, 0, "NB");
            CASE_OPCODE(OPCODE::FMUSC, "FMUSC", FieldMediaInstruction, 0, "B");
            CASE_OPCODE(OPCODE::CMUSC, "CMUSC", FieldMediaInstruction, 0, "BBBBBBB");
            CASE_OPCODE(OPCODE::CHMST, "CHMST", FieldMediaInstruction, 0, "NB");

            // Uncat
            CASE_OPCODE(OPCODE::MPDSP, "MPDSP", FieldUncategorizedInstruction, 0, "B");
            CASE_OPCODE(OPCODE::SETX, "SETX", FieldUncategorizedInstruction, 0, "BBBBBB");
            CASE_OPCODE(OPCODE::GETX, "GETX", FieldUncategorizedInstruction, 0, "BBBBBB");
            CASE_OPCODE(OPCODE::SEARCHX, "SEARCHX", FieldUncategorizedInstruction, 0, "BBBBBBBBBB");

            default:
                throw UnknownOpcodeException(this->address_, opcode);
        }
        INC_ADDR;

        // Is it within an "if" statement tracking?
        InstPtr i = this->_insts.back();
        if (i->isCondJump()) exitAddrs.push_back(i->GetDestAddress());
        if (!exitAddrs.empty())
            if (i->_address == exitAddrs.back()) exitAddrs.pop_back();

        // Only bail if its the first RET that isn't within an "if" block.
        if (full_opcode == OPCODE::RET && exitAddrs.empty()) return is_line;
    }
    return is_line;
}
