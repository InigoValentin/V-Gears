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

