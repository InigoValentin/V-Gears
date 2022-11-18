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
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"
#include "decompiler/Engine.h"
#include "decompiler/field/FieldEngine.h"
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
#include "common/Lzs.h"


const int FieldDisassembler::MAGIC(0x0502);

const int FieldDisassembler::NUM_SECTIONS(7);

FieldDisassembler::FieldDisassembler(
  FieldScriptFormatter& formatter, FieldEngine* engine,
  InstVec& insts, const std::vector<unsigned char>& raw_script_data
): Disassembler(insts), engine_(engine), formatter_(formatter){
    loaded_from_raw_data_ = true;
    // If loading a raw section then skip the header section.
    section_pointers_size_ = 0;
    auto data_copy = raw_script_data;
    stream_ = std::make_unique<BinaryReader>(std::move(data_copy));
    ReadHeader();
}

FieldDisassembler::FieldDisassembler(
  FieldScriptFormatter& formatter, FieldEngine *engine, InstVec &insts
): Disassembler(insts), engine_(engine), formatter_(formatter){
    section_pointers_size_ = (sizeof(uint32) * NUM_SECTIONS);
}

float FieldDisassembler::GetScaleFactor() const{return scale_factor_;}

FieldDisassembler::~FieldDisassembler(){}

void FieldDisassembler::ReadHeader(){
    if (!loaded_from_raw_data_){
        // First read the file section pointers.
        for (int i = 0; i < NUM_SECTIONS; i ++) sections_[i] = stream_->ReadU32();
        // Now fix up from PSX RAM pointers to simple file offsets.
        const uint32 basePtr = sections_[0];
        for (int i = 0; i < NUM_SECTIONS; i++)
            sections_[i] = (sections_[i] - basePtr) + section_pointers_size_;
        // Now seek to the script section.
        stream_->Seek(sections_[SCRIPT]);
    }
    // Read the script header
    header_.Read(*stream_);
    scale_factor_ = static_cast<float>(header_.scale) / 512.0f;
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
            // If this is the very last script, so use the end of its
            // data which is the offset to strings.
            if (entity_index + 1 >= header_.entity_scripts.size()) return header_.offset_to_strings;
            else{
                // Wrap around to the next entity
                entity_index ++;
                script_index = 0;
            }
        }
        // Get the next script in the same entity.
        else script_index ++;
        next_entry_point = header_.entity_scripts[entity_index][script_index];
    } while (next_entry_point == cur_entry_point);
    return next_entry_point;
}

std::unique_ptr<Function> FieldDisassembler::StartFunction(size_t script_index){
    auto func = std::make_unique<Function>();
    func->ret_val = false;
    func->num_args = 0;
    func->name = "script_" + std::to_string(script_index);
    func->start_addr = address_;
    return func;
}

void FieldDisassembler::DoDisassemble(){
    // Loop through the scripts for each entity.
    for (size_t entity_number = 0; entity_number < header_.entity_scripts.size(); entity_number ++){
        std::string original_name = header_.field_entity_names[entity_number].data();
        // If the entity name was blank in the file then use a consistent generated name
        if (original_name.empty()) original_name = "entity_" + std::to_string(entity_number);
        const std::string entity_name = formatter_.GetFriendlyEntityName(original_name);
        // Only parse each script one.
        std::set<uint16> parsed_scripts;
        std::vector<ScriptInfo> script_info;
        // Collect the scripts to parse
        for (
          size_t script_index = 0;
          script_index < header_.entity_scripts[entity_number].size();
          script_index ++
        ){
            uint16 script_entry_point = header_.entity_scripts[entity_number][script_index];
            // If this scripts entry point is already parsed, don't do it again as it means
            // two scripts have the same entry, which only seems to be true for "empty" scripts.
            if (parsed_scripts.find(script_entry_point) != std::end(parsed_scripts)) continue;
            parsed_scripts.insert(script_entry_point);
            const uint32 next_script_entry_point = GetEndOfScriptOffset(
              script_entry_point, entity_number, script_index
            );
            const uint32 script_size = next_script_entry_point - script_entry_point;
            if (script_size > 0){
                ScriptInfo info = {script_entry_point, next_script_entry_point, script_index};
                script_info.push_back(info);
            }
        }
        for (auto it = script_info.begin(); it != script_info.end(); it ++){
            const bool is_start = it == script_info.begin();
            const bool is_end = it == (-- script_info.end());
            DisassembleIndivdualScript(
              entity_name, entity_number, it->index, it->entry_point,
              it->next_entry_point, is_start, is_end
            );
        }
    }
}

