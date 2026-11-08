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
#include "core/EntityManager.h"

BOOST_AUTO_TEST_CASE(TestEntityManagerInteractionPicksTheEntityTheCharacterFaces){
    // The maximum angle is 90 degrees, and no candidate yet.
    const float max_angle = 90.0f;
    const float no_distance = 1000.0f;
    BOOST_CHECK(EntityManager::IsBetterInteractionTarget(30.0f, 1.0f, max_angle, no_distance));
    // Facing the character more directly is better, whatever the order.
    BOOST_CHECK(EntityManager::IsBetterInteractionTarget(5.0f, 1.0f, 30.0f, 1.0f));
    BOOST_CHECK(!EntityManager::IsBetterInteractionTarget(30.0f, 1.0f, 5.0f, 1.0f));
    // A candidate further from the front than the best one is not better.
    BOOST_CHECK(!EntityManager::IsBetterInteractionTarget(120.0f, 1.0f, max_angle, no_distance));
    // The same angle: the nearest wins, and the first one is kept on a tie.
    BOOST_CHECK(EntityManager::IsBetterInteractionTarget(10.0f, 0.5f, 10.0f, 1.0f));
    BOOST_CHECK(!EntityManager::IsBetterInteractionTarget(10.0f, 1.0f, 10.0f, 0.5f));
    BOOST_CHECK(!EntityManager::IsBetterInteractionTarget(10.0f, 1.0f, 10.0f, 1.0f));

    // Two characters side by side, in the list order Jessie then Wedge: facing Jessie must
    // choose Jessie, and facing Wedge must choose Wedge.
    const float jessie_angle_facing_jessie = 4.0f;
    const float wedge_angle_facing_jessie = 40.0f;
    float best_angle = max_angle;
    float best_distance = no_distance;
    int chosen = -1;
    const float angles[] = {jessie_angle_facing_jessie, wedge_angle_facing_jessie};
    for (int i = 0; i < 2; ++i){
        if (EntityManager::IsBetterInteractionTarget(angles[i], 0.2f, best_angle, best_distance)){
            best_angle = angles[i];
            best_distance = 0.2f;
            chosen = i;
        }
    }
    BOOST_CHECK_EQUAL(chosen, 0);
}

BOOST_AUTO_TEST_CASE(TestEntityManagerLinesTriggerEvenIfAScriptZeroedTheRadius){
    // A script sets the radius of the player to zero while a scene plays: lines must still work.
    BOOST_CHECK_CLOSE(EntityManager::TriggerReach(0.0f), 0.21f, 0.01f);
    BOOST_CHECK_CLOSE(EntityManager::TriggerReach(0.005f), 0.21f, 0.01f);
    // A radius that a script chose is respected.
    BOOST_CHECK_CLOSE(EntityManager::TriggerReach(0.265625f), 0.265625f, 0.01f);
    BOOST_CHECK_CLOSE(EntityManager::TriggerReach(0.078125f), 0.078125f, 0.01f);
}

BOOST_AUTO_TEST_CASE(TestEntityManagerScriptedWalksAreNotStoppedByTheWalkmeshEdge){
    // The player and the characters that wander are stopped.
    BOOST_CHECK(EntityManager::WalkmeshEdgeBlocks(true, false));
    BOOST_CHECK(EntityManager::WalkmeshEdgeBlocks(false, false));
    // Even if a script moves the player, the player stays on the walkmesh.
    BOOST_CHECK(EntityManager::WalkmeshEdgeBlocks(true, true));
    // A character a script walks to a position is not.
    BOOST_CHECK(!EntityManager::WalkmeshEdgeBlocks(false, true));
}

BOOST_AUTO_TEST_CASE(TestEntityManager){
    BOOST_CHECK_CLOSE(EntityManager::ScriptedMoveStep(0.5f, 0.0f, 0.1f), 0.1f, 0.01f);
    BOOST_CHECK_CLOSE(EntityManager::ScriptedMoveStep(0.05f, 0.0f, 0.1f), 0.05f, 0.01f);
    BOOST_CHECK_CLOSE(EntityManager::ScriptedMoveStep(0.55f, 0.5f, 0.1f), 0.05f, 0.01f);
    BOOST_CHECK_EQUAL(EntityManager::ScriptedMoveStep(0.4f, 0.5f, 0.1f), 0.0f);
    BOOST_CHECK_EQUAL(EntityManager::ScriptedMoveStep(1.0f, 0.0f, 0.0f), 0.0f);

    for (float frame_time : {1.0f / 120.0f, 1.0f / 60.0f, 1.0f / 30.0f}){
        float distance = 2.0f;
        for (int step = 0; step < 1000 && distance > 0.0001f; ++step){
            distance -= EntityManager::ScriptedMoveStep(distance, 0.0f, 6.0f * frame_time);
        }
        BOOST_CHECK_SMALL(distance, 0.0001f);
    }
}

