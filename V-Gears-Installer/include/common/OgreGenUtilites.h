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
#include "Logger.h"
#include "Vram.h"

/**
 * Mesh data
 */
struct MeshData{

    /**
     * Mesh name
     */
    Ogre::String name;

    /**
     * Width of the texture for the mesh.
     */
    int tex_width;

    /**
     * Height of the texture for the mesh.
     */
    int tex_height;
};

/**
 * Bits per pixel modes.
 */
enum BPP{

    /**
     * 4 bites per pixel.
     */
    BPP_4 = 0,

    /**
     * 8 bits per pixel.
     */
    BPP_8 = 1,

    /**
     * No color data, always black.
     */
    BPP_BLACK = -1
};

/**
 * Texture data.
 */
struct TexForGen{

    /**
     * Texture X coordinate.
     */
    u16 texture_x;

    /**
     * Texture Y coordinate.
     */
    u16 texture_y;

    /**
     * X coordinate of the texture palette.
     */
    u16 palette_y;

    /**
     * X coordinate of the texture palette.
     */
    u16 palette_x;

    /**
     * BPP mode of the texture colour.
     */
    BPP bpp;

    /**
     * @todo Understand and document.
     */
    int start_x;

    /**
     * @todo Understand and document.
     */
    int start_y;

    /**
     * Texture comparator.
     *
     * It compares texture coordinates, palette coordinates and colour BPP
     * modes.
     *
     * @param[in] i Texture to compare this one with
     * @return True if the texture coordinates, palette coordinates and colour
     * BPP modes of both textures are equal, false otherwise.
     */
    bool operator==(const TexForGen& i ) const{
        return
          i.texture_x == texture_x
          && i.texture_y == texture_y
          && i.palette_y == palette_y
          && i.palette_x == palette_x
          && i.bpp == bpp;
    }
};

typedef std::vector< TexForGen > VectorTexForGen;

/**
 * Creates a texture from video memory and saves it to a file.
 *
 * @param[in] vram Memory block to load the texture from.
 * @param[in] mesh_data Information about the mesh the texture is assigned to.
 * @param[in] texture_file_name Path to the texture file to save.
 * @param[in] textures @todo It's used to get some coordinates, but for what?
 */
void CreateTexture(
  Vram* vram, const MeshData& mesh_data,
  const Ogre::String& texture_file_name, const VectorTexForGen& textures
);

/**
 * Creates a material and saves it to a file.
 *
 * @param[in] material_name The material name.
 * @param[in] material_file_name Path to the material file to save.
 * @param[in] texture_name Name of the texture to assign to the material.
 * @param[in] vertex_program @todo Understand and document. Can be empty.
 * @param[in] fragment_program @todo Understand and document. Can be empty.
 */
void CreateMaterial(
  const Ogre::String& material_name, const Ogre::String& material_file_name,
  const Ogre::String& texture_name, const Ogre::String& vertex_program,
  const Ogre::String& fragment_program
);

/**
 * Creates a material from video memory and adds it to the texture list.
 *
 * @param[in] pixel_box Texture image descriptor.
 * @param[in] vram Memory block to load the material from.
 * @param[in] start_x X coordinate for the texture.
 * @param[in] start_y Y coordinate for the texture.
 * @param[in] clut_x X coordinate for the texture CLUT.
 * @param[in] clut_y Y coordinate for the texture CLUT.
 * @param[in] texture_x X coordinate for the texture.
 * @param[in] texture_y Y coordinate for the texture.
 * @param[in] bpp BPP mode for the texture colour.
 * @param[in] transparency Indicates if the texture has transparencies.
 * @todo What's the difference between start_x/start_y and texture_x/texture_y?
 */
void CreateTextureFromVram(
  const Ogre::PixelBox& pixel_box, Vram* vram, const int start_x, const int start_y,
  const int clut_x, const int clut_y, const int texture_x, const int texture_y,
  const BPP bpp, const bool transparency
);

/**
 * Adds a texture to the list.
 *
 * @param[in] texture Texture information.
 * @param[in] data Mesh information.
 * @param[in,out] textures The texture will be added to this list.
 * @param[in] logger Custom logger to print info about the process. It can be
 * NULL, but then nothing will be printed.
 */
void AddTexture(
  TexForGen& texture, const MeshData& data, VectorTexForGen& textures, Logger* logger
);

/**
 * Adds or removes a transparency to a colour.
 *
 * @param[in,out] colour The colour to add or remove the transparency to.
 * @param[in] transparency True to add transparency, false to remove it.
 * @param[in] stp @todo Understand and document.
 */
void AddTransparency(u32& colour, const bool transparency, const bool stp);

