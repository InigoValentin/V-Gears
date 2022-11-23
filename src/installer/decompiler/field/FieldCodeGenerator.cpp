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

#include <boost/algorithm/string/predicate.hpp>
#include "FieldCodeGenerator.h"
#include "FieldEngine.h"

const std::string FunctionMetaData::DELIMITER("_");

const std::string FunctionMetaData::START("start");

const std::string FunctionMetaData::END("end");

FunctionMetaData::FunctionMetaData(std::string meta_data){Parse(meta_data);}

bool FunctionMetaData::IsStart() const{return start_;}

bool FunctionMetaData::IsEnd() const{return end_;}

std::string FunctionMetaData::GetEntityName(){return entity_name_;}

int FunctionMetaData::GetCharacterId(){return character_id_;}

void FunctionMetaData::Parse(std::string meta_data){
    std::deque<std::string> strs;
    boost::split(strs, meta_data, boost::is_any_of(DELIMITER), boost::token_compress_on);
    if (!strs.empty()){
        auto tmp = strs.front();
        strs.pop_front();
        ParseStart(tmp, strs);
    }
}

void FunctionMetaData::ParseStart(const std::string& item, std::deque<std::string>& strs){
    if (item == START){
        start_ = true;
        if (!strs.empty()){
            auto tmp = strs.front();
            strs.pop_front();
            ParseEnd(tmp, strs);
        }
    }
    else ParseEnd(item, strs);
}

void FunctionMetaData::ParseEnd(const std::string& item, std::deque<std::string>& strs){
    if (item == END){
        end_ = true;
        if (!strs.empty()){
            auto tmp = strs.front();
            strs.pop_front();
            ParseCharId(tmp, strs);
        }
    }
    else ParseCharId(item, strs);
}

void FunctionMetaData::ParseCharId(const std::string& item, std::deque<std::string>& strs){
    if (!item.empty()) character_id_ = std::stoi(item);
    if (!strs.empty()){
        auto tmp = strs.front();
        strs.pop_front();
        ParseEntity(tmp, strs);
    }
}

void FunctionMetaData::ParseEntity(const std::string& item, std::deque<std::string>& strs){
    entity_name_ = item;
    for (auto& part : strs) if (!part.empty()) entity_name_ += "_" + part;
}

void FieldCodeGenerator::Generate(InstVec& insts, const Graph& graph){
    // TODO: Break into parts
    auto instruction = insts.begin();
    
    std::vector<std::pair<Function&, InstVec>> functions_with_bodies;
    for (
      auto function = engine_->functions.begin(); function != engine_->functions.end(); ++ function
    ){
        InstVec body;
        for (size_t i = 0; i < function->second.num_instructions; ++ i, ++ instruction)
            body.push_back(*instruction);
        functions_with_bodies.push_back(std::pair<Function&, InstVec> {function->second, body});
    }

    for (
      auto function = functions_with_bodies.begin();
      function != functions_with_bodies.end();
      ++ function
    ){

        OnBeforeStartFunction(function->first);
        auto signature = ConstructFuncSignature(function->first);
        AddOutputLine(signature, false, true);
        OnStartFunction(function->first);
        
        // Generate labels for jump instruction targets.
        std::unordered_map<uint32, InstVec> labels;
        for (
          auto instruction = function->second.begin();
          instruction != function->second.end();
          ++ instruction
        ){
            if ((*instruction)->IsCondJump() || (*instruction)->IsUncondJump()){
                auto targetAddr = (*instruction)->GetDestAddress();
                auto label = labels.find(targetAddr);
                if (label == labels.end()) labels.insert({targetAddr, InstVec()});
                labels[targetAddr].push_back(*instruction);
            }
        }

        // Implemented instructions.

        // Some conditional jumps point to an address outside the function (i.e the starting
        // address of the next function) and the 'end' are not written to the Lua script. Keep
        // track of how many are needed and add them at the end of the function.
        int ends_needed = 0;
        for (
          auto instruction = function->second.begin();
          instruction != function->second.end();
          ++ instruction
        ){
            auto label = labels.find((*instruction)->GetAddress());
            if (label != labels.end()){
                bool needs_label = false;
                bool needs_new_line = false;
                for (auto origin = label->second.begin(); origin != label->second.end(); ++ origin){
                    if ((*origin)->IsCondJump()){
                        ends_needed --;
                        AddOutputLine("end", true, false);
                        needs_new_line = true;
                    }
                    else needs_label = true;
                }
                if (needs_new_line) AddOutputLine("");
                if (needs_label)
                    AddOutputLine((boost::format("::label_0x%1$X::") % label->first).str());
            }
            ValueStack stack;
            (*instruction)->ProcessInst(function->first, stack, engine_, this);
            //if (end_needed){
            //    AddOutputLine("end -- end if", true, false);
            //    end_needed = false;
            //}
            if ((*instruction)->IsCondJump()){
                ends_needed ++;
                AddOutputLine(
                  (boost::format("if (%s) then") % stack.Pop()->GetString()).str(), false, true
                );
                // If the next instruction is the last in the function, mark the next pass to
                // add an 'end' after the instruction to close the if.
                //if ((*(instruction + 1))->GetAddress() == (*(function->second.back())).GetAddress())
                //    end_needed = true;
            }
            else if ((*instruction)->IsUncondJump()){
                // If destination address is outside the functions, turn goto into a return.
                if ((*instruction)->GetDestAddress() > function->first.end_addr){
                    AddOutputLine(
                      "-- Overflowed jump to "
                      + (boost::format("0x%1$X") % (*instruction)->GetDestAddress()).str()
                      + " (last address in function is "
                      + (boost::format("0x%1$X)") % function->first.end_addr).str()
                    );
                    AddOutputLine("do return 0 end");
                }
                // Prevent backward jumps in the on_start script.
                else if (
                  "on_start" == function->first.name
                  && (*instruction)->GetDestAddress() <= (*instruction)->GetAddress()
                ){
                    AddOutputLine("-- No infinite loops in the on_start script.");
                    AddOutputLine((
                      boost::format("-- goto label_0x%1$X") % (*instruction)->GetDestAddress()
                    ).str());
                }
                else{
                    AddOutputLine(
                      (boost::format("goto label_0x%1$X") % (*instruction)->GetDestAddress()).str()
                    );
                }
            }
            // Else, already output'd.
        }
        for(; ends_needed > 0; ends_needed --) AddOutputLine("end -- clean-up", true, false);
        // Add missing return:
        if (
          "return 0" != lines_.at(lines_.size() - 1).line
          && "do return 0 end" != lines_.at(lines_.size() - 1).line
        ){
            AddOutputLine("do return 0 end", false, false);
        }
        OnEndFunction(function->first);
    }

    for (auto i = lines_.begin(); i != lines_.end(); ++i){
        if (i->unindent_before && indent_level_ > 0){
            indent_level_ --;
        }
        output_ << IndentString(i->line) << std::endl;
        if (i->indent_after) indent_level_++;
    }
}

