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
#include <OgreLogManager.h>
#include <OgreMaterialManager.h>
#include <OgreException.h>
#include <OgreViewport.h>
#include <OgreTechnique.h>
#include "core/Background2D.h"
#include "core/CameraManager.h"
#include "core/ConfigVar.h"
#include "core/DebugDraw.h"
#include "core/Logger.h"
#include "core/Timer.h"

ConfigVar cv_debug_background2d("debug_background2d", "Draw background debug info", "false");
ConfigVar cv_show_background2d("show_background2d", "Draw background", "true");
ConfigVar cv_background2d_manual("background2d_manual", "Manual 2d background scrolling", "false");

Background2D::Background2D():
  alpha_max_vertex_count_(0),
  add_max_vertex_count_(0),
  subtract_max_vertex_count_(0),
  scroll_entity_(nullptr),
  scroll_position_start_(Ogre::Vector2::ZERO),
  scroll_position_end_(Ogre::Vector2::ZERO),
  scroll_type_(Background2D::NONE),
  scroll_seconds_(0),
  scroll_current_seconds_(0),
  position_(Ogre::Vector2::ZERO),
  position_real_(Ogre::Vector2::ZERO),
  range_(Ogre::AxisAlignedBox::BOX_INFINITE),
  virtual_screen_size_(320, 240) // FFVII original resolution
{
    scene_manager_ = Ogre::Root::getSingleton().getSceneManager("Scene");
    render_system_ = Ogre::Root::getSingletonPtr()->getRenderSystem();
    CreateVertexBuffers();
    alpha_material_ = Ogre::MaterialManager::getSingleton().create("Background2DAlpha", "General");
    Ogre::Pass* pass = alpha_material_->getTechnique(0)->getPass(0);
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
    add_material = Ogre::MaterialManager::getSingleton().create("Background2DAdd", "General");
    pass = add_material->getTechnique(0)->getPass(0);
    pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
    pass->setCullingMode(Ogre::CULL_NONE);
    pass->setDepthCheckEnabled(true);
    pass->setDepthWriteEnabled(true);
    pass->setLightingEnabled(false);
    pass->setSceneBlending(Ogre::SBT_ADD);
    pass->setAlphaRejectFunction(Ogre::CMPF_GREATER);
    pass->setAlphaRejectValue(0);
    tex = pass->createTextureUnitState();
    tex->setTextureName("system/blank.png");
    tex->setNumMipmaps(-1);
    tex->setTextureFiltering(Ogre::TFO_NONE);
    subtract_material_ = Ogre::MaterialManager::getSingleton().create(
      "Background2DSubtract", "General"
    );
    pass = subtract_material_->getTechnique(0)->getPass(0);
    pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
    pass->setCullingMode(Ogre::CULL_NONE);
    pass->setDepthCheckEnabled(true);
    pass->setDepthWriteEnabled(true);
    pass->setLightingEnabled(false);
    pass->setSceneBlending(Ogre::SBT_ADD);
    pass->setSceneBlendingOperation(Ogre::SBO_SUBTRACT);
    pass->setAlphaRejectFunction(Ogre::CMPF_LESS);
    pass->setAlphaRejectValue(0);
    tex = pass->createTextureUnitState();
    tex->setTextureName("system/blank.png");
    tex->setNumMipmaps(-1);
    tex->setTextureFiltering(Ogre::TFO_NONE);
    scene_manager_->addRenderQueueListener(this);
}

Background2D::~Background2D(){
    scene_manager_->removeRenderQueueListener(this);
    for (unsigned int i = 0; i < animations_.size(); ++ i) delete animations_[i];
    DestroyVertexBuffers();
}

void Background2D::InputDebug(const VGears::Event& event){
    if (cv_background2d_manual.GetB() == true && event.type == VGears::ET_KEY_IMPULSE){
        if (event.param1 == OIS::KC_W)
            position_real_.y += 2;
        else if (event.param1 == OIS::KC_A)
            position_real_.x += 2;
        else if (event.param1 == OIS::KC_S)
            position_real_.y -= 2;
        else if (event.param1 == OIS::KC_D)
            position_real_.x -= 2;
        CameraManager::getSingleton().Set2DScroll(position_real_);
    }
}

