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
#include "engine/entity/WalkmeshMovement.h"

BOOST_AUTO_TEST_CASE(TestWalkmeshMovementScriptedWalksAreNotStoppedByTheWalkmeshEdge){
    // The player and the characters that wander are stopped.
    BOOST_CHECK(WalkmeshMovement::EdgeBlocks(true, false));
    BOOST_CHECK(WalkmeshMovement::EdgeBlocks(false, false));
    // Even if a script moves the player, the player stays on the walkmesh.
    BOOST_CHECK(WalkmeshMovement::EdgeBlocks(true, true));
    // A character a script walks to a position is not.
    BOOST_CHECK(!WalkmeshMovement::EdgeBlocks(false, true));
}

BOOST_AUTO_TEST_CASE(TestWalkmeshMovementScriptedSteps){
    BOOST_CHECK_CLOSE(WalkmeshMovement::ScriptedStep(0.5f, 0.0f, 0.1f), 0.1f, 0.01f);
    BOOST_CHECK_CLOSE(WalkmeshMovement::ScriptedStep(0.05f, 0.0f, 0.1f), 0.05f, 0.01f);
    BOOST_CHECK_CLOSE(WalkmeshMovement::ScriptedStep(0.55f, 0.5f, 0.1f), 0.05f, 0.01f);
    BOOST_CHECK_EQUAL(WalkmeshMovement::ScriptedStep(0.4f, 0.5f, 0.1f), 0.0f);
    BOOST_CHECK_EQUAL(WalkmeshMovement::ScriptedStep(1.0f, 0.0f, 0.0f), 0.0f);

    for (float frame_time : {1.0f / 120.0f, 1.0f / 60.0f, 1.0f / 30.0f}){
        float distance = 2.0f;
        for (int step = 0; step < 1000 && distance > 0.0001f; ++step){
            distance -= WalkmeshMovement::ScriptedStep(distance, 0.0f, 6.0f * frame_time);
        }
        BOOST_CHECK_SMALL(distance, 0.0001f);
    }
}

BOOST_AUTO_TEST_CASE(TestWalkmeshMovementHeightOfAPointOfATriangle){
    // A slope that goes up one unit for each two units along X.
    const Ogre::Vector3 a(0.0f, 0.0f, 0.0f);
    const Ogre::Vector3 b(2.0f, 0.0f, 1.0f);
    const Ogre::Vector3 c(0.0f, 2.0f, 0.0f);
    BOOST_CHECK_CLOSE(
      WalkmeshMovement::PointElevation(Ogre::Vector2(1.0f, 0.5f), a, b, c), 0.5f, 0.01f
    );
    BOOST_CHECK_CLOSE(
      WalkmeshMovement::PointElevation(Ogre::Vector2(2.0f, 0.0f), a, b, c), 1.0f, 0.01f
    );
}

BOOST_AUTO_TEST_CASE(TestWalkmeshMovementSideOfALine){
    const Ogre::Vector2 start(0.0f, 0.0f);
    const Ogre::Vector2 end(1.0f, 0.0f);
    BOOST_CHECK(WalkmeshMovement::SideOfVector(Ogre::Vector2(0.5f, 1.0f), start, end) > 0.0f);
    BOOST_CHECK(WalkmeshMovement::SideOfVector(Ogre::Vector2(0.5f, -1.0f), start, end) < 0.0f);
    BOOST_CHECK_EQUAL(
      WalkmeshMovement::SideOfVector(Ogre::Vector2(3.0f, 0.0f), start, end), 0.0f
    );
}

/**
 * Steers a step that goes along X.
 *
 * @return The angle in degrees that the step has been turned, or 1000 if the entity is blocked.
 */
static float SteerAngle(
  const bool is_player, const bool slip, const WalkmeshMovement::Obstacles& obstacles
){
    Ogre::Vector2 direction(1.0f, 0.0f);
    if (!WalkmeshMovement::Steer(is_player, slip, obstacles, direction)) return 1000.0f;
    return Ogre::Math::ATan2(direction.y, direction.x).valueDegrees();
}

BOOST_AUTO_TEST_CASE(TestWalkmeshMovementThePlayerSlidesAlongWalls){
    WalkmeshMovement::Obstacles left;
    left.left_wall = true;
    BOOST_CHECK_CLOSE(SteerAngle(true, true, left), -11.25f, 0.01f);
    WalkmeshMovement::Obstacles right;
    right.right_wall = true;
    BOOST_CHECK_CLOSE(SteerAngle(true, true, right), 11.25f, 0.01f);
    // Not with SLIP off, nor in a corner.
    BOOST_CHECK_EQUAL(SteerAngle(true, false, left), 1000.0f);
    WalkmeshMovement::Obstacles corner = left;
    corner.right_wall = true;
    BOOST_CHECK_EQUAL(SteerAngle(true, true, corner), 1000.0f);
    // The player doesn't go around other entities.
    WalkmeshMovement::Obstacles somebody = left;
    somebody.ahead_entity = true;
    BOOST_CHECK_EQUAL(SteerAngle(true, true, somebody), 1000.0f);
}

BOOST_AUTO_TEST_CASE(TestWalkmeshMovementCharactersGoAroundObstacles){
    WalkmeshMovement::Obstacles wall_ahead;
    wall_ahead.ahead_wall = true;
    BOOST_CHECK_CLOSE(SteerAngle(false, true, wall_ahead), -7.0f, 0.01f);
    // Somebody ahead: it turns to one side. Somebody to the left or right too.
    WalkmeshMovement::Obstacles ahead;
    ahead.ahead_entity = true;
    BOOST_CHECK_CLOSE(SteerAngle(false, true, ahead), -11.25f, 0.01f);
    WalkmeshMovement::Obstacles left;
    left.left_entity = true;
    BOOST_CHECK_CLOSE(SteerAngle(false, true, left), -11.25f, 0.01f);
    WalkmeshMovement::Obstacles right;
    right.right_entity = true;
    BOOST_CHECK_CLOSE(SteerAngle(false, true, right), 11.25f, 0.01f);
    // Walls on both sides block it.
    WalkmeshMovement::Obstacles corner;
    corner.left_wall = true;
    corner.right_wall = true;
    BOOST_CHECK_EQUAL(SteerAngle(false, true, corner), 1000.0f);
}
