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
#include <OgreMath.h>
#include <cstring>
#include <vector>
#include "core/MovieCameraFrame.h"

namespace{

    // The last frame of the opening movie. It's the camera of the field md1stin, whose background
    // the movie ends with: position 28.4846 210.83 3.10751, orientation 0.742188 0.667516
    // -0.0403783 -0.0450658, and a vertical field of view of 34.5389 degrees.
    const std::vector<uint8_t> LAST_FRAME_OF_THE_OPENING = {
      0xe2, 0x0f, 0x11, 0xfe, 0x00, 0x00, 0xcb, 0xff, 0x52, 0xfe,
      0x16, 0xf0, 0xeb, 0x01, 0xcb, 0x0f, 0x4f, 0xfe, 0x4f, 0xfe,
      0x93, 0xfe, 0xff, 0xff, 0xcf, 0x0c, 0x00, 0x00, 0x69, 0x96,
      0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x82, 0x01, 0x00, 0x00
    };
}

BOOST_AUTO_TEST_CASE(TestMovieCameraFrameWithoutCameraIsNotValid){
    const uint8_t frame[MovieCameraFrame::SIZE] = {};
    BOOST_CHECK(!MovieCameraFrame::Parse(frame, 1.0f).valid);
    // The projection alone isn't a camera.
    uint8_t projection_only[MovieCameraFrame::SIZE] = {};
    projection_only[36] = 0x82;
    projection_only[37] = 0x01;
    BOOST_CHECK(!MovieCameraFrame::Parse(projection_only, 1.0f).valid);
}

BOOST_AUTO_TEST_CASE(TestMovieCameraFrameIdentityRotation){
    // Identity rotation (4096 on the diagonal), no translation, focal length 386.
    uint8_t data[MovieCameraFrame::SIZE] = {};
    const int16_t one = 4096;
    for (int i = 0; i < 3; ++ i) std::memcpy(data + (i * 3 + i) * 2, &one, 2);
    const uint16_t focal = 386;
    std::memcpy(data + 36, &focal, 2);
    const MovieCameraFrame frame = MovieCameraFrame::Parse(data, 1.0f);
    BOOST_REQUIRE(frame.valid);
    BOOST_CHECK_SMALL(frame.position.length(), 0.0001f);
    // The field of view the field md1stin has with this focal length.
    BOOST_CHECK_CLOSE(frame.fov.valueDegrees(), 34.5389f, 0.01f);
}

BOOST_AUTO_TEST_CASE(TestMovieCameraFrameTranslationIsScaledByTheField){
    uint8_t data[MovieCameraFrame::SIZE] = {};
    const int16_t one = 4096;
    for (int i = 0; i < 3; ++ i) std::memcpy(data + (i * 3 + i) * 2, &one, 2);
    const int32_t translation = -128 * 10;
    std::memcpy(data + 20, &translation, 4);
    const uint16_t focal = 386;
    std::memcpy(data + 36, &focal, 2);
    // The position is the opposite of the translation, divided by 128 and by the field scale.
    BOOST_CHECK_CLOSE(MovieCameraFrame::Parse(data, 1.0f).position.x, 10.0f, 0.01f);
    BOOST_CHECK_CLOSE(MovieCameraFrame::Parse(data, 2.0f).position.x, 5.0f, 0.01f);
}

BOOST_AUTO_TEST_CASE(TestMovieCameraFrameLastFrameOfTheOpeningIsTheCameraOfTheField){
    const MovieCameraFrame frame = MovieCameraFrame::Parse(LAST_FRAME_OF_THE_OPENING.data(), 1.0f);
    BOOST_REQUIRE(frame.valid);
    // The values the installer wrote for md1stin, to the precision of the camera data.
    BOOST_CHECK_CLOSE(frame.position.x, 28.4846f, 0.1f);
    BOOST_CHECK_CLOSE(frame.position.y, 210.83f, 0.1f);
    BOOST_CHECK_CLOSE(frame.position.z, 3.10751f, 3.0f);
    BOOST_CHECK_CLOSE(frame.fov.valueDegrees(), 34.5389f, 0.01f);
    const Ogre::Quaternion expected(0.742188f, 0.667516f, -0.0403783f, -0.0450658f);
    BOOST_CHECK(frame.orientation.equals(expected, Ogre::Radian(0.01f)));
}
