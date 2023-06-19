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

#include <string>
#include <iostream>
#include <OgreBlendMode.h>
#include <OgreCommon.h>
#include <Overlay/OgreFontManager.h>
#include <Overlay/OgreUTFString.h>
#include <OgreHardwareBuffer.h>
#include <OgreHardwareBufferManager.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include "core/Logger.h"
#include "core/UiManager.h"
#include "core/UiTextArea.h"
#include "core/Timer.h"
#include "core/UiSprite.h"
#include "core/Utilites.h"
#include "TextHandler.h"

UiTextArea::UiTextArea(const Ogre::String& name): UiWidget(name){
    Initialise();
}

UiTextArea::UiTextArea(const Ogre::String& name, const Ogre::String& path_name, UiWidget* parent):
  UiWidget(name, path_name, parent)
{Initialise();}

UiTextArea::~UiTextArea(){DestroyVertexBuffer();}

void UiTextArea::Initialise(){
    font_ = nullptr;
    max_letters_ = 0;
    text_align_ = UiTextArea::LEFT;
    text_limit_ = 0;
    text_print_speed_ = -1; // -1 instant
    text_print_speed_mod_ = 1;
    text_state_ = TS_DONE;
    text_y_offset_ = 0;
    text_y_offset_target_ = 0;
    pause_time_ = 0;
    next_page_start_ = 0;
    padding_top_ = 0;
    padding_right_ = 0;
    padding_bottom_ = 0;
    padding_left_ = 0;
    next_pressed_ = false;
    next_repeated_ = false;
    timer_ = false;
    timer_time_ = 0;
    TextVariable var;
    var.name = "UITextAreaTimer";
    var.value = "00:00";
    text_variable_.push_back(var);
    scene_manager_ = Ogre::Root::getSingleton().getSceneManager("Scene");
    render_system_ = Ogre::Root::getSingletonPtr()->getRenderSystem();
    CreateVertexBuffer();
}

void UiTextArea::Update(){
    if (visible_ == true){
        if (timer_ == true){
            int time = Timer::getSingleton().GetGameTimer();
            if (time > 5999) time = 5999; // 99:59

            if (time != timer_time_){
                timer_time_ = time;
                Ogre::String time_string = "";
                time_string += Ogre::StringUtil::format("%2d", timer_time_ / 60);
                time_string += (timer_time_ & 1) ? ":" : ";";
                time_string += Ogre::StringUtil::format("%2d", timer_time_ % 60);
                SetVariable("UITextAreaTimer", time_string);
            }
        }
        switch(text_state_){
            case TS_SHOW_TEXT:
                if (text_print_speed_ == -1) text_limit_ = max_letters_;
                else{
                    float time = Timer::getSingleton().GetGameTimeDelta() * 30;
                    float addition;
                    float limit;
                    if (next_pressed_ == true || next_repeated_ == true)
                        text_print_speed_mod_ += (text_print_speed_mod_ >= 128) ? 0 : 1.0f * time;
                    else text_print_speed_mod_ -= (text_print_speed_mod_ <= 1) ? 0 : 1.0f * time;
                    if (text_print_speed_ < 128){
                        addition = ((128 - text_print_speed_) / 32) + 2;
                        limit = 1;
                    }
                    else{
                        addition = 2;
                        limit = ((text_print_speed_ - 128) / 32) + 1;
                    }
                    float char_to_add = (limit * text_print_speed_mod_ / 16 + addition) / limit;
                    text_limit_ += time * char_to_add;
                }
                update_transformation_ = true;
                break;
            case TS_SCROLL_TEXT:
                text_y_offset_
                  -= font_->GetHeight()
                  * Timer::getSingleton().GetGameTimeDelta()
                  / text_scroll_time_;
                if (text_y_offset_ <= text_y_offset_target_){
                    text_y_offset_ = text_y_offset_target_;
                    text_print_speed_mod_ = 1;
                    text_state_ = TS_SHOW_TEXT;
                }
                update_transformation_ = true;
                break;
            case TS_PAUSE_OK:
                if (next_pressed_ == true){
                    text_print_speed_mod_ = 1;
                    text_state_ = TS_SHOW_TEXT;
                }
                break;
            case TS_PAUSE_TIME:
                pause_time_ -= Timer::getSingleton().GetGameTimeDelta();
                if (pause_time_ <= 0){
                    text_print_speed_mod_ = 1;
                    text_state_ = TS_SHOW_TEXT;
                }
                break;
            case TS_OVERFLOW:
                if (next_pressed_ == true){
                    text_y_offset_target_ = text_y_offset_ - font_->GetHeight();
                    text_state_ = TS_SCROLL_TEXT;
                }
                break;
            case TS_NEXT_PAGE:
                if (next_pressed_ == true){
                    RemoveSpritesFromText(next_page_start_);
                    text_.erase(text_.begin(), text_.begin() + next_page_start_);
                    text_limit_ = 0;
                    text_print_speed_mod_ = 1;
                    text_y_offset_ = 0;
                    text_y_offset_target_ = 0;
                    text_state_ = TS_SHOW_TEXT;
                }
                break;
        }
    }
    next_pressed_ = false;
    next_repeated_ = false;
    UiWidget::Update();
}