void Background2D::Update(){
    for (unsigned int i = 0; i < animation_played_.size(); ++ i){
        for (unsigned int j = 0; j < animations_.size(); ++ j){
            if (animations_[j]->GetName() == animation_played_[i].name){
                float delta_time = Timer::getSingleton().GetGameTimeDelta();
                float time = animations_[j]->GetTime();
                float end_time = animations_[j]->GetLength();

                // If animation ended
                if (time + delta_time >= end_time){
                    // Set to last frame of animation.
                    if (time != end_time) animations_[j]->SetTime(end_time);

                    if (animation_played_[i].state == Background2DAnimation::ONCE){
                        for (unsigned int k = 0; k < animation_played_[i].sync.size(); ++k){
                            ScriptManager::getSingleton().ContinueScriptExecution(
                              animation_played_[i].sync[k]
                            );
                        }
                        animation_played_[i].sync.clear();
                        // Mark to delete this way:
                        animation_played_[i].name = "";
                    }
                    // In case of looped, sync with end:
                    else animations_[j]->SetTime(time + delta_time - end_time);
                }
                else animations_[j]->AddTime(delta_time);
            }
        }
    }

    // Remove stopped animations.
    std::vector<AnimationPlayed>::iterator i = animation_played_.begin();
    for(; i != animation_played_.end();){
        if ((*i).name == "") i = animation_played_.erase(i);
        else ++ i;
    }
}

void Background2D::UpdateDebug(){
    // TODO: is this necessary? does it cost to apply camera 2d Scroll?
    // if so maybe move this check to applyScroll
    if (position_real_ != GetScreenScroll()) applyScroll();
    if (cv_debug_background2d.GetB() == true){
        DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.LEFT);
        DEBUG_DRAW.SetScreenSpace(true);
        DEBUG_DRAW.SetColour(Ogre::ColourValue(0.0f, 0.8f, 0.8f, 1.0f));
        for (unsigned int i = 0; i < animation_played_.size(); ++ i){
            DEBUG_DRAW.Text(
              150.0f, static_cast<float>(34 + i * 12),
              "Background 2D animation: " + animation_played_[i].name
            );
        }
    }
}

void Background2D::calculateScreenScale(){
    Ogre::Viewport *viewport(CameraManager::getSingleton().getViewport());
    Ogre::Real scale_width = static_cast<Ogre::Real>(viewport->getActualWidth());
    Ogre::Real scale_height = static_cast<Ogre::Real>(viewport->getActualHeight());
    scale_width /= virtual_screen_size_.x;
    scale_height /= virtual_screen_size_.y;
    screen_scale_ = scale_height;
    screen_proportion_.x = screen_scale_ / scale_width;
    screen_proportion_.y = screen_scale_ / scale_height;
}

void Background2D::OnResize(){
    calculateScreenScale();
    for (unsigned int i = 0; i < tiles_.size(); ++ i){
        Tile &tile(tiles_[i]);
        Ogre::Vector2 top_left(static_cast<Ogre::Real>(tile.x), static_cast<Ogre::Real>(- tile.y));
        Ogre::Vector2 top_right(
          static_cast<Ogre::Real>(tile.x + tile.width), static_cast<Ogre::Real>(top_left.y)
        );
        Ogre::Vector2 bottom_right(
          static_cast<Ogre::Real>(top_right.x), static_cast<Ogre::Real>(-(tile.y + tile.height))
        );
        Ogre::Vector2 bottom_left(
          static_cast<Ogre::Real>(top_left.x), static_cast<Ogre::Real>(bottom_right.y)
        );
        virtualScreenToWorldSpace(top_left);
        virtualScreenToWorldSpace(top_right);
        virtualScreenToWorldSpace(bottom_right);
        virtualScreenToWorldSpace(bottom_left);
        float new_x1 = top_left.x;
        float new_y1 = top_left.y;
        float new_x2 = top_right.x;
        float new_y2 = top_right.y;
        float new_x3 = bottom_right.x;
        float new_y3 = bottom_right.y;
        float new_x4 = bottom_left.x;
        float new_y4 = bottom_left.y;
        Ogre::HardwareVertexBufferSharedPtr vertex_buffer;
        if (tiles_[i].blending == VGears::B_ALPHA)
            vertex_buffer = alpha_vertex_buffer_;
        else if(tiles_[i].blending == VGears::B_ADD)
            vertex_buffer = add_vertex_buffer_;
        else if(tiles_[i].blending == VGears::B_SUBTRACT)
            vertex_buffer = subtract_vertex_buffer_;
        float* write_iterator = (float*) vertex_buffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
        write_iterator += tiles_[i].start_vertex_index * TILE_VERTEX_INDEX_SIZE;
        *write_iterator ++ = new_x1;
        *write_iterator ++ = new_y1;
        write_iterator += 7;
        *write_iterator ++ = new_x2;
        *write_iterator ++ = new_y2;
        write_iterator += 7;
        *write_iterator ++ = new_x3;
        *write_iterator ++ = new_y3;
        write_iterator += 7;
        *write_iterator++ = new_x1;
        *write_iterator++ = new_y1;
        write_iterator += 7;
        *write_iterator ++ = new_x3;
        *write_iterator ++ = new_y3;
        write_iterator += 7;
        *write_iterator ++ = new_x4;
        *write_iterator ++ = new_y4;
        vertex_buffer->unlock();
    }
    applyScroll();
}

