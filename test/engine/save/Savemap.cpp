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

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <memory>
#include <OgreLogManager.h>
#include "engine/save/Savemap.h"
#include "engine/save/SavemapHandler.h"

/**
 * Writes a savemap to a temporary file and reads it into another one.
 */
static std::unique_ptr<Savemap> WriteAndRead(Savemap& savemap){
    const boost::filesystem::path file
      = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    savemap.Write(3, file.string());
    auto read = std::make_unique<Savemap>();
    read->Read(file.string());
    boost::filesystem::remove(file);
    return read;
}

/**
 * A savemap with a bit of everything. It's too big for the stack.
 */
static std::unique_ptr<Savemap> MakeSavemap(){
    auto savemap = std::make_unique<Savemap>();
    savemap->SetControlKey("abcdefgh");
    savemap->SetMoney(1234);
    savemap->SetGameTime(100);
    savemap->SetCountdownTime(50);
    savemap->SetParty(0, -1, 2);
    savemap->SetItem(4, 17, 9);
    savemap->SetMateria(2, 5, 3000);
    savemap->SetData(1, 20, -7);
    savemap->SetSetting("field_leader", 2);
    savemap->SetCharacterInfo(1, 1, "Barret", true, false, 12, 3, true, 500, 90, 2, 40, 4, 5, 6);
    savemap->SetCharacterStat(1, Savemap::STAT::HP, 300, 250);
    savemap->SetCharacterMateria(1, true, 3, 8, 700);
    savemap->SetKeyItem(0, true);
    savemap->SetKeyItem(30, true);
    savemap->SetMateria(7, 44, 0);
    savemap->SetESkillMateria(7, 5, true);
    savemap->SetMateriaStash(1, 44, 0);
    savemap->SetESkillMateriaStash(1, 23, true);
    savemap->SetCharacterMateria(1, false, 2, 44, 0);
    savemap->SetCharacterESkillMateria(1, false, 2, 0, true);
    return savemap;
}

BOOST_AUTO_TEST_CASE(TestSavemapIsReadAsItWasWritten){
    const std::unique_ptr<Savemap> savemap = MakeSavemap();
    const std::unique_ptr<Savemap> read = WriteAndRead(*savemap);
    BOOST_CHECK(!read->IsEmpty());
    BOOST_CHECK_EQUAL(read->GetControlKey(), "abcdefgh");
    BOOST_CHECK_EQUAL(read->GetMoney(), 1234u);
    BOOST_CHECK_EQUAL(read->GetGameTime(), 100u);
    BOOST_CHECK_EQUAL(read->GetCountdownTime(), 50u);
    BOOST_CHECK_EQUAL(read->GetPartyMember(0), 0);
    BOOST_CHECK_EQUAL(read->GetPartyMember(1), -1);
    BOOST_CHECK_EQUAL(read->GetPartyMember(2), 2);
    BOOST_CHECK_EQUAL(read->GetItemAtPosId(4), 17u);
    BOOST_CHECK_EQUAL(read->GetItemAtPosQty(4), 9u);
    BOOST_CHECK_EQUAL(read->GetMateriaAtPosId(2), 5);
    BOOST_CHECK_EQUAL(read->GetMateriaAtPosAp(2), 3000u);
    BOOST_CHECK_EQUAL(read->GetData(1, 20), -7);
    BOOST_CHECK_EQUAL(read->GetSetting("field_leader"), 2);
    BOOST_CHECK_EQUAL(read->GetCharacterName(1), "Barret");
    BOOST_CHECK_EQUAL(read->GetCharacterLevel(1), 12u);
    BOOST_CHECK(read->IsCharacterBackRow(1));
    BOOST_CHECK_EQUAL(read->GetCharacterStatBase(1, Savemap::STAT::HP), 300u);
    BOOST_CHECK_EQUAL(read->GetCharacterStatExtra(1, Savemap::STAT::HP), 250u);
    BOOST_CHECK_EQUAL(read->GetCharacterMateriaId(1, true, 3), 8);
}

