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
#include "installer/FieldDataInstaller.h"

#pragma message("Unimplemented test file: " __FILE__)

BOOST_AUTO_TEST_CASE(TestFieldScriptHeaderResetsTemporaryBanks){
    const std::string header = FieldDataInstaller::FieldScriptHeader();
    // The container must exist before anything else in the script uses it.
    BOOST_CHECK(header.find("EntityContainer = {}") == 0);
    BOOST_CHECK(header.find("for address = 0, Banks.SIZE do") != std::string::npos);
    BOOST_CHECK(header.find("Banks[5][address] = 0") != std::string::npos);
    // Only the temporary banks are reset: saved variables must survive a field change.
    BOOST_CHECK(header.find("Banks[1]") == std::string::npos);
    BOOST_CHECK(header.find("Banks[2]") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldDataInstaller){
    // TODO
    BOOST_WARN_MESSAGE(false, "Unimplemented test file: FieldDataInstaller");
    BOOST_CHECK(0 == 0);
}

