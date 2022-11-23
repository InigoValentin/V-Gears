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
#include "core/EntityCollision.h"

EntityCollision::EntityCollision(){
    mRenderOp.vertexData = new Ogre::VertexData();
    mRenderOp.indexData = 0;
    mRenderOp.vertexData->vertexCount = 74;
    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.operationType = Ogre::RenderOperation::OT_LINE_LIST;
    mRenderOp.useIndexes = false;
    Ogre::VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;
    Ogre::VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;
    decl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    Ogre::HardwareVertexBufferSharedPtr vbuf0
      = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
        decl->getVertexSize(0),
        mRenderOp.vertexData->vertexCount,
        Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY
    );
    bind->setBinding(0, vbuf0);

    float* pPos
      = static_cast<float*>(vbuf0->lock(Ogre::HardwareBuffer::HBL_DISCARD));

    *pPos ++ = -1.0f; *pPos ++ =  0.0f; *pPos ++ =  0.0f;
    *pPos ++ = -1.0f; *pPos ++ =  0.0f; *pPos ++ =  1.0f;

    *pPos ++ = -1.0f; *pPos ++ =  0.0f; *pPos ++ =  0.0f;
    *pPos ++ = -0.9f; *pPos ++ =  0.5f; *pPos ++ =  0.0f;
    *pPos ++ = -0.9f; *pPos ++ =  0.5f; *pPos ++ =  0.0f;
    *pPos ++ = -0.9f; *pPos ++ =  0.5f; *pPos ++ =  1.0f;
    *pPos ++ = -0.9f; *pPos ++ =  0.5f; *pPos ++ =  1.0f;
    *pPos ++ = -1.0f; *pPos ++ =  0.0f; *pPos ++ =  1.0f;

    *pPos ++ = -0.9f; *pPos ++ =  0.5f; *pPos ++ =  0.0f;
    *pPos ++ = -0.5f; *pPos ++ =  0.9f; *pPos ++ =  0.0f;
    *pPos ++ = -0.5f; *pPos ++ =  0.9f; *pPos ++ =  0.0f;
    *pPos ++ = -0.5f; *pPos ++ =  0.9f; *pPos ++ =  1.0f;
    *pPos ++ = -0.5f; *pPos ++ =  0.9f; *pPos ++ =  1.0f;
    *pPos ++ = -0.9f;  *pPos ++ =  0.5f;*pPos ++ =  1.0f;

    *pPos ++ = -0.5f; *pPos ++ =  0.9f; *pPos ++ =  0.0f;
    *pPos ++ =  0.0f; *pPos ++ =  1.0f; *pPos ++ =  0.0f;
    *pPos ++ =  0.0f; *pPos ++ =  1.0f; *pPos ++ =  0.0f;
    *pPos ++ =  0.0f; *pPos ++ =  1.0f; *pPos ++ =  1.0f;
    *pPos ++ =  0.0f; *pPos ++ =  1.0f; *pPos ++ =  1.0f;
    *pPos ++ = -0.5f; *pPos ++ =  0.9f; *pPos ++ =  1.0f;

    *pPos ++ =  0.0f; *pPos ++ =  1.0f; *pPos ++ =  0.0f;
    *pPos ++ =  0.5f; *pPos ++ =  0.9f; *pPos ++ =  0.0f;
    *pPos ++ =  0.5f; *pPos ++ =  0.9f; *pPos ++ =  0.0f;
    *pPos ++ =  0.5f; *pPos ++ =  0.9f; *pPos ++ =  1.0f;
    *pPos ++ =  0.5f; *pPos ++ =  0.9f; *pPos ++ =  1.0f;
    *pPos ++ =  0.0f; *pPos ++ =  1.0f; *pPos ++ =  1.0f;

    *pPos ++ =  0.5f; *pPos ++ =  0.9f; *pPos ++ =  0.0f;
    *pPos ++ =  0.9f; *pPos ++ =  0.5f; *pPos ++ =  0.0f;
    *pPos ++ =  0.9f; *pPos ++ =  0.5f; *pPos ++ =  0.0f;
    *pPos ++ =  0.9f; *pPos ++ =  0.5f; *pPos ++ =  1.0f;
    *pPos ++ =  0.9f; *pPos ++ =  0.5f; *pPos ++ =  1.0f;
    *pPos ++ =  0.5f; *pPos ++ =  0.9f; *pPos ++ =  1.0f;

    *pPos ++ =  0.9f; *pPos ++ =  0.5f; *pPos ++ =  0.0f;
    *pPos ++ =  1.0f; *pPos ++ =  0.0f; *pPos ++ =  0.0f;
    *pPos ++ =  1.0f; *pPos ++ =  0.0f; *pPos ++ =  0.0f;
    *pPos ++ =  1.0f; *pPos ++ =  0.0f; *pPos ++ =  1.0f;
    *pPos ++ =  1.0f; *pPos ++ =  0.0f; *pPos ++ =  1.0f;
    *pPos ++ =  0.9f; *pPos ++ =  0.5f; *pPos ++ =  1.0f;

    *pPos ++ = -1.0f; *pPos ++ =  0.0f; *pPos ++ =  0.0f;
    *pPos ++ = -0.9f; *pPos ++ = -0.5f; *pPos ++ =  0.0f;
    *pPos ++ = -0.9f; *pPos ++ = -0.5f; *pPos ++ =  0.0f;
    *pPos ++ = -0.9f; *pPos ++ = -0.5f; *pPos ++ =  1.0f;
    *pPos ++ = -0.9f; *pPos ++ = -0.5f; *pPos ++ =  1.0f;
    *pPos ++ = -1.0f; *pPos ++ =  0.0f; *pPos ++ =  1.0f;

    *pPos ++ = -0.9f; *pPos ++ = -0.5f; *pPos ++ =  0.0f;
    *pPos ++ = -0.5f; *pPos ++ = -0.9f; *pPos ++ =  0.0f;
    *pPos ++ = -0.5f; *pPos ++ = -0.9f; *pPos ++ =  0.0f;
    *pPos ++ = -0.5f; *pPos ++ = -0.9f; *pPos ++ =  1.0f;
    *pPos ++ = -0.5f; *pPos ++ = -0.9f; *pPos ++ =  1.0f;
    *pPos ++ = -0.9f; *pPos ++ = -0.5f; *pPos ++ =  1.0f;

    *pPos ++ = -0.5f; *pPos ++ = -0.9f; *pPos ++ =  0.0f;
    *pPos ++ =  0.0f; *pPos ++ = -1.0f; *pPos ++ =  0.0f;
    *pPos ++ =  0.0f; *pPos ++ = -1.0f; *pPos ++ =  0.0f;
    *pPos ++ =  0.0f; *pPos ++ = -1.0f; *pPos ++ =  1.0f;
    *pPos ++ =  0.0f; *pPos ++ = -1.0f; *pPos ++ =  1.0f;
    *pPos ++ = -0.5f; *pPos ++ = -0.9f; *pPos ++ =  1.0f;

    *pPos ++ =  0.0f; *pPos ++ = -1.0f; *pPos ++ =  0.0f;
    *pPos ++ =  0.5f; *pPos ++ = -0.9f; *pPos ++ =  0.0f;
    *pPos ++ =  0.5f; *pPos ++ = -0.9f; *pPos ++ =  0.0f;
    *pPos ++ =  0.5f; *pPos ++ = -0.9f; *pPos ++ =  1.0f;
    *pPos ++ =  0.5f; *pPos ++ = -0.9f; *pPos ++ =  1.0f;
    *pPos ++ =  0.0f; *pPos ++ = -1.0f; *pPos ++ =  1.0f;

    *pPos ++ =  0.5f; *pPos ++ = -0.9f; *pPos ++ =  0.0f;
    *pPos ++ =  0.9f; *pPos ++ = -0.5f; *pPos ++ =  0.0f;
    *pPos ++ =  0.9f; *pPos ++ = -0.5f; *pPos ++ =  0.0f;
    *pPos ++ =  0.9f; *pPos ++ = -0.5f; *pPos ++ =  1.0f;
    *pPos ++ =  0.9f; *pPos ++ = -0.5f; *pPos ++ =  1.0f;
    *pPos ++ =  0.5f; *pPos ++ = -0.9f; *pPos ++ =  1.0f;

    *pPos ++ =  0.9f; *pPos ++ = -0.5f; *pPos ++ =  0.0f;
    *pPos ++ =  1.0f; *pPos ++ =  0.0f; *pPos ++ =  0.0f;
    *pPos ++ =  1.0f; *pPos ++ =  0.0f; *pPos ++ =  0.0f;
    *pPos ++ =  1.0f; *pPos ++ =  0.0f; *pPos ++ =  1.0f;
    *pPos ++ =  1.0f; *pPos ++ =  0.0f; *pPos ++ =  1.0f;
    *pPos ++ =  0.9f; *pPos ++ = -0.5f; *pPos ++ =  1.0f;

    vbuf0->unlock();

    Ogre::AxisAlignedBox aabb;
    aabb.setInfinite();
    setBoundingBox(aabb);
}

EntityCollision::~EntityCollision(){delete mRenderOp.vertexData;}

Ogre::Real EntityCollision::getSquaredViewDepth(const Ogre::Camera* cam) const{
    return 0.0f;
}

Ogre::Real EntityCollision::getBoundingRadius() const{return 0.0f;}
