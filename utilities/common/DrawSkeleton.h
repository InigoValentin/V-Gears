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

#include <Ogre.h>

/**
 * A skeleton bone.
 */
struct Bone{

    /**
     * Initializer
     */
    Bone(): length(0), parent_id(-1){}

    /**
     * Bone length.
     */
    float length;

    /**
     * Parent bone ID.
     */
    int parent_id;
};

typedef std::vector<Bone> Skeleton;

/**
 * Draws a skeleton.
 *
 * @param skeleton[in] The skeleton to draw.
 * @param mesh[in] Parent mesh for the skeleton.
 */
void DrawSkeleton(const Skeleton& skeleton, const Ogre::MeshPtr& mesh);

/**
 * Draws a bone.
 *
 * @param skeleton[in] Skeleton the bone belongs to.
 * @param parent_index[in] The parent bone ID.
 * @param colours[in] Colour for the bone.
 */
void DrawBone(const Skeleton& skeleton, int parent_index, Ogre::RGBA* colours);

