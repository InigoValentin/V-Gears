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
#include <algorithm>
#include "core/Background2D.h"
#include "core/CameraManager.h"
#include "core/EntityManager.h"
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
  scroll_target_entity_(nullptr),
  scroll_follow_target_(false),
  position_(Ogre::Vector2::ZERO),
  position_real_(Ogre::Vector2::ZERO),
  offset_(Ogre::Vector2::ZERO),
  range_(Ogre::AxisAlignedBox::BOX_INFINITE),
  virtual_screen_size_(320, 240) // FFVII original resolution
{
    scene_manager_ = Ogre::Root::getSingleton().getSceneManager("Scene");
    render_system_ = Ogre::Root::getSingletonPtr()->getRenderSystem();
    CreateVertexBuffers();
    alpha_material_ = Ogre::MaterialManager::getSingleton().create("Background2DAlpha", "FIELDS");
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
    add_material = Ogre::MaterialManager::getSingleton().create("Background2DAdd", "FIELDS");
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
      "Background2DSubtract", "FIELDS"
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
        if (event.param1 == OIS::KC_W) position_real_.y += 2;
        else if (event.param1 == OIS::KC_A) position_real_.x += 2;
        else if (event.param1 == OIS::KC_S) position_real_.y -= 2;
        else if (event.param1 == OIS::KC_D) position_real_.x -= 2;
        CameraManager::getSingleton().Set2DScroll(position_real_);
    }
}

void Background2D::Hide(){scene_manager_->removeRenderQueueListener(this);}

void Background2D::Show(){scene_manager_->addRenderQueueListener(this);}

void Background2D::Update(){
    if (has_pending_scroll_){
        has_pending_scroll_ = false;
        SetScroll(pending_scroll_);
    }
    UpdateShake(Timer::getSingleton().GetGameTimeDelta());
    // The layers that scroll by themselves.
    for (int layer = 0; layer < LAYER_COUNT; ++ layer){
        layer_scroll_offset_[layer][0] += layer_scroll_speed_[layer][0] * Timer::getSingleton().GetGameTimeDelta();
        layer_scroll_offset_[layer][1] += layer_scroll_speed_[layer][1] * Timer::getSingleton().GetGameTimeDelta();
    }
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
    for (unsigned int i = 0; i < tiles_.size(); ++ i) WriteTilePosition(tiles_[i]);
    applyScroll();
}

void Background2D::WriteTilePosition(const Tile& tile){
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
    // A tile that is not drawn has all its corners in the same point.
    if (!tile.visible){
        top_right = top_left;
        bottom_right = top_left;
        bottom_left = top_left;
    }
    Ogre::HardwareVertexBufferSharedPtr vertex_buffer;
    if (tile.blending == VGears::B_ALPHA) vertex_buffer = alpha_vertex_buffer_;
    else if (tile.blending == VGears::B_ADD) vertex_buffer = add_vertex_buffer_;
    else if (tile.blending == VGears::B_SUBTRACT) vertex_buffer = subtract_vertex_buffer_;
    else return;
    float* write_iterator
      = static_cast<float*>(vertex_buffer->lock(Ogre::HardwareBuffer::HBL_NORMAL));
    write_iterator += tile.start_vertex_index * TILE_VERTEX_INDEX_SIZE;
    *write_iterator ++ = top_left.x;
    *write_iterator ++ = top_left.y;
    write_iterator += 7;
    *write_iterator ++ = top_right.x;
    *write_iterator ++ = top_right.y;
    write_iterator += 7;
    *write_iterator ++ = bottom_right.x;
    *write_iterator ++ = bottom_right.y;
    write_iterator += 7;
    *write_iterator ++ = top_left.x;
    *write_iterator ++ = top_left.y;
    write_iterator += 7;
    *write_iterator ++ = bottom_right.x;
    *write_iterator ++ = bottom_right.y;
    write_iterator += 7;
    *write_iterator ++ = bottom_left.x;
    *write_iterator ++ = bottom_left.y;
    vertex_buffer->unlock();
}

