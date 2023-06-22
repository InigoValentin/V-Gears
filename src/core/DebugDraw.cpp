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

#include <OgreFontManager.h>
#include <OgreHardwareBufferManager.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgreViewport.h>
#include "core/DebugDraw.h"
#include "core/CameraManager.h"
#include "core/DebugDraw.h"
#include "core/Logger.h"
#include "core/UiTextArea.h"

/**
 * Debug drawer singleton.
 */
template<>DebugDraw *Ogre::Singleton<DebugDraw>::msSingleton = nullptr;

DebugDraw::DebugDraw():
  colour_(1, 1, 1, 1),
  screen_space_(true),
  z_coordinate_(0),
  fade_start_square_(999999),
  fade_end_square_(999999),
  font_height_(16),
  line_max_vertex_(0),
  line_3d_max_vertex_(0),
  quad_max_vertex_(0),
  text_max_vertex_(0)
{
    scene_manager_ = Ogre::Root::getSingleton().getSceneManager("Scene");
    render_system_ = Ogre::Root::getSingletonPtr()->getRenderSystem();
    CreateLineVertexBuffer();
    CreateLine3dVertexBuffer();
    CreateTriangle3dVertexBuffer();
    CreateQuadVertexBuffer();
    CreateTextVertexBuffer();
    material_ = Ogre::MaterialManager::getSingleton().create("DebugDraw", "General");
    Ogre::Pass* pass = material_->getTechnique(0)->getPass(0);
    pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
    pass->setCullingMode(Ogre::CULL_NONE);
    pass->setDepthCheckEnabled(true);
    pass->setDepthWriteEnabled(true);
    pass->setLightingEnabled(false);
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    material_3d_ = Ogre::MaterialManager::getSingleton().create("DebugDraw3d", "General");
    pass = material_3d_->getTechnique(0)->getPass(0);
    pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
    pass->setCullingMode(Ogre::CULL_NONE);
    pass->setDepthCheckEnabled(true);
    pass->setDepthWriteEnabled(true);
    pass->setLightingEnabled(false);
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    font_ = Ogre::FontManager::getSingleton().getByName("CourierNew", "SYSTEM");
    if (font_ == nullptr){
        LOG_ERROR("Could not find font \"CourierNew\" for debug draw.");
        throw std::runtime_error("Font load failure");
    }
    font_->load();
    pass = font_->getMaterial()->getTechnique(0)->getPass(0);
    pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
    pass->setCullingMode(Ogre::CULL_NONE);
    pass->setDepthCheckEnabled(true);
    pass->setDepthWriteEnabled(true);
    pass->setLightingEnabled(false);
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    //Ogre::MaterialSerializer mat;
    //mat.exportMaterial(font_->getMaterial(), "font.material");
    scene_manager_->addRenderQueueListener(this);
}


DebugDraw::~DebugDraw(){
    scene_manager_->removeRenderQueueListener(this);
    DestroyLineVertexBuffer();
    DestroyLine3dVertexBuffer();
    DestroyTriangle3dVertexBuffer();
    DestroyQuadVertexBuffer();
    DestroyTextVertexBuffer();
}

void DebugDraw::SetColour(const Ogre::ColourValue& colour){colour_ = colour;}

void DebugDraw::SetScreenSpace(const bool screen_space){screen_space_ = screen_space;}

void DebugDraw::SetZ(const float z){z_coordinate_ = z;}

void DebugDraw::SetFadeDistance(const float fade_s, const float fade_e){
    fade_start_square_ = fade_s * fade_s;
    fade_end_square_ = fade_e * fade_e;
}

void DebugDraw::SetTextAlignment(TextAlignment alignment){text_alignment_ = alignment;}

