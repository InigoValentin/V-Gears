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

#include <OgreLogManager.h>
#include <OgreException.h>
#include <OgreViewport.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <algorithm>
#include "engine/graphics/Background2D.h"
#include "engine/graphics/CameraManager.h"
#include "engine/entity/EntityManager.h"
#include "engine/config/ConfigVar.h"
#include "engine/graphics/DebugDraw.h"
#include "common/Logger.h"
#include "engine/Timer.h"

ConfigVar cv_debug_background2d("debug_background2d", "Draw background debug info", "false");
ConfigVar cv_show_background2d("show_background2d", "Draw background", "true");
ConfigVar cv_background2d_manual("background2d_manual", "Manual 2d background scrolling", "false");

Background2D::Background2D():
  alpha_tiles_(VGears::B_ALPHA, "Background2DAlpha", MAX_TILES),
  add_tiles_(VGears::B_ADD, "Background2DAdd", MAX_TILES),
  subtract_tiles_(VGears::B_SUBTRACT, "Background2DSubtract", MAX_SUBTRACT_TILES),
  scroll_([](const ScriptId& script){
      ScriptManager::getSingleton().ContinueScriptExecution(script);
  }),
  offset_(Ogre::Vector2::ZERO),
  position_real_(Ogre::Vector2::ZERO),
  screen_scale_(1),
  screen_proportion_(1, 1),
  virtual_screen_size_(320, 240) // FFVII original resolution
{
    scene_manager_ = Ogre::Root::getSingleton().getSceneManager("Scene");
    render_system_ = Ogre::Root::getSingletonPtr()->getRenderSystem();
    ResetLayers();
    scene_manager_->addRenderQueueListener(this);
}

Background2D::~Background2D(){
    scene_manager_->removeRenderQueueListener(this);
    for (unsigned int i = 0; i < animations_.size(); ++ i) delete animations_[i];
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
    if (scroll_.ApplyPending()) applyScroll();
    UpdatePalette();
    const float delta = Timer::getSingleton().GetGameTimeDelta();
    UpdateShake(delta);
    // The layers that scroll by themselves.
    for (int layer = 0; layer < LAYER_COUNT; ++ layer){
        layer_scroll_offset_[layer][0] += layer_scroll_speed_[layer][0] * delta;
        layer_scroll_offset_[layer][1] += layer_scroll_speed_[layer][1] * delta;
    }
    UpdateAnimations(delta);
}

void Background2D::UpdateAnimations(const float delta){
    for (AnimationPlayed& played : animation_played_){
        for (Background2DAnimation* animation : animations_){
            if (animation->GetName() != played.name) continue;
            const float time = animation->GetTime();
            const float end_time = animation->GetLength();
            if (time + delta < end_time) animation->AddTime(delta);
            else{
                // Set to last frame of animation.
                if (time != end_time) animation->SetTime(end_time);
                if (played.state == Background2DAnimation::ONCE){
                    for (const ScriptId& script : played.sync)
                        ScriptManager::getSingleton().ContinueScriptExecution(script);
                    played.sync.clear();
                    // Mark to delete this way:
                    played.name = "";
                }
                // In case of looped, sync with end:
                else animation->SetTime(time + delta - end_time);
            }
        }
    }
    // Remove stopped animations.
    animation_played_.erase(
      std::remove_if(
        animation_played_.begin(), animation_played_.end(),
        [](const AnimationPlayed& played){return played.name == "";}
      ), animation_played_.end()
    );
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
    BackgroundTileBuffer* buffer = BufferFor(tile.blending);
    if (buffer == nullptr) return;
    QuadWriter::Corners corners = TileCorners(tile);
    // A tile that is not drawn has all its corners in the same point.
    if (!tile.visible) corners.fill(corners[0]);
    buffer->SetPositions(tile.start_vertex_index, corners);
}

QuadWriter::Corners Background2D::TileCorners(const Tile& tile) const{
    const Ogre::Real left = static_cast<Ogre::Real>(tile.x);
    const Ogre::Real top = static_cast<Ogre::Real>(-tile.y);
    const Ogre::Real right = static_cast<Ogre::Real>(tile.x + tile.width);
    const Ogre::Real bottom = static_cast<Ogre::Real>(-(tile.y + tile.height));
    QuadWriter::Corners corners{{{left, top}, {right, top}, {right, bottom}, {left, bottom}}};
    for (Ogre::Vector2& corner : corners) virtualScreenToWorldSpace(corner);
    return corners;
}

