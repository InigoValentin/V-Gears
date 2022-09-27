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

#include <OgrePlatform.h>
#include <OgreString.h>

typedef Ogre::uint8 u8;
typedef Ogre::uint16 u16;
typedef Ogre::uint32 u32;
typedef Ogre::int8 s8;
typedef Ogre::int16 s16;
typedef Ogre::int32 s32;

namespace VGears{

    typedef Ogre::uchar uchar;
    typedef Ogre::uint8 uint8;
    typedef Ogre::uint16 uint16;
    typedef Ogre::uint32 uint32;
    typedef Ogre::int8 sint8;
    typedef Ogre::int16 sint16;
    typedef Ogre::int32 sint32;
    typedef Ogre::String String;

    /**
     * Color components.
     */
    struct ColorComponents{

        /**
         * Blue component of the color.
         */
        uint8 blue;

        /**
         * Green component of the color.
         */
        uint8 green;

        /**
         * Red component of the color.
         */
        uint8 red;

        /**
         * Alpha component of the color.
         */
        uint8 alpha;
    };

    /**
     * @todo: Understand and document.
     */
    union ColorA8R8G8B8{
        ColorComponents comp;
        uint32 all;
    };

    /**
     * Pixel coordinates.
     */
    struct Pixel{

        /**
         * X coordinate of the pixel.
         */
        sint16 x;

        /**
         * Y coordinate of the pixel.
         */
        sint16 y;
    };

    /**
     * Extension for material files.
     */
    extern const String EXT_MATERIAL;

    /**
     * Extension for mesh files.
     */
    extern const String EXT_MESH;

    /**
     * Extension for skeleton files.
     */
    extern const String EXT_SKELETON;

    /**
     * Extension for configuration files.
     */
    extern const String EXT_CONFIG;

    /**
     * Extension for log files.
     */
    extern const String EXT_LOG;

    /**
     * Extension for P files.
     *
     * P files are binary files containing data which form 3D model. The files
     * specifies model's vertices, polygons, colors, texture coordinates and
     * model sub-groups. The files do not specify references to the texture
     * files, animations, model skeleton or anything else.
     *
     * P-files are used as parts of field models, battle models, battle
     * locations on PC version of FF7.
     */
    extern const String EXT_P;

    /**
     * Extension for PLY files.
     *
     * PLY file contain polygon data.
     */
    extern const String EXT_PLY;

    /**
     * Exteesion for RSD files.
     *
     * RSD files are product of the original Playstation Psy-Q 3D development
     * libraries. They are often expoted by 3D modelers when converting from an
     * LWO or DXF file to something more understandable by the PSX. It is in
     * ascii, making it easy to edit by hand. When a 3D editor does an export,
     * four text files are actually created, .rsd, .ply, .mat, and .grp. These
     * can then be "compiled" into a binary .rsd file for the PSX.
     *
     * In the PC version of Final Fantasy 7, the text .rsd file is used while
     * the other files were "compiled" into Polygon(.p) format.
     */
    extern const String EXT_RSD;

    /**
     * Extension for TIM files.
     *
     * TIM files are a standard image file format for the Sony PlayStation. The
     * file structure closely mimics the way textures are managed in the frame
     * buffer by the GPU. TIM files are little endian-based.
     */
    extern const String EXT_TIM;

    /**
     * Extension for TEX files.
     *
     * TEX files are the PC's version of Final Fantasy VII texture files.
     */
    extern const String EXT_TEX;

    /**
     * Extension for trigger files.
     *
     * Triggers are located in fields maps.
     */
    extern const String EXT_TRIGGERS;

    /**
     * Extension for background files.
     *
     * Backgrounds are located in field maps.
     */
    extern const String EXT_BACKGROUND;

    /**
     * Extension for cam matrix files.
     *
     * Camera matrices are located in field maps.
     */
    extern const String EXT_CAMERA_MATRIX;

    /**
     * Extension for model list files.
     *
     * Model lists are a part of field maps.
     */
    extern const String EXT_MODEL_LIST;

    /**
     * Extension for palette files.
     */
    extern const String EXT_PALETTE;

    /**
     * Extension for walkmesh files.
     *
     * Walkmeshes are part of field maps.
     */
    extern const String EXT_WALKMESH;
}

