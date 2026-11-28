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
#include "installer/extractors/FieldBackgroundInstaller.h"
#include "installer/extractors/FieldDataTypes.h"
#include "installer/extractors/FieldMapWriter.h"
#include "installer/extractors/FieldScriptInstaller.h"

BOOST_AUTO_TEST_CASE(TestFieldScriptHeaderResetsTemporaryBanks){
    const std::string header = FieldScriptInstaller::FieldScriptHeader();
    // The container must exist before anything else in the script uses it.
    BOOST_CHECK(header.find("EntityContainer = {}") == 0);
    BOOST_CHECK(header.find("for address = 0, Banks.SIZE do") != std::string::npos);
    BOOST_CHECK(header.find("Banks[5][address] = 0") != std::string::npos);
    // Only the temporary banks are reset: saved variables must survive a field change.
    BOOST_CHECK(header.find("Banks[1]") == std::string::npos);
    BOOST_CHECK(header.find("Banks[2]") == std::string::npos);
    // Without a field ID there is nothing for LSTMP.
    BOOST_CHECK(header.find("Data.FieldId") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldScriptHeaderRecordsTheFieldIds){
    const std::string header = FieldScriptInstaller::FieldScriptHeader(117);
    BOOST_CHECK(header.find("if Data.FieldId ~= 117 then") != std::string::npos);
    BOOST_CHECK(header.find("Data.PreviousFieldId = Data.FieldId or 0") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(TestFieldIdAndScaleFactor){
    const MapList map_list{"startmap", "md1stin", "md1_1"};
    BOOST_CHECK_EQUAL(FieldData::FieldId("md1_1", map_list), 2u);
    BOOST_CHECK_THROW(FieldData::FieldId("nowhere", map_list), std::runtime_error);
    const FieldScaleFactorMap scale_factors{{1, 0.5f}};
    BOOST_CHECK_EQUAL(FieldData::ScaleFactor(scale_factors, 1), 0.5f);
    BOOST_CHECK_THROW(FieldData::ScaleFactor(scale_factors, 2), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(TestSpawnPointsAreGroupedByDestination){
    FieldSpawnPointsMap spawn_points;
    SpawnPointDb::Record record;
    record.gateway = {};
    record.gateway.destination_field_id = 7;
    record.field_id = 1;
    FieldData::AddSpawnPoint(spawn_points, record);
    record.field_id = 2;
    FieldData::AddSpawnPoint(spawn_points, record);
    record.gateway.destination_field_id = 9;
    FieldData::AddSpawnPoint(spawn_points, record);
    BOOST_REQUIRE_EQUAL(spawn_points.size(), 2u);
    BOOST_CHECK_EQUAL(spawn_points[7].target_field_id, 7);
    BOOST_REQUIRE_EQUAL(spawn_points[7].gateways_to_this_field.size(), 2u);
    BOOST_CHECK_EQUAL(spawn_points[7].gateways_to_this_field[0].field_id, 1);
    BOOST_CHECK_EQUAL(spawn_points[7].gateways_to_this_field[1].field_id, 2);
    BOOST_CHECK_EQUAL(spawn_points[9].gateways_to_this_field.size(), 1u);
}

BOOST_AUTO_TEST_CASE(TestUnusedBackgroundTriggersAreAllZeroes){
    VGears::TriggersFile::Trigger trigger = {};
    BOOST_CHECK(!FieldData::IsBackgroundTriggerUsed(trigger));
    trigger.trigger_line[1].z = -3;
    BOOST_CHECK(FieldData::IsBackgroundTriggerUsed(trigger));
}

BOOST_AUTO_TEST_CASE(TestMusicTracksFollowTheAkaoStrings){
    const std::vector<u8> script{
      0, 'A', 'K', 'A', 'O', 5, 1, 2, 'A', 'K', 'O', 9, 'A', 'K', 'A', 'O', 14, 0, 0, 0, 0
    };
    const std::vector<int> tracks = FieldMapWriter::ExtractMusicTrackIds(script);
    // The ID is the byte after the string, minus 2.
    BOOST_REQUIRE_EQUAL(tracks.size(), 2u);
    BOOST_CHECK_EQUAL(tracks[0], 3);
    BOOST_CHECK_EQUAL(tracks[1], 12);
}

BOOST_AUTO_TEST_CASE(TestMusicTracksAtTheEndOfShortScripts){
    // The string can end the script, and a script shorter than the string has no tracks.
    BOOST_CHECK_EQUAL(FieldMapWriter::ExtractMusicTrackIds({'A', 'K', 'A', 'O', 7}).size(), 1u);
    BOOST_CHECK(FieldMapWriter::ExtractMusicTrackIds({'A', 'K'}).empty());
    BOOST_CHECK(FieldMapWriter::ExtractMusicTrackIds({}).empty());
}

BOOST_AUTO_TEST_CASE(TestBackgroundBlendingNames){
    BOOST_CHECK_EQUAL(FieldBackgroundInstaller::BlendingName(0), "alpha");
    BOOST_CHECK_EQUAL(FieldBackgroundInstaller::BlendingName(1), "add");
    BOOST_CHECK_EQUAL(FieldBackgroundInstaller::BlendingName(2), "subtract");
    // Type 3 is drawn as an addition with a quarter of the intensity.
    BOOST_CHECK_EQUAL(FieldBackgroundInstaller::BlendingName(3), "add");
    BOOST_CHECK_EQUAL(FieldBackgroundInstaller::BlendingName(4), "unknown");
}