void DebugDraw::Line(const float x1, const float y1, const float x2, const float y2){
    if (line_render_operation_.vertexData->vertexCount + 2 > line_max_vertex_){
        LOG_ERROR(
          "Max number of lines reached. Can't create more than "
          + Ogre::StringConverter::toString(line_max_vertex_ / 2) + " lines."
        );
        return;
    }
    Ogre::Viewport *viewport(CameraManager::getSingleton().getViewport());
    float width = static_cast<float>(viewport->getActualWidth());
    float height = static_cast<float>(viewport->getActualHeight());
    float new_x1 = (screen_space_ == true) ?  ((int) x1 / width) * 2 - 1 : x1;
    float new_y1 = (screen_space_ == true) ? -(((int) y1 / height) * 2 - 1) : y1;
    float new_x2 = (screen_space_ == true) ?  ((int) x2 / width) * 2 - 1 : x2;
    float new_y2 = (screen_space_ == true) ? -(((int) y2 / height) * 2 - 1) : y2;

    float* write_iterator = static_cast<float*>(
      line_vertex_buffer_->lock(Ogre::HardwareBuffer::HBL_NORMAL)
    );
    write_iterator += line_render_operation_.vertexData->vertexCount * 7;

    // TODO: This and the method below could be refactored.
    *write_iterator ++ = new_x1;
    *write_iterator ++ = new_y1;
    *write_iterator ++ = z_coordinate_;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    *write_iterator ++ = new_x2;
    *write_iterator ++ = new_y2;
    *write_iterator ++ = z_coordinate_;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    line_render_operation_.vertexData->vertexCount += 2;
    line_vertex_buffer_->unlock();
}


void DebugDraw::Line3d(const Ogre::Vector3& point1, const Ogre::Vector3& point2){
    if (line_3d_render_operation_.vertexData->vertexCount + 2 > line_3d_max_vertex_){
        LOG_ERROR(
          "Max number of 3d lines reached. Can't create more than "
          + Ogre::StringConverter::toString(line_3d_max_vertex_ / 2) + " 3d lines."
        );
        return;
    }
    float* write_iterator = static_cast<float*>(
      line_3d_vertex_buffer_->lock(Ogre::HardwareBuffer::HBL_NORMAL)
    );
    write_iterator += line_3d_render_operation_.vertexData->vertexCount * 7;
    *write_iterator ++ = point1.x;
    *write_iterator ++ = point1.y;
    *write_iterator ++ = point1.z;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    *write_iterator ++ = point2.x;
    *write_iterator ++ = point2.y;
    *write_iterator ++ = point2.z;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    line_3d_render_operation_.vertexData->vertexCount += 2;
    line_3d_vertex_buffer_->unlock();
}


void DebugDraw::Triangle3d(
  const Ogre::Vector3& point1, const Ogre::Vector3& point2, const Ogre::Vector3& point3
){
    if (triangle_3d_render_operation_.vertexData->vertexCount + 3 > triangle_3d_max_vertex_){
        LOG_ERROR(
          "Max number of 3d triangles reached. Can't create more than "
          + Ogre::StringConverter::toString(triangle_3d_max_vertex_ / 3) + " 3d triangles."
        );
        return;
    }
    float* write_iterator = static_cast<float*>(triangle_3d_vertex_buffer_->lock(
        Ogre::HardwareBuffer::HBL_NORMAL
    ));
    write_iterator += triangle_3d_render_operation_.vertexData->vertexCount * 7;
    *write_iterator ++ = point1.x;
    *write_iterator ++ = point1.y;
    *write_iterator ++ = point1.z;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    *write_iterator ++ = point2.x;
    *write_iterator ++ = point2.y;
    *write_iterator ++ = point2.z;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    *write_iterator ++ = point3.x;
    *write_iterator ++ = point3.y;
    *write_iterator ++ = point3.z;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    triangle_3d_render_operation_.vertexData->vertexCount += 3;
    triangle_3d_vertex_buffer_->unlock();
}

