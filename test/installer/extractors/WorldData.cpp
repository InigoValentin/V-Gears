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

#include <sstream>
#include <boost/test/unit_test.hpp>
#include "installer/extractors/WorldEncounterInstaller.h"
#include "installer/extractors/WorldScriptInstaller.h"

BOOST_AUTO_TEST_CASE(TestWorldEventFileNames){
    BOOST_CHECK(WorldScriptInstaller::IsEventFile("wm0.ev"));
    BOOST_CHECK(WorldScriptInstaller::IsEventFile("wm12.ev"));
    BOOST_CHECK(!WorldScriptInstaller::IsEventFile("wm.ev"));
    BOOST_CHECK(!WorldScriptInstaller::IsEventFile("wmx.ev"));
    BOOST_CHECK(!WorldScriptInstaller::IsEventFile("wm0.tex"));
    BOOST_CHECK(!WorldScriptInstaller::IsEventFile("ab0.ev"));
}

BOOST_AUTO_TEST_CASE(TestWorldFunctionTables){
    // Function 0 is of an entity (type 1) and function 1 of a mesh (type 2).
    const unsigned int entity = (1 << 14) | (5 << 8) | 3;
    const unsigned int mesh = (2 << 14) | (100 << 4) | 2;
    const std::vector<u8> events{
      static_cast<u8>(entity & 0xFF), static_cast<u8>(entity >> 8), 0, 0,
      static_cast<u8>(mesh & 0xFF), static_cast<u8>(mesh >> 8), 0, 0
    };
    std::ostringstream meshes;
    WorldScriptInstaller::WriteMeshFunctions(meshes, events, {true, true});
    BOOST_CHECK(meshes.str().find("System.WorldMap.triangle_functions[100][2] = 1\n") != std::string::npos);
    BOOST_CHECK(meshes.str().find("= 0\n") == std::string::npos);
    std::ostringstream entities;
    WorldScriptInstaller::WriteEntityFunctions(entities, events, {true, true});
    BOOST_CHECK(entities.str().find("System.WorldMap.entity_functions[5][3] = 0\n") != std::string::npos);
    BOOST_CHECK(entities.str().find("= 1\n") == std::string::npos);
    // Functions that were not decompiled are not in the tables.
    std::ostringstream none;
    WorldScriptInstaller::WriteMeshFunctions(none, events, {false, false});
    BOOST_CHECK(none.str().empty());
}

BOOST_AUTO_TEST_CASE(TestWorldFieldTableSkipsFieldsNotInstalled){
    std::vector<u8> table(WorldScriptInstaller::FIELD_TABLE_SIZE, 0xFF);
    // First record, first scenario: field 1, at (-256, 128), triangle 7, direction 64.
    const u8 first[12] = {0x00, 0xFF, 0x80, 0x00, 7, 0, 1, 0, 64, 0, 0, 0};
    std::copy(first, first + 12, table.begin());
    // First record, second scenario: field 2, which is not installed.
    const u8 second[12] = {0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0};
    std::copy(second, second + 12, table.begin() + 12);
    std::ostringstream script;
    WorldScriptInstaller::WriteFieldTable(
      script, table, {"startmap", "md1stin", "md1_1"}, {{1, 1.0f}, {2, 1.0f}}, {"md1stin"}
    );
    BOOST_CHECK(script.str().find(
      "System.WorldMap.field_table[1][0] = {field = \"md1stin\", x = -2, y = 1, triangle = 7, direction = 90}"
    ) != std::string::npos);
    BOOST_CHECK(script.str().find("md1_1") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestWorldEncounters){
    BOOST_CHECK(!WorldEncounterInstaller::CreateEncounters(std::vector<u8>(100)));
    std::vector<u8> data(0xA0 + 64 * 32, 0);
    // Region 1, terrain group 2: enabled, rate 40, first word 0x1234.
    const size_t record = 0xA0 + 6 * 32;
    data[record] = 1;
    data[record + 1] = 40;
    data[record + 2] = 0x34;
    data[record + 3] = 0x12;
    const auto encounters = WorldEncounterInstaller::CreateEncounters(data);
    BOOST_REQUIRE(encounters);
    const TiXmlElement* element = encounters->FirstChildElement("record");
    for (int i = 0; i < 6; ++ i) element = element->NextSiblingElement("record");
    BOOST_CHECK_EQUAL(std::string(element->Attribute("region")), "1");
    BOOST_CHECK_EQUAL(std::string(element->Attribute("enabled")), "1");
    BOOST_CHECK_EQUAL(std::string(element->Attribute("rate")), "40");
    BOOST_CHECK_EQUAL(std::string(element->Attribute("words")).substr(0, 7), "4660 0 ");
}