bool Background2D::IsAreaStateOn(const int area, const int layers) const{
    const auto it = area_states_on_.find(area);
    return it != area_states_on_.end() && (it->second & static_cast<unsigned int>(layers)) != 0;
}

void Background2D::RefreshArea(const int area){
    for (Tile& tile : tiles_){
        if (tile.area != area) continue;
        const bool visible = IsAreaStateOn(tile.area, tile.state);
        if (visible == tile.visible) continue;
        tile.visible = visible;
        WriteTilePosition(tile);
    }
}

void Background2D::ScriptAreaOn(const int area, const int layer){
    LOG_TRIVIAL(
      "Background2d area " + std::to_string(area) + " layer " + std::to_string(layer) + " on."
    );
    if (layer < 0 || layer > 31){
        LOG_WARNING("Background2d layer " + std::to_string(layer) + " is out of range.");
        return;
    }
    area_states_on_[area] |= 1u << layer;
    RefreshArea(area);
}

void Background2D::ScriptAreaOff(const int area, const int layer){
    LOG_TRIVIAL(
      "Background2d area " + std::to_string(area) + " layer " + std::to_string(layer) + " off."
    );
    if (layer < 0 || layer > 31){
        LOG_WARNING("Background2d layer " + std::to_string(layer) + " is out of range.");
        return;
    }
    const auto it = area_states_on_.find(area);
    if (it != area_states_on_.end()) it->second &= ~(1u << layer);
    RefreshArea(area);
}

namespace{
    // Random factors the original game applies to each bounce of a shake.
    const float SHAKE_FACTORS[] = {
      -1.0f, 1.14f, -1.34f, 0.61f, -0.49f, 0.64f, -0.25f, 0.47f, -1.20f, -0.01f, 1.14f, -0.86f,
      0.81f, -0.58f, 0.68f, -1.31f, 0.44f, -0.91f, 0.03f, -0.97f, 1.25f, -0.76f, 0.48f, -0.99f,
      1.34f, -0.85f, 0.55f, -0.65f, 0.07f, -0.83f
    };
    const size_t SHAKE_FACTOR_COUNT = sizeof(SHAKE_FACTORS) / sizeof(SHAKE_FACTORS[0]);

    // Frames per second the shake times are given in.
    const float SHAKE_FRAMES_PER_SECOND = 30.0f;

    // Pixels of the original screen.
    const float ORIGINAL_SCREEN_WIDTH = 320.0f;

    // Quadratic in and out easing.
    float ShakeEase(const float t){return t < 0.5f ? 2 * t * t : 1 - 2 * (1 - t) * (1 - t);}
}

void Background2D::ShakeAxis::Configure(const float new_amplitude, const float new_seconds){
    amplitude = new_amplitude;
    seconds = std::max(new_seconds, 1.0f / SHAKE_FRAMES_PER_SECOND);
    stopping = false;
    if (!running){
        running = true;
        StartBounce();
    }
}

void Background2D::ShakeAxis::Stop(const float seconds_to_center){
    if (!running) return;
    stopping = true;
    stop_seconds = std::max(seconds_to_center, 1.0f / SHAKE_FRAMES_PER_SECOND);
}

void Background2D::ShakeAxis::StartBounce(){
    from = value;
    elapsed = 0;
    if (stopping){
        to = 0;
        duration = stop_seconds;
    }
    else{
        // Half the amplitude, times the random factor, is the distance in pixels.
        to = amplitude / 2 * SHAKE_FACTORS[step % SHAKE_FACTOR_COUNT];
        ++ step;
        duration = seconds;
    }
}

bool Background2D::ShakeAxis::Advance(const float delta){
    if (!running) return false;
    const float old_value = value;
    elapsed += delta;
    while (elapsed >= duration){
        elapsed -= duration;
        value = to;
        if (stopping && to == 0){
            // Back in the center: the shake is over.
            *this = ShakeAxis();
            return old_value != 0;
        }
        StartBounce();
    }
    value = from + (to - from) * ShakeEase(elapsed / duration);
    return value != old_value;
}

