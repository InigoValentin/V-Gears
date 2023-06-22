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

#include <iostream>
#include <OgreHardwareBufferManager.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include "core/Logger.h"
#include "core/UiSprite.h"


UiSprite::UiSprite(const Ogre::String& name): UiWidget(name){Initialise();}

UiSprite::UiSprite(const Ogre::String& name, const Ogre::String& path_name, UiWidget* parent):
  UiWidget(name, path_name, parent)
{Initialise();}

UiSprite::~UiSprite(){DestroyVertexBuffer();}

void UiSprite::Initialise(){

    scene_manager_ = Ogre::Root::getSingleton().getSceneManager("Scene");
    render_system_ = Ogre::Root::getSingletonPtr()->getRenderSystem();
    material_ = Ogre::MaterialManager::getSingleton().create("UiMaterials." + path_name_, "IMAGES");
    Ogre::Pass* pass = material_->getTechnique(0)->getPass(0);
    pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
    pass->setCullingMode(Ogre::CULL_NONE);
    pass->setDepthCheckEnabled(true);
    pass->setDepthWriteEnabled(true);
    pass->setLightingEnabled(false);
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    pass->setAlphaRejectFunction(Ogre::CMPF_GREATER);
    pass->setAlphaRejectValue(0);
    Ogre::TextureUnitState* tex = pass->createTextureUnitState();
    tex->setTextureName("system/blank.png");
    tex->setNumMipmaps(-1);
    tex->setTextureFiltering(Ogre::TFO_NONE);
    CreateVertexBuffer();
}

void UiSprite::Update(){UiWidget::Update();}

void UiSprite::Render(){
    if(visible_ == true){
        if(render_operation_.vertexData->vertexCount != 0){
            render_system_->_setWorldMatrix(Ogre::Matrix4::IDENTITY);
            render_system_->_setProjectionMatrix(Ogre::Matrix4::IDENTITY);
            render_system_->_setViewMatrix(Ogre::Matrix4::IDENTITY);
            scene_manager_->_setPass(material_->getTechnique(0)->getPass(0), true, false);
            render_system_->setScissorTest(
              true, scissor_left_, scissor_top_, scissor_right_, scissor_bottom_
            );
            render_system_->_render(render_operation_);
            render_system_->setScissorTest(false);
        }
    }
    UiWidget::Render();
}

void UiSprite::UpdateTransformation(){
    UiWidget::UpdateTransformation();
    UpdateGeometry();
}

void UiSprite::SetImage(const Ogre::String& image){
    Ogre::Pass* pass = material_->getTechnique(0)->getPass(0);
    Ogre::TextureUnitState* tex = pass->getTextureUnitState(0);
    tex->setTextureName(image);
}

void UiSprite::SetImage(const char* image){SetImage(Ogre::String(image));}

void UiSprite::SetVertexShader(const Ogre::String& shader){
    Ogre::Pass* pass = material_->getTechnique(0)->getPass(0);
    pass->setVertexProgram(shader, true);
    pass->getVertexProgram()->load();
}

void UiSprite::SetFragmentShader(const Ogre::String& shader){
    Ogre::Pass* pass = material_->getTechnique(0)->getPass(0);
    pass->setFragmentProgram(shader, true);
    pass->getFragmentProgram()->load();
}