void Background2D::Clear(){
    scroll_entity_ = nullptr;
    scroll_position_start_ = Ogre::Vector2::ZERO;
    scroll_position_end_ = Ogre::Vector2::ZERO;
    scroll_seconds_ = 0;
    scroll_current_seconds_ = 0;
    position_ = Ogre::Vector2::ZERO;
    position_real_ = Ogre::Vector2::ZERO;
    range_ = Ogre::AxisAlignedBox::BOX_INFINITE;
    UnsetScroll();
    for(unsigned int i = 0; i < animations_.size(); ++i) delete animations_[i];
    animations_.clear();
    for(unsigned int i = 0; i < animation_played_.size(); ++ i)
        for(unsigned int j = 0; j < animation_played_[i].sync.size(); ++j)
            ScriptManager::getSingleton().ContinueScriptExecution(animation_played_[i].sync[j]);
    animation_played_.clear();
    tiles_.clear();
    DestroyVertexBuffers();
    CreateVertexBuffers();
}

void Background2D::ScriptAutoScrollToEntity(Entity* entity){scroll_entity_ = entity;}

Entity* Background2D::GetAutoScrollEntity() const{return scroll_entity_;}

void Background2D::ScriptScrollToPosition(
  const float x, const float y, const SCROLL_TYPE type, const float seconds
){
    LOG_TRIVIAL(
      "[SCRIPT] Background2d set scroll to position \""
      + Ogre::StringConverter::toString(Ogre::Vector2(x, y)) + "\"."
    );
    Ogre::Vector2 position = Ogre::Vector2(x, y);
    scroll_entity_ = nullptr;
    if(type == Background2D::NONE){
        SetScroll(position);
        return;
    }
    scroll_position_start_ = position_;
    scroll_position_end_ = position;
    scroll_type_ = type;
    scroll_seconds_ = seconds;
    scroll_current_seconds_ = 0;
}

int Background2D::ScriptScrollSync(){
    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "[SCRIPT] Wait Background2d scroll for function \""
      + script.function + "\" in script entity \"" + script.entity + "\"."
    );
    scroll_sync_.push_back(script);
    return -1;
}

void Background2D::UnsetScroll(){
    scroll_type_ = Background2D::NONE;
    for(unsigned int i = 0; i < scroll_sync_.size(); ++ i)
        ScriptManager::getSingleton().ContinueScriptExecution(scroll_sync_[i]);
    scroll_sync_.clear();
}

const Ogre::Vector2& Background2D::GetScrollPositionStart() const{return scroll_position_start_;}

const Ogre::Vector2& Background2D::GetScrollPositionEnd() const{return scroll_position_end_;}

Background2D::SCROLL_TYPE Background2D::GetScrollType() const{return scroll_type_;}

float Background2D::GetScrollSeconds() const{return scroll_seconds_;}

void Background2D::SetScrollCurrentSeconds(const float seconds){scroll_current_seconds_ = seconds;}

float Background2D::GetScrollCurrentSeconds() const{return scroll_current_seconds_;}

void Background2D::SetScreenScroll(const Ogre::Vector2& position){
    SetScroll(position / screen_scale_);
}

