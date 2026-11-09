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
#include "installer/decompiler/field/FieldCodeGenerator.h"

BOOST_AUTO_TEST_CASE(TestFieldCodeGenerator){
    FunctionMetaData entity_only("start_end_Cloud");
    BOOST_CHECK(entity_only.IsStart());
    BOOST_CHECK(entity_only.IsEnd());
    BOOST_CHECK_EQUAL(entity_only.GetEntityName(), "Cloud");
    BOOST_CHECK_EQUAL(entity_only.GetCharacterId(), -1);

    FunctionMetaData with_character("start_end_0_Cloud");
    BOOST_CHECK(with_character.IsStart());
    BOOST_CHECK(with_character.IsEnd());
    BOOST_CHECK_EQUAL(with_character.GetEntityName(), "Cloud");
    BOOST_CHECK_EQUAL(with_character.GetCharacterId(), 0);
}

