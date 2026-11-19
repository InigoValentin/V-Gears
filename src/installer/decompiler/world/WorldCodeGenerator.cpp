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

WorldCodeGenerator::WorldCodeGenerator(Engine *engine, std::ostream &output)
: CodeGenerator(engine, output, FIFO_ARGUMENT_ORDER, LIFO_ARGUMENT_ORDER){}

std::string WorldCodeGenerator::ConstructFuncSignature(const Function& function){
    return "System.WorldMap." + function.name + " = function(self)";
}

bool WorldCodeGenerator::OutputOnlyRequiredLabels() const{return true;}

void WorldCodeGenerator::OnEndFunction(const Function&){
    AddOutputLine("end", true, false);
}

const InstPtr WorldCodeGenerator::FindFirstCall(){
    ConstInstIterator it = cur_group_->start;
    do{
        if ((*it)->IsFuncCall() || (*it)->IsKernelCall()) return *it;
    } while (it ++ != cur_group_->end);
    return *cur_group_->start;
}

const InstPtr WorldCodeGenerator::FindLastCall(){
    ConstInstIterator it = cur_group_->end;
    do {
        if ((*it)->IsFuncCall() || (*it)->IsKernelCall()) return *it;
    } while (it -- != cur_group_->start);
    return *cur_group_->end;
}

void WorldCodeGenerator::ProcessSpecialMetadata(const InstPtr inst, char c, int pos){}

void WorldCodeGenerator::ProcessUnstructuredCondJump(InstVec& insts, const InstPtr inst){
    // "Jump if false" pops the condition, and jumps when it is false.
    const std::string condition = WorldEngine::AsCondition(stack_.Pop());
    const uint32 dst_addr = inst->GetDestAddress();
    if (is_label_pass_){
        for (auto& i : insts){
            if (i->GetAddress() == dst_addr){
                i->SetLabelRequired(true);
                break;
            }
        }
    }
    // The jump is taken when the condition is false: "not not x" is "x".
    const std::string jump_if = condition.compare(0, 4, "not ") == 0
      ? condition.substr(4) : "not " + condition;
    AddOutputLine("if " + jump_if + " then " + target_lang_->Goto(dst_addr) + " end");
}
