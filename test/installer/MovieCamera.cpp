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
#include <cstring>
#include "installer/MovieCamera.h"

namespace{

    std::vector<uint8_t> Frames(const int count, const int first_with_camera){
        std::vector<uint8_t> camera(count * MovieCamera::FRAME_SIZE, 0);
        for (int frame = first_with_camera; first_with_camera >= 0 && frame < count; ++ frame)
            camera[frame * MovieCamera::FRAME_SIZE] = 1;
        // Every frame has the projection, which doesn't count.
        for (int frame = 0; frame < count; ++ frame)
            camera[frame * MovieCamera::FRAME_SIZE + 36] = 1;
        return camera;
    }

    void Append(std::vector<uint8_t>& data, const void* source, const size_t size){
        const uint8_t* bytes = static_cast<const uint8_t*>(source);
        data.insert(data.end(), bytes, bytes + size);
    }

    // An archive with one file.
    std::vector<uint8_t> Archive(const std::string& name, const std::vector<uint8_t>& content){
        std::vector<uint8_t> archive(12, 0);
        const uint32_t count = 1;
        Append(archive, &count, 4);
        char entry_name[20] = {};
        std::memcpy(entry_name, name.c_str(), name.size());
        Append(archive, entry_name, 20);
        const uint32_t offset = 16 + 27;
        Append(archive, &offset, 4);
        archive.push_back(0);
        archive.push_back(0);
        archive.push_back(0);
        Append(archive, entry_name, 20);
        const uint32_t size = static_cast<uint32_t>(content.size());
        Append(archive, &size, 4);
        archive.insert(archive.end(), content.begin(), content.end());
        return archive;
    }
}

BOOST_AUTO_TEST_CASE(TestMovieCameraFirstFrameWithACamera){
    BOOST_CHECK_EQUAL(MovieCamera::FirstCameraFrame(Frames(10, 4)), 4);
    BOOST_CHECK_EQUAL(MovieCamera::FirstCameraFrame(Frames(10, 0)), 0);
    // The projection alone is not a camera.
    BOOST_CHECK_EQUAL(MovieCamera::FirstCameraFrame(Frames(10, -1)), -1);
    BOOST_CHECK_EQUAL(MovieCamera::FirstCameraFrame({}), -1);
}

BOOST_AUTO_TEST_CASE(TestMovieCameraReadsAFileOfAnArchive){
    const std::vector<uint8_t> camera = Frames(5, 3);
    const std::vector<uint8_t> archive = Archive("opening.cam", camera);
    std::vector<uint8_t> content;
    BOOST_REQUIRE(MovieCamera::ReadLgpFile(archive, "opening.cam", content));
    BOOST_CHECK(content == camera);
    BOOST_CHECK(!MovieCamera::ReadLgpFile(archive, "other.cam", content));
    // A truncated archive is not read.
    std::vector<uint8_t> truncated(archive.begin(), archive.end() - 10);
    BOOST_CHECK(!MovieCamera::ReadLgpFile(truncated, "opening.cam", content));
    BOOST_CHECK(!MovieCamera::ReadLgpFile({}, "opening.cam", content));
}
