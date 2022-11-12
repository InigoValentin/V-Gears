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

#include "common/DrawSkeleton.h"
#include "common/Logger.h"
#include "common/TypeDefine.h"

/**
 * Used to bind a skeleton position.
 */
#define POSITION_BINDING 0

/**
 * Used for skeleton bone colours.
 */
#define COLOUR_BINDING 1

/**
 * Used to keep track of the skeleton and it's bones positions.
 */
float* position;

/**
 * The bone index for skeleton currently being drawn.
 */
u32 cur_index;

/**
 * Indexes for a skeleton and it's bones
 */
u16* index_data;

/**
 * Mesh for a skeleton and it's bones.
 */
Ogre::SubMesh* sub_mesh;

void DrawSkeleton(const Skeleton& skeleton, const Ogre::MeshPtr& mesh){
    if (skeleton.size() == 0) return;
    sub_mesh = mesh->createSubMesh("Skeleton");
    sub_mesh->setMaterialName("Skeleton");
    sub_mesh->useSharedVertices = false;
    sub_mesh->operationType = Ogre::RenderOperation::OT_LINE_LIST;
    // Allocate and prepare vertex data.
    sub_mesh->vertexData = new Ogre::VertexData();
    sub_mesh->vertexData->vertexStart = 0;
    sub_mesh->vertexData->vertexCount
      = static_cast<size_t>(skeleton.size() * 2);
    sub_mesh->indexData = new Ogre::IndexData();
    sub_mesh->indexData->indexStart = 0;
    sub_mesh->indexData->indexCount = sub_mesh->vertexData->vertexCount;
    sub_mesh->indexData->indexBuffer
      = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
        Ogre::HardwareIndexBuffer::IT_16BIT,
        sub_mesh->indexData->indexCount,
        Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY
      );
    index_data = static_cast<u16*> (
      sub_mesh->indexData->indexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD)
    );
    cur_index = 0;
    Ogre::VertexDeclaration* decl = sub_mesh->vertexData->vertexDeclaration;
    Ogre::VertexBufferBinding* bind = sub_mesh->vertexData->vertexBufferBinding;
    // 1st buffer.
    decl->addElement(POSITION_BINDING, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    Ogre::HardwareVertexBufferSharedPtr vbuf0
      = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
        decl->getVertexSize(POSITION_BINDING),
        sub_mesh->vertexData->vertexCount,
        Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY
      );
    bind->setBinding(POSITION_BINDING, vbuf0);

    // 2nd buffer.
    decl->addElement(COLOUR_BINDING, 0, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
    Ogre::HardwareVertexBufferSharedPtr vbuf1
      = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
        decl->getVertexSize(COLOUR_BINDING),
        sub_mesh->vertexData->vertexCount,
        Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    // Set vertex buffer binding so buffer 1 is bound to our colour buffer.
    bind->setBinding(COLOUR_BINDING, vbuf1);
    position = static_cast<float*> (
      vbuf0->lock(Ogre::HardwareBuffer::HBL_DISCARD)
    );
    std::vector<Ogre::RGBA> colours(sub_mesh->vertexData->vertexCount);
    DrawBone(skeleton, -1, colours.data());
    vbuf0->unlock();
    vbuf1->writeData(0, vbuf1->getSizeInBytes(), colours.data(), true);
    sub_mesh->indexData->indexBuffer->unlock();
    sub_mesh->indexData->optimiseVertexCacheTriList();
}



void DrawBone(const Skeleton& skeleton, int parent_index, Ogre::RGBA* colours){
    for (size_t i = 0; i < skeleton.size(); ++ i){
        if (skeleton[ i ].parent_id == parent_index){
            *position ++ = 0;
            *position ++ = 0;
            *position ++ = 0;
            *position ++ = 0;
            *position ++ = 0;
            *position ++ = 0;
            Ogre::ColourValue colour1 = Ogre::ColourValue(1, 0, 0);
            Ogre::ColourValue colour2 = Ogre::ColourValue(1, 1, 0);
            Ogre::RenderSystem* rs
              = Ogre::Root::getSingleton().getRenderSystem();
            colours[cur_index] = colour1.getAsBYTE();
            colours[cur_index + 1] = colour2.getAsBYTE();
            index_data[cur_index + 0] = cur_index + 0;
            index_data[cur_index + 1] = cur_index + 1;
            Ogre::VertexBoneAssignment vba;
            vba.vertexIndex = cur_index + 0;
            vba.boneIndex = (parent_index == -1) ? 1 : parent_index * 2 + 3;
            vba.weight = 1.0f;
            sub_mesh->addBoneAssignment(vba);
            vba.vertexIndex = cur_index + 1;
            vba.boneIndex = i * 2 + 3;
            vba.weight = 1.0f;
            sub_mesh->addBoneAssignment(vba);
            cur_index += 2;
            DrawBone(skeleton, i, colours);
        }
    }
}
