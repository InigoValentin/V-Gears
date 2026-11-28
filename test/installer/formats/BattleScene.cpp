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

#include <memory>
#include <boost/test/unit_test.hpp>
#include "installer/formats/BattleSceneConverter.h"
#include "installer/formats/BattleSceneReader.h"

namespace{

    /**
     * A scene with no enemies and no attacks. It's too big for the stack.
     */
    std::unique_ptr<BattleSceneData> EmptyScene(){
        auto scene = std::make_unique<BattleSceneData>();
        for (u16& enemy : scene->enemy) enemy = 0xFFFF;
        for (u16& id : scene->attack_ids) id = 0xFFFF;
        return scene;
    }
}

BOOST_AUTO_TEST_CASE(TestBattleSceneValues){
    BOOST_CHECK_EQUAL(BattleSceneConverter::ElementFactor(0x02), 2.0f);
    BOOST_CHECK_EQUAL(BattleSceneConverter::ElementFactor(0x06), -1.0f);
    BOOST_CHECK_EQUAL(BattleSceneConverter::ElementFactor(0x55), 1.0f);
    BOOST_CHECK(BattleSceneConverter::Layout(5) == Formation::Layout::PINCER_ATTACK);
    BOOST_CHECK(BattleSceneConverter::Layout(8) == Formation::Layout::LOCKED);
    BOOST_CHECK(BattleSceneConverter::Layout(9) == Formation::Layout::NORMAL);
    BOOST_CHECK_EQUAL(BattleSceneConverter::LocationName(2), "grassland");
    BOOST_CHECK_EQUAL(BattleSceneConverter::LocationName(0x59), "ultimate_weapon_forest");
    BOOST_CHECK_EQUAL(BattleSceneConverter::LocationName(0x5A), "unknonw_location");
}

BOOST_AUTO_TEST_CASE(TestBattleSceneAttacks){
    auto scene = EmptyScene();
    // The same attack in two slots is listed once.
    scene->attack_ids[0] = scene->attack_ids[1] = 7;
    scene->attack_data[0].effect = 0xFF;
    scene->attack_data[0].impact_effect = 3;
    scene->attack_data[0].status = 0x00000005;
    scene->attack_data[0].status_change = 0x80 | 63;
    scene->attack_data[0].element = 0x0003;
    // The special properties are on when their bits are not set.
    scene->attack_data[0].special = 0xFFFE;
    const std::vector<Attack> attacks = BattleSceneConverter::Attacks(*scene);
    BOOST_REQUIRE_EQUAL(attacks.size(), 1u);
    BOOST_CHECK_EQUAL(attacks[0].animation.effect, -1);
    BOOST_CHECK_EQUAL(attacks[0].animation.impact_effect, 3);
    BOOST_REQUIRE_EQUAL(attacks[0].status_effects.size(), 2u);
    BOOST_CHECK_EQUAL(attacks[0].status_effects[1].id, 2);
    BOOST_CHECK_EQUAL(attacks[0].status_effects[1].chance, 1.0f);
    // A copy of the constant, which has no definition to take the address of.
    const int toggle = GameData::STATUS_EFFECT_MODE::TOGGLE;
    BOOST_CHECK_EQUAL(attacks[0].status_effects[1].type, toggle);
    BOOST_CHECK_EQUAL(attacks[0].elements.size(), 2u);
    BOOST_CHECK(attacks[0].damage.damage_mp);
    BOOST_CHECK(!attacks[0].reflectable);
}

BOOST_AUTO_TEST_CASE(TestBattleSceneEnemyItems){
    auto scene = EmptyScene();
    scene->enemy[0] = 0;
    BattleSceneData::EnemyData& data = scene->enemy_data[0];
    for (u16& attack : data.attacks) attack = 0xFFFF;
    // An armor to drop, nothing, a weapon to steal and nothing.
    const u16 items[4] = {297, 0xFFFF, 255, 0xFFFF};
    const u8 rates[4] = {63, 0, 0x80 + 63, 0};
    std::copy(items, items + 4, data.item);
    std::copy(rates, rates + 4, data.item_rate);
    const std::vector<Enemy> enemies = BattleSceneConverter::Enemies(*scene);
    BOOST_REQUIRE_EQUAL(enemies.size(), 1u);
    BOOST_REQUIRE_EQUAL(enemies[0].drop.size(), 1u);
    BOOST_CHECK_EQUAL(enemies[0].drop[0].id, 297u);
    BOOST_CHECK_EQUAL(enemies[0].drop[0].rate, 1.0f);
    BOOST_REQUIRE_EQUAL(enemies[0].steal.size(), 1u);
    BOOST_CHECK_EQUAL(enemies[0].steal[0].id, 255u);
}

BOOST_AUTO_TEST_CASE(TestBattleSceneFormations){
    auto scene = EmptyScene();
    for (auto& formation : scene->formation) for (auto& enemy : formation) enemy.id = 0xFFFF;
    for (auto& setup : scene->setup){
        for (u16& candidate : setup.next_arena_candidates) candidate = 999;
    }
    scene->formation[1][0].id = 5;
    scene->formation[1][0].cover_flags = 0x11;
    scene->setup[1].next_arena_candidates[2] = 30;
    const std::vector<Formation> formations = BattleSceneConverter::Formations(*scene);
    BOOST_REQUIRE_EQUAL(formations.size(), 4u);
    BOOST_CHECK(formations[0].enemies.empty());
    BOOST_CHECK(!formations[0].is_arena_battle);
    BOOST_CHECK(formations[1].is_arena_battle);
    BOOST_REQUIRE_EQUAL(formations[1].enemies.size(), 1u);
    BOOST_CHECK_EQUAL(std::string(formations[1].enemies[0].cover, 5), "10001");
}

BOOST_AUTO_TEST_CASE(TestBattleSceneAiScripts){
    // The block of the second enemy at 6 bytes from the AI section: two scripts after 32 bytes.
    std::vector<u8> bytes(0x0E80 + 6 + 32 + 6, 0xFF);
    bytes[0x0E82] = 6;
    bytes[0x0E83] = 0;
    const size_t block = 0x0E80 + 6;
    bytes[block] = 32;
    bytes[block + 1] = 0;
    bytes[block + 4] = 35;
    bytes[block + 5] = 0;
    const u8 scripts[6] = {1, 0x73, 0xFF, 2, 0x73, 0xFF};
    std::copy(scripts, scripts + 6, bytes.begin() + block + 32);
    auto scene = EmptyScene();
    BattleSceneReader::ReadAiScripts(bytes, *scene);
    BOOST_REQUIRE_EQUAL(scene->ai_scripts[1].size(), 16u);
    // Without the padding.
    BOOST_CHECK(scene->ai_scripts[1][0] == std::vector<u8>({1, 0x73}));
    BOOST_CHECK(scene->ai_scripts[1][1].empty());
    BOOST_CHECK(scene->ai_scripts[1][2] == std::vector<u8>({2, 0x73}));
    BOOST_CHECK(scene->ai_scripts[0][0].empty());
    // A scene cut short has no scripts.
    BattleSceneReader::ReadAiScripts(std::vector<u8>(100), *scene);
    BOOST_CHECK(scene->ai_scripts[1][0].empty());
}