void Background2D::SetScroll(const Ogre::Vector2& position){
    position_ = position;
    Ogre::Vector3 pos_3d(position_.x, position_.y, 0);
    if (!range_.contains(pos_3d)){
        Ogre::Vector3 max_3d(range_.getMaximum());
        Ogre::Vector3 min_3d(range_.getMinimum());
        Ogre::Vector2 max(max_3d.x, max_3d.y);
        Ogre::Vector2 min(min_3d.x, min_3d.y);
        position_.makeCeil(min);
        position_.makeFloor(max);
    }
    applyScroll();
}

void Background2D::applyScroll(){
    if(cv_background2d_manual.GetB() != true){
        position_real_ = GetScreenScroll();
        CameraManager::getSingleton().Set2DScroll(position_real_);
    }
}

const Ogre::Vector2& Background2D::GetScroll() const{return position_;}

const Ogre::Vector2 Background2D::GetScreenScroll() const{return GetScroll() * screen_scale_;}

void Background2D::SetImage(const Ogre::String& image){
    Ogre::LogManager::getSingleton().stream() << "Background2D::SetImage " << image;
    Ogre::Pass* pass = alpha_material_->getTechnique(0)->getPass(0);
    Ogre::TextureUnitState* tex = pass->getTextureUnitState(0);
    tex->setTextureName(image);
    pass = add_material->getTechnique(0)->getPass(0);
    tex = pass->getTextureUnitState(0);
    tex->setTextureName(image);
    pass = subtract_material_->getTechnique(0)->getPass(0);
    tex = pass->getTextureUnitState(0);
    tex->setTextureName(image);
}

void Background2D::SetRange(const Ogre::Vector4& range){
    SetRange((int)range.x, (int)range.y, (int)range.z, (int)range.w);
}

void Background2D::SetRange(const int min_x, const int min_y, const int max_x, const int max_y){
    Ogre::LogManager::getSingleton().stream()
      << "Background2D::SetRange " << min_x << " " << min_y << " " << max_x << " " << max_y;
    Ogre::Vector2 half_virtual_screen_size(virtual_screen_size_ / 2);
    half_virtual_screen_size /= screen_proportion_;
    range_.setMaximum(max_x - half_virtual_screen_size.x , max_y - half_virtual_screen_size.y, 1);
    range_.setMinimum(min_x + half_virtual_screen_size.x , min_y + half_virtual_screen_size.y, 0);
    Ogre::LogManager::getSingleton().stream() << "Background2D::SetRange " << range_;
    calculateScreenScale();
}

void Background2D::AddTile(const VGears::Tile& tile){
    // TODO: move depth calculation to flevelBackgroundLoader maybe? and let
    // Backgorund2D only handle 0 <= depth <= 1 or so?
    // Maybe just move the < 4095 part to flevel background loader?
    Ogre::Real depth(0.0001f);
    if (tile.depth >= 1){
        if (tile.depth < 4095){
            const Ogre::Matrix4 &cam_projection(
              CameraManager::getSingleton().GetCurrentCamera()->getProjectionMatrixWithRSDepth()
            );
            Ogre::Vector4 res(0, 0, -tile.depth, 1);
            res = cam_projection * res;
            res /= res.w;
            depth = res.z;
        }
        else depth = 0.9999f;
    }
    AddTile(tile.destination, tile.width, tile.height, depth, tile.uv, tile.blending);
}

void Background2D::AddTile(
  const Ogre::Vector2& destination, const int width, const int height,
  const float depth, const Ogre::Vector4& uv, const Blending blending
){
    AddTile(
      static_cast<int>(destination.x), static_cast<int>(destination.y),
      width, height, depth, uv.x, uv.y, uv.z, uv.w, blending
    );
}

void Background2D::virtualScreenToWorldSpace(Ogre::Vector2& pos) const{
    pos.x /= virtual_screen_size_.x / 2;
    pos.y /= virtual_screen_size_.y / 2;
    pos.x *= screen_proportion_.x;
    pos.y *= screen_proportion_.y;
}

