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

#include "decompiler/Engine.h"

std::unique_ptr<Disassembler> Engine::GetDisassembler(
  InstVec& insts, const std::vector<unsigned char>& c
){
    throw NotImplementedException();
}

void Engine::PostCFG(InstVec& insts, Graph graph){}

bool Engine::SupportsCodeFlow() const{return true;}

bool Engine::SupportsCodeGen() const{return true;}

void Engine::GetVariants(std::vector<std::string>&) const{};

bool Engine::UsePureGrouping() const{return false;}
