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
#include "decompiler/field/FieldEngine.h"
#include "decompiler/field/FieldCodeGenerator.h"
#include "decompiler/field/FieldDisassembler.h"
#include "decompiler/field/instruction/FieldNoOperationInstruction.h"

/**
 * @todo Understand and document
 */
#define GET(vertex) (boost::get(boost::vertex_name, graph, vertex))

/*
 * TODO: OpCodes which need implementing.
 *
 * BLINK
 * XYI
 * CMOVE
 * MOVA
 * TURA
 * ANIMW
 * FMOVE
 * ANIME2
 * ANIM_1
 * CANIM1 ?
 * CANM_1
 * TURN
 * DIRA
 * GETDIR
 * GETAXY
 * TALKR
 * ANIMB
 * TURNW
 */

FF7::FieldEngine::Entity::Entity(const std::string& name, size_t index):
  name_(name), index_(index), is_line_(false)
{}

std::string FF7::FieldEngine::Entity::GetName() const{return name_;}

size_t FF7::FieldEngine::Entity::GetIndex() const{return index_;}

std::string FF7::FieldEngine::Entity::FunctionByIndex(size_t index) const{
    auto it = functions_.find(index);
    if (it == std::end(functions_)) throw InternalDecompilerError();
    return it->second;
}

void FF7::FieldEngine::Entity::AddFunction(const std::string& name, size_t index){
    functions_[index] = name;
}

void FF7::FieldEngine::Entity::MarkAsLine(
  bool line, std::vector<float> point_a, std::vector<float> point_b
){
    is_line_ = line;
    point_a_.clear();
    point_b_.clear();
    if (line){
        if (point_a.size() >= 3 && point_b.size() >= 3){
            point_a_.push_back(point_a[0]);
            point_a_.push_back(point_a[1]);
            point_a_.push_back(point_a[2]);
            point_b_.push_back(point_b[0]);
            point_b_.push_back(point_b[1]);
            point_b_.push_back(point_b[2]);
        }
        // TODO: Notify on else.
    }
    // TODO: These are not getting to the final script.
    // Maybe this can be removed?
    AddFunction("on_enter_line", 1);
    AddFunction("on_move_to_line", 2);
    AddFunction("on_cross_line", 3);
    AddFunction("on_leave_line", 4);
}

bool FF7::FieldEngine::Entity::IsLine(){return is_line_;}

std::vector<float> FF7::FieldEngine::Entity::GetLinePointA(){return point_a_;}

std::vector<float> FF7::FieldEngine::Entity::GetLinePointB(){return point_b_;}

FF7::FieldEngine::FieldEngine(SUDM::IScriptFormatter& formatter, std::string script_name) :
  formatter_(formatter), script_name_(script_name), scale_factor_(1.0f)
{setOutputStackEffect(false);}

std::unique_ptr<Disassembler> FF7::FieldEngine::GetDisassembler(
  InstVec &insts, const std::vector<unsigned char>& raw_script_data
){
    auto ret = std::make_unique<FieldDisassembler>(formatter_, this, insts, raw_script_data);
    scale_factor_ = ret->GetScaleFactor();
    return std::move(ret);
}

std::unique_ptr<Disassembler> FF7::FieldEngine::GetDisassembler(InstVec &insts){
    auto ret = std::make_unique<FieldDisassembler>(formatter_, this, insts);
    scale_factor_ = ret->GetScaleFactor();
    return std::move(ret);
}

std::unique_ptr<CodeGenerator> FF7::FieldEngine::GetCodeGenerator(
  const InstVec& insts, std::ostream &output
){
    // The broken version:
    //return std::make_unique<FieldCodeGenerator>(this, insts, output);
    // The not-as-nice-but-at-least-it-works version:
    return std::make_unique<FF7::FieldCodeGenerator>(this, insts, output, formatter_);
}

void FF7::FieldEngine::PostCFG(InstVec& insts, Graph graph){
    // In FF7 some scripts ends with an infinite loop to "keep it alive"
    // in V-Gears this isn't required so they can be removed.
    //RemoveTrailingInfiniteLoops(insts, graph);

    // This could generate bad code, but it always seems to follow that pattern that if the last
    // instruction is an uncond jump back into the script then it simply nests all of those blocks
    // in an infinite loop.
    //MarkInfiniteLoopGroups(insts, graph);

    // Scripts end with a "return". This isn't required so strip them out.
    //RemoveExtraneousReturnStatements(insts, graph);
}

bool FF7::FieldEngine::UsePureGrouping() const{return false;}

std::map<std::string, int> FF7::FieldEngine::GetEntities() const{
    std::map<std::string, int> r;
    for (auto& f : functions){
        const Function& func = f.second;
        FF7::FunctionMetaData meta(func.metadata);
        auto it = r.find(meta.GetEntityName());
        if (it != std::end(r)){
            // Try to find a function in this entity has that has a char id.
            // don't overwrite a valid char id with a "blank" one.
            if (it->second == -1) it->second = meta.GetCharacterId();
        }
        // TODO: Don't add line entities here:
        else r[meta.GetEntityName()] = meta.GetCharacterId();
    }
    return r;
}