void Background2D::AddTile(
  const int x, const int y, const int width, const int height, const float depth, const float u1,
  const float v1, const float u2, const float v2, const Blending blending
){
    Ogre::RenderOperation render_op;
    Ogre::HardwareVertexBufferSharedPtr vertex_buffer;
    unsigned int max_vertex_count;
    if (blending == VGears::B_ALPHA){
        render_op = alpha_render_op_;
        vertex_buffer = alpha_vertex_buffer_;
        max_vertex_count = alpha_max_vertex_count_;
    }
    else if (blending == VGears::B_ADD){
        render_op = add_render_op_;
        vertex_buffer = add_vertex_buffer_;
        max_vertex_count = add_max_vertex_count_;
    }
    else if (blending == VGears::B_SUBTRACT){
        render_op = subtract_render_op_;
        vertex_buffer = subtract_vertex_buffer_;
        max_vertex_count = subtract_max_vertex_count_;
    }
    else{
        LOG_ERROR("Unknown blending type.");
        return;
    }
    if (render_op.vertexData->vertexCount + TILE_VERTEX_COUNT > max_vertex_count){
        LOG_ERROR(
          "Max number of tiles reached. Can't create more than "
          + Ogre::StringConverter::toString(max_vertex_count / TILE_VERTEX_COUNT) + " tiles."
        );
        return;
    }
    Tile tile;
    tile.x = x;
    tile.y = y;
    tile.width = width;
    tile.height = height;
    tile.start_vertex_index = render_op.vertexData->vertexCount;
    tile.blending = blending;
    size_t index(tiles_.size());
    tiles_.push_back(tile);
    Ogre::Vector2 top_left(static_cast<Ogre::Real>(x), static_cast<Ogre::Real>(-y));
    Ogre::Vector2 top_right(
      static_cast<Ogre::Real>(x + width), static_cast<Ogre::Real>(top_left.y)
    );
    Ogre::Vector2 bottom_right(
      static_cast<Ogre::Real>(top_right.x),
      static_cast<Ogre::Real>(-(y + height))
    );
    Ogre::Vector2 bottom_left(top_left.x, bottom_right.y);
    virtualScreenToWorldSpace(top_left);
    virtualScreenToWorldSpace(top_right);
    virtualScreenToWorldSpace(bottom_right);
    virtualScreenToWorldSpace(bottom_left);
    float new_x1 = top_left.x;
    float new_y1 = top_left.y;
    float new_x2 = top_right.x;
    float new_y2 = top_right.y;
    float new_x3 = bottom_right.x;
    float new_y3 = bottom_right.y;
    float new_x4 = bottom_left.x;
    float new_y4 = bottom_left.y;
    float* write_iterator = (float*) vertex_buffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
    write_iterator += render_op.vertexData->vertexCount * TILE_VERTEX_INDEX_SIZE;

    // TODO: Can use WriteGlyph
    *write_iterator ++ = new_x1;
    *write_iterator ++ = new_y1;
    *write_iterator ++ = depth;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = u1;
    *write_iterator ++ = v1;
    *write_iterator ++ = new_x2;
    *write_iterator ++ = new_y2;
    *write_iterator ++ = depth;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = u2;
    *write_iterator ++ = v1;
    *write_iterator ++ = new_x3;
    *write_iterator ++ = new_y3;
    *write_iterator ++ = depth;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = u2;
    *write_iterator ++ = v2;
    *write_iterator ++ = new_x1;
    *write_iterator ++ = new_y1;
    *write_iterator ++ = depth;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = u1;
    *write_iterator ++ = v1;
    *write_iterator ++ = new_x3;
    *write_iterator ++ = new_y3;
    *write_iterator ++ = depth;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = u2;
    *write_iterator ++ = v2;
    *write_iterator ++ = new_x4;
    *write_iterator ++ = new_y4;
    *write_iterator ++ = depth;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = 1;
    *write_iterator ++ = u1;
    *write_iterator ++ = v2;
    render_op.vertexData->vertexCount += TILE_VERTEX_COUNT;
    vertex_buffer->unlock();
}

void Background2D::UpdateTileUV(
  const unsigned int tile_id, const float u1, const float v1, const float u2, const float v2
){
    if (tile_id >= tiles_.size()){
        LOG_ERROR("Tile with id " + Ogre::StringConverter::toString( tile_id ) + " doesn't exist.");
        return;
    }
    Ogre::HardwareVertexBufferSharedPtr vertex_buffer;
    if (tiles_[tile_id].blending == VGears::B_ALPHA)
        vertex_buffer = alpha_vertex_buffer_;
    else if (tiles_[tile_id].blending == VGears::B_ADD)
        vertex_buffer = add_vertex_buffer_;
    else if(tiles_[tile_id].blending == VGears::B_SUBTRACT)
        vertex_buffer = subtract_vertex_buffer_;
    float* write_iterator = (float*) vertex_buffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
    write_iterator += tiles_[tile_id].start_vertex_index * TILE_VERTEX_INDEX_SIZE;
    write_iterator += 7;
    *write_iterator ++ = u1;
    *write_iterator ++ = v1;
    write_iterator += 7;
    *write_iterator ++ = u2;
    *write_iterator ++ = v1;
    write_iterator += 7;
    *write_iterator ++ = u2;
    *write_iterator ++ = v2;
    write_iterator += 7;
    *write_iterator ++ = u1;
    *write_iterator ++ = v1;
    write_iterator += 7;
    *write_iterator ++ = u2;
    *write_iterator ++ = v2;
    write_iterator += 7;
    *write_iterator ++ = u1;
    *write_iterator ++ = v2;
    vertex_buffer->unlock();
}

