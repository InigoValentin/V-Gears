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

#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreTextureUnitState.h>
#include <OgreVertexIndexData.h>
#include "BackgroundTileBuffer.h"

void BackgroundLayerRanges::Clear(){
    for (Range& range : ranges_) range = Range();
    last_layer_ = -1;
    contiguous_ = true;
}

void BackgroundLayerRanges::Add(
  const int layer, const unsigned int first, const unsigned int count
){
    const int clamped = (layer < 0) ? 0 : ((layer >= LAYER_COUNT) ? LAYER_COUNT - 1 : layer);
    if (last_layer_ > clamped) contiguous_ = false;
    if (clamped > last_layer_) last_layer_ = clamped;
    Range& range = ranges_[clamped];
    if (range.count == 0) range.first = first;
    range.count += count;
}

BackgroundTileBuffer::BackgroundTileBuffer(
  const VGears::Blending blending, const Ogre::String& material_name,
  const unsigned int max_tiles
):
  max_tiles_(max_tiles)
{
    CreateBuffer();
    material_ = Ogre::MaterialManager::getSingleton().create(material_name, "FIELDS");
    Ogre::Pass* pass = material_->getTechnique(0)->getPass(0);
    pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
    pass->setCullingMode(Ogre::CULL_NONE);
    pass->setDepthCheckEnabled(true);
    pass->setLightingEnabled(false);
    // The tiles that add or subtract their colour blend with what is behind. They don't write
    // depth, or a tile in front (such as the light beams, at depth 0) would hide the ones behind
    // it, like the waterfall of eals_1.
    pass->setDepthWriteEnabled(blending == VGears::B_ALPHA);
    if (blending == VGears::B_ALPHA) pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    else pass->setSceneBlending(Ogre::SBT_ADD);
    if (blending == VGears::B_SUBTRACT) pass->setSceneBlendingOperation(Ogre::SBO_SUBTRACT);
    pass->setAlphaRejectFunction(
      blending == VGears::B_SUBTRACT ? Ogre::CMPF_LESS : Ogre::CMPF_GREATER
    );
    pass->setAlphaRejectValue(0);
    Ogre::TextureUnitState* texture = pass->createTextureUnitState();
    texture->setTextureName("system/blank.png");
    texture->setNumMipmaps(-1);
    texture->setTextureFiltering(Ogre::TFO_NONE);
}

BackgroundTileBuffer::~BackgroundTileBuffer(){DestroyBuffer();}

void BackgroundTileBuffer::Clear(){
    DestroyBuffer();
    CreateBuffer();
    layers_.Clear();
}

void BackgroundTileBuffer::SetImage(const Ogre::String& image){
    material_->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(image);
}

bool BackgroundTileBuffer::IsFull() const{
    return operation_.vertexData->vertexCount + QuadWriter::VERTEX_COUNT
      > max_tiles_ * QuadWriter::VERTEX_COUNT;
}

unsigned int BackgroundTileBuffer::Add(
  const QuadWriter::Corners& corners, const QuadWriter::Rectangle& texture, const float depth,
  const float intensity, const int layer
){
    const unsigned int first = operation_.vertexData->vertexCount;
    layers_.Add(layer, first, QuadWriter::VERTEX_COUNT);
    float* vertices = Lock(first);
    QuadWriter::Write(
      vertices, corners, texture, depth, Ogre::ColourValue(intensity, intensity, intensity, 1)
    );
    buffer_->unlock();
    operation_.vertexData->vertexCount += QuadWriter::VERTEX_COUNT;
    return first;
}

void BackgroundTileBuffer::SetPositions(
  const unsigned int first_vertex, const QuadWriter::Corners& corners
){
    QuadWriter::SetPositions(Lock(first_vertex), corners);
    buffer_->unlock();
}

void BackgroundTileBuffer::SetTexture(
  const unsigned int first_vertex, const QuadWriter::Rectangle& texture
){
    QuadWriter::SetTexture(Lock(first_vertex), texture);
    buffer_->unlock();
}

bool BackgroundTileBuffer::IsEmpty() const{return operation_.vertexData->vertexCount == 0;}

void BackgroundTileBuffer::Draw(
  Ogre::SceneManager& scene_manager, Ogre::RenderSystem& render_system
){
    scene_manager._setPass(material_->getTechnique(0)->getPass(0), true, false);
    render_system._render(operation_);
}

void BackgroundTileBuffer::Draw(
  Ogre::SceneManager& scene_manager, Ogre::RenderSystem& render_system,
  const BackgroundLayerRanges::Range& range
){
    const size_t total = operation_.vertexData->vertexCount;
    operation_.vertexData->vertexStart = range.first;
    operation_.vertexData->vertexCount = range.count;
    Draw(scene_manager, render_system);
    operation_.vertexData->vertexStart = 0;
    operation_.vertexData->vertexCount = total;
}

void BackgroundTileBuffer::CreateBuffer(){
    buffer_ = QuadWriter::CreateBuffer(operation_, max_tiles_ * QuadWriter::VERTEX_COUNT);
    operation_.vertexData->vertexCount = 0;
}

void BackgroundTileBuffer::DestroyBuffer(){
    delete operation_.vertexData;
    operation_.vertexData = nullptr;
    buffer_.reset();
}

float* BackgroundTileBuffer::Lock(const unsigned int first_vertex){
    return static_cast<float*>(buffer_->lock(Ogre::HardwareBuffer::HBL_NORMAL))
      + first_vertex * QuadWriter::VERTEX_SIZE;
}
