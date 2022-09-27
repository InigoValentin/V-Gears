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
     * @param i[in] Texture to compare this one with
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
 * @param vram[in] Memory block to load the texture from.
 * @param mesh_data[in] Information about the mesh the texture is assigned to.
 * @param texture_file_name[in] Path to the texture file to save.
 * @parm textures[in] @todo It's used to get some coordinates, but for what?
 */
void CreateTexture(
  Vram* vram, const MeshData& mesh_data,
  const Ogre::String& texture_file_name, const VectorTexForGen& textures
);

/**
 * Creates a material from video memory and saves it to a file.
 *
 * @param vram[in] Memory block to load the material from.
 * @param texture_file_name[in] Path to the texture file to save.
 * @param texture_name[in] Name of the texture to assign to the material.
 * @param vertex_program[in] @todo Understand and document. Can be empty.
 * @param fragment_program[in] @todo Understand and document. Can be empty.
 */
void CreateMaterial(
  const Ogre::String& material_name, const Ogre::String& material_file_name,
  const Ogre::String& texture_name, const Ogre::String& vertex_program,
  const Ogre::String& fragment_program
);

/**
 * Creates a material from video memory and adds it to the texture list.
 *
 * @param pixel_box[in] Texture image descriptor.
 * @param vram[in] Memory block to load the material from.
 * @param start_x[in] X coordinate for the texture.
 * @param start_y[in] Y coordinate for the texture.
 * @param clut_x[in] X coordinate for the texture CLUT.
 * @param clut_y[in] Y coordinate for the texture CLUT.
 * @param texture_x[in] X coordinate for the texture.
 * @param texture_y[in] Y coordinate for the texture.
 * @param bpp[in] BPP mode for the texture colour.
 * @param transparency[in] Indicates if the texture has transparencies.
 * @todo Whats the difference betwwen start_x/start_y and texture_x/texture_y?
 */
void CreateTextureFromVram(
  const Ogre::PixelBox& pixel_box, Vram* vram, const int start_x, const int start_y,
  const int clut_x, const int clut_y, const int texture_x, const int texture_y,
  const BPP bpp, const bool transparency
);

/**
 * Adds a texture to the list.
 *
 * @param texture[in] Texture information/
 * @param data[in] Mehs information.
 * @param textures[in|out] The texture will be added to this list.
 * @param logger[in] Custom logger to print info abou the process. It can be
 * NULL, but then nothing will be printed.
 */
void AddTexture(
  TexForGen& texture, const MeshData& data, VectorTexForGen& textures,
  Logger* logger
);

/**
 * Adds or removes a transparency to a colour.
 *
 * @param colour[in|out] The colour to add or remove the transparency to.
 * @param transparency[in] True to add transparency, false to remove it.
 * @param stp @todo Understand and document.
 */
void AddTransparency(u32& colour, const bool transparency, const bool stp);

