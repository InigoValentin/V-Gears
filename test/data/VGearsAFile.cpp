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
#include <OgreAnimation.h>
#include <OgreAnimationTrack.h>
#include <OgreKeyFrame.h>
#include <OgreSkeleton.h>
#include "data/VGearsAFile.h"

BOOST_AUTO_TEST_CASE(TestVGearsAFile){
    VGears::AFile animation(nullptr, "animation", 0, "test", true);
    VGears::AFile::Frame first_frame;
    first_frame.root_rotation = Ogre::Vector3::ZERO;
    first_frame.root_translation = Ogre::Vector3(2.0f, 3.0f, 4.0f);
    animation.GetFrames().push_back(first_frame);
    VGears::AFile::Frame last_frame;
    last_frame.root_rotation = Ogre::Vector3::ZERO;
    last_frame.root_translation = Ogre::Vector3(-1.0f, 5.0f, -6.0f);
    animation.GetFrames().push_back(last_frame);

    Ogre::SkeletonPtr skeleton(new Ogre::Skeleton(nullptr, "skeleton", 0, "test", true));
    skeleton->createBone("root");
    animation.AddTo(skeleton, "jump", true);

    Ogre::NodeAnimationTrack* track = skeleton->getAnimation("jump")->getNodeTrack(0);
    BOOST_CHECK(track->getNodeKeyFrame(0)->getTranslate() == Ogre::Vector3(2.0f, 3.0f, -4.0f));
    BOOST_CHECK(track->getNodeKeyFrame(1)->getTranslate() == Ogre::Vector3(-1.0f, 5.0f, 6.0f));
    animation.AddTo(skeleton, "unmodified");
    track = skeleton->getAnimation("unmodified")->getNodeTrack(0);
    BOOST_CHECK(track->getNodeKeyFrame(0)->getTranslate() == first_frame.root_translation);
    BOOST_CHECK(track->getNodeKeyFrame(1)->getTranslate() == last_frame.root_translation);

    const Ogre::AxisAlignedBox bounds = animation.GetRootTranslationBounds(true);
    BOOST_CHECK(bounds.getMinimum() == Ogre::Vector3(-1.0f, 0.0f, -4.0f));
    BOOST_CHECK(bounds.getMaximum() == Ogre::Vector3(2.0f, 5.0f, 6.0f));
    const Ogre::AxisAlignedBox original_bounds = animation.GetRootTranslationBounds();
    BOOST_CHECK(original_bounds.getMinimum() == Ogre::Vector3(-1.0f, 0.0f, -6.0f));
    BOOST_CHECK(original_bounds.getMaximum() == Ogre::Vector3(2.0f, 5.0f, 4.0f));
}

