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

#include <boost/test/unit_test.hpp>
#include <sstream>
#include <utility>
#include <vector>
#include "installer/decompiler/ControlFlow.h"
#include "installer/decompiler/world/WorldEngine.h"

BOOST_AUTO_TEST_CASE(TestWorldDisassembler){
    std::vector<unsigned char> data(1026, 0);
    data[4] = 1;
    data[1024] = 0x03;
    data[1025] = 0x02;

    WorldEngine engine(1);
    InstVec instructions;
    std::unique_ptr<Disassembler> disassembler = engine.GetDisassembler(instructions);
        disassembler->Open(std::move(data));
    disassembler->Disassemble();

    BOOST_REQUIRE_EQUAL(instructions.size(), 1u);
    BOOST_REQUIRE_EQUAL(engine.functions.size(), 1u);
    BOOST_CHECK_EQUAL(engine.functions.begin()->second.name, "world_function_1");

        ControlFlow control_flow(instructions, engine);
        control_flow.CreateGroups();
        std::ostringstream generated_script;
        std::unique_ptr<CodeGenerator> code_generator
            = engine.GetCodeGenerator(instructions, generated_script);
        code_generator->Generate(instructions, control_flow.GetGraph());
        BOOST_CHECK_NE(
            generated_script.str().find(
                "System.WorldMap.world_function_1 = function(self)"
            ),
            std::string::npos
        );
        BOOST_CHECK_NE(generated_script.str().find("return;"), std::string::npos);
        BOOST_CHECK_NE(generated_script.str().find("end"), std::string::npos);
        BOOST_CHECK_EQUAL(generated_script.str().find("}\n"), std::string::npos);
}