void DebugDraw::Quad(
  const float x1, const float y1, const float x2, const float y2,
  const float x3, const float y3, const float x4, const float y4
){
    if (quad_render_operation_.vertexData->vertexCount + 6 > quad_max_vertex_){
        LOG_ERROR(
          "Max number of quads reached. Can't create more than "
          + Ogre::StringConverter::toString(quad_max_vertex_ / 6) + " quads."
        );
        return;
    }
    Ogre::Viewport *viewport(CameraManager::getSingleton().getViewport());
    float width = static_cast<float>(viewport->getActualWidth());
    float height = static_cast<float>(viewport->getActualHeight());
    float new_x1 = (screen_space_ == true) ?  ((int) x1 / width) * 2 - 1 : x1;
    float new_y1 = (screen_space_ == true) ? -(((int) y1 / height) * 2 - 1) : y1;
    float new_x2 = (screen_space_ == true) ?  ((int) x2 / width) * 2 - 1 : x2;
    float new_y2 = (screen_space_ == true) ? -((y2 / height) * 2 - 1) : y2;
    float new_x3 = (screen_space_ == true) ?  ((int) x3 / width) * 2 - 1 : x3;
    float new_y3 = (screen_space_ == true) ? -(((int) y3 / height) * 2 - 1) : y3;
    float new_x4 = (screen_space_ == true) ?  ((int) x4 / width) * 2 - 1 : x4;
    float new_y4 = (screen_space_ == true) ? -(((int) y4 / height) * 2 - 1) : y4;
    float* write_iterator = static_cast<float*>(
      quad_vertex_buffer_->lock(Ogre::HardwareBuffer::HBL_NORMAL)
    );
    write_iterator += quad_render_operation_.vertexData->vertexCount * 7;
    *write_iterator ++ = new_x1;
    *write_iterator ++ = new_y1;
    *write_iterator ++ = z_coordinate_;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    *write_iterator ++ = new_x2;
    *write_iterator ++ = new_y2;
    *write_iterator ++ = z_coordinate_;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    *write_iterator ++ = new_x3;
    *write_iterator ++ = new_y3;
    *write_iterator ++ = z_coordinate_;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    *write_iterator ++ = new_x1;
    *write_iterator ++ = new_y1;
    *write_iterator ++ = z_coordinate_;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    *write_iterator ++ = new_x3;
    *write_iterator ++ = new_y3;
    *write_iterator ++ = z_coordinate_;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    *write_iterator ++ = new_x4;
    *write_iterator ++ = new_y4;
    *write_iterator ++ = z_coordinate_;
    *write_iterator ++ = colour_.r;
    *write_iterator ++ = colour_.g;
    *write_iterator ++ = colour_.b;
    *write_iterator ++ = colour_.a;
    quad_render_operation_.vertexData->vertexCount += 6;
    quad_vertex_buffer_->unlock();
}


