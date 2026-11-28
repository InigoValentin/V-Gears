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
#include "engine/graphics/BackgroundScroll.h"
#include "engine/graphics/BackgroundShake.h"
#include "engine/graphics/BackgroundTileBuffer.h"
#include "engine/graphics/QuadWriter.h"

namespace{

    /**
     * An entity pointer. The scroll only stores it.
     */
    Entity* FakeEntity(){
        static int entity;
        return reinterpret_cast<Entity*>(&entity);
    }

    Ogre::AxisAlignedBox Limits(){return Ogre::AxisAlignedBox(-10, -20, 0, 10, 20, 1);}
}

BOOST_AUTO_TEST_CASE(TestQuadWriter){
    std::vector<float> vertices(QuadWriter::VERTEX_COUNT * QuadWriter::VERTEX_SIZE);
    float* end = vertices.data();
    const QuadWriter::Corners corners{{{1, 2}, {3, 4}, {5, 6}, {7, 8}}};
    QuadWriter::Write(end, corners, {0, 0.25f, 0.5f, 0.75f}, 9, Ogre::ColourValue(1, 0, 0, 1));
    BOOST_CHECK(end == vertices.data() + vertices.size());
    // The vertices are the corners 0, 1, 2, 0, 2 and 3.
    const auto vertex = [&vertices](int index){return &vertices[index * QuadWriter::VERTEX_SIZE];};
    const float expected[] = {5, 6, 9, 1, 0, 0, 1, 0.5f, 0.75f};
    BOOST_CHECK_EQUAL_COLLECTIONS(vertex(2), vertex(3), expected, expected + 9);
    BOOST_CHECK_EQUAL(vertex(5)[0], 7);
    BOOST_CHECK_EQUAL(vertex(5)[7], 0);
    QuadWriter::SetPositions(vertices.data(), {{{10, 11}, {12, 13}, {14, 15}, {16, 17}}});
    QuadWriter::SetTexture(vertices.data(), {1, 2, 3, 4});
    BOOST_CHECK_EQUAL(vertex(4)[0], 14);
    BOOST_CHECK_EQUAL(vertex(4)[2], 9);
    BOOST_CHECK_EQUAL(vertex(4)[7], 3);
    BOOST_CHECK_EQUAL(vertex(5)[8], 4);
}

BOOST_AUTO_TEST_CASE(TestBackgroundLayerRanges){
    BackgroundLayerRanges ranges;
    ranges.Add(0, 0, 6);
    ranges.Add(0, 6, 6);
    ranges.Add(2, 12, 6);
    // Out of range layers count as the nearest one.
    ranges.Add(9, 18, 6);
    BOOST_CHECK(ranges.AreContiguous());
    BOOST_CHECK_EQUAL(ranges.Of(0).count, 12u);
    BOOST_CHECK_EQUAL(ranges.Of(2).first, 12u);
    BOOST_CHECK_EQUAL(ranges.Of(3).first, 18u);
    ranges.Add(1, 24, 6);
    BOOST_CHECK(!ranges.AreContiguous());
    ranges.Clear();
    BOOST_CHECK(ranges.AreContiguous());
    BOOST_CHECK_EQUAL(ranges.Of(0).count, 0u);
}

BOOST_AUTO_TEST_CASE(TestBackgroundShake){
    BackgroundShake shake;
    BOOST_CHECK(!shake.Update(0.1f));
    BOOST_CHECK(!shake.Set(7, 1, 1, 1, 1));
    // Horizontal, 20 pixels, bounces of one second.
    BOOST_CHECK(shake.Set(1, 20, 30, 0, 0));
    BOOST_CHECK(shake.Update(1.0f));
    // The first bounce goes to half the amplitude times the first factor, -1.
    BOOST_CHECK_CLOSE(shake.Offset().x, -10.0f, 0.001f);
    BOOST_CHECK_EQUAL(shake.Offset().y, 0);
    // To stop, it ends the bounce, goes back to the center and stays there.
    BOOST_CHECK(shake.Set(0, 0, 30, 0, 30));
    shake.Update(1.0f);
    shake.Update(1.0f);
    BOOST_CHECK_EQUAL(shake.Offset().x, 0);
    BOOST_CHECK(!shake.IsRunning());
}

