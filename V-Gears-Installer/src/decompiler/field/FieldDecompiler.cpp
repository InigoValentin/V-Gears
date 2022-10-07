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

#include "decompiler/field/FieldDecompiler.h"
#include "decompiler/field/FieldEngine.h"
#include "decompiler/ControlFlow.h"

float FieldDecompiler::ScaleFactor(const std::vector<unsigned char>& script_bytes){
    // Could be cleaner, but just does enough work to pull out the fields scale.
    FieldScriptFormatter formatter;
    FieldEngine engine(formatter, "Unused");
    InstVec insts;
    engine.GetDisassembler(insts, script_bytes);
    return engine.GetScaleFactor();
}

FieldDecompiler::DecompiledScript FieldDecompiler::Decompile(
  std::string script_name, const std::vector<unsigned char>& script_bytes,
  FieldScriptFormatter& formatter, std::string text_after, std::string text_before
){
    // Disassemble the script.
    FieldEngine engine(formatter, script_name);
    InstVec insts;
    auto disassembler = engine.GetDisassembler(insts, script_bytes);
    disassembler->Disassemble();
    // Create control flow group.
    auto control_flow = std::make_unique<ControlFlow>(insts, engine);
    control_flow->CreateGroups();
    // Decompile/analyze
    //Graph graph = controlFlow->analyze();
    //engine.PostCFG(insts, graph);
    Graph graph;
    DecompiledScript ds;
    // Generate code and return it.
    std::stringstream output;
    auto cg = engine.GetCodeGenerator(insts, output);
    cg->Generate(insts, graph);
    ds.luaScript = text_before + output.str() + text_after;
    for (auto entity : engine.GetEntityList()) ds.entities.push_back(entity);
    for (auto line : engine.GetLineList()) ds.lines.push_back(line);
    return ds;
}
