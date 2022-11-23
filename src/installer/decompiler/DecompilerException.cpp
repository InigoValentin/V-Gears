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

#include "decompiler/DecompilerException.h"
#include "common/scummsys.h"

UnknownOpcodeParameterException::UnknownOpcodeParameterException(std::string param){
    what_ = "Unknown opcode parameter string: " + param;
}

const char* UnknownOpcodeParameterException::what() const noexcept{return what_.c_str();}

UnknownConditionalOperatorException::UnknownConditionalOperatorException(
  unsigned int address, unsigned int op
){
    what_ = "Unknown conditional operator: " + std::to_string(op)
      + " at address " + std::to_string(address);
}

const char* UnknownConditionalOperatorException::what() const noexcept{return what_.c_str();}

UnknownOpcodeException::UnknownOpcodeException(uint32 address, uint32 opcode):
  address_(address), opcode_(opcode){}

const char* UnknownOpcodeException::what() const noexcept{
    sprintf(buffer_, "Unknown %s (address: 0x%08x, opcode: 0x%04x)", Type(), address_, opcode_);
    return buffer_;
}

const char*  UnknownOpcodeException::Type() const{return "Opcode";}

UnknownJumpTypeException::UnknownJumpTypeException(unsigned int address, unsigned int opcode){
    what_ = "Unknown jump type: " + std::to_string(opcode)
      + " at address " + std::to_string(address);
}

const char* UnknownJumpTypeException::what() const noexcept{return what_.c_str();}

UnknownSubOpcodeException::UnknownSubOpcodeException(unsigned int address, unsigned int opcode)
  : UnknownOpcodeException(address, opcode){}

const char* UnknownSubOpcodeException::Type() const{return "SubOpcode";}