void DebugDraw::Text(const float x, const float y, const Ogre::String& text){
    if (text_render_operation_.vertexData->vertexCount + text.size() * 6 > text_max_vertex_){
        LOG_ERROR(
          "Max number of text reached. Can't add text \"" + text
          + "\". Max number of letters is "
          + Ogre::StringConverter::toString(text_max_vertex_ / 6) + "."
        );
        return;
    }
    float* write_iterator = static_cast<float*>(
      text_vertex_buffer_->lock(Ogre::HardwareBuffer::HBL_NORMAL)
    );
    write_iterator += text_render_operation_.vertexData->vertexCount * 9;
    Ogre::Viewport *viewport(CameraManager::getSingleton().getViewport());
    float width = static_cast<float>(viewport->getActualWidth());
    float height = static_cast<float>(viewport->getActualHeight());
    float length = 0;
    if (text_alignment_ != LEFT){
        for (auto &c : text) length += ((font_->getGlyphAspectRatio(c) * font_height_) / width) * 2;
        if (text_alignment_ == CENTER) length /= 2;
    }

    float current_x = (screen_space_ == true) ? ((int) x / width) * 2 - 1 : x;
    current_x -= length;
    float current_y  = (screen_space_ == true) ? -(((int) y / height) * 2 - 1) : y;
    float char_height = -(font_height_ / height) * 2;
    for (unsigned int i = 0; i < text.size(); ++ i){
        float char_width = ((font_->getGlyphAspectRatio(text[i]) * font_height_) / width) * 2;
        float new_x1 = current_x;
        float new_y1 = current_y;
        float new_x2 = current_x + char_width;
        float new_y2 = current_y;
        float new_x3 = current_x + char_width;
        float new_y3 = current_y + char_height;
        float new_x4 = current_x;
        float new_y4 = current_y + char_height;
        current_x += char_width;
        const Ogre::Font::UVRect& uv = font_->getGlyphTexCoords(text[i]);
        *write_iterator ++ = new_x1;
        *write_iterator ++ = new_y1;
        *write_iterator ++ = z_coordinate_;
        *write_iterator ++ = colour_.r;
        *write_iterator ++ = colour_.g;
        *write_iterator ++ = colour_.b;
        *write_iterator ++ = colour_.a;
        *write_iterator ++ = uv.left;
        *write_iterator ++ = uv.top;
        *write_iterator ++ = new_x2;
        *write_iterator ++ = new_y2;
        *write_iterator ++ = z_coordinate_;
        *write_iterator ++ = colour_.r;
        *write_iterator ++ = colour_.g;
        *write_iterator ++ = colour_.b;
        *write_iterator ++ = colour_.a;
        *write_iterator ++ = uv.right;
        *write_iterator ++ = uv.top;
        *write_iterator ++ = new_x3;
        *write_iterator ++ = new_y3;
        *write_iterator ++ = z_coordinate_;
        *write_iterator ++ = colour_.r;
        *write_iterator ++ = colour_.g;
        *write_iterator ++ = colour_.b;
        *write_iterator ++ = colour_.a;
        *write_iterator ++ = uv.right;
        *write_iterator ++ = uv.bottom;
        *write_iterator ++ = new_x1;
        *write_iterator ++ = new_y1;
        *write_iterator ++ = z_coordinate_;
        *write_iterator ++ = colour_.r;
        *write_iterator ++ = colour_.g;
        *write_iterator ++ = colour_.b;
        *write_iterator ++ = colour_.a;
        *write_iterator ++ = uv.left;
        *write_iterator ++ = uv.top;
        *write_iterator ++ = new_x3;
        *write_iterator ++ = new_y3;
        *write_iterator ++ = z_coordinate_;
        *write_iterator ++ = colour_.r;
        *write_iterator ++ = colour_.g;
        *write_iterator ++ = colour_.b;
        *write_iterator ++ = colour_.a;
        *write_iterator ++ = uv.right;
        *write_iterator ++ = uv.bottom;
        *write_iterator ++ = new_x4;
        *write_iterator ++ = new_y4;
        *write_iterator ++ = z_coordinate_;
        *write_iterator ++ = colour_.r;
        *write_iterator ++ = colour_.g;
        *write_iterator ++ = colour_.b;
        *write_iterator ++ = colour_.a;
        *write_iterator ++ = uv.left;
        *write_iterator ++ = uv.bottom;
        text_render_operation_.vertexData->vertexCount += 6;
    }
    text_vertex_buffer_->unlock();
}

void DebugDraw::Text(
  const Ogre::Vector3& point, const float x, const float y, const Ogre::String& text
){
    Ogre::Vector3 point2d = CameraManager::getSingleton().ProjectPointToScreen(point);
    if (point2d.z <= 0){
        float dist_sq = point.squaredDistance(
          CameraManager::getSingleton().GetCurrentCamera()->getPosition()
        );
        //std::cout << "[DEBUG TEXT ORIG]: " << dist_sq << "\n";
        // TODO: This always give fixed values.
        //dist_sq = point.squaredDistance(
        //  Ogre::Root::getSingleton().getSceneManager("Scene")
        //  ->getRootSceneNode()->get->getPosition()
        //);
        //std::cout << "[DEBUG TEXT MODI]: " << dist_sq << "\n";
        if (dist_sq < fade_end_square_){
            float a
              = (dist_sq > fade_start_square_)
                ? (1.0f - (dist_sq - fade_start_square_) / (fade_end_square_ - fade_start_square_))
                : 1.0f;
            Ogre::ColourValue colour = colour_;
            colour.a = a;
            SetColour(colour);
            Text(point2d.x + x, point2d.y + y, text);
        }
    }
}