int FieldDisassembler::FindId(uint32 start_addr, uint32 end_addr, const InstVec& insts){
    for (const InstPtr& instruction : insts){
        if (instruction->GetAddress() >= start_addr && instruction->GetAddress() <= end_addr){
            if (instruction->GetOpcode() == OPCODES::opCodeCHAR)
                return instruction->GetParam(0)->GetSigned();
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
    const size_t old_num_instructions = insts_.size();
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
        // Keep going until all of the scriptis fectched, i.e if bailed at a
        // return, then call again until everything is parsed.
        while (stream_->GetPosition() != next_script_entry_point + section_pointers_size_){
            is_line = ReadOpCodesToPositionOrReturn(
              next_script_entry_point + section_pointers_size_, point_a, point_b
            );
        }
    }
    // Read metadata, mark start and/or end.
    std::string meta_data;
    if (is_start && is_end) meta_data = "start_end_";
    else if (is_start) meta_data = "start_";
    else if (is_end) meta_data = "end_";

    const size_t new_num_instructions = insts_.size();
    func->num_instructions = new_num_instructions - old_num_instructions;
    func->end_addr = insts_.back()->GetAddress();
    if (!func_name.empty()) func->name = func_name;
    if (engine_->EntityIsLine(entity_index)){
        switch (script_index){
            // main - on_update
            case 0: break;
            // [OK] - on_interact
            case 1: break;
            // Move - on_approach
            case 2: func->name = "on_approach"; break;
            // Move - on_cross
            case 3: func->name = "on_cross"; break;
            // Go - on_near
            case 4: func->name = "on_near"; break;
            // Go1x - on_near
            case 5: func->name = "on_near_once"; break;
            // GoAway - on_leave
            case 6: func->name = "on_leave"; break;
        }
    }
    int id = FindId(func->start_addr, func->end_addr, insts_);
    // If there is no ID check if there was an ID for this entity in any of
    // its other functions and use that instead.
    if (id == -1){
        for (auto& func : engine_->functions){
            FunctionMetaData func_meta_data(func.second.metadata);
            if (func_meta_data.GetEntityName() == entity_name && func_meta_data.GetCharacterId() != -1){
                id = func_meta_data.GetCharacterId();
                break;
            }
        }
    }
    meta_data += std::to_string(id) + "_" + entity_name;
    func->metadata = meta_data;
    engine_->functions[SCRIPT_ENTRY_POINT] = *func;
    engine_->AddEntityFunction(entity_name, entity_index, func->name, script_index);
    // If the entity is a line, mark it as so.
    if (is_line) engine_->MarkEntityAsLine(entity_index, true, point_a, point_b);
}

void FieldDisassembler::DisassembleIndivdualScript(
  std::string entity_name, size_t entity_index, size_t script_index,
  size_t script_entry_point, uint32 next_script_entry_point, bool is_start, bool is_end)
{
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
        // Read the init script, which means stop at the first return.
        AddFunc(
          entity_name, entity_index, script_index, next_script_entry_point,
          is_start, is_end, true, "on_start"
        );
        // Not at the end of this script? Then the remaining data is the "main" script
        auto stream_pos = stream_->GetPosition();
        if (stream_pos != end_pos){
            // The "main" script can have more than one return statement...
            AddFunc(
              entity_name, entity_index, script_index, next_script_entry_point,
              false, is_end, false, "on_update"
            );
            stream_pos = stream_->GetPosition();
            // ... but should end exactly on the end pos
            if (stream_pos != end_pos) throw DecompilerException();
        }
    }
}


FieldDisassembler::InstructionRecord FieldDisassembler::FLOW_OPCODES[] ={
    {1, OPCODES::RET, "RET", "", FieldControlFlowInstruction::Create},
    {1, OPCODES::REQ, "REQ", "BU", FieldControlFlowInstruction::Create},
    {1, OPCODES::REQSW, "REQSW", "BU", FieldControlFlowInstruction::Create},
    {1, OPCODES::REQEW, "REQEW", "BU", FieldControlFlowInstruction::Create},
    {1, OPCODES::NOP, "NOP", "", FieldNoOperationInstruction::Create},
    {1, OPCODES::IFUB, "IFUB", "NNBBBL", FieldNoOperationInstruction::Create}
};

std::map<std::string, const FieldDisassembler::InstructionRecord*>
  FieldDisassembler::FieldInstructions(){
    // Convert the array to a map that we can query on by opcode name.
    std::map<std::string, const InstructionRecord*> name_to_instruction_records;
    for (size_t i = 0; i < boost::size(FLOW_OPCODES); i ++)
        name_to_instruction_records[FLOW_OPCODES[i].opcode_name] = &FLOW_OPCODES[i];
    return name_to_instruction_records;
}