void UiTextArea::Render(){
    if (update_transformation_ == false && visible_ == true){
        if (render_operation_.vertexData->vertexCount != 0){
            auto render_parameters = render_system_->getFixedFunctionParams(
              Ogre::TVC_NONE, Ogre::FOG_NONE
            );
            render_parameters->setConstant(
              Ogre::GpuProgramParameters::ACT_WORLD_MATRIX, Ogre::Matrix4::IDENTITY
            );
            render_parameters->setConstant(
              Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX, Ogre::Matrix4::IDENTITY
            );
            render_parameters->setConstant(
              Ogre::GpuProgramParameters::ACT_VIEW_MATRIX, Ogre::Matrix4::IDENTITY
            );
            render_system_->applyFixedFunctionParams(render_parameters, Ogre::GPV_GLOBAL);
            scene_manager_->_setPass(material_->getTechnique(0)->getPass(0), true, false);
            render_system_->setScissorTest(true, Ogre::Rect(
              scissor_left_, scissor_top_, scissor_right_, scissor_bottom_
            ));
            render_system_->_render(render_operation_);
            render_system_->setScissorTest(false);
        }
    }
    UiWidget::Render();
}

void UiTextArea::UpdateTransformation(){
    UiWidget::UpdateTransformation();
    UpdateGeometry();
    update_transformation_ = false;
}

void UiTextArea::InputPressed(){next_pressed_ = true;}

void UiTextArea::InputRepeated(){next_repeated_ = true;}

void UiTextArea::SetTextAlign(const TextAlign align){
    text_align_ = align;
    update_transformation_ = true;
}

void UiTextArea::SetPadding(
  const float top, const float right, const float bottom, const float left
){
    padding_top_ = top;
    padding_right_ = right;
    padding_bottom_ = bottom;
    padding_left_ = left;
}

void UiTextArea::SetText(const char* text){
    // Don't call PrepareTextFromNode from here!
    // Use the non-recursive SetTextFromNode.
    Ogre::UTFString str_text = Ogre::UTFString(text);
    text_.clear();
    TiXmlDocument doc;
    Ogre::UTFString xml_text = "<container>" + str_text + "</container>";
    doc.Parse(xml_text.asUTF8_c_str(), 0, TIXML_ENCODING_UTF8);
    SetTextFromNode(doc.RootElement(), colour_1_);
    text_state_ = TS_SHOW_TEXT;
    update_transformation_ = true;
    if (text_.size() > max_letters_){
        text_.clear();
        LOG_ERROR(
          "Max number of text reached in '" + path_name_ + "'. Can't render text from node. "
          + "Max number of letters is " + Ogre::StringConverter::toString(max_letters_) + "."
        );
    }
    text_state_ = TS_SHOW_TEXT;
    update_transformation_ = true;
}

void UiTextArea::SetText(const Ogre::UTFString& text){
    TiXmlDocument doc;
    Ogre::UTFString xml_text = "<container>" + text + "</container>";
    doc.Parse(xml_text.asUTF8_c_str(), 0, TIXML_ENCODING_UTF8);
    if (doc.Error() == true){
        LOG_ERROR("Can't parse text '" + text + "'. TinyXml Error: " + doc.ErrorDesc());
        return;
    }
    SetText(doc.RootElement());
}

