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

#include <OgreMath.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include "core/ConfigVar.h"
#include "core/CameraManager.h"
#include "core/DebugDraw.h"
#include "core/Logger.h"
#include "core/ScriptManager.h"
#include "core/Timer.h"
#include "core/UiWidget.h"


ConfigVar cv_debug_ui("debug_ui", "Draw ui debug info", "0");


UiWidget::UiWidget(const Ogre::String& name):
  name_(name),
  path_name_(name),
  parent_(nullptr)
{Initialise();}


UiWidget::UiWidget(const Ogre::String& name, const Ogre::String& path_name, UiWidget* parent):
  name_(name),
  path_name_(path_name),
  parent_(parent)
{Initialise();}


UiWidget::~UiWidget(){
    for (unsigned int i = 0; i < animations_.size(); ++ i)
        delete animations_[i];
    ScriptManager::getSingleton().RemoveEntity(ScriptManager::UI, path_name_);
    RemoveAllChildren();
}

void UiWidget::Initialise(){
    Ogre::Viewport *viewport(CameraManager::getSingleton().getViewport());
    screen_width_ = static_cast<float>(viewport->getActualWidth());
    screen_height_ = static_cast<float>(viewport->getActualHeight());
    visible_ = false;
    align_ = LEFT;
    vertical_align_ = TOP;
    update_transformation_ = true;
    final_z_ = 0;
    final_origin_ = Ogre::Vector2::ZERO;
    final_translate_ = Ogre::Vector2::ZERO;
    final_size_ = Ogre::Vector2(screen_width_, screen_height_);
    final_scale_ = Ogre::Vector2(1, 1);
    final_rotation_ = 0;
    origin_x_percent_ = 0;
    origin_x_ = 0;
    origin_y_percent_ = 0;
    origin_y_ = 0;
    x_percent_ = 0;
    x_ = 0;
    y_percent_ = 0;
    y_ = 0;
    z_ = 0;
    width_percent_ = 100;
    width_ = 0;
    height_percent_ = 100;
    height_ = 0;
    scale_ = Ogre::Vector2(1.0f, 1.0f);
    rotation_ = 0.0f;
    scissor_ = false;
    local_scissor_ = false;
    global_scissor_ = true;
    scissor_top_ = 0;
    scissor_x_percent_top_ = 0;
    scissor_x_top_ = 0;
    scissor_bottom_ = screen_height_;
    scissor_x_percent_bottom_ = 100;
    scissor_x_bottom_ = 0;
    scissor_left_ = 0;
    scissor_y_percent_left_ = 0;
    scissor_y_left_ = 0;
    scissor_right_ = screen_width_;
    scissor_y_percent_right_ = 100;
    scissor_y_right_ = 0;
    animation_current_ = nullptr;
    animation_default_ = "";
    animation_state_ = UiAnimation::DEFAULT;
    colour_1_ = Ogre::ColourValue(1, 1, 1, 1);
    colour_2_ = Ogre::ColourValue(1, 1, 1, 1);
    colour_3_ = Ogre::ColourValue(1, 1, 1, 1);
    colour_4_ = Ogre::ColourValue(1, 1, 1, 1);
    ScriptManager::getSingleton().AddEntity(
      ScriptManager::UI, path_name_, nullptr
    );
}