bool FieldDisassembler::ReadOpCodesToPositionOrReturn(
  size_t end_pos, std::vector<float>& point_a, std::vector<float>& point_b
){
    bool is_line = false;
    while (stream_->GetPosition() < end_pos){

        uint8 opcode = stream_->ReadU8();
        uint32 full_opcode = 0;
        full_opcode = (full_opcode << 8) + opcode;
        switch (opcode){

            // Flow
            case OPCODES::IFUB:
                ParseOpcode(full_opcode, "IFUB", new FieldCondJumpInstruction(), 0, "NBBBB");
                break;
            case OPCODES::RET:
                ParseOpcode(full_opcode, "RET", new FieldControlFlowInstruction(), 0, "");
                break;
            case OPCODES::REQ:
                ParseOpcode(full_opcode, "REQ", new FieldControlFlowInstruction(), 0, "BU");
                break;
            case OPCODES::REQSW:
                ParseOpcode(full_opcode, "REQSW", new FieldControlFlowInstruction(), 0, "BU");
                break;
            case OPCODES::REQEW:
                ParseOpcode(full_opcode, "REQEW", new FieldControlFlowInstruction(), 0, "BU");
                break;
            case OPCODES::PREQ:
                ParseOpcode(full_opcode, "PREQ", new FieldControlFlowInstruction(), 0, "BU");
                break;
            case OPCODES::PRQSW:
                ParseOpcode(full_opcode, "PRQSW", new FieldControlFlowInstruction(), 0, "BU");
                break;
            case OPCODES::PRQEW:
                ParseOpcode(full_opcode, "PRQEW", new FieldControlFlowInstruction(), 0, "BU");
                break;
            case OPCODES::RETTO:
                ParseOpcode(full_opcode, "RETTO", new FieldControlFlowInstruction(), 0, "U");
                break;
            case OPCODES::JMPF:
                ParseOpcode(full_opcode, "JMPF", new FieldUncondJumpInstruction(), 0, "B");
                break;
            case OPCODES::JMPFL:
                ParseOpcode(full_opcode, "JMPFL", new FieldUncondJumpInstruction(), 0, "w");
                break;
            case OPCODES::JMPB:
                ParseOpcode(full_opcode, "JMPB", new FieldUncondJumpInstruction(), 0, "B");
                break;
            case OPCODES::JMPBL:
                ParseOpcode(full_opcode, "JMPBL", new FieldUncondJumpInstruction(), 0, "w");
                break;
            case OPCODES::IFUBL:
                ParseOpcode(full_opcode, "IFUBL", new FieldCondJumpInstruction(), 0, "NBBBw");
                break;
            case OPCODES::IFSW:
                ParseOpcode(full_opcode, "IFSW", new FieldCondJumpInstruction(), 0, "NwwBB");
                break;
            case OPCODES::IFSWL:
                ParseOpcode(full_opcode, "IFSWL", new FieldCondJumpInstruction(), 0, "NwwBw");
                break;
            case OPCODES::IFUW:
                ParseOpcode(full_opcode, "IFUW", new FieldCondJumpInstruction(), 0, "NwwBB");
                break;
            case OPCODES::IFUWL:
                ParseOpcode(full_opcode, "IFUWL", new FieldCondJumpInstruction(), 0, "NwwBw");
                break;
            case OPCODES::WAIT:
                ParseOpcode(full_opcode, "WAIT", new FieldControlFlowInstruction(), 0, "w");
                break;
            case OPCODES::IFKEY:
                ParseOpcode(full_opcode, "IFKEY", new FieldCondJumpInstruction(), 0, "wB");
                break;
            case OPCODES::IFKEYON:
                ParseOpcode(full_opcode, "IFKEYON", new FieldCondJumpInstruction(), 0, "wB");
                break;
            case OPCODES::IFKEYOFF:
                ParseOpcode(full_opcode, "IFKEYOFF", new FieldCondJumpInstruction(), 0, "wB");
                break;
            case OPCODES::NOP:
                ParseOpcode(full_opcode, "NOP", new FieldNoOperationInstruction(), 0, "");
                break;
            case OPCODES::IFPRTYQ:
                ParseOpcode(full_opcode, "IFPRTYQ", new FieldCondJumpInstruction(), 0, "BB");
                break;
            case OPCODES::IFMEMBQ:
                ParseOpcode(full_opcode, "IFMEMBQ", new FieldCondJumpInstruction(), 0, "BB");
                break;

            // Module
            case OPCODES::DSKCG:
                ParseOpcode(full_opcode, "DSKCG", new FieldModuleInstruction(), 0, "B");
                break;
            case OPCODES::SPECIAL:
                full_opcode = 0;
                full_opcode = (full_opcode << 8) + OPCODES::SPECIAL;
                opcode = this->stream_->ReadU8();
                switch (opcode){
                    case OPCODES_SPECIAL::ARROW:
                        ParseOpcode(full_opcode, "ARROW", new FieldModuleInstruction(), 0, "B");
                        break;
                    case OPCODES_SPECIAL::PNAME:
                        ParseOpcode(full_opcode, "PNAME", new FieldModuleInstruction(), 0, "B");
                        break;
                    case OPCODES_SPECIAL::GMSPD:
                        ParseOpcode(full_opcode, "GMSPD", new FieldModuleInstruction(), 0, "B");
                        break;
                    case OPCODES_SPECIAL::SMSPD:
                        ParseOpcode(full_opcode, "SMSPD", new FieldModuleInstruction(), 0, "BB");
                        break;
                    case OPCODES_SPECIAL::FLMAT:
                        ParseOpcode(full_opcode, "FLMAT", new FieldModuleInstruction(), 0, "");
                        break;
                    case OPCODES_SPECIAL::FLITM:
                        ParseOpcode(full_opcode, "FLITM", new FieldModuleInstruction(), 0, "");
                        break;
                    case OPCODES_SPECIAL::BTLCK:
                        ParseOpcode(full_opcode, "BTLCK", new FieldModuleInstruction(), 0, "B");
                        break;
                    case OPCODES_SPECIAL::MVLCK:
                        ParseOpcode(full_opcode, "MVLCK", new FieldModuleInstruction(), 0, "B");
                        break;
                    case OPCODES_SPECIAL::SPCNM:
                        ParseOpcode(full_opcode, "SPCNM", new FieldModuleInstruction(), 0, "BB");
                        break;
                    case OPCODES_SPECIAL::RSGLB:
                        ParseOpcode(full_opcode, "RSGLB", new FieldModuleInstruction(), 0, "");
                        break;
                    case OPCODES_SPECIAL::CLITM:
                        ParseOpcode(full_opcode, "CLITM", new FieldModuleInstruction(), 0, "");
                        break;
                    default:
                        throw UnknownSubOpcodeException(this->address_, opcode);\
                }
                this->address_++ ;
                break;
            case OPCODES::MINIGAME:
                ParseOpcode(full_opcode, "MINIGAME", new FieldModuleInstruction(), 0, "wsswBB");
                break;
            case OPCODES::BTMD2:
                ParseOpcode(full_opcode, "BTMD2", new FieldModuleInstruction(), 0, "d");
                break;
            case OPCODES::BTRLD:
                ParseOpcode(full_opcode, "BTRLD", new FieldModuleInstruction(), 0, "NB");
                break;
            case OPCODES::BTLTB:
                ParseOpcode(full_opcode, "BTLTB", new FieldModuleInstruction(), 0, "B");
                break;
            case OPCODES::MAPJUMP:
                ParseOpcode(full_opcode, "MAPJUMP", new FieldModuleInstruction(), 0, "wsswB");
                break;
            case OPCODES::LSTMP:
                ParseOpcode(full_opcode, "LSTMP", new FieldModuleInstruction(), 0, "NB");
                break;
            case OPCODES::BATTLE:
                ParseOpcode(full_opcode, "BATTLE", new FieldModuleInstruction(), 0, "Nw");
                break;
            case OPCODES::BTLON:
                ParseOpcode(full_opcode, "BTLON", new FieldModuleInstruction(), 0, "B");
                break;
            case OPCODES::BTLMD:
                ParseOpcode(full_opcode, "BTLMD", new FieldModuleInstruction(), 0, "w");
                break;
            case OPCODES::MPJPO:
                ParseOpcode(full_opcode, "MPJPO", new FieldModuleInstruction(), 0, "B");
                break;
            case OPCODES::PMJMP:
                ParseOpcode(full_opcode, "PMJMP", new FieldModuleInstruction(), 0, "w");
                break;
            case OPCODES::PMJMP2:
                ParseOpcode(full_opcode, "PMJMP2", new FieldModuleInstruction(), 0, "");
                break;
            case OPCODES::GAMEOVER:
                ParseOpcode(full_opcode, "GAMEOVER", new FieldModuleInstruction(), 0, "");
                break;

            // Math
            case OPCODES::PLUS_:
                ParseOpcode(full_opcode, "PLUS!", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::PLUS2_:
                ParseOpcode(full_opcode, "PLUS2!", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::MINUS_:
                ParseOpcode(full_opcode, "MINUS!", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::MINUS2_:
                ParseOpcode(full_opcode, "MINUS2!", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::INC_:
                ParseOpcode(full_opcode, "INC!", new FieldMathInstruction(), 0, "BB");
                break;
            case OPCODES::INC2_:
                ParseOpcode(full_opcode, "INC2!", new FieldMathInstruction(), 0, "BB");
                break;
            case OPCODES::DEC_:
                ParseOpcode(full_opcode, "DEC!", new FieldMathInstruction(), 0, "BB");
                break;
            case OPCODES::DEC2_:
                ParseOpcode(full_opcode, "DEC2!", new FieldMathInstruction(), 0, "BB");
                break;
            case OPCODES::RDMSD:
                ParseOpcode(full_opcode, "RDMSD", new FieldMathInstruction(), 0, "NB");
                break;
            case OPCODES::SETBYTE:
                ParseOpcode(full_opcode, "SETBYTE", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::SETWORD:
                ParseOpcode(full_opcode, "SETWORD", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::BITON:
                ParseOpcode(full_opcode, "BITON", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::BITOFF:
                ParseOpcode(full_opcode, "BITOFF", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::BITXOR:
                ParseOpcode(full_opcode, "BITXOR", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::PLUS:
                ParseOpcode(full_opcode, "PLUS", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::PLUS2:
                ParseOpcode(full_opcode, "PLUS2", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::MINUS:
                ParseOpcode(full_opcode, "MINUS", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::MINUS2:
                ParseOpcode(full_opcode, "MINUS2", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::MUL:
                ParseOpcode(full_opcode, "MUL", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::MUL2:
                ParseOpcode(full_opcode, "MUL2", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::DIV:
                ParseOpcode(full_opcode, "DIV", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::DIV2:
                ParseOpcode(full_opcode, "DIV2", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::MOD:
                ParseOpcode(full_opcode, "MOD", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::MOD2:
                ParseOpcode(full_opcode, "MOD2", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::AND:
                ParseOpcode(full_opcode, "AND", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::AND2:
                ParseOpcode(full_opcode, "AND2", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::OR:
                ParseOpcode(full_opcode, "OR", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::OR2:
                ParseOpcode(full_opcode, "OR2", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::XOR:
                ParseOpcode(full_opcode, "XOR", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::XOR2:
                ParseOpcode(full_opcode, "XOR2", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::INC:
                ParseOpcode(full_opcode, "INC", new FieldMathInstruction(), 0, "BB");
                break;
            case OPCODES::INC2:
                ParseOpcode(full_opcode, "INC2", new FieldMathInstruction(), 0, "BB");
                break;
            case OPCODES::DEC:
                ParseOpcode(full_opcode, "DEC", new FieldMathInstruction(), 0, "BB");
                break;
            case OPCODES::DEC2:
                ParseOpcode(full_opcode, "DEC2", new FieldMathInstruction(), 0, "BB");
                break;
            case OPCODES::RANDOM:
                ParseOpcode(full_opcode, "RANDOM", new FieldMathInstruction(), 0, "BB");
                break;
            case OPCODES::LBYTE:
                ParseOpcode(full_opcode, "LBYTE", new FieldMathInstruction(), 0, "NBB");
                break;
            case OPCODES::HBYTE:
                ParseOpcode(full_opcode, "HBYTE", new FieldMathInstruction(), 0, "NBw");
                break;
            case OPCODES::TWOBYTE:
                ParseOpcode(full_opcode, "2BYTE", new FieldMathInstruction(), 0, "NNBBB");
                break;
            case OPCODES::SIN:
                ParseOpcode(full_opcode, "SIN", new FieldMathInstruction(), 0, "NNwwwB");
                break;
            case OPCODES::COS:
                ParseOpcode(full_opcode, "COS", new FieldMathInstruction(), 0, "NNwwwB");
                break;

            // Window
            case OPCODES::TUTOR:
                ParseOpcode(full_opcode, "TUTOR", new FieldWindowInstruction(), 0, "B");
                break;
            case OPCODES::WCLS:
                ParseOpcode(full_opcode, "WCLS", new FieldWindowInstruction(), 0, "B");
                break;
            case OPCODES::WSIZW:
                ParseOpcode(full_opcode, "WSIZW", new FieldWindowInstruction(), 0, "Bwwww");
                break;
            case OPCODES::WSPCL:
                ParseOpcode(full_opcode, "WSPCL", new FieldWindowInstruction(), 0, "BBBB");
                break;
            case OPCODES::WNUMB:
                ParseOpcode(full_opcode, "WNUMB", new FieldWindowInstruction(), 0, "NBwwB");
                break;
            case OPCODES::STTIM:
                ParseOpcode(full_opcode, "STTIM", new FieldWindowInstruction(), 0, "NNBBB");
                break;
            case OPCODES::MESSAGE:
                ParseOpcode(full_opcode, "MESSAGE", new FieldWindowInstruction(), 0, "BB");
                break;
            case OPCODES::MPARA:
                ParseOpcode(full_opcode, "MPARA", new FieldWindowInstruction(), 0, "NBBB");
                break;
            case OPCODES::MPRA2:
                ParseOpcode(full_opcode, "MPRA2", new FieldWindowInstruction(), 0, "NBBw");
                break;
            case OPCODES::MPNAM:
                ParseOpcode(full_opcode, "MPNAM", new FieldWindowInstruction(), 0, "B");
                break;
            case OPCODES::ASK:
                ParseOpcode(full_opcode, "ASK", new FieldWindowInstruction(), 0, "NBBBBB");
                break;
            case OPCODES::MENU:
                ParseOpcode(full_opcode, "MENU", new FieldWindowInstruction(), 0, "NBB");
                break;
            case OPCODES::MENU2:
                ParseOpcode(full_opcode, "MENU2", new FieldWindowInstruction(), 0, "B");
                break;
            case OPCODES::WINDOW:
                ParseOpcode(full_opcode, "WINDOW", new FieldWindowInstruction(), 0, "Bwwww");
                break;
            case OPCODES::WMOVE:
                ParseOpcode(full_opcode, "WMOVE", new FieldWindowInstruction(), 0, "Bss");
                break;
            case OPCODES::WMODE:
                ParseOpcode(full_opcode, "WMODE", new FieldWindowInstruction(), 0, "BBB");
                break;
            case OPCODES::WREST:
                ParseOpcode(full_opcode, "WREST", new FieldWindowInstruction(), 0, "B");
                break;
            case OPCODES::WCLSE:
                ParseOpcode(full_opcode, "WCLSE", new FieldWindowInstruction(), 0, "B");
                break;
            case OPCODES::WROW:
                ParseOpcode(full_opcode, "WROW", new FieldWindowInstruction(), 0, "BB");
                break;
            case OPCODES::GWCOL:
                ParseOpcode(full_opcode, "GWCOL", new FieldWindowInstruction(), 0, "NNBBBB");
                break;
            case OPCODES::SWCOL:
                ParseOpcode(full_opcode, "SWCOL", new FieldWindowInstruction(), 0, "NNBBBB");
                break;

            // Party
            case OPCODES::SPTYE:
                ParseOpcode(full_opcode, "SPTYE", new FieldPartyInstruction(), 0, "NNBBB");
                break;
            case OPCODES::GTPYE:
                ParseOpcode(full_opcode, "GTPYE", new FieldPartyInstruction(), 0, "NNBBB");
                break;
            case OPCODES::GOLDU:
                // Goldu can have two argument formats: (nybble, word) or (nybble, byte, 3 bytes)
                // Read it as nybble, byte, byte, byte, byte and operate later
                ParseOpcode(full_opcode, "GOLDU", new FieldPartyInstruction(), 0, "NBBBB");
                break;
            case OPCODES::GOLDD:
                ParseOpcode(full_opcode, "GOLDD", new FieldPartyInstruction(), 0, "NBBBB");
                break;
            case OPCODES::CHGLD:
                ParseOpcode(full_opcode, "CHGLD", new FieldPartyInstruction(), 0, "NBB");
                break;
            case OPCODES::HMPMAX1:
                ParseOpcode(full_opcode, "HMPMAX1", new FieldPartyInstruction(), 0, "");
                break;
            case OPCODES::HMPMAX2:
                ParseOpcode(full_opcode, "HMPMAX2", new FieldPartyInstruction(), 0, "");
                break;
            case OPCODES::MHMMX:
                ParseOpcode(full_opcode, "MHMMX", new FieldPartyInstruction(), 0, "");
                break;
            case OPCODES::HMPMAX3:
                ParseOpcode(full_opcode, "HMPMAX3", new FieldPartyInstruction(), 0, "");
                break;
            case OPCODES::MPU:
                ParseOpcode(full_opcode, "MPU", new FieldPartyInstruction(), 0, "NBw");
                break;
            case OPCODES::MPD:
                ParseOpcode(full_opcode, "MPD", new FieldPartyInstruction(), 0, "NBw");
                break;
            case OPCODES::HPU:
                ParseOpcode(full_opcode, "HPU", new FieldPartyInstruction(), 0, "NBw");
                break;
            case OPCODES::HPD:
                ParseOpcode(full_opcode, "HPD", new FieldPartyInstruction(), 0, "NBw");
                break;
            case OPCODES::STITM:
                ParseOpcode(full_opcode, "STITM", new FieldPartyInstruction(), 0, "NwB");
                break;
            case OPCODES::DLITM:
                ParseOpcode(full_opcode, "DLITM", new FieldPartyInstruction(), 0, "NwB");
                break;
            case OPCODES::CKITM:
                ParseOpcode(full_opcode, "CKITM", new FieldPartyInstruction(), 0, "NwB");
                break;
            case OPCODES::SMTRA:
                ParseOpcode(full_opcode, "SMTRA", new FieldPartyInstruction(), 0, "NNBBBB");
                break;
            case OPCODES::DMTRA:
                ParseOpcode(full_opcode, "DMTRA", new FieldPartyInstruction(), 0, "NNBBBBB");
                break;
            case OPCODES::CMTRA:
                ParseOpcode(full_opcode, "CMTRA", new FieldPartyInstruction(), 0, "NNNBBBBBB");
                break;
            case OPCODES::GETPC:
                ParseOpcode(full_opcode, "GETPC", new FieldPartyInstruction(), 0, "NBB");
                break;
            case OPCODES::PRTYP:
                ParseOpcode(full_opcode, "PRTYP", new FieldPartyInstruction(), 0, "B");
                break;
            case OPCODES::PRTYM:
                ParseOpcode(full_opcode, "PRTYM", new FieldPartyInstruction(), 0, "B");
                break;
            case OPCODES::PRTYE:
                ParseOpcode(full_opcode, "PRTYE", new FieldPartyInstruction(), 0, "BBB");
                break;
            case OPCODES::MMBUD:
                ParseOpcode(full_opcode, "MMBUD", new FieldPartyInstruction(), 0, "BB");
                break;
            case OPCODES::MMBLK:
                ParseOpcode(full_opcode, "MMBLK", new FieldPartyInstruction(), 0, "B");
                break;
            case OPCODES::MMBUK:
                ParseOpcode(full_opcode, "MMBUK", new FieldPartyInstruction(), 0, "B");
                break;

            // Model
            case OPCODES::JOIN:
                ParseOpcode(full_opcode, "JOIN", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::SPLIT:
                ParseOpcode(full_opcode, "SPLIT", new FieldModelInstruction(), 0, "NNNssBssBB");
                break;
            case OPCODES::BLINK:
                ParseOpcode(full_opcode, "BLINK", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::KAWAI:
                {
                    full_opcode = 0;
                    full_opcode = (full_opcode << 8) + OPCODES::KAWAI;
                    int length = this->stream_->ReadU8();
                    assert(length >= 3);
                    std::ostringstream param_stream;
                    for (int i = 3; i < length; ++ i) param_stream << "B";
                    auto parameters = param_stream.str();
                    opcode = this->stream_->ReadU8();
                    switch (opcode){
                        case OPCODES_KAWAI::EYETX:
                            // Parameters were "BBBB"
                            ParseOpcode(
                              opcode, "EYETX", new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::TRNSP:
                            // Parameters were "B"
                            ParseOpcode(
                              opcode, "TRNSP", new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::AMBNT:
                            // Parameters were "AMBNT"
                            ParseOpcode(
                              opcode, "AMBNT",
                              new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::Unknown03:
                            ParseOpcode(
                              opcode, "Unknown03",
                              new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::Unknown04:
                            ParseOpcode(
                              opcode, "Unknown04",
                              new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::Unknown05:
                            ParseOpcode(
                              opcode, "Unknown03",
                              new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::LIGHT:
                            ParseOpcode(
                              opcode, "LIGHT",
                              new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::Unknown07:
                            ParseOpcode(
                              opcode, "Unknown07",
                              new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::Unknown08:
                            ParseOpcode(
                              opcode, "Unknown08",
                              new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::Unknown09:
                            ParseOpcode(
                              opcode, "Unknown09",
                              new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::SBOBJ:
                            ParseOpcode(
                              opcode, "SBOBJ", new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::Unknown0B:
                            ParseOpcode(
                              opcode, "Unknown0B",
                              new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::Unknown0C:
                            ParseOpcode(
                              opcode, "Unknown0C", new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::SHINE:
                            ParseOpcode(
                              opcode, "SHINE", new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        case OPCODES_KAWAI::RESET:
                            ParseOpcode(
                              opcode, "RESET", new FieldModelInstruction(), 0, parameters.c_str()
                            );
                            break;
                        default:
                            throw UnknownSubOpcodeException(this->address_, opcode);
                    }
                    this->address_ ++;
                    this->address_ ++;
                    break;
                }
            case OPCODES::KAWIW:
                ParseOpcode(full_opcode, "KAWIW", new FieldModelInstruction(), 0, "");
                break;
            case OPCODES::PMOVA:
                ParseOpcode(full_opcode, "PMOVA", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::PDIRA:
                ParseOpcode(full_opcode, "PDIRA", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::PTURA:
                ParseOpcode(full_opcode, "PTURA", new FieldModelInstruction(), 0, "BBB");
                break;
            case OPCODES::PGTDR:
                ParseOpcode(full_opcode, "PGTDR", new FieldModelInstruction(), 0, "NBB");
                break;
            case OPCODES::PXYZI:
                ParseOpcode(full_opcode, "PXYZI", new FieldModelInstruction(), 0, "NNBBBBB");
                break;
            case OPCODES::TLKON:
                ParseOpcode(full_opcode, "TLKON", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::PC:
                ParseOpcode(full_opcode, "PC", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::opCodeCHAR:
                ParseOpcode(full_opcode, "CHAR", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::DFANM:
                ParseOpcode(full_opcode, "DFANM", new FieldModelInstruction(), 0, "BB");
                break;
            case OPCODES::ANIME1:
                ParseOpcode(full_opcode, "ANIME1", new FieldModelInstruction(), 0, "BB");
                break;
            case OPCODES::VISI:
                ParseOpcode(full_opcode, "VISI", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::XYZI:
                ParseOpcode(full_opcode, "XYZI", new FieldModelInstruction(), 0, "NNsssw");
                break;
            case OPCODES::XYI:
                ParseOpcode(full_opcode, "XYI", new FieldModelInstruction(), 0, "NNssw");
                break;
            case OPCODES::XYZ:
                ParseOpcode(full_opcode, "XYZ", new FieldModelInstruction(), 0, "NNsss");
                break;
            case OPCODES::MOVE:
                ParseOpcode(full_opcode, "MOVE", new FieldModelInstruction(), 0, "Nss");
                break;
            case OPCODES::CMOVE:
                ParseOpcode(full_opcode, "CMOVE", new FieldModelInstruction(), 0, "Nss");
                break;
            case OPCODES::MOVA:
                ParseOpcode(full_opcode, "MOVA", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::TURA:
                ParseOpcode(full_opcode, "TURA", new FieldModelInstruction(), 0, "BBB");
                break;
            case OPCODES::ANIMW:
                ParseOpcode(full_opcode, "ANIMW", new FieldModelInstruction(), 0, "");
                break;
            case OPCODES::FMOVE:
                ParseOpcode(full_opcode, "FMOVE", new FieldModelInstruction(), 0, "Nss");
                break;
            case OPCODES::ANIME2:
                ParseOpcode(full_opcode, "ANIME2", new FieldModelInstruction(), 0, "BB");
                break;
            case OPCODES::ANIM_1:
                ParseOpcode(full_opcode, "ANIM!1", new FieldModelInstruction(), 0, "BB");
                break;
            case OPCODES::CANIM1:
                ParseOpcode(full_opcode, "CANIM1", new FieldModelInstruction(), 0, "BBBB");
                break;
            case OPCODES::CANM_1:
                ParseOpcode(full_opcode, "CANM!1", new FieldModelInstruction(), 0, "BBBB");
                break;
            case OPCODES::MSPED:
                ParseOpcode(full_opcode, "MSPED", new FieldModelInstruction(), 0, "Nw");
                break;
            case OPCODES::DIR:
                ParseOpcode(full_opcode, "DIR", new FieldModelInstruction(), 0, "BB");
                break;
            case OPCODES::TURNGEN:
                ParseOpcode(full_opcode, "TURNGEN", new FieldModelInstruction(), 0, "NBBBB");
                break;
            case OPCODES::TURN:
                ParseOpcode(full_opcode, "TURN", new FieldModelInstruction(), 0, "NBBBB");
                break;
            case OPCODES::DIRA:
                ParseOpcode(full_opcode, "DIRA", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::GETDIR:
                ParseOpcode(full_opcode, "GETDIR", new FieldModelInstruction(), 0, "NBB");
                break;
            case OPCODES::GETAXY:
                ParseOpcode(full_opcode, "GETAXY", new FieldModelInstruction(), 0, "NBBB");
                break;
            case OPCODES::GETAI:
                ParseOpcode(full_opcode, "GETAI", new FieldModelInstruction(), 0, "NBB");
                break;
            case OPCODES::ANIM_2:
                ParseOpcode(full_opcode, "ANIM!2", new FieldModelInstruction(), 0, "BB");
                break;
            case OPCODES::CANIM2:
                ParseOpcode(full_opcode, "CANIM2", new FieldModelInstruction(), 0, "BBBB");
                break;
            case OPCODES::CANM_2:
                ParseOpcode(full_opcode, "CANM!2", new FieldModelInstruction(), 0, "BBBB");
                break;
            case OPCODES::ASPED:
                ParseOpcode(full_opcode, "ASPED", new FieldModelInstruction(), 0, "Nw");
                break;
            case OPCODES::CC:
                ParseOpcode(full_opcode, "CC", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::JUMP:
                ParseOpcode(full_opcode, "JUMP", new FieldModelInstruction(), 0, "NNssww");
                break;
            case OPCODES::AXYZI:
                ParseOpcode(full_opcode, "AXYZI", new FieldModelInstruction(), 0, "NNBBBBB");
                break;
            case OPCODES::LADER:
                ParseOpcode(full_opcode, "LADER", new FieldModelInstruction(), 0, "NNssswBBBB");
                break;
            case OPCODES::OFST:
                ParseOpcode(full_opcode, "OFST", new FieldModelInstruction(), 0, "NNBsssw");
                break;
            case OPCODES::OFSTW:
                ParseOpcode(full_opcode, "OFSTW", new FieldModelInstruction(), 0, "");
                break;
            case OPCODES::TALKR:
                ParseOpcode(full_opcode, "TALKR", new FieldModelInstruction(), 0, "NB");
                break;
            case OPCODES::SLIDR:
                ParseOpcode(full_opcode, "SLIDR", new FieldModelInstruction(), 0, "NB");
                break;
            case OPCODES::SOLID:
                ParseOpcode(full_opcode, "SOLID", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::TLKR2:
                ParseOpcode(full_opcode, "TLKR2", new FieldModelInstruction(), 0, "Nw");
                break;
            case OPCODES::SLDR2:
                ParseOpcode(full_opcode, "SLDR2", new FieldModelInstruction(), 0, "Nw");
                break;
            case OPCODES::CCANM:
                ParseOpcode(full_opcode, "CCANM", new FieldModelInstruction(), 0, "BBB");
                break;
            case OPCODES::FCFIX:
                ParseOpcode(full_opcode, "FCFIX", new FieldModelInstruction(), 0, "B");
                break;
            case OPCODES::ANIMB:
                ParseOpcode(full_opcode, "ANIMB", new FieldModelInstruction(), 0, "");
                break;
            case OPCODES::TURNW:
                ParseOpcode(full_opcode, "TURNW", new FieldModelInstruction(), 0, "");
                break;

            // Walkmesh
            case OPCODES::SLIP:
                ParseOpcode(full_opcode, "SLIP", new FieldWalkmeshInstruction(), 0, "B");
                break;
            case OPCODES::UC:
                ParseOpcode(full_opcode, "UC", new FieldWalkmeshInstruction(), 0, "B");
                break;
            case OPCODES::IDLCK:
                ParseOpcode(full_opcode, "IDLCK", new FieldWalkmeshInstruction(), 0, "wB");
                break;
            case OPCODES::LINON:
                ParseOpcode(full_opcode, "LINON", new FieldWalkmeshInstruction(), 0, "B");
                break;
            case OPCODES::SLINE:
                ParseOpcode(full_opcode, "SLINE", new FieldWalkmeshInstruction(), 0, "NNNssssss");
                break;
            case OPCODES::LINE:
                // Mark function entity owner as line.
                is_line = true;
                ParseOpcode(full_opcode, "LINE", new FieldWalkmeshInstruction(), 0, "ssssss");
                point_a[0] = this->insts_.back()->GetParam(0)->GetSigned();
                point_a[1] = this->insts_.back()->GetParam(1)->GetSigned();
                point_a[2] = this->insts_.back()->GetParam(2)->GetSigned();
                point_b[0] = this->insts_.back()->GetParam(3)->GetSigned();
                point_b[1] = this->insts_.back()->GetParam(4)->GetSigned();
                point_b[2] = this->insts_.back()->GetParam(5)->GetSigned();
                break;

            // Backgnd
            case OPCODES::BGPDH:
                ParseOpcode(full_opcode, "BGPDH", new FieldBackgroundInstruction(), 0, "NBs");
                break;
            case OPCODES::BGSCR:
                ParseOpcode(full_opcode, "BGSCR", new FieldBackgroundInstruction(), 0, "NBss");
                break;
            case OPCODES::MPPAL:
                ParseOpcode(full_opcode, "MPPAL", new FieldBackgroundInstruction(), 0, "NNNBBBBBBB");
                break;
            case OPCODES::BGON:
                ParseOpcode(full_opcode, "BGON", new FieldBackgroundInstruction(), 0, "NBB");
                break;
            case OPCODES::BGOFF:
                ParseOpcode(full_opcode, "BGOFF", new FieldBackgroundInstruction(), 0, "NBB");
                break;
            case OPCODES::BGROL:
                ParseOpcode(full_opcode, "BGROL", new FieldBackgroundInstruction(), 0, "NB");
                break;
            case OPCODES::BGROL2:
                ParseOpcode(full_opcode, "BGROL2", new FieldBackgroundInstruction(), 0, "NB");
                break;
            case OPCODES::BGCLR:
                ParseOpcode(full_opcode, "BGCLR", new FieldBackgroundInstruction(), 0, "NB");
                break;
            case OPCODES::STPAL:
                ParseOpcode(full_opcode, "STPAL", new FieldBackgroundInstruction(), 0, "NBBB");
                break;
            case OPCODES::LDPAL:
                ParseOpcode(full_opcode, "LDPAL", new FieldBackgroundInstruction(), 0, "NBBB");
                break;
            case OPCODES::CPPAL:
                ParseOpcode(full_opcode, "CPPAL", new FieldBackgroundInstruction(), 0, "NBBB");
                break;
            case OPCODES::RTPAL:
                ParseOpcode(full_opcode, "RTPAL", new FieldBackgroundInstruction(), 0, "NNBBBB");
                break;
            case OPCODES::ADPAL:
                ParseOpcode(full_opcode, "ADPAL", new FieldBackgroundInstruction(), 0, "NNNBBBBBB");
                break;
            case OPCODES::MPPAL2:
                ParseOpcode(
                  full_opcode, "MPPAL2", new FieldBackgroundInstruction(), 0, "NNNBBBBBB"
                );
                break;
            case OPCODES::STPLS:
                ParseOpcode(full_opcode, "STPLS", new FieldBackgroundInstruction(), 0, "BBBB");
                break;
            case OPCODES::LDPLS:
                ParseOpcode(full_opcode, "LDPLS", new FieldBackgroundInstruction(), 0, "BBBB");
                break;
            case OPCODES::CPPAL2:
                ParseOpcode(full_opcode, "CPPAL2", new FieldBackgroundInstruction(), 0, "BBBBBBB");
                break;
            case OPCODES::RTPAL2:
                ParseOpcode(full_opcode, "RTPAL2", new FieldBackgroundInstruction(), 0, "BBBBBBB");
                break;
            case OPCODES::ADPAL2:
                ParseOpcode(full_opcode, "ADPAL2", new FieldBackgroundInstruction(), 0, "BBBBBBBBBB");
                break;

            // Camera
            case OPCODES::NFADE:
                ParseOpcode(full_opcode, "NFADE", new FieldCameraInstruction(), 0, "NNBBBBBB");
                break;
            case OPCODES::SHAKE:
                ParseOpcode(full_opcode, "SHAKE", new FieldCameraInstruction(), 0, "BBBBBBB");
                break;
            case OPCODES::SCRLO:
                ParseOpcode(full_opcode, "SCRLO", new FieldCameraInstruction(), 0, "B");
                break;
            case OPCODES::SCRLC:
                ParseOpcode(full_opcode, "SCRLC", new FieldCameraInstruction(), 0, "BBBB");
                break;
            case OPCODES::SCRLA:
                ParseOpcode(full_opcode, "SCRLA", new FieldCameraInstruction(), 0, "NwBB");
                break;
            case OPCODES::SCR2D:
                ParseOpcode(full_opcode, "SCR2D", new FieldCameraInstruction(), 0, "Nss");
                break;
            case OPCODES::SCRCC:
                ParseOpcode(full_opcode, "SCRCC", new FieldCameraInstruction(), 0, "");
                break;
            case OPCODES::SCR2DC:
                ParseOpcode(full_opcode, "SCR2DC", new FieldCameraInstruction(), 0, "NNssw");
                break;
            case OPCODES::SCRLW:
                ParseOpcode(full_opcode, "SCRLW", new FieldCameraInstruction(), 0, "");
                break;
            case OPCODES::SCR2DL:
                ParseOpcode(full_opcode, "SCR2DL", new FieldCameraInstruction(), 0, "NNssw");
                break;
            case OPCODES::VWOFT:
                ParseOpcode(full_opcode, "VWOFT", new FieldCameraInstruction(), 0, "NssB");
                break;
            case OPCODES::FADE:
                ParseOpcode(full_opcode, "FADE", new FieldCameraInstruction(), 0, "NNBBBBBB");
                break;
            case OPCODES::FADEW:
                ParseOpcode(full_opcode, "FADEW", new FieldCameraInstruction(), 0, "");
                break;
            case OPCODES::SCRLP:
                ParseOpcode(full_opcode, "SCRLP", new FieldCameraInstruction(), 0, "NwBB");
                break;
            case OPCODES::MVCAM:
                ParseOpcode(full_opcode, "MVCAM", new FieldCameraInstruction(), 0, "B");
                break;

            // AV
            case OPCODES::BGMOVIE:
                ParseOpcode(full_opcode, "BGMOVIE", new FieldMediaInstruction(), 0, "B");
                break;
            case OPCODES::AKAO2:
                ParseOpcode(full_opcode, "AKAO2", new FieldMediaInstruction(), 0, "NNNBwwwww");
                break;
            case OPCODES::MUSIC:
                ParseOpcode(full_opcode, "MUSIC", new FieldMediaInstruction(), 0, "B");
                break;
            case OPCODES::SOUND:
                ParseOpcode(full_opcode, "SOUND", new FieldMediaInstruction(), 0, "NwB");
                break;
            case OPCODES::AKAO:
                ParseOpcode(full_opcode, "AKAO", new FieldMediaInstruction(), 0, "NNNBBwwww");
                break;
            case OPCODES::MUSVT:
                ParseOpcode(full_opcode, "MUSVT", new FieldMediaInstruction(), 0, "B");
                break;
            case OPCODES::MUSVM:
                ParseOpcode(full_opcode, "MUSVM", new FieldMediaInstruction(), 0, "B");
                break;
            case OPCODES::MULCK:
                ParseOpcode(full_opcode, "MULCK", new FieldMediaInstruction(), 0, "B");
                break;
            case OPCODES::BMUSC:
                ParseOpcode(full_opcode, "BMUSC", new FieldMediaInstruction(), 0, "B");
                break;
            case OPCODES::CHMPH:
                ParseOpcode(full_opcode, "CHMPH", new FieldMediaInstruction(), 0, "BBB");
                break;
            case OPCODES::PMVIE:
                ParseOpcode(full_opcode, "PMVIE", new FieldMediaInstruction(), 0, "B");
                break;
            case OPCODES::MOVIE:
                ParseOpcode(full_opcode, "MOVIE", new FieldMediaInstruction(), 0, "");
                break;
            case OPCODES::MVIEF:
                ParseOpcode(full_opcode, "MVIEF", new FieldMediaInstruction(), 0, "NB");
                break;
            case OPCODES::FMUSC:
                ParseOpcode(full_opcode, "FMUSC", new FieldMediaInstruction(), 0, "B");
                break;
            case OPCODES::CMUSC:
                ParseOpcode(full_opcode, "CMUSC", new FieldMediaInstruction(), 0, "BBBBBBB");
                break;
            case OPCODES::CHMST:
                ParseOpcode(full_opcode, "CHMST", new FieldMediaInstruction(), 0, "NB");
                break;

            // Uncat
            case OPCODES::MPDSP:
                ParseOpcode(full_opcode, "MPDSP", new FieldUncategorizedInstruction(), 0, "B");
                break;
            case OPCODES::SETX:
                ParseOpcode(full_opcode, "SETX", new FieldUncategorizedInstruction(), 0, "BBBBBB");
                break;
            case OPCODES::GETX:
                ParseOpcode(full_opcode, "GETX", new FieldUncategorizedInstruction(), 0, "BBBBBB");
                break;
            case OPCODES::SEARCHX:
                ParseOpcode(full_opcode, "SEARCHX", new FieldUncategorizedInstruction(), 0, "BBBBBBBBBB");
                break;

            default:
                throw UnknownOpcodeException(this->address_, opcode);
        }
        this->address_++;

        // Exit on return found.
        if (full_opcode == OPCODES::RET) return is_line;
    }
    return is_line;
}
