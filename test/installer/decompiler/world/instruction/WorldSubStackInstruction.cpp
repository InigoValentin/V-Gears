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
#include "installer/decompiler/world/instruction/WorldSubStackInstruction.h"

BOOST_AUTO_TEST_CASE(TestWorldSubStackInstructionLeftShiftIsValidLua){
    WorldSubStackInstruction instruction;
    instruction.SetOpcode(0x51);

    ValueStack stack;
    stack.Push(new VarValue("2"));
    stack.Push(new VarValue("2928 - Read(912)"));
    Function function;
    instruction.ProcessInst(function, stack, nullptr, nullptr);

    BOOST_CHECK_EQUAL(
        stack.Pop()->GetString(), "((2928 - Read(912)) * (2 ^ 2))"
    );
}