void Background2D::AddAnimation(Background2DAnimation* animation){animations_.push_back(animation);}

void Background2D::PlayAnimation(
  const Ogre::String& animation, const Background2DAnimation::State state
){
    bool found = false;
    for (unsigned int i = 0; i < animations_.size(); ++ i){
        if (animations_[i]->GetName() == animation){
            found = true;
            animations_[i]->SetTime(0);
            animations_[i]->AddTime(0);
        }
    }
    for (unsigned int i = 0; i < animation_played_.size(); ++ i){
        if (animation_played_[i].name == animation)
            animation_played_.erase(animation_played_.begin() + i);
    }
    if(found == true){
        AnimationPlayed anim;
        anim.name = animation;
        anim.state = state;
        animation_played_.push_back(anim);
    }
    else
        LOG_ERROR("Background2D doesn't has animation \"" + animation + "\".");
}

void Background2D::ScriptPlayAnimationLooped(const char* name){
    PlayAnimation(Ogre::String(name), Background2DAnimation::LOOPED);
}


void Background2D::ScriptPlayAnimationOnce(const char* name){
    PlayAnimation(Ogre::String(name), Background2DAnimation::ONCE);
}

int Background2D::ScriptAnimationSync(const char* animation){
    for (unsigned int i = 0; i < animation_played_.size(); ++ i){
        if (animation_played_[i].name == animation){
            ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
            animation_played_[i].sync.push_back(script);
            return -1;
        }
    }
    return 1;
}

void Background2D::renderQueueEnded(
  Ogre::uint8 queue_group_id, const Ogre::String& invocation, bool& repeat_this_invocation
){
    if (cv_show_background2d.GetB() == false) return;
    if (queue_group_id == Ogre::RENDER_QUEUE_MAIN){
        Ogre::GpuProgramParametersPtr rs_params = render_system_->getFixedFunctionParams(
          Ogre::TVC_NONE, Ogre::FOG_NONE
        );
        rs_params->setConstant(
          Ogre::GpuProgramParameters::ACT_WORLD_MATRIX, Ogre::Matrix4::IDENTITY
        );
        rs_params->setConstant(
          Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX, Ogre::Matrix4::IDENTITY
        );
        Ogre::Viewport *viewport(CameraManager::getSingleton().getViewport());
        float width = static_cast<float>(viewport->getActualWidth());
        float height = static_cast<float>(viewport->getActualHeight());
        Ogre::Matrix4 view;
        view.makeTrans(Ogre::Vector3(position_real_.x / width, -position_real_.y / height, 0));
        // TODO This is deprecated, but if not done , the background image disappears.
        render_system_->_setViewMatrix(view);
        rs_params->setConstant(Ogre::GpuProgramParameters::ACT_VIEW_MATRIX, view);
        render_system_->applyFixedFunctionParams(rs_params, Ogre::GPV_GLOBAL);
        if (alpha_render_op_.vertexData->vertexCount != 0){
            scene_manager_->_setPass(alpha_material_->getTechnique(0)->getPass(0), true, false);
            render_system_->_render(alpha_render_op_);
        }
        if (add_render_op_.vertexData->vertexCount != 0){
            scene_manager_->_setPass(add_material->getTechnique(0)->getPass(0), true, false);
            render_system_->_render(add_render_op_);
        }
        if(subtract_render_op_.vertexData->vertexCount != 0){
            scene_manager_->_setPass(subtract_material_->getTechnique(0)->getPass(0), true, false);
            render_system_->_render(subtract_render_op_);
        }

    }
}