void UiTextArea::SetText(TiXmlNode* text){
    if (text == NULL){
        LOG_ERROR("Text pointer is NULL.");
        return;
    }
    // Reload font if language was changed.
    if (font_ != NULL){
        Ogre::String language = font_->GetLanguage();
        if (language != "")
            if (TextHandler::getSingleton().GetLanguage() != language) SetFont(font_->GetName());
    }

    TextClear();
    PrepareTextFromNode(text, colour_1_);
    text_state_ = TS_SHOW_TEXT;
    update_transformation_ = true;
    if (text_.size() > max_letters_){
        text_.clear();
        LOG_ERROR(
          "Max number of text reached in '" + path_name_ + "'. Can't render text from node. "
          + "Max number of letters is " + Ogre::StringConverter::toString(max_letters_) + "."
        );
    }
}

void UiTextArea::TextClear(){
    RemoveSpritesFromText(text_.size());
    text_.clear();
    text_limit_ = 0;
    text_print_speed_mod_ = 1;
    text_y_offset_ = 0;
    text_y_offset_target_ = 0;
}

void UiTextArea::RemoveSpritesFromText(const unsigned int end){
    for (unsigned int i = 0; i < end; ++ i){
        if (text_[i].sprite != NULL){
            for (unsigned int j = 0; j < children_.size(); ++j){
                if (text_[i].sprite == children_[j]){
                    delete children_[j];
                    children_.erase(children_.begin() + j, children_.begin() + j + 1);
                    break;
                }
            }
        }
    }
}

void UiTextArea::SetFont(const Ogre::String& font){
    font_ = UiManager::getSingleton().GetFont(font);
    if (font_ == nullptr){
        LOG_ERROR("Could not find font '" + font + "' for '" + path_name_ + "'.");
        return;
    }
    material_ = Ogre::MaterialManager::getSingleton().getByName(
      "UiMaterials." + path_name_, "General"
    );
    if (material_ == nullptr){
        material_ = Ogre::MaterialManager::getSingleton().create(
          "UiMaterials." + path_name_, "General"
        );
    }
    Ogre::Pass* pass = material_->getTechnique(0)->getPass(0);
    pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
    pass->setCullingMode(Ogre::CULL_NONE);
    pass->setDepthCheckEnabled(false);
    pass->setDepthWriteEnabled(false);
    pass->setLightingEnabled(false);
    pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    pass->setAlphaRejectFunction(Ogre::CMPF_GREATER);
    pass->setAlphaRejectValue(0);
    Ogre::TextureUnitState* tex;
    if (pass->getTextureUnitStates().size() == 0)
        tex = pass->createTextureUnitState();
    else tex = pass->getTextureUnitState(0);//createTextureUnitState();
    tex->setTextureName(font_->GetImageName());
    tex->setNumMipmaps(-1);
    tex->setTextureFiltering(Ogre::TFO_NONE);
    update_transformation_ = true;
}

const UiFont* UiTextArea::GetFont() const{return font_;}

void UiTextArea::SetTextPrintSpeed(const float speed){text_print_speed_ = speed;}

void UiTextArea::SetTextScrollTime(const float time){text_scroll_time_ = time;}

void UiTextArea::SetVariable(const Ogre::String& name, const Ogre::UTFString& value){
    if (name == "") return;
    bool update = false;
    for (unsigned int i = 0; i < text_variable_.size(); ++ i){
        if (text_variable_[i].name == name){
            text_variable_[i].value = value;
            update = true;
            for (unsigned int j = 0; j < text_.size(); ++ j){
                if (text_[j].variable == name){
                    text_.erase(
                      text_.begin() + j + 1, text_.begin() + j + 1 + text_[j].variable_len
                    );
                }
            }
        }
    }
    if (update == false){
        TextVariable var;
        var.name = name;
        var.value = value;
        text_variable_.push_back(var);
    }

    for (unsigned int i = 0; i < text_.size(); ++ i){
        if (text_[i].variable == name){
            text_[i].variable_len = value.size();
            unsigned int j = 0;
            for (; j < text_[i].variable_len; ++ j){
                TextChar text_char;
                text_char.char_code = value[j];
                text_char.colour = text_[i].colour;
                text_.insert(text_.begin() + i + 1 + j, text_char);
            }
            i += j;
        }
    }
    update_transformation_ = true;
}

Ogre::UTFString UiTextArea::GetVariable(const Ogre::String& name) const{
    for (unsigned int i = 0; i < text_variable_.size(); ++ i)
        if (text_variable_[i].name == name) return text_variable_[i].value;
    return "";
}