void UiWidget::Update(){
    if (visible_ != true) return;

    if (animation_current_ != nullptr){
        float delta_time = Timer::getSingleton().GetGameTimeDelta();
        float time = animation_current_->GetTime();

        // If animation has ended
        if (time + delta_time >= animation_end_time_){
            if (time != animation_end_time_)
                animation_current_->AddTime(animation_end_time_ - time);
            for (unsigned int i = 0; i < animation_sync_.size(); ++ i){
                ScriptManager::getSingleton().ContinueScriptExecution(
                  animation_sync_[i]
                );
            }
            animation_sync_.clear();
            if (
              animation_state_ == UiAnimation::DEFAULT && animation_default_
              != ""
            ){
                // In case of cycled default, it's required to sync with end.
                time = time + delta_time - animation_current_->GetLength();
                PlayAnimation(
                  animation_default_, UiAnimation::DEFAULT, time, -1
                );
            }
            else animation_current_ = NULL;
        }
        else animation_current_->AddTime(delta_time);
    }
    else if (
      animation_current_ == NULL && animation_state_ == UiAnimation::DEFAULT
      && animation_default_ != ""
    ){
        PlayAnimation(animation_default_, UiAnimation::DEFAULT, 0, -1);
    }

    if (update_transformation_ == true) UpdateTransformation();

    for (unsigned int i = 0; i < children_.size(); ++ i)
        children_[i]->Update();


    // Debug output
    if (cv_debug_ui.GetI() >= 1){
        float local_x1 = -final_origin_.x;
        float local_y1 = -final_origin_.y;
        float local_x2 = final_size_.x + local_x1;
        float local_y2 = final_size_.y + local_y1;
        float x = final_translate_.x;
        float y = final_translate_.y;
        DEBUG_DRAW.SetScreenSpace(true);
        DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 0, 0, 1));
        int x1, y1, x2, y2, x3, y3, x4, y4;

        if (final_rotation_ != 0){
            float cos
              = Ogre::Math::Cos(Ogre::Radian(Ogre::Degree(final_rotation_)));
            float sin
              = Ogre::Math::Sin(Ogre::Radian(Ogre::Degree(final_rotation_)));
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

        // Slightly modify to let show things that are on board of screen.
        DEBUG_DRAW.Line(
          static_cast<float>(x1), static_cast<float>(y1 + 1),
          static_cast<float>(x2), static_cast<float>(y2 + 1)
        );
        DEBUG_DRAW.Line(
          static_cast<float>(x2 - 1), static_cast<float>(y2),
          static_cast<float>(x3 - 1), static_cast<float>(y3)
        );
        DEBUG_DRAW.Line(
          static_cast<float>(x3), static_cast<float>(y3),
          static_cast<float>(x4), static_cast<float>(y4)
        );
        DEBUG_DRAW.Line(
          static_cast<float>(x4), static_cast<float>(y4),
          static_cast<float>(x1), static_cast<float>(y1)
        );

        // Draw translation.
        DEBUG_DRAW.SetColour(Ogre::ColourValue(0, 1, 0, 1));
        Ogre::Vector2 area_origin
          = (parent_ != nullptr)
            ? parent_->GetFinalOrigin() : Ogre::Vector2::ZERO;
        Ogre::Vector2 area_translate
          = (parent_ != nullptr)
            ? parent_->GetFinalTranslate() : Ogre::Vector2::ZERO;
        Ogre::Vector2 pos = area_translate - area_origin;
        DEBUG_DRAW.Line(pos.x, pos.y, x, y);
        DEBUG_DRAW.Quad(x - 2, y - 2, x + 2, y - 2, x + 2, y + 2, x - 2, y + 2);

        if (cv_debug_ui.GetI() >= 2){
            DEBUG_DRAW.SetColour(Ogre::ColourValue::White);
            DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.LEFT);
            DEBUG_DRAW.Text(
              static_cast<float>(x1 + 3), static_cast<float>(y1), path_name_
            );
            DEBUG_DRAW.Text(
              static_cast<float>(x1 + 3), static_cast<float>(y1 + 12),
              GetCurrentAnimationName()
            );
        }

        // Draw the origin.
        DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 0, 0, 1));
        DEBUG_DRAW.Line(
          x, y, static_cast<float>(x1), static_cast<float>(y1 + 1)
        );
    }
}

void UiWidget::OnResize(){
    Ogre::Viewport *viewport(CameraManager::getSingleton().getViewport());
    screen_width_ = static_cast<float>(viewport->getActualWidth());
    screen_height_ = static_cast<float>(viewport->getActualHeight());
    for (size_t i = 0; i < children_.size(); ++ i) children_[i]->OnResize();
    update_transformation_ = true;
}

void UiWidget::Render(){
    if (visible_ == true)
        for (size_t i = 0; i < children_.size(); ++ i) children_[i]->Render();
}

void UiWidget::SetVisible(const bool visible){visible_ = visible;}

bool UiWidget::IsVisible() const{return visible_;}

const Ogre::String& UiWidget::GetName() const{return name_;}

void UiWidget::AddChild(UiWidget *widget){children_.push_back(widget);}

UiWidget* UiWidget::GetChild(const Ogre::String& name){
    for (unsigned int i = 0; i < children_.size(); ++ i)
        if (children_[i]->GetName() == name) return children_[i];
    return NULL;
}

UiWidget* UiWidget::GetChild(const unsigned int id){
    if (id >= children_.size()) return NULL;
    return children_[id];
}

