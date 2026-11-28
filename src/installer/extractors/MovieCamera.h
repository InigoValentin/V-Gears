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

#pragma once

#include <cstdint>
#include <string>
#include <vector>

/**
 * The cameras of the movies, in moviecam.lgp.
 *
 * Each movie has a camera file, with 40 bytes per frame: a rotation matrix, a translation and the
 * projection. The models of a field are drawn with it over the movie. The frames in which the
 * movie has no camera are all zero.
 */
namespace MovieCamera{

    /**
     * Size of the camera data of a frame.
     */
    const size_t FRAME_SIZE = 40;

    /**
     * Retrieves the first frame of a movie that has a camera.
     *
     * @param[in] camera Content of the camera file.
     * @return The frame, or -1 if no frame has a camera.
     */
    int FirstCameraFrame(const std::vector<uint8_t>& camera);

    /**
     * Reads a file of an LGP archive.
     *
     * @param[in] archive Content of the archive.
     * @param[in] name Name of the file, such as "opening.cam".
     * @param[out] content The content of the file.
     * @return True if the file was found.
     */
    bool ReadLgpFile(
      const std::vector<uint8_t>& archive, const std::string& name, std::vector<uint8_t>& content
    );

    /**
     * Reads the camera of a movie from moviecam.lgp.
     *
     * @param[in] lgp_path Path to moviecam.lgp.
     * @param[in] movie Name of the movie, without extension.
     * @return The content of the camera file. Empty if the archive or the camera can't be read.
     */
    std::vector<uint8_t> LoadCamera(const std::string& lgp_path, const std::string& movie);
}
