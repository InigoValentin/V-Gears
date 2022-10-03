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

#include "decompiler/Disassembler.h"

Disassembler::Disassembler(InstVec &insts) : insts_(insts), address_base_(0){}

void Disassembler::Open(const char *filename){
    stream_ = std::make_unique<BinaryReader>(BinaryReader::ReadAll(filename));
}

void Disassembler::DoDumpDisassembly(std::ostream &output){
    InstIterator inst;
    for (inst = insts_.begin(); inst != insts_.end(); ++ inst) output << *inst << "\n";
}

void Disassembler::Disassemble() {
    if (insts_.empty()) {
        stream_->Seek(0);
        DoDisassemble();
    }
}

void Disassembler::DumpDisassembly(std::ostream &output){
    Disassemble();
    DoDumpDisassembly(output);
}

/**
 * NIB is probably for nibble.
 *
 * @param v[in] @todo Understand and document.
 * @return @todo Understand and document.
 */
static inline unsigned int Nib2(unsigned int v){return (v & 0xF);}

/**
 * NIB is probably for nibble.
 *
 * @param v[in] @todo Understand and document.
 * @return @todo Understand and document.
 */
static inline unsigned int Nib1(unsigned int v){return (v >> 4) & 0xF;}


void Disassembler::ReadParams(InstPtr inst, const char *types){
    // Handle [] blocks as working on an individual element (i.e a BYTE,WORD etc)
    // this syntax allows picking of nibbles and bit fields into their own parameters.
    while (*types){
        std::string type(types, 1);
        if (type == "N"){ // Read nibbles
            const uint8 byte = stream_->ReadU8();
            inst->AddParam(new IntValue(Nib1(byte), false));
            inst->AddParam(new IntValue(Nib2(byte), false));
            address_ ++;
        }
        else if (type == "U"){
            const uint8 byte = stream_->ReadU8();
            inst->AddParam(new IntValue((byte >> 5) & 0x7, false));
            inst->AddParam(new IntValue((byte & 0x1F), false));
            address_ ++;
        }
        else inst->AddParam(ReadParameter(inst, type));
        types ++;
    }
}

void Disassembler::ReadParams(
  InstPtr inst, const char *types, const std::vector<std::string>& params
){
    while (*types){
        std::string type(types, 1);
        if (type == "N") address_ ++;
        else if (type == "U") address_ ++;
        types++;
    }
}

ValuePtr Disassembler::ReadParameter(InstPtr inst, std::string type){
    ValuePtr ret_val = NULL;
    if (type == "b"){ // signed byte.
        ret_val = new IntValue(stream_->ReadS8(), true);
        address_++;
    }
    else if (type == "B"){ // unsigned byte.
        ret_val = new IntValue((uint32)stream_->ReadU8(), false);
        address_++;
    }
    else if (type == "s"){ // 16-bit signed integer (short), little-endian.
        ret_val = new IntValue(stream_->ReadS16(), true);
        address_ += 2;
    }
    else if (type == "w"){ // 16-bit unsigned integer (word), little-endian.
        ret_val = new IntValue((uint32)stream_->ReadU16(), false);
        address_ += 2;
    }
    else if (type == "i"){ // 32-bit signed integer (int), little-endian.
        ret_val = new IntValue(stream_->ReadS32(), true);
        address_ += 4;
    }
    else if (type == "d"){ // 32-bit unsigned integer (dword), little-endian.
        ret_val = new IntValue(stream_->ReadU32(), false);
        address_ += 4;
    }
    else throw UnknownOpcodeParameterException(type);
    return ret_val;
}