unsigned int UiWidget::GetNumberOfChildren(){return children_.size();}

void UiWidget::RemoveAllChildren(){
    for (size_t i = 0; i < children_.size(); ++ i) delete children_[i];
    children_.clear();
}

void UiWidget::AddAnimation(UiAnimation* animation){
    animations_.push_back(animation);
}

const Ogre::String& UiWidget::GetCurrentAnimationName() const{
    if (animation_current_ != nullptr) return animation_current_->GetName();
    return Ogre::BLANKSTRING;
}

void UiWidget::PlayAnimation(
  const Ogre::String& animation, UiAnimation::State state,
  const float start, const float end
){
    for (size_t i = 0; i < animations_.size(); ++ i){
        if (animations_[i]->GetName() == animation){
            animation_current_ = animations_[i];
            animation_current_->SetTime(
              (start == -1) ? animation_current_->GetLength() : start
            );
            animation_current_->AddTime(0);
            animation_end_time_
              = (end == -1) ? animation_current_->GetLength() : end;
            animation_state_ = state;
            return;
        }
    }

    // Stop current state and animation.
    animation_current_ = nullptr;
    animation_state_ = UiAnimation::ONCE;
    LOG_ERROR(
      "Widget '" + name_ + "' doesn't has animation '" + animation + "'."
    );
}

void UiWidget::ScriptPlayAnimation(const char* name){
    PlayAnimation(Ogre::String(name), UiAnimation::DEFAULT, 0, -1);
}

void UiWidget::ScriptPlayAnimationStop(const char* name){
    PlayAnimation(Ogre::String(name), UiAnimation::ONCE, 0, -1);
}

void UiWidget::ScriptPlayAnimation(
  const char* name, const float start, const float end
){
    PlayAnimation(Ogre::String(name), UiAnimation::DEFAULT, start, end);
}

void UiWidget::ScriptPlayAnimationStop(
  const char* name, const float start, const float end
){
    PlayAnimation(Ogre::String(name), UiAnimation::ONCE, start, end);
}

void UiWidget::ScriptSetDefaultAnimation(const char* animation){
    animation_default_ = Ogre::String(animation);
    animation_state_ = UiAnimation::DEFAULT;
}

int UiWidget::ScriptAnimationSync(){
    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    animation_sync_.push_back(script);
    return -1;
}

void UiWidget::SetUpdateTransformation(){
    for (unsigned int i = 0; i < children_.size(); ++ i)
        children_[i]->SetUpdateTransformation();
    update_transformation_ = true;
}

