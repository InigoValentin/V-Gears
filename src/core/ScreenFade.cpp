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
#include <OgreMaterialManager.h>
#include <OgreRoot.h>
#include <OgreTechnique.h>
#include "core/CameraManager.h"
#include "core/EntityManager.h"
#include "core/Logger.h"
#include "core/ScreenFade.h"

namespace{

    // The quad is two triangles.
    const unsigned int QUAD_VERTEX_COUNT = 6;

    // Position (3), colour (4) and texture coordinates (2).
    const unsigned int QUAD_FLOATS_PER_VERTEX = 9;

    // Resource group of the materials. The background creates its own there as well.
    const char* const MATERIAL_GROUP = "FIELDS";

    /**
     * Creates a material that is drawn over everything in the field, without lighting or texture.
     */
    Ogre::MaterialPtr CreateMaterial(const Ogre::String& name){
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
          name, MATERIAL_GROUP
        );
        Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
        pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
        pass->setCullingMode(Ogre::CULL_NONE);
        pass->setDepthCheckEnabled(false);
        pass->setDepthWriteEnabled(false);
        pass->setLightingEnabled(false);
        // No texture: the fade is a flat colour. A texture that can't be loaded is replaced with a
        // dark placeholder, which turned every colour black.
        return material;
    }
}

ScreenFade::ScreenFade(){
    scene_manager_ = Ogre::Root::getSingleton().getSceneManager("Scene");
    render_system_ = Ogre::Root::getSingletonPtr()->getRenderSystem();

    normal_material_ = CreateMaterial("ScreenFadeNormal");
    normal_material_->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);

    add_material_ = CreateMaterial("ScreenFadeAdd");
    add_material_->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_ADD);

    // The colour is taken from the field: field - colour, and not colour - field.
    subtract_material_ = CreateMaterial("ScreenFadeSubtract");
    Ogre::Pass* pass = subtract_material_->getTechnique(0)->getPass(0);
    pass->setSceneBlending(Ogre::SBF_ONE, Ogre::SBF_ONE);
    pass->setSceneBlendingOperation(Ogre::SBO_REVERSE_SUBTRACT);

    render_op_.vertexData = new Ogre::VertexData;
    render_op_.vertexData->vertexStart = 0;
    render_op_.vertexData->vertexCount = QUAD_VERTEX_COUNT;
    Ogre::VertexDeclaration* declaration = render_op_.vertexData->vertexDeclaration;
    size_t offset = 0;
    declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
    declaration->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
    vertex_buffer_ = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
      declaration->getVertexSize(0), QUAD_VERTEX_COUNT, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY,
      false
    );
    render_op_.vertexData->vertexBufferBinding->setBinding(0, vertex_buffer_);
    render_op_.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    render_op_.useIndexes = false;

    scene_manager_->addRenderQueueListener(this);
}

ScreenFade::~ScreenFade(){
    scene_manager_->removeRenderQueueListener(this);
    delete render_op_.vertexData;
}

void ScreenFade::Update(const float delta){
    if (!state_.Advance(delta)) return;
    for (const ScriptId& script : waiting_) ScriptManager::getSingleton().ContinueScriptExecution(script);
    waiting_.clear();
}

void ScreenFade::Clear(){
    state_.Reset();
    draw_logged_ = false;
    waiting_.clear();
}

void ScreenFade::ScriptFade(const int type, const int r, const int g, const int b, const int speed){
    LOG_TRIVIAL(
      "Screen fade type " + std::to_string(type) + ", colour " + std::to_string(r) + " "
      + std::to_string(g) + " " + std::to_string(b) + ", speed " + std::to_string(speed) + "."
    );
    draw_logged_ = false;
    if (!state_.StartFade(type, r, g, b, speed))
        LOG_WARNING("Screen fade type " + std::to_string(type) + " is not known.");
}