TextState UiTextArea::GetTextState() const{return text_state_;}

float UiTextArea::GetTextLimit() const{return text_limit_;}

unsigned int UiTextArea::GetTextSize() const{return text_.size();}

float UiTextArea::GetPauseTime() const{return pause_time_;}

void UiTextArea::UpdateGeometry(){
    if (font_ == NULL){
        LOG_ERROR("Font for '" + path_name_ + "' if not set.");
        return;
    }
    float width = 0;
    if (text_align_ != LEFT){
        width = GetTextWidth();
        if (text_align_ == CENTER) width /= 2;
    }
    float* write_iterator
      = static_cast<float*>(vertex_buffer_->lock(Ogre::HardwareBuffer::HBL_NORMAL));
    render_operation_.vertexData->vertexCount = 0;
    float local_x_start
      = -final_origin_.x - (width - padding_left_) * final_scale_.x * screen_height_ / 720.0f;
    float local_x1 = local_x_start;
    float local_y1 = -final_origin_.y + ((text_y_offset_ + padding_top_)
      * final_scale_.y * screen_height_ / 720.0f);
    float local_x2;
    float local_y2;
    float x = final_translate_.x;
    float y = final_translate_.y;
    unsigned int i = 0;
    for (; (i < text_limit_) && (i < text_.size()); ++ i){
        if (text_[i].skip == true) continue;
        else if (text_[i].pause_ok == true){
            text_[i].skip = true;
            text_state_ = TS_PAUSE_OK;
            break;
        }
        else if (text_[i].pause_time != 0){
            text_[i].skip = true;
            pause_time_ = text_[i].pause_time;
            text_state_ = TS_PAUSE_TIME;
            break;
        }
        else if (text_[i].next_page == true){
            text_[i].skip = true;
            next_page_start_ = i + 1;
            text_state_ = TS_NEXT_PAGE;
            break;
        }
        else if (text_[i].sprite != NULL){
            float width_percent = 0;
            float img_width = 0;
            text_[i].sprite->GetWidth(width_percent, img_width);
            text_[i].sprite->SetX(0, local_x1 / (final_scale_.x * screen_height_ / 720.0f));
            local_x1 += img_width * final_scale_.x * screen_height_ / 720.0f;
            float height_percent = 0;
            float img_height = 0;
            text_[i].sprite->GetHeight(height_percent, img_height);
            text_[i].sprite->SetY(
              0, text_[i].sprite_y + (local_y1 / (final_scale_.y * screen_height_ / 720.0f))
            );
            text_[i].sprite->SetVisible(true);
            text_[i].sprite->UpdateGeometry();
            continue;
        }
        UiCharData char_data = font_->GetCharData(text_[i].char_code);
        Ogre::ColourValue colour = text_[i].colour;
        if (char_data.char_code == 10){ // LF (line feed): \n
            local_x1 = local_x_start;
            local_y1 += font_->GetHeight() * final_scale_.y * screen_height_ / 720.0f;
            continue;
        }
        local_x1 += char_data.pre * final_scale_.x * screen_height_ / 720.0f;
        local_x2 = local_x1 + char_data.width * final_scale_.x * screen_height_ / 720.0f;
        local_y2 = local_y1 + char_data.height * final_scale_.y * screen_height_ / 720.0f;
        if (
          local_x2 + char_data.post * final_scale_.x * screen_height_ / 720.0f
          > final_size_.x - padding_right_ * final_scale_.x * screen_height_ / 720.0f
        ){
            local_x1 = local_x_start;
            local_x2 = local_x1 + char_data.width * final_scale_.x * screen_height_ / 720.0f;
            local_y1 += font_->GetHeight() * final_scale_.y * screen_height_ / 720.0f;
            local_y2 = local_y1 + char_data.height * final_scale_.y * screen_height_ / 720.0f;
        }

        // If the lower border of the textarea is crossed:
        // generate event and stop rendering
        if (local_y2 > final_size_.y - padding_bottom_ * final_scale_.y * screen_height_ / 720.0f){
            if (text_state_ != TS_SCROLL_TEXT) text_state_ = TS_OVERFLOW;
            break;
        }
        int x1, y1, x2, y2, x3, y3, x4, y4;
        if (final_rotation_ != 0){
            float cos = Ogre::Math::Cos(Ogre::Radian(Ogre::Degree(final_rotation_)));
            float sin = Ogre::Math::Sin(Ogre::Radian(Ogre::Degree(final_rotation_)));
            x1 = local_x1 * cos - local_y1 * sin + x;
            y1 = local_x1 * sin + local_y1 * cos + y;
            x2 = local_x2 * cos - local_y1 * sin + x;
            y2 = local_x2 * sin + local_y1 * cos + y;
            x3 = local_x2 * cos - local_y2 * sin + x;
            y3 = local_x2 * sin + local_y2 * cos + y;
            x4 = local_x1 * cos - local_y2 * sin + x;
            y4 = local_x1 * sin + local_y2 * cos + y;
        }
        else{
            x1 = local_x1 + x;
            y1 = local_y1 + y;
            x2 = local_x2 + x;
            y2 = local_y1 + y;
            x3 = local_x2 + x;
            y3 = local_y2 + y;
            x4 = local_x1 + x;
            y4 = local_y2 + y;
        }
        float new_x1 = (x1 / screen_width_) * 2 - 1;
        float new_y1 = -((y1 / screen_height_) * 2 - 1);
        float new_x2 = (x2 / screen_width_) * 2 - 1;
        float new_y2 = -((y2 / screen_height_) * 2 - 1);
        float new_x3 = (x3 / screen_width_) * 2 - 1;
        float new_y3 = -((y3 / screen_height_) * 2 - 1);
        float new_x4 = (x4 / screen_width_) * 2 - 1;
        float new_y4 = -((y4 / screen_height_) * 2 - 1);
        local_x1 += (char_data.width + char_data.post) * final_scale_.x * screen_height_ / 720.0f;
        float img_width = font_->GetImageWidth();
        float img_height = font_->GetImageHeight();
        float left = (float)char_data.x / img_width;
        float right = (float)(char_data.x + char_data.width) / img_width;
        float top = (float)char_data.y / img_height;
        float bottom = (float)(char_data.y + char_data.height) / img_height;

        *write_iterator ++ = new_x1;
        *write_iterator ++ = new_y1;
        *write_iterator ++ = final_z_;
        *write_iterator ++ = colour.r;
        *write_iterator ++ = colour.g;
        *write_iterator ++ = colour.b;
        *write_iterator ++ = colour.a;
        *write_iterator ++ = left;
        *write_iterator ++ = top;

        *write_iterator ++ = new_x2;
        *write_iterator ++ = new_y2;
        *write_iterator ++ = final_z_;
        *write_iterator ++ = colour.r;
        *write_iterator ++ = colour.g;
        *write_iterator ++ = colour.b;
        *write_iterator ++ = colour.a;
        *write_iterator ++ = right;
        *write_iterator ++ = top;

        *write_iterator ++ = new_x3;
        *write_iterator ++ = new_y3;
        *write_iterator ++ = final_z_;
        *write_iterator ++ = colour.r;
        *write_iterator ++ = colour.g;
        *write_iterator ++ = colour.b;
        *write_iterator ++ = colour.a;
        *write_iterator ++ = right;
        *write_iterator ++ = bottom;

        *write_iterator ++ = new_x1;
        *write_iterator ++ = new_y1;
        *write_iterator ++ = final_z_;
        *write_iterator ++ = colour.r;
        *write_iterator ++ = colour.g;
        *write_iterator ++ = colour.b;
        *write_iterator ++ = colour.a;
        *write_iterator ++ = left;
        *write_iterator ++ = top;

        *write_iterator ++ = new_x3;
        *write_iterator ++ = new_y3;
        *write_iterator ++ = final_z_;
        *write_iterator ++ = colour.r;
        *write_iterator ++ = colour.g;
        *write_iterator ++ = colour.b;
        *write_iterator ++ = colour.a;
        *write_iterator ++ = right;
        *write_iterator ++ = bottom;

        *write_iterator ++ = new_x4;
        *write_iterator ++ = new_y4;
        *write_iterator ++ = final_z_;
        *write_iterator ++ = colour.r;
        *write_iterator ++ = colour.g;
        *write_iterator ++ = colour.b;
        *write_iterator ++ = colour.a;
        *write_iterator ++ = left;
        *write_iterator ++ = bottom;

        render_operation_.vertexData->vertexCount += 6;
    }
    vertex_buffer_->unlock();
    if (i == text_.size()) text_state_ = TS_DONE;
}