void UiSprite::UpdateGeometry(){
    float local_x1 = -final_origin_.x;
    float local_y1 = -final_origin_.y;
    float local_x2 = final_size_.x + local_x1;
    float local_y2 = final_size_.y + local_y1;
    float x = final_translate_.x;
    float y = final_translate_.y;
    int x1, y1, x2, y2, x3, y3, x4, y4;
    if(final_rotation_ != 0){
        float cos = Ogre::Math::Cos(Ogre::Radian(Ogre::Degree(final_rotation_)));
        float sin = Ogre::Math::Sin(Ogre::Radian(Ogre::Degree(final_rotation_)));
        x1 = static_cast<int>(local_x1 * cos - local_y1 * sin + x);
        y1 = static_cast<int>(local_x1 * sin + local_y1 * cos + y);
        x2 = static_cast<int>(local_x2 * cos - local_y1 * sin + x);
        y2 = static_cast<int>(local_x2 * sin + local_y1 * cos + y);
        x3 = static_cast<int>(local_x2 * cos - local_y2 * sin + x);
        y3 = static_cast<int>(local_x2 * sin + local_y2 * cos + y);
        x4 = static_cast<int>(local_x1 * cos - local_y2 * sin + x);
        y4 = static_cast<int>(local_x1 * sin + local_y2 * cos + y);
    }
    else{
        x1 = static_cast<int>(local_x1 + x);
        y1 = static_cast<int>(local_y1 + y);
        x2 = static_cast<int>(local_x2 + x);
        y2 = static_cast<int>(local_y1 + y);
        x3 = static_cast<int>(local_x2 + x);
        y3 = static_cast<int>(local_y2 + y);
        x4 = static_cast<int>(local_x1 + x);
        y4 = static_cast<int>(local_y2 + y);
    }
    float new_x1 = (x1 / screen_width_) * 2 - 1;
    float new_y1 = -((y1 / screen_height_) * 2 - 1);
    float new_x2 = (x2 / screen_width_) * 2 - 1;
    float new_y2 = -((y2 / screen_height_) * 2 - 1);
    float new_x3 = (x3 / screen_width_) * 2 - 1;
    float new_y3 = -((y3 / screen_height_) * 2 - 1);
    float new_x4 = (x4 / screen_width_) * 2 - 1;
    float new_y4 = -((y4 / screen_height_) * 2 - 1);

    float* write_iterator
      = static_cast<float*>(vertex_buffer_->lock(Ogre::HardwareBuffer::HBL_NORMAL));
    *write_iterator ++ = new_x1;
    *write_iterator ++ = new_y1;
    *write_iterator ++ = final_z_;
    *write_iterator ++ = colour_1_.r;
    *write_iterator ++ = colour_1_.g;
    *write_iterator ++ = colour_1_.b;
    *write_iterator ++ = colour_1_.a;
    *write_iterator ++ = 0;
    *write_iterator ++ = 0;

    *write_iterator ++ = new_x2;
    *write_iterator ++ = new_y2;
    *write_iterator ++ = final_z_;
    *write_iterator ++ = colour_2_.r;
    *write_iterator ++ = colour_2_.g;
    *write_iterator ++ = colour_2_.b;
    *write_iterator ++ = colour_2_.a;
    *write_iterator ++ = 1;
    *write_iterator ++ = 0;

    *write_iterator ++ = new_x3;
    *write_iterator ++ = new_y3;
    *write_iterator ++ = final_z_;
    *write_iterator ++ = colour_3_.r;
    *write_iterator ++ = colour_3_.g;
    *write_iterator ++ = colour_3_.b;
    *write_iterator ++ = colour_3_.a;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;

    *write_iterator ++ = new_x1;
    *write_iterator ++ = new_y1;
    *write_iterator ++ = final_z_;
    *write_iterator ++ = colour_1_.r;
    *write_iterator ++ = colour_1_.g;
    *write_iterator ++ = colour_1_.b;
    *write_iterator ++ = colour_1_.a;
    *write_iterator ++ = 0;
    *write_iterator ++ = 0;

    *write_iterator ++ = new_x3;
    *write_iterator ++ = new_y3;
    *write_iterator ++ = final_z_;
    *write_iterator ++ = colour_3_.r;
    *write_iterator ++ = colour_3_.g;
    *write_iterator ++ = colour_3_.b;
    *write_iterator ++ = colour_3_.a;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;

    *write_iterator ++ = new_x4;
    *write_iterator ++ = new_y4;
    *write_iterator ++ = final_z_;
    *write_iterator ++ = colour_4_.r;
    *write_iterator ++ = colour_4_.g;
    *write_iterator ++ = colour_4_.b;
    *write_iterator ++ = colour_4_.a;
    *write_iterator ++ = 0;
    *write_iterator ++ = 1;

    render_operation_.vertexData->vertexCount = 6;
    vertex_buffer_->unlock();
}

void UiSprite::CreateVertexBuffer(){
    render_operation_.vertexData = new Ogre::VertexData;
    render_operation_.vertexData->vertexStart = 0;
    Ogre::VertexDeclaration* vertex_declaration = render_operation_.vertexData->vertexDeclaration;
    size_t offset = 0;
    vertex_declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
    vertex_buffer_ = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
      vertex_declaration->getVertexSize(0), 6, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
    );
    render_operation_.vertexData->vertexBufferBinding->setBinding(0, vertex_buffer_);
    render_operation_.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    render_operation_.useIndexes = false;
}

void UiSprite::DestroyVertexBuffer(){
    delete render_operation_.vertexData;
    render_operation_.vertexData = 0;
    vertex_buffer_.reset();
}