void Background2D::CreateVertexBuffers(){
    alpha_max_vertex_count_ = 2048 * TILE_VERTEX_COUNT;
    alpha_render_op_.vertexData = new Ogre::VertexData;
    alpha_render_op_.vertexData->vertexStart = 0;
    Ogre::VertexDeclaration* vertex_declaration
      = alpha_render_op_.vertexData->vertexDeclaration;
    size_t offset = 0;
    vertex_declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
    alpha_vertex_buffer_ = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
      vertex_declaration->getVertexSize(0), alpha_max_vertex_count_,
      Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
    );
    alpha_render_op_.vertexData->vertexBufferBinding->setBinding(0, alpha_vertex_buffer_);
    alpha_render_op_.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    alpha_render_op_.useIndexes = false;
    add_max_vertex_count_ = 1024 * TILE_VERTEX_COUNT;
    add_render_op_.vertexData = new Ogre::VertexData;
    add_render_op_.vertexData->vertexStart = 0;
    vertex_declaration = add_render_op_.vertexData->vertexDeclaration;
    offset = 0;
    vertex_declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
    add_vertex_buffer_ = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
      vertex_declaration->getVertexSize(0), add_max_vertex_count_,
      Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
    );
    add_render_op_.vertexData->vertexBufferBinding->setBinding(0, add_vertex_buffer_);
    add_render_op_.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    add_render_op_.useIndexes = false;
    subtract_max_vertex_count_ = 512 * TILE_VERTEX_COUNT;
    subtract_render_op_.vertexData = new Ogre::VertexData;
    subtract_render_op_.vertexData->vertexStart = 0;
    vertex_declaration = subtract_render_op_.vertexData->vertexDeclaration;
    offset = 0;
    vertex_declaration->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES
    );
    subtract_vertex_buffer_ = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
      vertex_declaration->getVertexSize(0), subtract_max_vertex_count_,
      Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
    );
    subtract_render_op_.vertexData->vertexBufferBinding->setBinding(0, subtract_vertex_buffer_);
    subtract_render_op_.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    subtract_render_op_.useIndexes = false;
}

void Background2D::DestroyVertexBuffers(){
    delete alpha_render_op_.vertexData;
    alpha_render_op_.vertexData = 0;
    alpha_vertex_buffer_.reset();
    alpha_max_vertex_count_ = 0;
    delete add_render_op_.vertexData;
    add_render_op_.vertexData = 0;
    add_vertex_buffer_.reset();
    add_max_vertex_count_ = 0;
    delete subtract_render_op_.vertexData;
    subtract_render_op_.vertexData = 0;
    subtract_vertex_buffer_.reset();
    subtract_max_vertex_count_ = 0;
}

void Background2D::load(const VGears::Background2DFilePtr& background){
    assert(background != nullptr);
    background->load();
    SetImage(background->GetTextureName());
    virtual_screen_size_ = background->GetClip();
    SetRange(background->GetRange());
    const Ogre::Vector3& position(background->GetPosition());
    const Ogre::Quaternion& orientation(background->GetOrientation());
    const Ogre::Radian& fov(background->GetFov());
    CameraManager::getSingleton().Set2DCamera(position, orientation, fov);
    load(background->GetTiles());
}

void Background2D::load(const VGears::Background2DFile::TileList& tiles){
    VGears::Background2DFile::TileList::const_iterator it(tiles.begin());
    VGears::Background2DFile::TileList::const_iterator it_end(tiles.end());
    size_t tile_index(0);
    while (it != it_end) {
        AddTile(*it);
        load(tile_index++, it->animations);
        ++ it;
    }
}

void Background2D::load(const size_t tile_index, const VGears::AnimationMap& animations){
    VGears::AnimationMap::const_iterator it(animations.begin());
    VGears::AnimationMap::const_iterator it_end(animations.end());
    while(it != it_end){
        const VGears::String& name(it->first);
        const VGears::Animation& animation(it->second);
        Background2DAnimation* anim(new Background2DAnimation(name, this, tile_index));
        anim->SetLength(animation.length);
        VGears::KeyFrameList::const_iterator itk(animation.key_frames.begin());
        VGears::KeyFrameList::const_iterator itk_end(animation.key_frames.end());
        while(itk != itk_end) anim->AddUVKeyFrame(*(itk++));
        AddAnimation(anim);
        ++ it;
    }
}