float UiTextArea::GetTextWidth() const{
    float width = 0;
    float width_max = 0;
    for (unsigned int i = 0; i < text_.size(); ++ i){
        UiCharData char_data = font_->GetCharData(text_[i].char_code);
        // If newline, store previous row max width
        if (char_data.char_code == 10){
            width_max = (width > width_max) ? width : width_max;
            width = 0;
        }
        else width += char_data.pre + char_data.width + char_data.post;
    }
    return (width > width_max) ? width : width_max;
}

void UiTextArea::PrepareTextFromNode(TiXmlNode* node, const Ogre::ColourValue& colour){

    while (node != NULL){
        switch(node->Type()){
            case TiXmlNode::TINYXML_TEXT:
                {
                    TiXmlText* childText = node->ToText();
                    if (childText) PrepareTextFromText(childText->Value(), colour);
                }
                break;
            case TiXmlNode::TINYXML_ELEMENT:
                {
                    Ogre::ColourValue colour_child = colour;
                    Ogre::String name = node->ValueStr();
                    if (name == "colour"){
                        colour_child = Ogre::StringConverter::parseColourValue(
                          node->ToElement()->Attribute("value")
                        );
                    }
                    else if (name == "pause_ok"){
                        TextChar new_char;
                        new_char.pause_ok = true;
                        text_.push_back(new_char);
                    }
                    else if (name == "pause"){
                        const std::string* string
                          = node->ToElement()->Attribute(Ogre::String("time"));
                        if (string != NULL){
                            TextChar new_char;
                            new_char.pause_time = Ogre::StringConverter::parseReal(*string);
                            text_.push_back(new_char);
                        }
                    }
                    else if (name == "next_page"){
                        TextChar new_char;
                        new_char.next_page = true;
                        text_.push_back(new_char);
                    }
                    else if (name == "timer"){
                        timer_ = true;
                        TextChar new_char;
                        new_char.skip = true;
                        new_char.variable = "UITextAreaTimer";
                        new_char.colour = colour;
                        Ogre::UTFString var = GetVariable("UITextAreaTimer");
                        new_char.variable_len = var.size();
                        text_.push_back(new_char);
                        for (unsigned int i = 0; i < var.size(); ++ i){
                            TextChar text_char;
                            text_char.char_code = var[i];
                            text_char.colour = colour;
                            text_.push_back(text_char);
                        }
                    }
                    else if (name == "variable"){
                        const std::string* string
                          = node->ToElement()->Attribute(Ogre::String("name"));
                        if (string != NULL){
                            TextChar new_char;
                            new_char.skip = true;
                            new_char.variable = *string;
                            new_char.colour = colour;
                            Ogre::UTFString var = GetVariable(*string);
                            new_char.variable_len = var.size();
                            text_.push_back(new_char);
                            for (unsigned int i = 0; i < var.size(); ++ i){
                                TextChar text_char;
                                text_char.char_code = var[i];
                                text_char.colour = colour;
                                text_.push_back(text_char);
                            }
                        }
                    }
                    else if (name == "character"){
                        const std::string* id = node->ToElement()->Attribute(Ogre::String("id"));
                        const std::string char_name = TextHandler::getSingleton().GetCharacterName(
                          std::stoi(*id)
                        );
                        for (unsigned int i = 0; i < char_name.length(); ++ i){
                            TextChar text_char;
                            text_char.char_code = char_name.at(i);
                            text_char.colour = colour;
                            text_.push_back(text_char);
                        }
                    }
                    else if (name == "party"){
                        const std::string* pos = node->ToElement()->Attribute(Ogre::String("pos"));
                        const std::string char_name
                          = TextHandler::getSingleton().GetPartyCharacterName(std::stoi(*pos));
                        for (unsigned int i = 0; i < char_name.length(); ++ i){
                            TextChar text_char;
                            text_char.char_code = char_name.at(i);
                            text_char.colour = colour;
                            text_.push_back(text_char);
                        }
                    }
                    else if (name == "include"){
                        const std::string* text_name = node->ToElement()->Attribute(
                          Ogre::String("name")
                        );
                        if (text_name != NULL){
                            TiXmlNode* text = TextHandler::getSingleton().GetText(*text_name);
                            if (text != NULL) PrepareTextFromNode(text, colour_child);
                        }
                    }
                    else if (name == "image"){
                        Ogre::String name1 = GetString(node, "sprite");
                        if (name1 != ""){
                            TiXmlNode* sprites = UiManager::getSingleton().GetPrototype(
                              "TextAreaSprite"
                            );
                            if (sprites != NULL){
                                sprites = sprites->FirstChild();
                                while (sprites != NULL){
                                    if (
                                      sprites->Type() == TiXmlNode::TINYXML_ELEMENT
                                      && sprites->ValueStr() == "sprite"
                                    ){
                                        Ogre::String name2 = GetString(sprites, "name");
                                        if (name1 == name2){
                                            TextChar new_char;
                                            UiSprite* sprite = new UiSprite(
                                              name1, name_ + "." + name1, this
                                            );
                                            Ogre::String image = GetString(sprites, "image");
                                            if (image != "") sprite->SetImage(image);
                                            Ogre::String y_str = GetString(sprites, "y");
                                            if (y_str != ""){
                                                float y_percent = 0;
                                                float y = 0;
                                                ParsePercent(y_percent, y, y_str);
                                                new_char.sprite_y = y;
                                            }
                                            Ogre::String width_str = GetString(sprites, "width");
                                            if (width_str != ""){
                                                float width_percent = 0;
                                                float width = 0;
                                                ParsePercent(width_percent, width, width_str);
                                                sprite->SetWidth(0, width);
                                            }
                                            Ogre::String height_str = GetString(sprites, "height");
                                            if (height_str != ""){
                                                float height_percent = 0;
                                                float height = 0;
                                                ParsePercent(height_percent, height, height_str);
                                                sprite->SetHeight(height_percent, height);
                                            }
                                            sprite->SetVisible(false);
                                            AddChild(sprite);
                                            new_char.sprite = sprite;
                                            text_.push_back(new_char);
                                            break;
                                        }
                                    }
                                    sprites = sprites->NextSibling();
                                }
                            }
                        }
                    }
                    TiXmlNode* node_child = node->FirstChild();
                    if (node_child != NULL) PrepareTextFromNode(node_child, colour_child);
                }
            break;
        }
        node = node->NextSibling();
    }
}

