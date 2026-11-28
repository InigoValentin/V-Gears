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

#include <string>
#include <OgreMaterial.h>
#include <OgreTexture.h>
#include "common/TypeDefine.h"

namespace VGears {

    /**
     * Converts a coordinate to a texture U component coordinate.
     *
     * @param[in] coord The coordinate to convert.
     * @param[in] id Texture ID.
     * @return Texture coordinate U component.
     * @todo Understand and document properly.
     */
    float ToTextureCoordU(uint8 coord, uint16 id);

    /**
     * Converts a coordinate to a texture V component coordinate.
     *
     * @param[in] coord The coordinate to convert.
     * @param[in] id Texture ID.
     * @return Texture coordinate V component.
     * @todo Understand and document properly.
     */
    float ToTextureCoordV(uint8 coord, uint16 id);

    /**
     * Loads a texture of the world map.
     *
     * @param[in] name Name of the texture file, without extension.
     * @return The texture, named "WorldMapTex_<name>".
     */
    Ogre::TexturePtr LoadWorldMapTexture(const std::string& name);

    /**
     * Shows the frame that the animated textures (water, waterfalls...) have now.
     *
     * @param[in] delta_time Seconds since the last frame.
     */
    void AnimateWorldMapTextures(const float delta_time);

    /**
     * Gets the material of a texture of the terrain, creating it if needed.
     *
     * @param[in] texture_id Id of the texture in the table of textures.
     * @return The material.
     */
    Ogre::MaterialPtr LoadTextureMaterial(uint16 texture_id);
}
