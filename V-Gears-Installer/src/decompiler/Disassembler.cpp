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
