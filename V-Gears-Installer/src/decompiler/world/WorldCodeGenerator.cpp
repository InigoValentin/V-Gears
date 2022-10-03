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

#include "decompiler/world/WorldCodeGenerator.h"

#include "decompiler/world/WorldEngine.h"

FF7::WorldCodeGenerator::WorldCodeGenerator(Engine *engine, std::ostream &output)
: CodeGenerator(engine, output, FIFO_ARGUMENT_ORDER, LIFO_ARGUMENT_ORDER){}

std::string FF7::WorldCodeGenerator::ConstructFuncSignature(const Function& function){
    // TODO: Implement.
    return function.name + " = function(self)";
}

const InstPtr FF7::WorldCodeGenerator::FindFirstCall(){
    ConstInstIterator it = cur_group_->start;
    do{
        if ((*it)->IsFuncCall() || (*it)->IsKernelCall()) return *it;
    } while (it ++ != cur_group_->end);
    return *cur_group_->start;
}

const InstPtr FF7::WorldCodeGenerator::FindLastCall(){
    ConstInstIterator it = cur_group_->end;
    do {
        if ((*it)->IsFuncCall() || (*it)->IsKernelCall()) return *it;
    } while (it -- != cur_group_->start);
    return *cur_group_->end;
}

void FF7::WorldCodeGenerator::ProcessSpecialMetadata(const InstPtr inst, char c, int pos){}
