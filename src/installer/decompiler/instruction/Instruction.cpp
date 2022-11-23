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

#include "decompiler/instruction/Instruction.h"
#include "decompiler/CodeGenerator.h"
#include "decompiler/Engine.h"

bool output_stack_effect = true;

void SetOutputStackEffect(bool value){output_stack_effect = value;}

int Instruction::INST_TYPE_BINARY_OP(0);

int Instruction::INST_TYPE_BOOL_NEGATE(1);

int Instruction::INST_TYPE_CALL(2);

int Instruction::INST_TYPE_COND_JUMP(3);

int Instruction::INST_TYPE_DUP(4);

int Instruction::INST_TYPE_JUMP(5);

int Instruction::INST_TYPE_KERNEL_CALL(6);

int Instruction::INST_TYPE_LOAD(7);

int Instruction::INST_TYPE_RETURN(8);

int Instruction::INST_TYPE_STACK(9);

int Instruction::INST_TYPE_STORE(10);

int Instruction::INST_TYPE_UNARY_OP_PRE(11);

int Instruction::INST_TYPE_UNARY_OP_POST(12);

std::ostream &Instruction::Print(std::ostream &output) const {
    output << boost::format("%08x: %s") % address_ % name_;
    std::vector<ValuePtr>::const_iterator param;
    for (param = params_.begin(); param != params_.end(); ++param) {
        if (param != params_.begin()) output << ",";
        output << " " << *param;
    }
    if (output_stack_effect) output << boost::format(" (%d)") % stack_change_;
    return output;
}

bool Instruction::IsJump() const{return IsCondJump() || IsUncondJump();}

bool Instruction::IsCondJump() const{return false;}

bool Instruction::IsUncondJump() const{return false;}

bool Instruction::IsStackOp() const{return false;}

bool Instruction::IsFuncCall() const{return false;}

bool Instruction::IsReturn() const{return false;}

bool Instruction::IsKernelCall() const{return false;}

bool Instruction::IsLoad() const{return false;}

bool Instruction::IsStore() const{return false;}

uint32 Instruction::GetDestAddress() const{throw WrongTypeException();}

uint32 Instruction::GetOpcode() const{return opcode_;}

void Instruction::SetOpcode(uint32 opcode){opcode_ = opcode;}

uint32 Instruction::GetAddress() const{return address_;}

void Instruction::SetAddress(uint32 address){address_ = address;}

std::string Instruction::GetName() const{return name_;}

void Instruction::SetName(std::string name){name_ = name;}

int16 Instruction::GetStackChange() const{return stack_change_;}

void Instruction::SetStackChange(int16 stack_change){stack_change_ = stack_change;}

std::vector<ValuePtr> Instruction::GetParams() const{return params_;}

ValuePtr Instruction::GetParam(uint32 index) const{
    if (params_.size() > index) return params_[index];
    else return nullptr;
}

void Instruction::SetParams(std::vector<ValuePtr> params){params_ = params;}

void Instruction::AddParam(ValuePtr value){params_.push_back(value);}

std::string Instruction::GetCodeGenData() const{return code_gen_data_;}

void Instruction::SetCodeGenData(std::string code_gen_data){code_gen_data_ = code_gen_data;}

bool Instruction::LabelRequired() const{return label_required_;}

void Instruction::SetLabelRequired(bool required){label_required_ = required;}































