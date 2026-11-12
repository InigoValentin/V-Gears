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

#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>
#include <tinyxml.h>
#include <OgreHardwareBufferManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreImage.h>
#include <OgreMaterialManager.h>
#include <OgreRoot.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include "core/AudioManager.h"
#include "core/CameraManager.h"
#include "core/EntityManager.h"
#include "core/Logger.h"
#include "core/MoviePlayer.h"

namespace{

    // Size of the frames of the original movies.
    const int FRAME_WIDTH = 320;
    const int FRAME_HEIGHT = 224;

    // The texture is the next power of two of the frame.
    const int TEXTURE_WIDTH = 512;
    const int TEXTURE_HEIGHT = 256;

    // The field is drawn as 320x240. The movie is centered, with bars above and below.
    const float FIELD_HEIGHT = 240.0f;

    const unsigned int QUAD_VERTEX_COUNT = 6;

    // Position (3), colour (4) and texture coordinates (2).
    const unsigned int QUAD_FLOATS_PER_VERTEX = 9;

    const char* const RESOURCE_GROUP = "FIELDS";

    std::string MovieDirectory(const std::string& name){return "./data/video/" + name + "/";}

    std::string FramePath(const std::string& name, const int frame){
        char number[16];
        snprintf(number, sizeof(number), "%05d", frame);
        return MovieDirectory(name) + number + ".jpg";
    }
}

MoviePlayer::MoviePlayer(){
    scene_manager_ = Ogre::Root::getSingleton().getSceneManager("Scene");
    render_system_ = Ogre::Root::getSingletonPtr()->getRenderSystem();

    texture_ = Ogre::TextureManager::getSingleton().createManual(
      "MovieFrame", RESOURCE_GROUP, Ogre::TEX_TYPE_2D, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0,
      Ogre::PF_BYTE_RGBA, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE
    );
    material_ = Ogre::MaterialManager::getSingleton().create("MovieFrame", RESOURCE_GROUP);
    Ogre::Pass* pass = material_->getTechnique(0)->getPass(0);
    pass->setCullingMode(Ogre::CULL_NONE);
    pass->setDepthCheckEnabled(false);
    pass->setDepthWriteEnabled(false);
    pass->setLightingEnabled(false);
    Ogre::TextureUnitState* unit = pass->createTextureUnitState();
    unit->setTexture(texture_);
    unit->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
    unit->setTextureFiltering(Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_NONE);

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
    WriteQuad();

    scene_manager_->addRenderQueueListener(this);
}

MoviePlayer::~MoviePlayer(){
    scene_manager_->removeRenderQueueListener(this);
    delete render_op_.vertexData;
}

bool MoviePlayer::ReadInfo(
  int& frame_count, float& frames_per_second, int& camera_frame, int& release_frame
) const{
    TiXmlDocument document(MovieDirectory(name_) + "movie.xml");
    if (!document.LoadFile()) return false;
    const TiXmlElement* root = document.RootElement();
    if (root == nullptr) return false;
    double fps = 15;
    camera_frame = -1;
    root->QueryIntAttribute("frames", &frame_count);
    root->QueryIntAttribute("camera_frame", &camera_frame);
    // The frame at which the script that plays the movie goes on. By default, the end of the movie,
    // which is what looks right in the opening.
    release_frame = -1;
    root->QueryIntAttribute("release_frame", &release_frame);
    root->QueryDoubleAttribute("fps", &fps);
    frames_per_second = static_cast<float>(fps);
    return frame_count > 0;
}

void MoviePlayer::ScriptSet(const char* name, const float field_scale){
    name_ = name;
    field_scale_ = field_scale;
    // A movie that is playing is not interrupted by choosing the next one.
    if (!state_.IsPlaying()) state_.Prepare();
    LOG_TRIVIAL("Movie '" + name_ + "' chosen.");
}

void MoviePlayer::ReleaseWaiting(){
    for (const ScriptId& script : waiting_) ScriptManager::getSingleton().ContinueScriptExecution(script);
    waiting_.clear();
}