std::vector<SUDM::FF7::Field::FieldEntity> FF7::FieldEngine::GetEntityList() const{
    std::vector<SUDM::FF7::Field::FieldEntity> entities;
    for (auto entity: entity_index_map_){
        if (entity.second.IsLine() == false){
            SUDM::FF7::Field::FieldEntity ent;
            ent.name = entity.second.GetName();
            ent.index = entity.second.GetIndex();

            // Get character ID.
            ent.char_id = -1;
            std::map<std::string, int> r;
            for (auto& f : functions){
                const Function& func = f.second;
                FF7::FunctionMetaData meta(func.metadata);
                if (meta.GetEntityName() == ent.name){
                    ent.char_id = meta.GetCharacterId();
                    break;
                }
            }
            entities.push_back(ent);
        }
    }
    return entities;
}

std::vector<SUDM::FF7::Field::Line> FF7::FieldEngine::GetLineList() const{
    std::vector<SUDM::FF7::Field::Line> lines;
    for (auto entity: entity_index_map_){
        if (entity.second.IsLine() == true){
            SUDM::FF7::Field::Line line;
            line.name = entity.second.GetName();
            line.point_a = entity.second.GetLinePointA();
            line.point_b = entity.second.GetLinePointB();
            lines.push_back(line);
        }
    }
    return lines;
}

void FF7::FieldEngine::AddEntityFunction(
  const std::string& entity_name, size_t entity_index,
  const std::string& func_name, size_t func_index
){
    auto it = entity_index_map_.find(entity_index);
    if (it != std::end(entity_index_map_)) (*it).second.AddFunction(func_name, func_index);
    else{
        Entity e(entity_name, entity_index);
        e.AddFunction(func_name, func_index);
        entity_index_map_.insert(std::make_pair(entity_index, e));
    }
}

void FF7::FieldEngine::MarkEntityAsLine(
  size_t entity_index, bool line, std::vector<float> point_a, std::vector<float> point_b
){
    auto it = entity_index_map_.find(entity_index);
    if (it != std::end(entity_index_map_)){
        (*it).second.MarkAsLine(line, point_a, point_b);
    }
}

bool FF7::FieldEngine::EntityIsLine(size_t entity_index){
    auto it = entity_index_map_.find(entity_index);
    if (it != std::end(entity_index_map_)) return (*it).second.IsLine();
    return false;
}

const FF7::FieldEngine::Entity& FF7::FieldEngine::EntityByIndex(size_t index) const{
    auto it = entity_index_map_.find(index);
    if (it == std::end(entity_index_map_)) throw InternalDecompilerError();
    return it->second;
}

float FF7::FieldEngine::GetScaleFactor() const {return scale_factor_;}

const std::string& FF7::FieldEngine::GetScriptName() const {return script_name_;}

void FF7::FieldEngine::RemoveExtraneousReturnStatements(InstVec& insts, Graph graph){
    for (auto& f : functions){
        Function& func = f.second;
        for (auto it = insts.begin(); it != insts.end(); it ++){
            // Is it the last instruction in the function, and is it a return statement?
            if ((*it)->_address == func.end_addr){
                if ((*it)->_opcode == OPCODES::RET){
                    // Set new end address to be before the NOP.
                    func.end_addr =(*(it - 1))->_address;
                    func.num_instructions --;
                    Instruction* nop = new FieldNoOperationInstruction();
                    nop->_opcode = OPCODES::NOP;
                    nop->_address = (*it)->_address;
                    (*it).reset(nop);
                    break;
                }
            }
        }
    }
}

void FF7::FieldEngine::RemoveTrailingInfiniteLoops(InstVec& insts, Graph graph){
    for (auto& f : functions){
        Function& func = f.second;
        for (auto it = insts.begin(); it != insts.end(); it ++){
            // Is it the last instruction in the function, a jump, and a jumping to itself?
            if ((*it)->_address == func.end_addr){
                if ((*it)->isJump() && (*it)->GetDestAddress() == (*it)->_address){
                    // Set new end address to be before the NOP
                    func.end_addr =(*(it - 1))->_address;
                    func.num_instructions --;
                    Instruction* nop = new FieldNoOperationInstruction();
                    nop->_opcode = OPCODES::NOP;
                    nop->_address = (*it)->_address;
                    (*it).reset(nop);
                    break;
                }
            }
        }
    }
}

void FF7::FieldEngine::MarkInfiniteLoopGroups(InstVec& insts, Graph graph){
    for (auto& f : functions){
        Function& func = f.second;
        for (auto it = insts.begin(); it != insts.end(); it ++){
            if ((*it)->_address == func.end_addr){
                // Note: This is a "best effort heuristic", so quite a few loops will
                // still end up as goto's. This could potentially generate invalid code too.
                if ((*it)->IsUncondJump()){
                    // Then assume its an infinite do { } while(true)
                    // loop that wraps part of the script.
                    VertexRange vr = boost::vertices(graph);
                    for (VertexIterator v = vr.first; v != vr.second; ++ v){
                        GroupPtr gr = GET(*v);
                        if ((*gr->start_)->_address == func.end_addr){
                            // Then assume its an infinite do { } while(true) loop
                            // that wraps part of the script.
                            gr->_type = kDoWhileCondGroupType;
                        }
                    }
                }
                break;
            }
        }
    }
}