void UiWidget::UpdateTransformation(){
    Ogre::Vector2 area_scale
      = (parent_ != nullptr) ? parent_->GetFinalScale() : Ogre::Vector2(1, 1);
    Ogre::Vector2 area_origin
      = (parent_ != nullptr) ? parent_->GetFinalOrigin() : Ogre::Vector2::ZERO;
    Ogre::Vector2 area_translate
      = (parent_ != nullptr)
        ? parent_->GetFinalTranslate() : Ogre::Vector2::ZERO;
    Ogre::Vector2 area_size
      = (parent_ != nullptr)
        ? parent_->GetFinalSize()
        : Ogre::Vector2(screen_width_, screen_height_);
    float area_rotation
      = (parent_ != nullptr) ? parent_->GetFinalRotation() : 0;
    float local_x
      = ((area_size.x * x_percent_) / 100.0f + (x_ * screen_height_ / 720.0f)
      * area_scale.x) - area_origin.x;
    float local_y
      = ((area_size.y * y_percent_) / 100.0f + (y_ * screen_height_ / 720.0f)
      * area_scale.y) - area_origin.y;
    float x = local_x;
    float y = local_y;
    if (area_rotation != 0){
        float cos = Ogre::Math::Cos(Ogre::Radian(Ogre::Degree(area_rotation)));
        float sin = Ogre::Math::Sin(Ogre::Radian(Ogre::Degree(area_rotation)));
        x = local_x * cos - local_y * sin;
        y = local_x * sin + local_y * cos;
    }

    final_translate_.x = area_translate.x;
    if (align_ == RIGHT) final_translate_.x = area_translate.x + area_size.x;
    else if (align_ == CENTER)
        final_translate_.x = area_translate.x + area_size.x / 2;
    final_translate_.x += x;
    final_translate_.y = area_translate.y;
    if (vertical_align_ == BOTTOM)
        final_translate_.y = area_translate.y + area_size.y;
    else if (vertical_align_ == MIDDLE)
        final_translate_.y = area_translate.y + area_size.y / 2;
    final_translate_.y += y;

    final_z_ = (parent_ != NULL) ? parent_->GetFinalZ() + z_ : z_;
    final_scale_ = area_scale * scale_;
    final_size_.x
      = (area_size.x * width_percent_ * scale_.x) / 100.0f
        + (width_ * screen_height_ / 720.0f) * final_scale_.x;
    final_size_.y
      = (area_size.y * height_percent_ * scale_.y) / 100.0f
        + (height_ * screen_height_ / 720.0f) * final_scale_.y;
    final_origin_.x
      = (final_size_.x * origin_x_percent_) / 100.0f + origin_x_
        * screen_height_ * final_scale_.x / 720.0f;
    final_origin_.y
      = (final_size_.y * origin_y_percent_) / 100.0f + origin_y_
        * screen_height_ * final_scale_.y / 720.0f;
    final_rotation_ = area_rotation + rotation_;

    // Scissor update
    if (local_scissor_ == true){
        float local_x1
          = final_size_.x * scissor_x_percent_top_ / 100.0f
            + scissor_x_top_ * screen_height_ * final_scale_.x / 720.0f
            - final_origin_.x;
        float local_y1
          = final_size_.y * scissor_y_percent_left_ / 100.0f
            + scissor_y_left_ * screen_height_ * final_scale_.y / 720.0f
            - final_origin_.y;
        float local_x2
          = final_size_.x * scissor_x_percent_bottom_ / 100.0f
            + scissor_x_bottom_ * screen_height_ * final_scale_.x / 720.0f
            - final_origin_.x;
        float local_y2
          = final_size_.y * scissor_y_percent_right_ / 100.0f
            + scissor_y_right_ * screen_height_ * final_scale_.y / 720.0f
            - final_origin_.y;
        float x = final_translate_.x;
        float y = final_translate_.y;

        int x1, y1, x2, y2, x3, y3, x4, y4;
        if (final_rotation_ != 0){
            float cos
              = Ogre::Math::Cos(Ogre::Radian(Ogre::Degree(final_rotation_)));
            float sin
              = Ogre::Math::Sin(Ogre::Radian(Ogre::Degree(final_rotation_)));
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
        scissor_ = true;
        scissor_top_ = std::min(std::min(y1 , y2), std::min(y3 , y4));
        scissor_bottom_ = std::max(std::max(y1 , y2), std::max(y3 , y4));
        scissor_left_ = std::min(std::min(x1 , x2), std::min(x3 , x4));
        scissor_right_ = std::max(std::max(x1 , x2), std::max(x3 , x4));
    }
    if (parent_ != NULL && global_scissor_ == true){
        bool use_scissor;
        Ogre::Vector4 scissor = parent_->GetFinalScissor(use_scissor);
        if (use_scissor == true){
            scissor_ = true;
            if (local_scissor_ == true){
                scissor_top_ = std::max((float) scissor_top_, scissor.x);
                scissor_bottom_ = std::min((float) scissor_bottom_, scissor.y);
                scissor_left_ = std::max((float) scissor_left_, scissor.z);
                scissor_right_ = std::min((float) scissor_right_, scissor.w);
            }
            else{
                scissor_top_ = scissor.x;
                scissor_bottom_ = scissor.y;
                scissor_left_ = scissor.z;
                scissor_right_ = scissor.w;
            }
        }
    }

    update_transformation_ = false;
}

void UiWidget::SetAlign(const UiWidget::Align align){align_ = align;}

void UiWidget::SetVerticalAlign(const UiWidget::VerticalAlign valign){
    vertical_align_ = valign;
}

float UiWidget::GetFinalZ() const{return final_z_;}

Ogre::Vector2 UiWidget::GetFinalOrigin() const{return final_origin_;}

Ogre::Vector2 UiWidget::GetFinalTranslate() const{return final_translate_;}

Ogre::Vector2 UiWidget::GetFinalSize() const{return final_size_;}

Ogre::Vector2 UiWidget::GetFinalScale() const{return final_scale_;}

Ogre::Vector4 UiWidget::GetFinalScissor(bool& scissor) const{
    if (scissor_ == true){
        scissor = true;
        return Ogre::Vector4(
          scissor_top_, scissor_bottom_, scissor_left_, scissor_right_
        );
    }
    scissor = false;
    return Ogre::Vector4::ZERO;
}

float UiWidget::GetFinalRotation() const{return final_rotation_;}

void UiWidget::SetOriginX(const float percent, const float x){
    origin_x_percent_ = percent;
    origin_x_ = x;
    SetUpdateTransformation();
}

void UiWidget::SetOriginY(const float percent, const float y){
    origin_y_percent_ = percent;
    origin_y_ = y;
    SetUpdateTransformation();
}

void UiWidget::SetX(const float percent, const float x){
    x_percent_ = percent;
    x_ = x;
    SetUpdateTransformation();
}

void UiWidget::GetX(float& percent, float& x){
    percent = x_percent_;
    x = x_;
}

void UiWidget::SetY(const float percent, const float y){
    y_percent_ = percent;
    y_ = y;
    SetUpdateTransformation();
}

void UiWidget::GetY(float& percent, float& y){
    percent = y_percent_;
    y = y_;
}

void UiWidget::SetZ(const float z){
    z_ = z;
    SetUpdateTransformation();
}

void UiWidget::SetWidth(const float percent, const float width){
    width_percent_ = percent;
    width_ = width;
    SetUpdateTransformation();
}

void UiWidget::GetWidth(float& percent, float& width){
    percent = width_percent_;
    width = width_;
}

void UiWidget::SetHeight(const float percent, const float height){
    height_percent_ = percent;
    height_ = height;
    SetUpdateTransformation();
}

void UiWidget::GetHeight(float& percent, float& height){
    percent = height_percent_;
    height = height_;
}

void UiWidget::SetScale(const Ogre::Vector2& scale){
    scale_ = scale;
    SetUpdateTransformation();
}

void UiWidget::SetRotation(const float degree){
    rotation_ = degree;
    SetUpdateTransformation();
}

void UiWidget::SetScissorArea(
  const float percent_x1, const float x1, const float percent_y1,
  const float y1, const float percent_x2, const float x2,
  const float percent_y2, const float y2
){
    local_scissor_ = true;
    scissor_x_percent_top_ = percent_x1;
    scissor_x_top_ = x1;
    scissor_x_percent_bottom_ = percent_x2;
    scissor_x_bottom_ = x2;
    scissor_y_percent_left_ = percent_y1;
    scissor_y_left_ = y1;
    scissor_y_percent_right_ = percent_y2;
    scissor_y_right_ = y2;
    SetUpdateTransformation();
}

void UiWidget::SetGlobalScissor(const bool global){
    global_scissor_ = global;
    SetUpdateTransformation();
}

void UiWidget::SetColour(const float r, const float g, const float b){
    colour_1_.r = r; colour_1_.g = g; colour_1_.b = b;
    colour_2_.r = r; colour_2_.g = g; colour_2_.b = b;
    colour_3_.r = r; colour_3_.g = g; colour_3_.b = b;
    colour_4_.r = r; colour_4_.g = g; colour_4_.b = b;
    SetUpdateTransformation();
}

void UiWidget::SetColours(
  const float r1, const float g1, const float b1,
  const float r2, const float g2, const float b2,
  const float r3, const float g3, const float b3,
  const float r4, const float g4, const float b4
){
    colour_1_.r = r1; colour_1_.g = g1; colour_1_.b = b1;
    colour_2_.r = r2; colour_2_.g = g2; colour_2_.b = b2;
    colour_3_.r = r3; colour_3_.g = g3; colour_3_.b = b3;
    colour_4_.r = r4; colour_4_.g = g4; colour_4_.b = b4;
    SetUpdateTransformation();
}

void UiWidget::SetAlpha(const float a){
    colour_1_.a = a;
    colour_2_.a = a;
    colour_3_.a = a;
    colour_4_.a = a;
    SetUpdateTransformation();
}

void UiWidget::SetText(const char* text){}

float UiWidget::ScriptGetWidth(){return width_;}

void UiWidget::ScriptSetWidth(float width){
    width_ = width;
    SetUpdateTransformation();
}

float UiWidget::ScriptGetHeight(){return width_;}

void UiWidget::ScriptSetHeight(float height){
    height_ = height;
    SetUpdateTransformation();
}
