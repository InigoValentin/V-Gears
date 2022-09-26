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

#include "sudm.h"
#include "decompiler/ff7_field/ff7_field_engine.h"
#include "decompiler/ff7_field/ff7_field_disassembler.h"
#include "decompiler/ff7_field/ff7_field_codegen.h"
#include "decompiler/control_flow.h"

namespace SUDM{
    namespace FF7{
        namespace Animation{} // TODO

        namespace AI{} // TODO

        namespace World{} // TODO

        namespace Field{

            float ScaleFactor(const std::vector<unsigned char>& script_bytes){
                // Could be cleaner, but just does enough work to pull out the fields scale.
                IScriptFormatter formatter;
                ::FF7::FF7FieldEngine engine(formatter, "Unused");
                InstVec insts;
                engine.getDisassembler(insts, script_bytes);
                return engine.ScaleFactor();
            }

            DecompiledScript Decompile(
              std::string script_name, const std::vector<unsigned char>& script_bytes,
              IScriptFormatter& formatter, std::string text_after, std::string text_before
            ){
                // Disassemble the script.
                ::FF7::FF7FieldEngine engine(formatter, script_name);
                InstVec insts;
                auto disassembler = engine.getDisassembler(insts, script_bytes);
                disassembler->disassemble();
                // Create control flow group.
                auto control_flow = std::make_unique<ControlFlow>(insts, engine);
                control_flow->createGroups();
                // Decompile/analyze
                //Graph graph = controlFlow->analyze();
                //engine.postCFG(insts, graph);
                Graph graph;
                DecompiledScript ds;
                // Generate code and return it.
                std::stringstream output;
                auto cg = engine.getCodeGenerator(insts, output);
                cg->generate(insts, graph);
                ds.luaScript = text_before + output.str() + text_after;
                for (auto entity : engine.GetEntityList()) ds.entities.push_back(entity);
                for (auto line : engine.GetLineList()) ds.lines.push_back(line);
                return ds;
            }
        }
    }

    namespace FF8{

        namespace Field{} // TODO
    }

    namespace FF9{

        namespace Field{} // TODO
    }
}
