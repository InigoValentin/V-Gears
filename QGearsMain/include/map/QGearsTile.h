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
#include "common/TypeDefine.h"

namespace QGears{

    enum Blending{

        /**
         * Tile blending mode: Alpha.
         */
        B_ALPHA,

        /**
         * Tile blending mode: Add
         */
        B_ADD,

        /**
         * Tile blending mode: Substract
         */
        B_SUBTRACT
    };

    /**
     * A tile key frame.
     */
    struct KeyFrame{

        /**
         * Key frame timestamp.
         */
        Ogre::Real time;

        /**
         * Key frame movement coordinates.
         */
        Ogre::Vector4 uv;
    };

    typedef std::vector<KeyFrame> KeyFrameList;

    /**
     * A tile animation
     */
    struct Animation{

        /**
         * Animation length.
         *
         * @todo Units? Seconds?
         */
        Ogre::Real length;

        /**
         * List of key frames of the animation.
         */
        KeyFrameList key_frames;
    };

    typedef std::map<String, Animation> AnimationMap;

    /**
     * A tile.
     */
    struct Tile{

        /**
         * Tile width.
         */
        int width;

        /**
         * Tile hecight.
         */
        int height;

        /**
         * Tile destination coordinates in the background.
         */
        Ogre::Vector2 destination;

        /**
         * @todo Understand and document.
         */
        Ogre::Vector4 uv;

        /**
         * Tile depth.
         */
        Ogre::Real depth;

        /**
         * Tile blending type.
         */
        Blending blending;

        /**
         * List of animations associated to the tile.
         */
        AnimationMap animations;
    };
}