void UiTextArea::SetTextFromNode(TiXmlNode* node, const Ogre::ColourValue& colour){
    switch(node->Type()){
        case TiXmlNode::TINYXML_TEXT:
            {
                TiXmlText* childText = node->ToText();
                if (childText) PrepareTextFromText(childText->Value(), colour);
            }
            break;
        case TiXmlNode::TINYXML_ELEMENT:
            {
                Ogre::ColourValue colour_child = colour;
                Ogre::String name = node->ValueStr();
                if (name == "colour"){
                    colour_child = Ogre::StringConverter::parseColourValue(
                      node->ToElement()->Attribute("value")
                    );
                }
                else if (name == "pause_ok"){
                    TextChar new_char;
                    new_char.pause_ok = true;
                    text_.push_back(new_char);
                }
                else if (name == "pause"){
                    const std::string* string
                      = node->ToElement()->Attribute(Ogre::String("time"));
                    if (string != NULL){
                        TextChar new_char;
                        new_char.pause_time = Ogre::StringConverter::parseReal(*string);
                        text_.push_back(new_char);
                    }
                }
                else if (name == "next_page"){
                    TextChar new_char;
                    new_char.next_page = true;
                    text_.push_back(new_char);
                }
                else if (name == "timer"){
                    timer_ = true;
                    TextChar new_char;
                    new_char.skip = true;
                    new_char.variable = "UITextAreaTimer";
                    new_char.colour = colour;
                    Ogre::UTFString var = GetVariable("UITextAreaTimer");
                    new_char.variable_len = var.size();
                    text_.push_back(new_char);
                    for (unsigned int i = 0; i < var.size(); ++ i){
                        TextChar text_char;
                        text_char.char_code = var[i];
                        text_char.colour = colour;
                        text_.push_back(text_char);
                    }
                }
                else if (name == "variable"){
                    const std::string* string
                      = node->ToElement()->Attribute(Ogre::String("name"));
                    if (string != NULL){
                        TextChar new_char;
                        new_char.skip = true;
                        new_char.variable = *string;
                        new_char.colour = colour;
                        Ogre::UTFString var = GetVariable(*string);
                        new_char.variable_len = var.size();
                        text_.push_back(new_char);
                        for (unsigned int i = 0; i < var.size(); ++ i){
                            TextChar text_char;
                            text_char.char_code = var[i];
                            text_char.colour = colour;
                            text_.push_back(text_char);
                        }
                    }
                }
                else if (name == "character"){
                    const std::string* id = node->ToElement()->Attribute(Ogre::String("id"));
                    const std::string char_name = TextHandler::getSingleton().GetCharacterName(
                      std::stoi(*id)
                    );
                    for (unsigned int i = 0; i < char_name.length(); ++ i){
                        TextChar text_char;
                        text_char.char_code = char_name.at(i);
                        text_char.colour = colour;
                        text_.push_back(text_char);
                    }
                }
                else if (name == "party"){
                    const std::string* pos = node->ToElement()->Attribute(Ogre::String("pos"));
                    const std::string char_name
                      = TextHandler::getSingleton().GetPartyCharacterName(std::stoi(*pos));
                    for (unsigned int i = 0; i < char_name.length(); ++ i){
                        TextChar text_char;
                        text_char.char_code = char_name.at(i);
                        text_char.colour = colour;
                        text_.push_back(text_char);
                    }
                }
                else if (name == "include"){
                    const std::string* text_name = node->ToElement()->Attribute(
                      Ogre::String("name")
                    );
                    if (text_name != NULL){
                        TiXmlNode* text = TextHandler::getSingleton().GetText(*text_name);
                        if (text != NULL) SetTextFromNode(text, colour_child);
                    }
                }
                else if (name == "image"){
                    Ogre::String name1 = GetString(node, "sprite");
                    if (name1 != ""){
                        TiXmlNode* sprites = UiManager::getSingleton().GetPrototype(
                          "TextAreaSprite"
                        );
                        if (sprites != NULL){
                            sprites = sprites->FirstChild();
                            while (sprites != NULL){
                                if (
                                  sprites->Type() == TiXmlNode::TINYXML_ELEMENT
                                  && sprites->ValueStr() == "sprite"
                                ){
                                    Ogre::String name2 = GetString(sprites, "name");
                                    if (name1 == name2){
                                        TextChar new_char;
                                        UiSprite* sprite = new UiSprite(
                                          name1, name_ + "." + name1, this
                                        );
                                        Ogre::String image = GetString(sprites, "image");
                                        if (image != "") sprite->SetImage(image);
                                        Ogre::String y_str = GetString(sprites, "y");
                                        if (y_str != ""){
                                            float y_percent = 0;
                                            float y = 0;
                                            ParsePercent(y_percent, y, y_str);
                                            new_char.sprite_y = y;
                                        }
                                        Ogre::String width_str = GetString(sprites, "width");
                                        if (width_str != ""){
                                            float width_percent = 0;
                                            float width = 0;
                                            ParsePercent(width_percent, width, width_str);
                                            sprite->SetWidth(0, width);
                                        }
                                        Ogre::String height_str = GetString(sprites, "height");
                                        if (height_str != ""){
                                            float height_percent = 0;
                                            float height = 0;
                                            ParsePercent(height_percent, height, height_str);
                                            sprite->SetHeight(height_percent, height);
                                        }
                                        sprite->SetVisible(false);
                                        AddChild(sprite);
                                        new_char.sprite = sprite;
                                        text_.push_back(new_char);
                                        break;
                                    }
                                }
                                sprites = sprites->NextSibling();
                            }
                        }
                    }
                }
                TiXmlNode* node_child = node->FirstChild();
                if (node_child != NULL) SetTextFromNode(node_child, colour_child);
            }
        break;
    }
}

void UiTextArea::PrepareTextFromText(
  const Ogre::UTFString& text, const Ogre::ColourValue& colour_child
){
    for (unsigned int i = 0; i < text.size(); ++ i){
        TextChar new_char;
        new_char.char_code = text[i];
        new_char.colour = colour_child;
        text_.push_back(new_char);
    };
}

void UiTextArea::CreateVertexBuffer(){
    max_letters_ = 1024;
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
      vertex_declaration->getVertexSize(0), max_letters_ * 6,
      Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, false
    );
    render_operation_.vertexData->vertexBufferBinding->setBinding(0, vertex_buffer_);
    render_operation_.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
    render_operation_.useIndexes = false;
}

void UiTextArea::DestroyVertexBuffer(){
    delete render_operation_.vertexData;
    render_operation_.vertexData = 0;
    vertex_buffer_.reset();
    max_letters_ = 0;
}