void FieldCodeGenerator::AddOutputLine(
  std::string line, bool unindent_before, bool indent_after
){lines_.push_back(CodeLine(line, unindent_before, indent_after));}

float FieldCodeGenerator::GetScaleFactor() const
{return static_cast<FieldEngine*>(engine_)->GetScaleFactor();}

void FieldCodeGenerator::OnBeforeStartFunction(const Function& function){
    FunctionMetaData meta_data(function.metadata);
    if (meta_data.IsStart()){
        AddOutputLine("EntityContainer[\"" + meta_data.GetEntityName() + "\"] = {", false, true);
        if (meta_data.GetCharacterId() != -1)
            AddOutputLine(meta_data.GetEntityName() + " = nil,\n");
    }
    const auto comment = formatter_.GetFunctionComment(meta_data.GetEntityName(), function.name);
    if (!comment.empty()) AddOutputLine("-- " + comment);
}

void FieldCodeGenerator::OnStartFunction(const Function& func){
    // Add some hacks for Director entity before the comments
    FunctionMetaData meta_data(func.metadata);
    if (meta_data.GetEntityName() == "Director" && func.name == "on_start"){
        // TODO: Don't hardcode Cloud, Use the one from the previous level,
        AddOutputLine("-- HACK: Set default playable entity an ensure the camera follows it.");
        AddOutputLine("entity_manager:set_player_entity(\"Cloud\")");
        AddOutputLine("background2d:autoscroll_to_entity(entity_manager:get_entity(\"Cloud\"))\n");
    }
    AddOutputLine("--[[");
    for (const auto& inst : insts_){
        if (inst->GetAddress() >= func.start_addr && inst->GetAddress() <= func.end_addr){
            std::stringstream output;
            output << inst;
            AddOutputLine(output.str());
        }
    }
    AddOutputLine("]]\n");
}

void FieldCodeGenerator::OnEndFunction(const Function& function){
    // End function.
    AddOutputLine("end,", true, false);
    // End class?
    FunctionMetaData meta_data(function.metadata);
    if (meta_data.IsEnd()) AddOutputLine("}\n\n\n", true, false);
    else AddOutputLine("\n");
}

std::string FieldCodeGenerator::ConstructFuncSignature(const Function &function){
    // Generate name
    FunctionMetaData meta_data(function.metadata);
    return formatter_.GetFriendlyFunctionName(meta_data.GetEntityName(), function.name)
      + " = function(self)";
}

bool FieldCodeGenerator::OutputOnlyRequiredLabels() const{return true;}

FieldScriptFormatter& FieldCodeGenerator::GetFormatter(){return formatter_;}

const std::string FieldCodeGenerator::FormatInstructionNotImplemented(
  const std::string& entity, uint32 address, uint32 opcode
){
    return (
      boost::format(
        "-- log:log(\"In entity \\\"%1%\\\", address 0x%2$08x: "
        "instruction 0x%3$04x not implemented\")"
      ) % entity % address % opcode
    ).str();
}

const std::string FieldCodeGenerator::FormatInstructionNotImplemented(
  const std::string& entity, uint32 address, const Instruction& instruction
){
    std::stringstream parameterList;
    //for (auto i = instruction._params.begin(); i != instruction._params.end(); ++ i){
    for (auto i = instruction.GetParams().begin(); i != instruction.GetParams().end(); ++ i){
        //if (i != instruction._params.begin()) parameterList << ", ";
        if (i != instruction.GetParams().begin()) parameterList << ", ";
        parameterList << *i;
    }
    return(
      boost::format(
        "-- log:log(\"In entity \\\"%1%\\\", address 0x%2$08x: "
        "instruction %3%(%4%) not implemented\")"
        ) % entity % address % instruction.GetName() % parameterList.str()
      ).str();
}

const std::string FieldCodeGenerator::FormatBool(uint32 value){
    return value == 0 ? "false" : "true";
}

const std::string FieldCodeGenerator::FormatInvertedBool(uint32 value){
    return value == 0 ? "true" : "false";
}
