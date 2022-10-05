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

#include "decompiler/Value.h"
#include <boost/format.hpp>
#include <map>
#include <sstream>
#include <string>

static int dupindex = 0;
static std::map<std::string, int> binary_op_precedence;
static std::map<std::string, std::string> negate_map;

void InitPrecedence() {
    binary_op_precedence["||"] = PRECEDENCE_LOGIC_OR;
    binary_op_precedence["&&"] = PRECEDENCE_LOGIC_AND;
    binary_op_precedence["|"] = PRECEDENCE_BIT_OR;
    binary_op_precedence["^"] = PRECEDENCE_BIT_XOR;
    binary_op_precedence["&"] = PRECEDENCE_BIT_AND;
    binary_op_precedence["=="] = PRECEDENCE_EQUALITY;
    binary_op_precedence["!="] = PRECEDENCE_EQUALITY;
    binary_op_precedence["<"] = PRECEDENCE_RELATION;
    binary_op_precedence["<="] = PRECEDENCE_RELATION;
    binary_op_precedence[">="] = PRECEDENCE_RELATION;
    binary_op_precedence[">"] = PRECEDENCE_RELATION;
    binary_op_precedence["<<"] = PRECEDENCE_SHIFT;
    binary_op_precedence[">>"] = PRECEDENCE_SHIFT;
    binary_op_precedence["+"] = PRECEDENCE_ADD;
    binary_op_precedence["-"] = PRECEDENCE_ADD;
    binary_op_precedence["*"] = PRECEDENCE_MULT;
    binary_op_precedence["/"] = PRECEDENCE_MULT;
    binary_op_precedence["%"] = PRECEDENCE_MULT;
}

void InitNegateMap() {
    negate_map["=="] = "!=";
    negate_map["!="] = "==";
    negate_map["<"] = ">=";
    negate_map["<="] = ">";
    negate_map[">="] = "<";
    negate_map[">"] = "<=";
}

Value::~Value(){}

bool Value::IsInteger(){return false;}

bool Value::IsAddress(){return false;}

bool Value::IsSignedValue(){throw WrongTypeException();}

int32 Value::GetSigned(){throw WrongTypeException();}

uint32 Value::GetUnsigned(){throw WrongTypeException();}

ValuePtr Value::Dup(std::ostream &output) {
    ValuePtr dup_value = new DupValue(++dupindex);
    output << dup_value << " = " << this << ";";
    return dup_value;
}

ValuePtr Value::Negate(){return new NegatedValue(this);}

std::string Value::GetString() const {
    std::stringstream s;
    Print(s);
    return s.str();
}

int Value::GetPrecedence() const{return PRECEDENCE_NO;}

IntValue::IntValue(int32 val, bool is_signed) : val_(val), signed_(is_signed){}

IntValue::IntValue(uint32 val, bool is_signed) : val_(val), signed_(is_signed){}

bool IntValue::IsInteger(){return true;}

bool IntValue::IsSignedValue(){return signed_;}

int32 IntValue::GetSigned(){return val_;}

uint32 IntValue::GetUnsigned(){return (uint32)val_;}

ValuePtr IntValue::Dup(std::ostream&){return new IntValue(val_, signed_);}

std::ostream &IntValue::Print(std::ostream &output) const{
    if (signed_) output << (int32)val_;
    else output << (uint32)val_;
    return output;
}

AddressValue::AddressValue(uint32 addr): IntValue(addr, false){}

bool AddressValue::IsAddress(){return true;}

int32 AddressValue::GetSigned(){throw WrongTypeException();}

ValuePtr AddressValue::Dup(std::ostream&){return new AddressValue(val_);}

std::ostream &AddressValue::Print(std::ostream &output) const{
    return output << boost::format("0x%X") % val_;
}

RelAddressValue::RelAddressValue(uint32 base_addr, int32 offset):
  IntValue(offset, true), base_addr_(base_addr){};

bool RelAddressValue::IsAddress(){return true;}

uint32 RelAddressValue::GetUnsigned(){return base_addr_ + val_;}

ValuePtr RelAddressValue::Dup(std::ostream&){return new RelAddressValue(base_addr_, val_);}

std::ostream &RelAddressValue::Print(std::ostream &output) const{
    if (val_ < 0)   return output << boost::format("-0x%X") % -val_;
    return output << boost::format("+0x%X") % val_;
}

DupValue::DupValue(int index): index_(index){}

ValuePtr DupValue::Dup(std::ostream&){return this;}

std::ostream &DupValue::Print(std::ostream &output) const{return output << "temp" << index_;}

StringValue::StringValue(std::string str): str_(str){}

std::ostream &StringValue::Print(std::ostream &output) const{
    return output << "\"" << str_ << "\"";
}

UnquotedStringValue::UnquotedStringValue(std::string str): StringValue(str){}

std::ostream& UnquotedStringValue::Print(std::ostream& output) const{return output << str_;}

VarValue::VarValue(std::string name): name_(name){}

std::ostream &VarValue::Print(std::ostream &output) const{return output << name_;}

ArrayValue::ArrayValue(const std::string name, const ValueList indexes):
  VarValue(name), indexes_(indexes){}

std::ostream &ArrayValue::Print(std::ostream &output) const {
    output << name_;
    for (ValueList::const_iterator i = indexes_.begin(); i != indexes_.end(); ++ i)
        output << "[" << *i << "]";
    return output;
}

BinaryOpValue::BinaryOpValue(ValuePtr left, ValuePtr right, std::string oper):
  left_val_(left), right_val_(right), oper_(oper){}

std::ostream &BinaryOpValue::Print(std::ostream &output) const {
    if (left_val_->GetPrecedence() > GetPrecedence()) output <<  "(" << left_val_ << ")";
    else output << left_val_;
    output << " " << oper_ << " ";
    if (right_val_->GetPrecedence() > GetPrecedence()) output << "(" << right_val_ << ")";
    else output << right_val_;
    return output;
}

int BinaryOpValue::GetPrecedence() const {
    if (binary_op_precedence.empty()) InitPrecedence();
    return binary_op_precedence[oper_];
}

ValuePtr BinaryOpValue::Negate(){
    if (negate_map.empty()) InitNegateMap();
    if (negate_map.find(oper_) == negate_map.end()) return Value::Negate();
    else return new BinaryOpValue(left_val_, right_val_, negate_map[oper_]);
}

UnaryOpValue::UnaryOpValue(ValuePtr operand, std::string oper, bool postfix):
  operand_(operand), oper_(oper), postfix_(postfix){}

std::ostream &UnaryOpValue::Print(std::ostream &output) const {
    if (!postfix_) output << oper_;
    if (operand_->GetPrecedence() > GetPrecedence()) output << "(" << operand_ << ")";
    else output << operand_;
    if (postfix_) output << oper_;
    return output;
}

int UnaryOpValue::GetPrecedence() const {return PRECEDENCE_UNARY;}

NegatedValue::NegatedValue(ValuePtr val): UnaryOpValue(val, "!", false){}

ValuePtr NegatedValue::Negate(){return operand_;}

CallValue::CallValue(std::string function, ValueList args): function_(function), args_(args){}

std::ostream &CallValue::Print(std::ostream &output) const {
    output << function_ << "(";
    for (ValueList::const_iterator i = args_.begin(); i != args_.end(); ++i) {
        if (i != args_.begin()) output << ", ";
        output << *i;
    }
    output << ")";
    return output;
}