void DebugDraw::renderQueueEnded(
  Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation
){
    if (queueGroupId == Ogre::RENDER_QUEUE_OVERLAY){
        render_system_->clearFrameBuffer(Ogre::FBT_DEPTH);
        auto render_params= render_system_->getFixedFunctionParams(Ogre::TVC_NONE, Ogre::FOG_NONE);
        render_params->setConstant(
          Ogre::GpuProgramParameters::ACT_WORLD_MATRIX, Ogre::Matrix4::IDENTITY
        );
        render_params->setConstant(
          Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX, Ogre::Matrix4::IDENTITY
        );
        render_params->setConstant(
          Ogre::GpuProgramParameters::ACT_VIEW_MATRIX, Ogre::Matrix4::IDENTITY
        );
        render_system_->applyFixedFunctionParams(render_params, Ogre::GPV_GLOBAL);

        if (line_render_operation_.vertexData->vertexCount != 0){
            scene_manager_->_setPass(material_->getTechnique(0)->getPass(0), true, false);
            render_system_->_render(line_render_operation_);
            line_render_operation_.vertexData->vertexCount = 0;
        }
        if (quad_render_operation_.vertexData->vertexCount != 0){
            scene_manager_->_setPass(material_->getTechnique(0)->getPass(0), true, false);
            render_system_->_render(quad_render_operation_);
            quad_render_operation_.vertexData->vertexCount = 0;
        }
        if (text_render_operation_.vertexData->vertexCount != 0){
            scene_manager_->_setPass(
              font_->getMaterial()->getTechnique(0)->getPass(0), true, false
            );
            render_system_->_render(text_render_operation_);
            text_render_operation_.vertexData->vertexCount = 0;
        }
    }
    else if (queueGroupId == Ogre::RENDER_QUEUE_MAIN){
        render_system_->_setWorldMatrix(Ogre::Matrix4::IDENTITY);
        render_system_->_setViewMatrix(
          CameraManager::getSingleton().GetCurrentCamera()->getViewMatrix()
        );
        render_system_->_setProjectionMatrix(
          CameraManager::getSingleton().GetCurrentCamera()->getProjectionMatrix()
        );
        if (line_3d_render_operation_.vertexData->vertexCount != 0){
            scene_manager_->_setPass(material_3d_->getTechnique(0)->getPass(0), true, false);
            render_system_->_render(line_3d_render_operation_);
            line_3d_render_operation_.vertexData->vertexCount = 0;
        }

        if (triangle_3d_render_operation_.vertexData->vertexCount != 0){
            scene_manager_->_setPass(material_3d_->getTechnique(0)->getPass(0), true, false);
            render_system_->_render(triangle_3d_render_operation_);
            triangle_3d_render_operation_.vertexData->vertexCount = 0;
        }
    }
}

void DebugDraw::CreateLineVertexBuffer(){
    line_max_vertex_ = 1024 * 2;
    line_render_operation_.vertexData = new Ogre::VertexData;
    line_render_operation_.vertexData->vertexStart = 0;
    Ogre::VertexDeclaration* vertex_declaration
      = line_render_operation_.vertexData->vertexDeclaration;
    size_t offset = 0;
    vertex_declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
    line_vertex_buffer_ = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
      vertex_declaration->getVertexSize(0), line_max_vertex_,
      Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
    );
    line_render_operation_.vertexData->vertexBufferBinding->setBinding(0, line_vertex_buffer_);
    line_render_operation_.operationType = Ogre::RenderOperation::OT_LINE_LIST;
    line_render_operation_.useIndexes = false;
}

void DebugDraw::DestroyLineVertexBuffer(){
    delete line_render_operation_.vertexData;
    line_render_operation_.vertexData = 0;
    line_vertex_buffer_.reset();
    line_max_vertex_ = 0;
}

