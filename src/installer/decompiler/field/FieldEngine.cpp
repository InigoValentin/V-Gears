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

FieldEngine::Entity::Entity(const std::string& name, size_t index): name_(name), index_(index), is_line_(false){}

std::string FieldEngine::Entity::GetName() const{return name_;}

size_t FieldEngine::Entity::GetIndex() const{return index_;}

std::string FieldEngine::Entity::FunctionByIndex(size_t index) const{
    auto it = functions_.find(index);
    if (it == std::end(functions_)) throw DecompilerException();
    return it->second;
}

void FieldEngine::Entity::AddFunction(const std::string& name, size_t index){functions_[index] = name;}

void FieldEngine::Entity::MarkAsLine(bool line, std::vector<float> point_a, std::vector<float> point_b){
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
    AddFunction("on_approach", 1);
    AddFunction("on_cross", 2);
    AddFunction("on_near", 3);
    AddFunction("on_leave", 4);
}

bool FieldEngine::Entity::IsLine(){return is_line_;}

std::vector<float> FieldEngine::Entity::GetLinePointA(){return point_a_;}

std::vector<float> FieldEngine::Entity::GetLinePointB(){return point_b_;}

FieldEngine::FieldEngine(FieldScriptFormatter& formatter, std::string script_name) :
  formatter_(formatter), script_name_(script_name), scale_factor_(1.0f)
{SetOutputStackEffect(false);}

std::unique_ptr<Disassembler> FieldEngine::GetDisassembler(
  InstVec &insts, const std::vector<unsigned char>& raw_script_data
){
    auto ret = std::make_unique<FieldDisassembler>(formatter_, this, insts, raw_script_data);
    scale_factor_ = ret->GetScaleFactor();
    return std::move(ret);
}

std::unique_ptr<Disassembler> FieldEngine::GetDisassembler(InstVec &insts){
    auto ret = std::make_unique<FieldDisassembler>(formatter_, this, insts);
    scale_factor_ = ret->GetScaleFactor();
    return std::move(ret);
}

std::unique_ptr<CodeGenerator> FieldEngine::GetCodeGenerator(const InstVec& insts, std::ostream &output){
    // The broken version:
    //return std::make_unique<FieldCodeGenerator>(this, insts, output);
    // The not-as-nice-but-at-least-it-works version:
    return std::make_unique<FieldCodeGenerator>(this, insts, output, formatter_);
}

bool FieldEngine::UsePureGrouping() const{return false;}

std::vector<FieldDecompiler::FieldEntity> FieldEngine::GetEntityList() const{
    std::vector<FieldDecompiler::FieldEntity> entities;
    for (auto entity: entity_index_map_){
        if (entity.second.IsLine() == false){
            FieldDecompiler::FieldEntity ent;
            ent.name = entity.second.GetName();
            ent.index = entity.second.GetIndex();

            // Get character ID.
            ent.char_id = -1;
            std::map<std::string, int> r;
            for (auto& f : functions){
                const Function& func = f.second;
                FunctionMetaData meta(func.metadata);
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

std::vector<FieldDecompiler::Line> FieldEngine::GetLineList() const{
    std::vector<FieldDecompiler::Line> lines;
    for (auto entity: entity_index_map_){
        if (entity.second.IsLine() == true){
            FieldDecompiler::Line line;
            line.name = entity.second.GetName();
            line.index = entity.second.GetIndex();
            line.point_a = entity.second.GetLinePointA();
            line.point_b = entity.second.GetLinePointB();
            lines.push_back(line);
        }
    }
    return lines;
}

void FieldEngine::AddBackgroundArea(int area){
    if (area < 0) background_areas_dynamic_ = true;
    else background_areas_.insert(area);
}

const std::set<int>& FieldEngine::GetBackgroundAreas() const{return background_areas_;}

bool FieldEngine::BackgroundAreasAreDynamic() const{return background_areas_dynamic_;}

void FieldEngine::AddPalettePage(int page){
    if (page < 0) palette_pages_dynamic_ = true;
    else palette_pages_.insert(page);
}

const std::set<int>& FieldEngine::GetPalettePages() const{return palette_pages_;}

bool FieldEngine::PalettePagesAreDynamic() const{return palette_pages_dynamic_;}

void FieldEngine::AddEntityFunction(
  const std::string& entity_name, size_t entity_index, const std::string& func_name, size_t func_index
){
    auto it = entity_index_map_.find(entity_index);
    if (it != std::end(entity_index_map_)) (*it).second.AddFunction(func_name, func_index);
    else{
        Entity e(entity_name, entity_index);
        e.AddFunction(func_name, func_index);
        entity_index_map_.insert(std::make_pair(entity_index, e));
    }
}

void FieldEngine::AddFunctionAlias(
  const std::string& entity_name, const std::string& alias, const std::string& target
){
    function_aliases_.push_back(FunctionAlias{entity_name, alias, target});
}

void FieldEngine::AddFunctionFallThrough(
  const std::string& entity_name, const std::string& function, const std::string& target
){
    function_fall_through_[entity_name + "/" + function] = target;
}

std::string FieldEngine::GetFunctionFallThrough(
  const std::string& entity_name, const std::string& function
) const{
    const auto it = function_fall_through_.find(entity_name + "/" + function);
    return it == function_fall_through_.end() ? "" : it->second;
}

void FieldEngine::MarkEntityAsLine(
  size_t entity_index, bool line, std::vector<float> point_a, std::vector<float> point_b
){
    auto it = entity_index_map_.find(entity_index);
    if (it != std::end(entity_index_map_)) (*it).second.MarkAsLine(line, point_a, point_b);
}

bool FieldEngine::EntityIsLine(size_t entity_index){
    auto it = entity_index_map_.find(entity_index);
    if (it != std::end(entity_index_map_)) return (*it).second.IsLine();
    return false;
}

const FieldEngine::Entity& FieldEngine::EntityByIndex(size_t index) const{
    auto it = entity_index_map_.find(index);
    if (it == std::end(entity_index_map_)) throw DecompilerException();
    return it->second;
}

float FieldEngine::GetScaleFactor() const {return scale_factor_;}

const std::string& FieldEngine::GetScriptName() const {return script_name_;}

