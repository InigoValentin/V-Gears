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
#include "decompiler/world/WorldCodeGenerator.h"
#include "decompiler/world/WorldDisassembler.h"
#include "decompiler/world/WorldEngine.h"

/**
 * @todo Understand and document
 */
#define GET(vertex) (boost::get(boost::vertex_name, g, vertex))

WorldEngine::BankValue::BankValue(std::string name) : VarValue(name){}

WorldEngine::BoolValue::BoolValue(std::string condition) : VarValue(condition){}

std::ostream& WorldEngine::BoolValue::Print(std::ostream& output) const{
    return output << "(" << name_ << " and 1 or 0)";
}

std::string WorldEngine::BoolValue::Condition() const{return name_;}

std::string WorldEngine::AsCondition(const ValuePtr& value){
    const BoolValue* boolean = dynamic_cast<const BoolValue*>(value.get());
    if (boolean != nullptr) return boolean->Condition();
    return "(" + value->GetString() + " ~= 0)";
}

WorldEngine::WorldEngine(int script_number) : script_number_(script_number){SetOutputStackEffect(true);}

std::unique_ptr<Disassembler> WorldEngine::GetDisassembler(InstVec &insts){
    return std::make_unique<WorldDisassembler>(this, insts, script_number_);
}

std::unique_ptr<CodeGenerator> WorldEngine::GetCodeGenerator(
  const InstVec& insts, std::ostream &output){
    return std::make_unique<WorldCodeGenerator>(this, output);
}

void WorldEngine::GetVariants(std::vector<std::string>&) const{}

bool WorldEngine::UsePureGrouping() const {return false;}