void Background2D::UpdateShake(const float delta){
    if (!shake_x_.running && !shake_y_.running) return;
    const bool x_changed = shake_x_.Advance(delta);
    const bool y_changed = shake_y_.Advance(delta);
    if (!x_changed && !y_changed) return;
    const float scale = virtual_screen_size_.x / ORIGINAL_SCREEN_WIDTH;
    shake_offset_ = Ogre::Vector2(shake_x_.value * scale, shake_y_.value * scale);
    applyScroll();
}

void Background2D::ScriptShake(
  const int type, const int x_amplitude, const int x_frames, const int y_amplitude,
  const int y_frames
){
    LOG_TRIVIAL("Background2d shake type " + std::to_string(type) + ".");
    const float x_seconds = x_frames / SHAKE_FRAMES_PER_SECOND;
    const float y_seconds = y_frames / SHAKE_FRAMES_PER_SECOND;
    switch (type){
        case 0:
            {
                const float seconds = std::max(x_seconds, y_seconds);
                shake_x_.Stop(seconds);
                shake_y_.Stop(seconds);
            }
            break;
        case 1: shake_x_.Configure(static_cast<float>(x_amplitude), x_seconds); break;
        case 2: shake_y_.Configure(static_cast<float>(y_amplitude), y_seconds); break;
        case 3:
            shake_x_.Configure(static_cast<float>(x_amplitude), x_seconds);
            shake_y_.Configure(static_cast<float>(y_amplitude), y_seconds);
            break;
        default: LOG_WARNING("Background2d shake type " + std::to_string(type) + " is unknown.");
    }
}

void Background2D::ScriptAreaClear(const int area){
    LOG_TRIVIAL("Background2d area " + std::to_string(area) + " cleared.");
    area_states_on_.erase(area);
    RefreshArea(area);
}

void Background2D::Clear(){
    scroll_entity_ = nullptr;
    scroll_position_start_ = Ogre::Vector2::ZERO;
    scroll_position_end_ = Ogre::Vector2::ZERO;
    scroll_seconds_ = 0;
    scroll_current_seconds_ = 0;
    scroll_target_entity_ = nullptr;
    scroll_follow_target_ = false;
    has_pending_scroll_ = false;
    position_ = Ogre::Vector2::ZERO;
    position_real_ = Ogre::Vector2::ZERO;
    offset_ = Ogre::Vector2::ZERO;
    range_ = Ogre::AxisAlignedBox::BOX_INFINITE;
    UnsetScroll();
    for(unsigned int i = 0; i < animations_.size(); ++i) delete animations_[i];
    animations_.clear();
    for(unsigned int i = 0; i < animation_played_.size(); ++ i)
        for(unsigned int j = 0; j < animation_played_[i].sync.size(); ++j)
            ScriptManager::getSingleton().ContinueScriptExecution(animation_played_[i].sync[j]);
    animation_played_.clear();
    tiles_.clear();
    area_states_on_.clear();
    shake_x_ = ShakeAxis();
    shake_y_ = ShakeAxis();
    shake_offset_ = Ogre::Vector2::ZERO;
    DestroyVertexBuffers();
    CreateVertexBuffers();
}

void Background2D::ScriptAutoScrollToEntity(Entity* entity){scroll_entity_ = entity;}

Entity* Background2D::GetAutoScrollEntity() const{return scroll_entity_;}

void Background2D::ScriptScrollToPlayer(const SCROLL_TYPE type, const float seconds){
    Entity* player = EntityManager::getSingleton().ScriptGetPlayerEntity();
    if (player == nullptr) return;
    scroll_entity_ = nullptr;
    StartScroll(GetEntityScroll(player), type, seconds, player, type == Background2D::NONE);
}