BackgroundTileBuffer* Background2D::BufferFor(const Blending blending){
    if (blending == VGears::B_ALPHA) return &alpha_tiles_;
    if (blending == VGears::B_ADD) return &add_tiles_;
    if (blending == VGears::B_SUBTRACT) return &subtract_tiles_;
    return nullptr;
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

void Background2D::UpdateShake(const float delta){
    if (!shake_.Update(delta)) return;
    // The shake is in pixels of the original screen.
    shake_offset_ = shake_.Offset() * (virtual_screen_size_.x / 320.0f);
    applyScroll();
}

void Background2D::ScriptShake(
  const int type, const int x_amplitude, const int x_frames, const int y_amplitude,
  const int y_frames
){
    LOG_TRIVIAL("Background2d shake type " + std::to_string(type) + ".");
    if (!shake_.Set(type, x_amplitude, x_frames, y_amplitude, y_frames))
        LOG_WARNING("Background2d shake type " + std::to_string(type) + " is unknown.");
}

void Background2D::ScriptAreaClear(const int area){
    LOG_TRIVIAL("Background2d area " + std::to_string(area) + " cleared.");
    area_states_on_.erase(area);
    RefreshArea(area);
}

void Background2D::SetPalette(const Ogre::String& name){
    if (palette_.Load(name)){
        palette_image_loaded_ = false;
        Ogre::LogManager::getSingleton().stream() << "Background2D: palette file loaded " << name;
    }
}

void Background2D::ScriptPaletteStore(int page, int slot, int count){
    palette_.Store(page, slot, count);
}

void Background2D::ScriptPaletteLoad(int slot, int page, int count){
    palette_.Load(slot, page, count);
}

void Background2D::ScriptPaletteCopy(int source, int destination, int count){
    palette_.Copy(source, destination, count);
}

void Background2D::ScriptPaletteMultiply(
  int source, int destination, int start, int count, int red, int green, int blue
){
    palette_.Multiply(source, destination, start, count, red, green, blue);
}

void Background2D::ScriptPaletteAdd(
  int source, int destination, int red, int green, int blue, int count
){
    palette_.Add(source, destination, red, green, blue, count);
}

void Background2D::UpdatePalette(){
    if (palette_.IsEmpty() || image_name_.empty()) return;
    if (!palette_image_loaded_){
        try{
            palette_image_.load(
              image_name_, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME
            );
        }
        catch (const Ogre::Exception& e){
            LOG_ERROR("Background palette: can't load image '" + image_name_ + "': " + e.what());
            palette_.Clear();
            return;
        }
        palette_image_loaded_ = true;
    }
    if (!palette_.Paint(palette_image_)) return;
    Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(image_name_);
    if (!texture){
        texture = Ogre::TextureManager::getSingleton().load(
          image_name_, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME
        );
    }
    if (texture){
        texture->getBuffer()->blitFromMemory(palette_image_.getPixelBox());
    }
}

void Background2D::Clear(){
    scroll_.Clear();
    position_real_ = Ogre::Vector2::ZERO;
    offset_ = Ogre::Vector2::ZERO;
    for (Background2DAnimation* animation : animations_) delete animation;
    animations_.clear();
    for (const AnimationPlayed& played : animation_played_){
        for (const ScriptId& script : played.sync)
            ScriptManager::getSingleton().ContinueScriptExecution(script);
    }
    animation_played_.clear();
    tiles_.clear();
    area_states_on_.clear();
    palette_.Clear();
    palette_image_loaded_ = false;
    shake_ = BackgroundShake();
    shake_offset_ = Ogre::Vector2::ZERO;
    alpha_tiles_.Clear();
    add_tiles_.Clear();
    subtract_tiles_.Clear();
    ResetLayers();
}

void Background2D::ScriptAutoScrollToEntity(Entity* entity){scroll_.Follow(entity);}

Entity* Background2D::GetAutoScrollEntity() const{return scroll_.FollowedEntity();}

bool Background2D::HasScrollControl() const{return scroll_.HasControl();}

void Background2D::ScriptScrollToPlayer(const SCROLL_TYPE type, const float seconds){
    Entity* player = EntityManager::getSingleton().ScriptGetPlayerEntity();
    if (player == nullptr) return;
    // The camera follows the player when the scroll ends, whatever the transition: scenes end with
    // SCRLC (scroll to the player) and give the control back without any other camera opcode, as
    // the first visit of mrkt2 does.
    if (scroll_.ToEntity(player, GetEntityScroll(player), type, seconds)) applyScroll();
}

void Background2D::ScriptScrollToEntity(
  Entity* entity, const SCROLL_TYPE type, const float seconds
){
    if (entity == nullptr){
        LOG_WARNING("Background2d can't scroll to an entity that doesn't exist.");
        return;
    }
    if (scroll_.ToEntity(entity, GetEntityScroll(entity), type, seconds)) applyScroll();
}

void Background2D::UpdateScrollTarget(){
    if (scroll_.TargetEntity() != nullptr)
        scroll_.SetTargetPosition(GetEntityScroll(scroll_.TargetEntity()));
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

void Background2D::ScriptScrollToPosition(
  const float x, const float y, const SCROLL_TYPE type, const float seconds
){
    LOG_TRIVIAL(
      "Background2d set scroll to position '"
      + Ogre::StringConverter::toString(Ogre::Vector2(x, y)) + "'."
    );
    scroll_.ToPosition(Ogre::Vector2(x, y), type, seconds);
}

int Background2D::ScriptScrollSync(){
    // Nothing would ever wake the script up if there is no scroll to wait for.
    if (scroll_.Type() == NONE) return 1;
    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "Wait Background2d scroll for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    scroll_.Wait(script);
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

void Background2D::UnsetScroll(){scroll_.Stop();}

const Ogre::Vector2& Background2D::GetScrollPositionStart() const{return scroll_.Start();}

const Ogre::Vector2& Background2D::GetScrollPositionEnd() const{return scroll_.End();}

Background2D::SCROLL_TYPE Background2D::GetScrollType() const{return scroll_.Type();}

float Background2D::GetScrollSeconds() const{return scroll_.Seconds();}

void Background2D::SetScrollCurrentSeconds(const float seconds){
    scroll_.SetCurrentSeconds(seconds);
}

float Background2D::GetScrollCurrentSeconds() const{return scroll_.CurrentSeconds();}

void Background2D::SetScroll(const Ogre::Vector2& position){
    scroll_.SetPosition(position);
    applyScroll();
}

void Background2D::applyScroll(){
    if(cv_background2d_manual.GetB() != true){
        position_real_ = (GetScroll() + shake_offset_) * screen_scale_;
        position_real_.x += offset_.x;
        position_real_.y += offset_.y;
        CameraManager::getSingleton().Set2DScroll(position_real_);
    }
}

const Ogre::Vector2& Background2D::GetScroll() const{return scroll_.Position();}

const Ogre::Vector2 Background2D::GetScreenScroll() const{return GetScroll() * screen_scale_;}

void Background2D::SetImage(const Ogre::String& image){
    Ogre::LogManager::getSingleton().stream() << "Background2D::SetImage " << image;
    image_name_ = image;
    palette_image_loaded_ = false;
    alpha_tiles_.SetImage(image);
    add_tiles_.SetImage(image);
    subtract_tiles_.SetImage(image);
}

void Background2D::SetRange(const Ogre::Vector4& range){
    SetRange((int)range.x, (int)range.y, (int)range.z, (int)range.w);
}

void Background2D::SetRange(const int min_x, const int min_y, const int max_x, const int max_y){
    Ogre::LogManager::getSingleton().stream()
      << "Background2D::SetRange " << min_x << " " << min_y << " " << max_x << " " << max_y;
    // The proportion of the screen is needed for the limits.
    calculateScreenScale();
    Ogre::Vector2 half_virtual_screen_size(virtual_screen_size_ / 2);
    half_virtual_screen_size /= screen_proportion_;
    // The scroll is the opposite of the position of the center of the camera (the scripts give it
    // like that, and the scene is moved, not the camera), so the limits of the camera are the
    // opposite ones. It only shows in the fields whose range is not centered (nivl_b1): the
    // camera went beyond the background.
    Ogre::AxisAlignedBox range;
    range.setMaximum(
      -(min_x + half_virtual_screen_size.x), -(min_y + half_virtual_screen_size.y), 1
    );
    range.setMinimum(
      -(max_x - half_virtual_screen_size.x), -(max_y - half_virtual_screen_size.y), 0
    );
    scroll_.SetRange(range);
    Ogre::LogManager::getSingleton().stream() << "Background2D::SetRange " << range;
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
    // The tiles that are less than one unit away (the pieces of a model that a script puts in,
    // for example) are only a hair away from the camera, but they keep their order: all of them at
    // the same depth made the tile that is drawn last cover the others, wherever it was.
    Ogre::Real depth(0.0001f + std::max(tile.depth, 0.0f) * 0.00005f);
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
  const int state, const int layer, const float intensity
){
    BackgroundTileBuffer* buffer = BufferFor(blending);
    if (buffer == nullptr){
        LOG_ERROR("Unknown blending type.");
        return;
    }
    if (buffer->IsFull()){
        LOG_ERROR(
          "Max number of tiles reached. Can't create more than "
          + Ogre::StringConverter::toString(buffer->MaxTiles()) + " tiles."
        );
        return;
    }
    Tile tile;
    tile.x = x;
    tile.y = y;
    tile.width = width;
    tile.height = height;
    tile.blending = blending;
    tile.area = area;
    tile.state = state;
    tile.visible = (area == 0) || IsAreaStateOn(area, state);
    tile.start_vertex_index = buffer->Add(
      TileCorners(tile), {u1, v1, u2, v2}, depth, intensity, layer
    );
    tiles_.push_back(tile);
    // Tiles of an area that is off are written at their position, and then collapsed.
    if (!tile.visible) WriteTilePosition(tile);
}

void Background2D::UpdateTileUV(
  const unsigned int tile_id, const float u1, const float v1, const float u2, const float v2
){
    if (tile_id >= tiles_.size()){
        LOG_ERROR("Tile with id " + Ogre::StringConverter::toString( tile_id ) + " doesn't exist.");
        return;
    }
    const Tile& tile = tiles_[tile_id];
    BufferFor(tile.blending)->SetTexture(tile.start_vertex_index, {u1, v1, u2, v2});
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
        SetView(position_real_, rs_params, width, height);
        RenderBuffer(alpha_tiles_, rs_params, width, height);
        RenderBuffer(add_tiles_, rs_params, width, height);
        RenderBuffer(subtract_tiles_, rs_params, width, height);
    }
}

void Background2D::ResetLayers(){
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
  BackgroundTileBuffer& buffer, Ogre::GpuProgramParametersPtr& rs_params, const float width,
  const float height
){
    if (buffer.IsEmpty()) return;
    if (!buffer.Layers().AreContiguous() || !HasLayerScroll()){
        // The view the layers of other buffers could have left.
        SetView(position_real_, rs_params, width, height);
        buffer.Draw(*scene_manager_, *render_system_);
        return;
    }
    for (int layer = 0; layer < LAYER_COUNT; ++ layer){
        const BackgroundLayerRanges::Range& range = buffer.Layers().Of(layer);
        if (range.count == 0) continue;
        // The scroll of the background, changed by the parallax of the layer, plus the one it has
        // by itself. The scroll by itself is in pixels of the virtual screen.
        const Ogre::Vector2 by_itself(
          layer_scroll_offset_[layer][0] * screen_scale_,
          layer_scroll_offset_[layer][1] * screen_scale_
        );
        SetView(
          Ogre::Vector2(
            position_real_.x * layer_parallax_[layer][0] + by_itself.x,
            position_real_.y * layer_parallax_[layer][1] + by_itself.y
          ), rs_params, width, height
        );
        buffer.Draw(*scene_manager_, *render_system_, range);
    }
}

void Background2D::SetView(
  const Ogre::Vector2& position, Ogre::GpuProgramParametersPtr& rs_params, const float width,
  const float height
){
    Ogre::Matrix4 view;
    view.makeTrans(Ogre::Vector3(position.x / width, -position.y / height, 0));
    // TODO This is deprecated, but if not done , the background image disappears.
    render_system_->_setViewMatrix(view);
    rs_params->setConstant(Ogre::GpuProgramParameters::ACT_VIEW_MATRIX, view);
    render_system_->applyFixedFunctionParams(rs_params, Ogre::GPV_GLOBAL);
}

void Background2D::load(const VGears::Background2DFilePtr& background){
    assert(background != nullptr);
    background->load();
    SetImage(background->GetTextureName());
    if (!background->GetPaletteName().empty()) SetPalette(background->GetPaletteName());
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
