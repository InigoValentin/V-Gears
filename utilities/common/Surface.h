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
 * A surface description.
 *
 * A surface is a plane of pixels. Each pixels doesn't have it's own
 * coordinates, but are sequentially added and their coordinates can be
 * calculated knowing the surface width.
 */
struct Surface{

    /**
     * Initializator.
     *
     * Initializes an empty surface.
     */
    Surface();

    /**
     * Initializator.
     *
     * It copies another surface.
     */
    Surface(const Surface &copy);

    /**
     * Assignment operation.
     *
     * It copies another surface.
     */
    Surface& operator =(const Surface &copy);

    /**
     * Destructor.
     */
    ~Surface();

    /**
     * The list of pixels in the surface.
     */
    std::vector<unsigned char> pixels;

    /**
     * The surface width.
     */
    int width;

    /**
     * The surface height.
     */
    int height;
};

/**
 * Creates an empty surface
 *
 * @param width[in] The width for the new surface.
 * @param height[in] The height for the new surface.
 */
Surface* CreateSurface(const int width, const int height);

/**
 * Copies one surface into another.
 *
 * It can copy just sections of surfaces, but always from the top left corner.
 *
 * @param dest[out] The target surface.
 * @param x_d[in] Number of columns to copy.
 * @param y_d[in] Number of rows to copy.
 * @param src[in] The source surface.
 *
 */
void CopyToSurface(Surface* dest, const int x_d, const int y_d, Surface* src);

/**
 * Creates a partial surface from a surface.
 *
 * @param x[in] Starting column to copy from the original surface.
 * @param y[in] Starting row to copy from the original surface.
 * @param width[in] Number of columns to copy.
 * @param height[in] Number of rows to copy.
 * @param surface[in] The original surface.
 * @return the resulting subsurface.
 */
Surface* CreateSubSurface(
  const int x, const int y, const int width, const int height, Surface* surface
);

/**
 * Creates a surface from a list of pixels.
 *
 * @param width[in] The width of the surface.
 * @param height[in] The height of the surface.
 * @param pixels[in] The list of pixels for the new surface. If there are not
 * enough pixels to cover the entire surface, the rest of pixels will be NULL.
 * @return The new surface.
 */
Surface* CreateSurfaceFrom(
  const int width, const int height, unsigned char* pixels
);