void Background2D::ScriptScrollToEntity(
  Entity* entity, const SCROLL_TYPE type, const float seconds
){
    if (entity == nullptr){
        LOG_WARNING("Background2d can't scroll to an entity that doesn't exist.");
        return;
    }
    scroll_entity_ = nullptr;
    StartScroll(GetEntityScroll(entity), type, seconds, entity, true);
}

void Background2D::UpdateScrollTarget(){
    if (scroll_target_entity_ != nullptr)
        scroll_position_end_ = ClampScroll(GetEntityScroll(scroll_target_entity_));
}

const Ogre::Vector2 Background2D::GetEntityScroll(const Entity* entity) const{
    CameraManager &camera_manager(CameraManager::getSingleton());
    const Ogre::Vector3 view = camera_manager.ProjectPointToScreen(entity->GetPosition());
    const Ogre::Vector2 scroll = GetScreenScroll();
    const Ogre::Viewport* viewport(camera_manager.getViewport());
    const float width = static_cast<float>(viewport->getActualWidth());
    const float height = static_cast<float>(viewport->getActualHeight());
    return Ogre::Vector2(
      scroll.x + width / 2 - view.x, scroll.y + height / 2 - view.y
    ) / screen_scale_;
}

void Background2D::StartScroll(
  const Ogre::Vector2& position, const SCROLL_TYPE type, const float seconds,
  Entity* target_entity, const bool follow_target
){
    // A newer scroll replaces an instant one that has not been applied yet.
    has_pending_scroll_ = false;
    // Finish the previous scroll first, without following its target.
    scroll_target_entity_ = nullptr;
    scroll_follow_target_ = false;
    UnsetScroll();
    scroll_target_entity_ = target_entity;
    scroll_follow_target_ = follow_target;
    if (type == Background2D::NONE || seconds <= 0){
        SetScroll(position);
        UnsetScroll();
        return;
    }
    scroll_position_start_ = position_;
    scroll_position_end_ = ClampScroll(position);
    scroll_type_ = type;
    scroll_seconds_ = seconds;
    scroll_current_seconds_ = 0;
}

void Background2D::ScriptScrollToPosition(
  const float x, const float y, const SCROLL_TYPE type, const float seconds
){
    LOG_TRIVIAL(
      "Background2d set scroll to position '"
      + Ogre::StringConverter::toString(Ogre::Vector2(x, y)) + "'."
    );
    scroll_entity_ = nullptr;
    if (type == Background2D::NONE || seconds <= 0){
        // The scripts set a position and then scroll from it to another one, in the same frame.
        // The camera is only moved at the end of the frame, so if another scroll comes first the
        // movement starts from where the camera was, and there is no jump.
        // TODO: Some scenes still show a small camera jump (mds7pb_1, when the NPCs go down the
        // pinball machine and when Marlene says "Papa"), and the camera stays fixed after them
        // until the script scrolls to the player. How the original game handles it is not known.
        scroll_target_entity_ = nullptr;
        scroll_follow_target_ = false;
        UnsetScroll();
        pending_scroll_ = Ogre::Vector2(x, y);
        has_pending_scroll_ = true;
        return;
    }
    StartScroll(Ogre::Vector2(x, y), type, seconds, nullptr, false);
}

int Background2D::ScriptScrollSync(){
    // Nothing would ever wake the script up if there is no scroll to wait for.
    if (scroll_type_ == Background2D::NONE) return 1;
    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "Wait Background2d scroll for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    scroll_sync_.push_back(script);
    return -1;
}

void Background2D::ScriptOffset(const float x, const float y){
    LOG_TRIVIAL(
      "Background2d offset '" + Ogre::StringConverter::toString(Ogre::Vector2(x, y)) + "'."
    );
    offset_.x = x;
    offset_.y = y;
    position_real_.x += offset_.x;
    position_real_.y += offset_.y;
    CameraManager::getSingleton().Set2DScroll(position_real_);
}

