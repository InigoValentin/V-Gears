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
#include "installer/data/FF7Data.h"


BOOST_AUTO_TEST_CASE(TestFF7DataFile){
    BOOST_CHECK(FF7Data::GetEnemyModelId(0) == "aa");
    BOOST_CHECK(FF7Data::GetEnemyModelId(19) == "at");
    BOOST_CHECK(FF7Data::GetEnemyModelId(25) == "az");
    BOOST_CHECK(FF7Data::GetEnemyModelId(26) == "ba");
    BOOST_CHECK(FF7Data::GetEnemyModelId(650) == "");
    BOOST_CHECK(FF7Data::GetEnemyModelId(30000) == "");
}