void ScreenFade::ScriptNFade(
  const int type, const int r, const int g, const int b, const int frames
){
    LOG_TRIVIAL(
      "Screen nfade type " + std::to_string(type) + ", colour " + std::to_string(r) + " "
      + std::to_string(g) + " " + std::to_string(b) + ", " + std::to_string(frames) + " frames."
    );
    draw_logged_ = false;
    if (!state_.StartNFade(type, r, g, b, frames))
        LOG_WARNING("Screen nfade type " + std::to_string(type) + " is not known.");
}

int ScreenFade::ScriptWait(){
    if (!state_.IsFading()) return 1;
    waiting_.push_back(ScriptManager::getSingleton().GetCurrentScriptId());
    return -1;
}

void ScreenFade::WriteQuad(const FadeColour& colour, const float alpha){
    // The corners of the screen, in the coordinates the background is drawn in.
    const float corners[QUAD_VERTEX_COUNT][2] = {
      {-1, 1}, {1, 1}, {1, -1}, {-1, 1}, {1, -1}, {-1, -1}
    };
    float* write_iterator
      = static_cast<float*>(vertex_buffer_->lock(Ogre::HardwareBuffer::HBL_DISCARD));
    for (unsigned int i = 0; i < QUAD_VERTEX_COUNT; ++ i){
        *write_iterator ++ = corners[i][0];
        *write_iterator ++ = corners[i][1];
        *write_iterator ++ = 0;
        *write_iterator ++ = colour.r;
        *write_iterator ++ = colour.g;
        *write_iterator ++ = colour.b;
        *write_iterator ++ = alpha;
        *write_iterator ++ = 0;
        *write_iterator ++ = 0;
    }
    vertex_buffer_->unlock();
}

void ScreenFade::renderQueueStarted(
  Ogre::uint8 queue_group_id, const Ogre::String& invocation, bool& skip_this_invocation
){
    if (queue_group_id != Ogre::RENDER_QUEUE_OVERLAY || !state_.IsVisible()) return;
    if (!EntityManager::getSingleton().IsFieldModule()) return;

    Ogre::MaterialPtr material;
    switch (state_.GetBlend()){
        case ScreenFadeState::BLEND_NORMAL: material = normal_material_; break;
        case ScreenFadeState::BLEND_ADD: material = add_material_; break;
        case ScreenFadeState::BLEND_SUBTRACT: material = subtract_material_; break;
        default: return;
    }
    WriteQuad(state_.GetColour(), state_.GetAlpha());
    if (!draw_logged_){
        draw_logged_ = true;
        LOG_TRIVIAL(
          "Screen fade drawn, blend " + std::to_string(static_cast<int>(state_.GetBlend()))
          + ", colour " + std::to_string(state_.GetColour().r) + " "
          + std::to_string(state_.GetColour().g) + " " + std::to_string(state_.GetColour().b)
          + ", alpha " + std::to_string(state_.GetAlpha()) + "."
        );
    }

    // Same setup the background uses: the quad is given in screen coordinates.
    Ogre::GpuProgramParametersPtr rs_params = render_system_->getFixedFunctionParams(
      Ogre::TVC_NONE, Ogre::FOG_NONE
    );
    rs_params->setConstant(Ogre::GpuProgramParameters::ACT_WORLD_MATRIX, Ogre::Matrix4::IDENTITY);
    rs_params->setConstant(
      Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX, Ogre::Matrix4::IDENTITY
    );
    // TODO This is deprecated, but if not done the quad is drawn with the last view matrix.
    render_system_->_setViewMatrix(Ogre::Matrix4::IDENTITY);
    rs_params->setConstant(Ogre::GpuProgramParameters::ACT_VIEW_MATRIX, Ogre::Matrix4::IDENTITY);
    render_system_->applyFixedFunctionParams(rs_params, Ogre::GPV_GLOBAL);
    scene_manager_->_setPass(material->getTechnique(0)->getPass(0), true, false);
    render_system_->_render(render_op_);
}