void DebugDraw::CreateLine3dVertexBuffer(){
    line_3d_max_vertex_ = 1024 * 2;
    line_3d_render_operation_.vertexData = new Ogre::VertexData;
    line_3d_render_operation_.vertexData->vertexStart = 0;
    Ogre::VertexDeclaration* vertex_declaration
      = line_3d_render_operation_.vertexData->vertexDeclaration;
    size_t offset = 0;
    vertex_declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);

    line_3d_vertex_buffer_ = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
      vertex_declaration->getVertexSize(0), line_3d_max_vertex_,
      Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
    );

    line_3d_render_operation_.vertexData->vertexBufferBinding->setBinding(
      0, line_3d_vertex_buffer_
    );
    line_3d_render_operation_.operationType = Ogre::RenderOperation::OT_LINE_LIST;
    line_3d_render_operation_.useIndexes = false;
}

void DebugDraw::DestroyLine3dVertexBuffer(){
    delete line_3d_render_operation_.vertexData;
    line_3d_render_operation_.vertexData = 0;
    line_3d_vertex_buffer_.reset();
    line_3d_max_vertex_ = 0;
}

void DebugDraw::CreateTriangle3dVertexBuffer(){
    triangle_3d_max_vertex_ = 128 * 3;
    triangle_3d_render_operation_.vertexData = new Ogre::VertexData;
    triangle_3d_render_operation_.vertexData->vertexStart = 0;
    Ogre::VertexDeclaration* vertex_declaration
      = triangle_3d_render_operation_.vertexData->vertexDeclaration;
    size_t offset = 0;
    vertex_declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);

    triangle_3d_vertex_buffer_ = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
      vertex_declaration->getVertexSize(0), triangle_3d_max_vertex_,
      Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
    );

    triangle_3d_render_operation_.vertexData->vertexBufferBinding->setBinding(
      0, triangle_3d_vertex_buffer_
    );
    triangle_3d_render_operation_.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    triangle_3d_render_operation_.useIndexes = false;
}

void DebugDraw::DestroyTriangle3dVertexBuffer(){
    delete triangle_3d_render_operation_.vertexData;
    triangle_3d_render_operation_.vertexData = 0;
    triangle_3d_vertex_buffer_.reset();
    triangle_3d_max_vertex_ = 0;
}

void DebugDraw::CreateQuadVertexBuffer(){
    quad_max_vertex_ = 128 * 6;
    quad_render_operation_.vertexData = new Ogre::VertexData;
    quad_render_operation_.vertexData->vertexStart = 0;
    Ogre::VertexDeclaration* vertex_declaration
      = quad_render_operation_.vertexData->vertexDeclaration;
    size_t offset = 0;
    vertex_declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
    quad_vertex_buffer_ = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
        vertex_declaration->getVertexSize(0), quad_max_vertex_,
        Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
    );
    quad_render_operation_.vertexData->vertexBufferBinding->setBinding(0, quad_vertex_buffer_);
    quad_render_operation_.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    quad_render_operation_.useIndexes = false;
}

void DebugDraw::DestroyQuadVertexBuffer(){
    delete quad_render_operation_.vertexData;
    quad_render_operation_.vertexData = 0;
    quad_vertex_buffer_.reset();
    quad_max_vertex_ = 0;
}

void DebugDraw::CreateTextVertexBuffer(){
    text_max_vertex_ = 4096 * 6;
    text_render_operation_.vertexData = new Ogre::VertexData;
    text_render_operation_.vertexData->vertexStart = 0;
    Ogre::VertexDeclaration* vertex_declaration
      = text_render_operation_.vertexData->vertexDeclaration;
    size_t offset = 0;
    vertex_declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
    text_vertex_buffer_ = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
      vertex_declaration->getVertexSize(0), text_max_vertex_,
      Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
    );
    text_render_operation_.vertexData->vertexBufferBinding->setBinding(0, text_vertex_buffer_);
    text_render_operation_.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    text_render_operation_.useIndexes = false;
}

void DebugDraw::DestroyTextVertexBuffer(){
    delete text_render_operation_.vertexData;
    text_render_operation_.vertexData = 0;
    text_vertex_buffer_.reset();
    text_max_vertex_ = 0;
}
