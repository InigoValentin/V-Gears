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

#include <array>
#include <OgreColourValue.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreRenderOperation.h>
#include <OgreVector2.h>

/**
 * Writes textured rectangles into vertex buffers, as two triangles each.
 *
 * A vertex has a position (3 values), a colour (4) and texture coordinates (2).
 */
namespace QuadWriter{

    /**
     * Number of values of a vertex.
     */
    const int VERTEX_SIZE = 9;

    /**
     * Number of vertices of a rectangle.
     */
    const int VERTEX_COUNT = 6;

    /**
     * A rectangle with horizontal and vertical sides.
     */
    struct Rectangle{

        /** Left side. */
        float left;

        /** Top side. */
        float top;

        /** Right side. */
        float right;

        /** Bottom side. */
        float bottom;
    };

    /**
     * The top left, top right, bottom right and bottom left corners of a rectangle.
     */
    typedef std::array<Ogre::Vector2, 4> Corners;

    /**
     * Creates the vertex buffer of a render operation that draws rectangles.
     *
     * @param[out] operation The render operation. Its vertex data is created, with no vertices.
     * @param[in] vertex_count Number of vertices that fit in the buffer.
     * @return The buffer.
     */
    Ogre::HardwareVertexBufferSharedPtr CreateBuffer(
      Ogre::RenderOperation& operation, const unsigned int vertex_count
    );

    /**
     * Writes the vertices of a rectangle.
     *
     * @param[in,out] vertices Where to write them. It's moved past them.
     * @param[in] corners Corners of the rectangle.
     * @param[in] texture The part of the texture, in the 0 to 1 range.
     * @param[in] depth Depth of the vertices.
     * @param[in] colour Colour of the vertices.
     */
    void Write(
      float*& vertices, const Corners& corners, const Rectangle& texture, const float depth,
      const Ogre::ColourValue& colour
    );

    /**
     * Changes the positions of the vertices of a rectangle, but not their depth.
     *
     * @param[in,out] vertices The first vertex of the rectangle.
     * @param[in] corners The new corners.
     */
    void SetPositions(float* vertices, const Corners& corners);

    /**
     * Changes the part of the texture of a rectangle.
     *
     * @param[in,out] vertices The first vertex of the rectangle.
     * @param[in] texture The new part of the texture.
     */
    void SetTexture(float* vertices, const Rectangle& texture);
}