int MoviePlayer::ScriptPlay(){
    int frames = 0;
    float fps = 15;
    loaded_frame_ = -1;
    int camera_frame = -1;
    int release_frame = -1;
    if (name_.empty() || !ReadInfo(frames, fps, camera_frame, release_frame)){
        LOG_WARNING("Movie '" + name_ + "' is not installed, so it can't be played.");
        state_.Start(0, fps);
        return 1;
    }
    LOG_TRIVIAL("Movie '" + name_ + "' starts, " + std::to_string(frames) + " frames.");
    state_.Start(frames, fps);
    // The scenes of the fields that play over a movie are timed to start when the models can be
    // drawn with the camera of the movie, so the script that plays it goes on from there.
    release_frame_ = release_frame;
    camera_data_.clear();
    camera_valid_ = false;
    std::ifstream camera_file(MovieDirectory(name_) + "camera.cam", std::ios::binary);
    if (camera_file){
        camera_data_.assign(
          (std::istreambuf_iterator<char>(camera_file)), std::istreambuf_iterator<char>()
        );
    }
    AudioManager::getSingleton().MoviePlay(MovieDirectory(name_) + name_ + ".ogg");
    waiting_.push_back(ScriptManager::getSingleton().GetCurrentScriptId());
    return -1;
}

bool MoviePlayer::IsDrawnBehind() const{
    return state_.IsPlaying() && !camera_data_.empty();
}

void MoviePlayer::SetModelsHidden(const bool hidden){
    if (hidden == models_hidden_) return;
    models_hidden_ = hidden;
    CameraManager::getSingleton().getViewport()->setVisibilityMask(hidden ? 0 : 0xFFFFFFFF);
}

void MoviePlayer::UpdateCamera(){
    if (camera_data_.empty()) return;
    if (!state_.IsPlaying()){
        camera_valid_ = false;
        SetModelsHidden(false);
        if (camera_in_use_){
            camera_in_use_ = false;
            CameraManager::getSingleton().EndMovieCamera();
        }
        return;
    }
    const size_t offset = static_cast<size_t>(state_.GetFrame()) * MovieCameraFrame::SIZE;
    camera_valid_ = false;
    if (offset + MovieCameraFrame::SIZE <= camera_data_.size()){
        const MovieCameraFrame frame = MovieCameraFrame::Parse(
          camera_data_.data() + offset, field_scale_
        );
        camera_valid_ = frame.valid;
        if (frame.valid){
            camera_in_use_ = true;
            CameraManager::getSingleton().SetMovieCamera(frame.position, frame.orientation, frame.fov);
        }
    }
    SetModelsHidden(!camera_valid_);
}

void MoviePlayer::ScriptStop(){
    state_.Stop();
    UpdateCamera();
    AudioManager::getSingleton().MovieStop();
    ReleaseWaiting();
}

int MoviePlayer::ScriptGetFrame() const{return state_.GetFrame();}

bool MoviePlayer::ScriptIsPlaying() const{return state_.IsPlaying();}

void MoviePlayer::Update(const float delta){
    const bool ended = state_.Advance(delta);
    if (state_.IsPlaying() && release_frame_ >= 0 && state_.GetFrame() >= release_frame_){
        LOG_TRIVIAL("Movie '" + name_ + "' reached frame " + std::to_string(state_.GetFrame()) + ".");
        release_frame_ = -1;
        ReleaseWaiting();
    }
    UpdateCamera();
    if (ended){
        LOG_TRIVIAL("Movie '" + name_ + "' ended.");
        AudioManager::getSingleton().MovieStop();
        ReleaseWaiting();
    }
}

