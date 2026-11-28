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

#include <OgreHardwareBufferManager.h>
#include <OgreVertexIndexData.h>
#include "QuadWriter.h"

namespace{

    /**
     * The corner of each vertex of the two triangles.
     */
    const int CORNERS[QuadWriter::VERTEX_COUNT] = {0, 1, 2, 0, 2, 3};

    /**
     * Position of the texture coordinates in a vertex.
     */
    const int TEXTURE_OFFSET = 7;

    /**
     * The texture coordinates of each corner.
     */
    QuadWriter::Corners TextureCorners(const QuadWriter::Rectangle& texture){
        return {{
          {texture.left, texture.top}, {texture.right, texture.top},
          {texture.right, texture.bottom}, {texture.left, texture.bottom}
        }};
    }
}

namespace QuadWriter{

    Ogre::HardwareVertexBufferSharedPtr CreateBuffer(
      Ogre::RenderOperation& operation, const unsigned int vertex_count
    ){
        operation.vertexData = new Ogre::VertexData;
        operation.vertexData->vertexStart = 0;
        Ogre::VertexDeclaration* declaration = operation.vertexData->vertexDeclaration;
        size_t offset = 0;
        declaration->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
        declaration->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
        Ogre::HardwareVertexBufferSharedPtr buffer
          = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
            declaration->getVertexSize(0), vertex_count,
            Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
          );
        operation.vertexData->vertexBufferBinding->setBinding(0, buffer);
        operation.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        operation.useIndexes = false;
        return buffer;
    }

    void Write(
      float*& vertices, const Corners& corners, const Rectangle& texture, const float depth,
      const Ogre::ColourValue& colour
    ){
        const Corners coordinates = TextureCorners(texture);
        for (const int corner : CORNERS){
            for (const float value : {
              corners[corner].x, corners[corner].y, depth, colour.r, colour.g, colour.b, colour.a,
              coordinates[corner].x, coordinates[corner].y
            }) *vertices ++ = value;
        }
    }

    void SetPositions(float* vertices, const Corners& corners){
        for (const int corner : CORNERS){
            vertices[0] = corners[corner].x;
            vertices[1] = corners[corner].y;
            vertices += VERTEX_SIZE;
        }
    }

    void SetTexture(float* vertices, const Rectangle& texture){
        const Corners coordinates = TextureCorners(texture);
        for (const int corner : CORNERS){
            vertices[TEXTURE_OFFSET] = coordinates[corner].x;
            vertices[TEXTURE_OFFSET + 1] = coordinates[corner].y;
            vertices += VERTEX_SIZE;
        }
    }
}