void Background2D::UnsetScroll(){
    scroll_type_ = Background2D::NONE;
    if (scroll_follow_target_) scroll_entity_ = scroll_target_entity_;
    scroll_target_entity_ = nullptr;
    scroll_follow_target_ = false;
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
    position_ = ClampScroll(position);
    applyScroll();
}

const Ogre::Vector2 Background2D::ClampScroll(const Ogre::Vector2& position) const{
    Ogre::Vector2 clamped = position;
    Ogre::Vector3 pos_3d(position.x, position.y, 0);
    if (!range_.contains(pos_3d)){
        Ogre::Vector3 max_3d(range_.getMaximum());
        Ogre::Vector3 min_3d(range_.getMinimum());
        Ogre::Vector2 max(max_3d.x, max_3d.y);
        Ogre::Vector2 min(min_3d.x, min_3d.y);
        clamped.makeCeil(min);
        clamped.makeFloor(max);
    }
    return clamped;
}

void Background2D::applyScroll(){
    if(cv_background2d_manual.GetB() != true){
        position_real_ = (GetScroll() + shake_offset_) * screen_scale_;
        position_real_.x += offset_.x;
        position_real_.y += offset_.y;
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

float Background2D::TileDepthToBuffer(const float depth, const Ogre::Matrix4& projection){
    Ogre::Vector4 result = projection * Ogre::Vector4(0, 0, -depth, 1);
    result /= result.w;
    // Starting from this depth the projected values are too close to each other for the depth
    // buffer to tell them apart.
    const float far_group_start = 127.0f;
    if (depth >= far_group_start){
        Ogre::Vector4 start = projection * Ogre::Vector4(0, 0, -far_group_start, 1);
        start /= start.w;
        // The depth of the render system goes from -1 to 1 (OpenGL), so between the start of the
        // group and the far plane there are about 1.4e-5 to use. With 1e-5 per unit of depth,
        // tiles whose depths differ by 0.06 (127.938 and 128) end up several steps of a 24 bit
        // depth buffer apart, and the biggest depth used (128) is still in front of the far plane.
        const float far_group_step = 0.00001f;
        return std::min(start.z + (depth - far_group_start) * far_group_step, 0.9999995f);
    }
    return result.z;
}

void Background2D::AddTile(const VGears::Tile& tile){
    // TODO: move depth calculation to flevelBackgroundLoader maybe? and let
    // Backgorund2D only handle 0 <= depth <= 1 or so?
    // Maybe just move the < 4095 part to flevel background loader?
    Ogre::Real depth(0.0001f);
    if (tile.depth >= 1){
        if (tile.depth < 4095){
            depth = TileDepthToBuffer(
              tile.depth,
              CameraManager::getSingleton().GetCurrentCamera()->getProjectionMatrixWithRSDepth()
            );
        }
        else depth = 0.9999f;
    }
    AddTile(
      static_cast<int>(tile.destination.x), static_cast<int>(tile.destination.y),
      tile.width, tile.height, depth, tile.uv.x, tile.uv.y, tile.uv.z, tile.uv.w, tile.blending,
      tile.area, tile.state
    );
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
  const float v1, const float u2, const float v2, const Blending blending, const int area,
  const int state, const int layer
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
    // Keep where the tiles of each layer are in the buffer.
    {
        const int buffer = (blending == VGears::B_ALPHA) ? 0 : ((blending == VGears::B_ADD) ? 1 : 2);
        const int clamped = (layer < 0) ? 0 : ((layer >= LAYER_COUNT) ? LAYER_COUNT - 1 : layer);
        if (last_layer_[buffer] > clamped) layers_contiguous_[buffer] = false;
        if (clamped > last_layer_[buffer]) last_layer_[buffer] = clamped;
        LayerRange& range = layer_ranges_[buffer][clamped];
        if (range.count == 0) range.first = render_op.vertexData->vertexCount;
        range.count += TILE_VERTEX_COUNT;
    }
    Tile tile;
    tile.x = x;
    tile.y = y;
    tile.width = width;
    tile.height = height;
    tile.start_vertex_index = render_op.vertexData->vertexCount;
    tile.blending = blending;
    tile.area = area;
    tile.state = state;
    tile.visible = (area == 0) || IsAreaStateOn(area, state);
    //size_t index(tiles_.size());
    tiles_.push_back(tile);
    Ogre::Vector2 top_left(static_cast<Ogre::Real>(x), static_cast<Ogre::Real>(-y));
    Ogre::Vector2 top_right(
      static_cast<Ogre::Real>(x + width), static_cast<Ogre::Real>(top_left.y)
    );
    Ogre::Vector2 bottom_right(
      static_cast<Ogre::Real>(top_right.x), static_cast<Ogre::Real>(-(y + height))
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
    float* write_iterator
      = static_cast<float*>(vertex_buffer->lock(Ogre::HardwareBuffer::HBL_NORMAL));
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
    // Tiles of an area that is off are written at their position, and then collapsed.
    if (!tiles_.back().visible) WriteTilePosition(tiles_.back());
}

void Background2D::UpdateTileUV(
  const unsigned int tile_id, const float u1, const float v1, const float u2, const float v2
){
    if (tile_id >= tiles_.size()){
        LOG_ERROR("Tile with id " + Ogre::StringConverter::toString( tile_id ) + " doesn't exist.");
        return;
    }
    Ogre::HardwareVertexBufferSharedPtr vertex_buffer;
    if (tiles_[tile_id].blending == VGears::B_ALPHA) vertex_buffer = alpha_vertex_buffer_;
    else if (tiles_[tile_id].blending == VGears::B_ADD) vertex_buffer = add_vertex_buffer_;
    else if(tiles_[tile_id].blending == VGears::B_SUBTRACT) vertex_buffer = subtract_vertex_buffer_;
    float* write_iterator
      = static_cast<float*>(vertex_buffer->lock(Ogre::HardwareBuffer::HBL_NORMAL));
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
    else LOG_ERROR("Background2D doesn't has animation '" + animation + "'.");
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
    // The movie takes the place of the background, and the models are drawn over it.
    if (EntityManager::getSingleton().GetMoviePlayer()->IsDrawnBehind()) return;
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
        RenderBuffer(0, alpha_render_op_, alpha_material_, rs_params, width, height);
        RenderBuffer(1, add_render_op_, add_material, rs_params, width, height);
        RenderBuffer(2, subtract_render_op_, subtract_material_, rs_params, width, height);
    }
}

void Background2D::ResetLayers(){
    for (int buffer = 0; buffer < 3; ++ buffer){
        for (int layer = 0; layer < LAYER_COUNT; ++ layer) layer_ranges_[buffer][layer] = LayerRange();
        last_layer_[buffer] = -1;
        layers_contiguous_[buffer] = true;
    }
    for (int layer = 0; layer < LAYER_COUNT; ++ layer){
        layer_parallax_[layer][0] = layer_parallax_[layer][1] = 1.0f;
        layer_scroll_speed_[layer][0] = layer_scroll_speed_[layer][1] = 0.0f;
        layer_scroll_offset_[layer][0] = layer_scroll_offset_[layer][1] = 0.0f;
    }
}

bool Background2D::HasLayerScroll() const{
    for (int layer = 0; layer < LAYER_COUNT; ++ layer){
        if (
          layer_parallax_[layer][0] != 1.0f || layer_parallax_[layer][1] != 1.0f
          || layer_scroll_speed_[layer][0] != 0.0f || layer_scroll_speed_[layer][1] != 0.0f
        ) return true;
    }
    return false;
}

void Background2D::ScriptSetLayerParallax(const int layer, const int distance){
    if (layer < 0 || layer >= LAYER_COUNT) return;
    const float ratio = static_cast<float>(distance) / 4096.0f;
    layer_parallax_[layer][0] = ratio;
    layer_parallax_[layer][1] = ratio;
}

void Background2D::ScriptSetLayerScroll(const int layer, const int x, const int y){
    if (layer < 0 || layer >= LAYER_COUNT) return;
    // 1/256 of a pixel of the original screen for each frame, at 30 frames per second. The
    // virtual screen is 3 times as big as the one of the original game.
    const float to_virtual_pixels_per_second = 30.0f * 3.0f / 256.0f;
    layer_scroll_speed_[layer][0] = static_cast<float>(x) * to_virtual_pixels_per_second;
    layer_scroll_speed_[layer][1] = static_cast<float>(y) * to_virtual_pixels_per_second;
}

void Background2D::RenderBuffer(
  const int buffer, Ogre::RenderOperation& render_op, const Ogre::MaterialPtr& material,
  Ogre::GpuProgramParametersPtr& rs_params, const float width, const float height
){
    if (render_op.vertexData->vertexCount == 0) return;
    const unsigned int total = render_op.vertexData->vertexCount;
    const bool by_layer = layers_contiguous_[buffer] && HasLayerScroll();
    if (!by_layer){
        // The view the layers of other buffers could have left.
        Ogre::Matrix4 view;
        view.makeTrans(Ogre::Vector3(position_real_.x / width, -position_real_.y / height, 0));
        render_system_->_setViewMatrix(view);
        rs_params->setConstant(Ogre::GpuProgramParameters::ACT_VIEW_MATRIX, view);
        render_system_->applyFixedFunctionParams(rs_params, Ogre::GPV_GLOBAL);
        scene_manager_->_setPass(material->getTechnique(0)->getPass(0), true, false);
        render_system_->_render(render_op);
        return;
    }
    for (int layer = 0; layer < LAYER_COUNT; ++ layer){
        const LayerRange& range = layer_ranges_[buffer][layer];
        if (range.count == 0) continue;
        // The scroll of the background, changed by the parallax of the layer, plus the one it has
        // by itself. The scroll by itself is in pixels of the virtual screen.
        const Ogre::Vector2 by_itself(
          layer_scroll_offset_[layer][0] * screen_scale_,
          layer_scroll_offset_[layer][1] * screen_scale_
        );
        Ogre::Matrix4 view;
        view.makeTrans(Ogre::Vector3(
          (position_real_.x * layer_parallax_[layer][0] + by_itself.x) / width,
          -(position_real_.y * layer_parallax_[layer][1] + by_itself.y) / height, 0
        ));
        // TODO This is deprecated, but if not done , the background image disappears.
        render_system_->_setViewMatrix(view);
        rs_params->setConstant(Ogre::GpuProgramParameters::ACT_VIEW_MATRIX, view);
        render_system_->applyFixedFunctionParams(rs_params, Ogre::GPV_GLOBAL);
        scene_manager_->_setPass(material->getTechnique(0)->getPass(0), true, false);
        render_op.vertexData->vertexStart = range.first;
        render_op.vertexData->vertexCount = range.count;
        render_system_->_render(render_op);
        render_op.vertexData->vertexStart = 0;
        render_op.vertexData->vertexCount = total;
    }
}

void Background2D::CreateVertexBuffers(){
    ResetLayers();
    alpha_max_vertex_count_ = 2048 * TILE_VERTEX_COUNT;
    alpha_render_op_.vertexData = new Ogre::VertexData;
    alpha_render_op_.vertexData->vertexStart = 0;
    Ogre::VertexDeclaration* vertex_declaration = alpha_render_op_.vertexData->vertexDeclaration;
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
    vertex_declaration->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
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
    size_t area_tiles(0);
    while (it != it_end) {
        AddTile(*it);
        load(tile_index ++, it->animations);
        if (it->area != 0) ++ area_tiles;
        ++ it;
    }
    LOG_TRIVIAL(
      "Background2D: " + std::to_string(tile_index) + " tiles loaded, " + std::to_string(area_tiles)
      + " of them in scripted areas."
    );
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
