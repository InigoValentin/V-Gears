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

FF7::WorldEngine::BankValue::BankValue(std::string name) : VarValue(name){}

FF7::WorldEngine::WorldEngine(int script_number) : script_number_(script_number){
    SetOutputStackEffect(true);
}

std::unique_ptr<Disassembler> FF7::WorldEngine::GetDisassembler(InstVec &insts){
    return std::make_unique<WorldDisassembler>(this, insts, script_number_);
}

std::unique_ptr<CodeGenerator> FF7::WorldEngine::GetCodeGenerator(
  const InstVec& insts, std::ostream &output){
    return std::make_unique<WorldCodeGenerator>(this, output);
}

void FF7::WorldEngine::PostCFG(InstVec&, Graph graph){
    /*
    VertexRange vr = boost::vertices(graph);
    for (VertexIterator v = vr.first; v != vr.second; ++ v){
        GroupPtr gr = GET(*v);
        // If this group is the last instruction and its an unconditional jump
        if ((*gr->_start)->_address == insts.back()->_address && insts.back()->IsUncondJump()){
            // Then assume its an infinite do { } while(true) loop that wraps part of the script
            gr->_type = kDoWhileCondGroupType;
        }
    }*/
}

void FF7::WorldEngine::GetVariants(std::vector<std::string>&) const{}

bool FF7::WorldEngine::UsePureGrouping() const {return false;}