BOOST_AUTO_TEST_CASE(TestBackgroundScrollLimits){
    BackgroundScroll scroll([](const ScriptId&){});
    scroll.SetRange(Limits());
    scroll.SetPosition({100, -100});
    BOOST_CHECK(scroll.Position() == Ogre::Vector2(10, -20));
    BOOST_CHECK(!scroll.HasControl());
    // A scroll of a script to a position is not limited, and an instant one waits.
    scroll.ToPosition({100, -100}, BackgroundScroll::NONE, 0);
    BOOST_CHECK(scroll.HasControl());
    BOOST_CHECK(scroll.Position() == Ogre::Vector2(10, -20));
    BOOST_CHECK(scroll.ApplyPending());
    BOOST_CHECK(scroll.Position() == Ogre::Vector2(100, -100));
    BOOST_CHECK(!scroll.ApplyPending());
    // Following an entity limits it again.
    scroll.Follow(FakeEntity());
    scroll.SetPosition({100, 0});
    BOOST_CHECK(scroll.Position() == Ogre::Vector2(10, 0));
}

BOOST_AUTO_TEST_CASE(TestBackgroundScrollToAnEntity){
    std::vector<std::string> resumed;
    BackgroundScroll scroll([&resumed](const ScriptId& script){resumed.push_back(script.entity);});
    scroll.SetRange(Limits());
    BOOST_CHECK(!scroll.Wait(ScriptId()));
    BOOST_CHECK(!scroll.ToEntity(FakeEntity(), {50, 5}, BackgroundScroll::LINEAR, 2));
    BOOST_CHECK(scroll.End() == Ogre::Vector2(10, 5));
    BOOST_CHECK(scroll.TargetEntity() == FakeEntity());
    BOOST_CHECK(scroll.FollowedEntity() == nullptr);
    ScriptId script;
    script.entity = "cloud";
    BOOST_CHECK(scroll.Wait(script));
    scroll.Stop();
    BOOST_CHECK(scroll.FollowedEntity() == FakeEntity());
    BOOST_CHECK_EQUAL(scroll.Type(), BackgroundScroll::NONE);
    BOOST_REQUIRE_EQUAL(resumed.size(), 1u);
    BOOST_CHECK_EQUAL(resumed[0], "cloud");
    // An instant scroll moves at once and follows the entity.
    BOOST_CHECK(scroll.ToEntity(FakeEntity(), {-50, 0}, BackgroundScroll::NONE, 0));
    BOOST_CHECK(scroll.Position() == Ogre::Vector2(-10, 0));
    BOOST_CHECK(scroll.FollowedEntity() == FakeEntity());
}

BOOST_AUTO_TEST_CASE(TestBackgroundScrollReplacedAndCleared){
    int resumed = 0;
    BackgroundScroll scroll([&resumed](const ScriptId&){++ resumed;});
    scroll.ToEntity(FakeEntity(), {5, 5}, BackgroundScroll::SMOOTH, 1);
    scroll.Wait(ScriptId());
    // A new scroll ends the previous one without following its entity.
    scroll.ToPosition({1, 1}, BackgroundScroll::LINEAR, 1);
    BOOST_CHECK_EQUAL(resumed, 1);
    BOOST_CHECK(scroll.FollowedEntity() == nullptr);
    BOOST_CHECK(scroll.TargetEntity() == nullptr);
    scroll.Wait(ScriptId());
    scroll.Clear();
    BOOST_CHECK_EQUAL(resumed, 2);
    BOOST_CHECK(!scroll.HasControl());
    BOOST_CHECK(scroll.Position() == Ogre::Vector2::ZERO);
}