BOOST_AUTO_TEST_CASE(TestSavemapCopyHasEverything){
    const std::unique_ptr<Savemap> savemap = MakeSavemap();
    auto copy = std::make_unique<Savemap>();
    *copy = *savemap;
    BOOST_CHECK_EQUAL(copy->GetMoney(), 1234u);
    BOOST_CHECK_EQUAL(copy->GetGameTime(), 100u);
    BOOST_CHECK_EQUAL(copy->GetItemAtPosId(4), 17u);
    BOOST_CHECK_EQUAL(copy->GetMateriaAtPosId(2), 5);
    BOOST_CHECK_EQUAL(copy->GetData(1, 20), -7);
}

BOOST_AUTO_TEST_CASE(TestSavemapKeepsKeyItemsAndEnemySkills){
    const std::unique_ptr<Savemap> savemap = MakeSavemap();
    const std::unique_ptr<Savemap> read = WriteAndRead(*savemap);
    for (unsigned int item = 0; item < 40; item ++)
        BOOST_CHECK_EQUAL(read->GetKeyItem(item), item == 0 || item == 30);
    BOOST_CHECK(read->IsMateriaAtPosESkillLearned(7, 5));
    BOOST_CHECK(!read->IsMateriaAtPosESkillLearned(7, 4));
    BOOST_CHECK(read->IsStashAtPosESkillLearned(1, 23));
    BOOST_CHECK(read->IsCharacterMateriaESkillLearned(1, false, 2, 0));
    BOOST_CHECK(!read->IsCharacterMateriaESkillLearned(1, false, 2, 1));
}

BOOST_AUTO_TEST_CASE(TestSavemapClearedContentsAreNotSaved){
    const std::unique_ptr<Savemap> savemap = MakeSavemap();
    savemap->ClearContents();
    // Saving the game again with less than before.
    savemap->SetItem(6, 20, 1);
    const std::unique_ptr<Savemap> read = WriteAndRead(*savemap);
    BOOST_CHECK_EQUAL(read->GetItemAtPosQty(4), 0u);
    BOOST_CHECK_EQUAL(read->GetItemAtPosQty(6), 1u);
    BOOST_CHECK(!read->GetKeyItem(30));
    BOOST_CHECK_EQUAL(read->GetMateriaAtPosId(2), -1);
    BOOST_CHECK(!read->IsMateriaAtPosESkillLearned(7, 5));
    BOOST_CHECK_EQUAL(read->GetCharacterMateriaId(1, true, 3), -1);
    BOOST_CHECK(!read->IsCharacterMateriaESkillLearned(1, false, 2, 0));
    BOOST_CHECK_EQUAL(read->GetData(1, 20), 0);
    // The rest stays.
    BOOST_CHECK_EQUAL(read->GetMoney(), 1234u);
    BOOST_CHECK_EQUAL(read->GetSetting("field_leader"), 2);
    BOOST_CHECK_EQUAL(read->GetCharacterName(1), "Barret");
    BOOST_CHECK(read->IsStashAtPosESkillLearned(1, 23));
}

BOOST_AUTO_TEST_CASE(TestSavemapHandlerSlotsDoNotShareTheirData){
    // The handler writes to the log, which needs a log manager.
    std::unique_ptr<Ogre::LogManager> log_manager;
    if (Ogre::LogManager::getSingletonPtr() == nullptr){
        log_manager = std::make_unique<Ogre::LogManager>();
        log_manager->createLog("test", true, false, true);
    }
    // The saved games are in a folder relative to the current one.
    const boost::filesystem::path previous = boost::filesystem::current_path();
    const boost::filesystem::path folder
      = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    boost::filesystem::create_directories(folder);
    boost::filesystem::current_path(folder);
    {
        auto handler = std::make_unique<SavemapHandler>();
        handler->SetControlKey("abcdefgh");
        handler->SetMoney(100);
        BOOST_CHECK(handler->Save(0, false));
        // Playing on, and saving in another slot.
        handler->SetMoney(200);
        BOOST_CHECK_EQUAL(handler->GetSlotMoney(0), 100u);
        BOOST_CHECK(handler->Save(1, false));
        handler->SetMoney(300);
        BOOST_CHECK_EQUAL(handler->GetSlotMoney(0), 100u);
        BOOST_CHECK_EQUAL(handler->GetSlotMoney(1), 200u);
    }
    boost::filesystem::current_path(previous);
    boost::filesystem::remove_all(folder);
}