void MoviePlayer::LoadFrame(const int frame){
    loaded_frame_ = frame;
    std::ifstream file(FramePath(name_, frame), std::ios::binary);
    if (!file) return;
    std::vector<unsigned char> bytes(
      (std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()
    );
    Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(bytes.data(), bytes.size(), false));
    Ogre::Image image;
    try{
        image.load(stream, "jpg");
    }
    catch (const Ogre::Exception& e){
        LOG_WARNING("Frame " + std::to_string(frame) + " of the movie can't be read: " + e.what());
        return;
    }
    const Ogre::Box area(0, 0, image.getWidth(), image.getHeight());
    texture_->getBuffer()->blitFromMemory(image.getPixelBox(), area);
}

void MoviePlayer::WriteQuad(){
    const float half_height = FRAME_HEIGHT / FIELD_HEIGHT;
    const float u = static_cast<float>(FRAME_WIDTH) / TEXTURE_WIDTH;
    const float v = static_cast<float>(FRAME_HEIGHT) / TEXTURE_HEIGHT;
    const float corners[QUAD_VERTEX_COUNT][4] = {
      {-1, half_height, 0, 0}, {1, half_height, u, 0}, {1, -half_height, u, v},
      {-1, half_height, 0, 0}, {1, -half_height, u, v}, {-1, -half_height, 0, v}
    };
    float* write_iterator
      = static_cast<float*>(vertex_buffer_->lock(Ogre::HardwareBuffer::HBL_DISCARD));
    for (unsigned int i = 0; i < QUAD_VERTEX_COUNT; ++ i){
        *write_iterator ++ = corners[i][0];
        *write_iterator ++ = corners[i][1];
        *write_iterator ++ = 0;
        *write_iterator ++ = 1;
        *write_iterator ++ = 1;
        *write_iterator ++ = 1;
        *write_iterator ++ = 1;
        *write_iterator ++ = corners[i][2];
        *write_iterator ++ = corners[i][3];
    }
    vertex_buffer_->unlock();
}

void MoviePlayer::DrawQuad(){
    const int frame = state_.GetFrame();
    if (frame != loaded_frame_) LoadFrame(frame);

    // Same setup the background and the fades use: the quad is given in screen coordinates.
    Ogre::GpuProgramParametersPtr rs_params = render_system_->getFixedFunctionParams(
      Ogre::TVC_NONE, Ogre::FOG_NONE
    );
    rs_params->setConstant(Ogre::GpuProgramParameters::ACT_WORLD_MATRIX, Ogre::Matrix4::IDENTITY);
    rs_params->setConstant(
      Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX, Ogre::Matrix4::IDENTITY
    );
    render_system_->_setViewMatrix(Ogre::Matrix4::IDENTITY);
    rs_params->setConstant(Ogre::GpuProgramParameters::ACT_VIEW_MATRIX, Ogre::Matrix4::IDENTITY);
    render_system_->applyFixedFunctionParams(rs_params, Ogre::GPV_GLOBAL);
    scene_manager_->_setPass(material_->getTechnique(0)->getPass(0), true, false);
    render_system_->_render(render_op_);
}

void MoviePlayer::renderQueueStarted(
  Ogre::uint8 queue_group_id, const Ogre::String& invocation, bool& skip_this_invocation
){
    if (!state_.IsPlaying()) return;
    if (!EntityManager::getSingleton().IsFieldModule()) return;
    if (!camera_data_.empty()){
        // Behind the models, which come in the main queue. They are only drawn if the movie has a
        // camera in the frame, or they would be in the wrong place. They are hidden with the
        // visibility mask of the viewport: skipping the queue would also skip the listeners that
        // are called when it ends, and the debug lines would pile up.
        if (queue_group_id != Ogre::RENDER_QUEUE_MAIN) return;
        DrawQuad();
        // The queue sets its own matrices, but not before it starts.
        Ogre::Camera* camera = CameraManager::getSingleton().GetCurrentCamera();
        render_system_->_setProjectionMatrix(camera->getProjectionMatrixRS());
        render_system_->_setViewMatrix(camera->getViewMatrix(true));
        return;
    }
    if (queue_group_id != Ogre::RENDER_QUEUE_OVERLAY) return;
    DrawQuad();
}
