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
#include "common/Logger.h"
#include "engine/ui/UiManager.h"
#include "engine/ui/UiTextArea.h"
#include "engine/ui/UiTextParser.h"
#include "engine/Timer.h"
#include "engine/ui/UiSprite.h"
#include "common/Utilities.h"
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
    text_scroll_time_ = 0;
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
    var.name = UiTextParser::TIMER_VARIABLE;
    var.value = "00:00";
    text_variable_.push_back(var);
    scene_manager_ = Ogre::Root::getSingleton().getSceneManager("Scene");
    render_system_ = Ogre::Root::getSingletonPtr()->getRenderSystem();
    CreateVertexBuffer();
}

void UiTextArea::Update(){
    if (visible_ == true){
        if (timer_ == true) UpdateTimer();
        switch(text_state_){
            case TS_SHOW_TEXT: UpdateShowText(); break;
            case TS_SCROLL_TEXT: UpdateScrollText(); break;
            case TS_PAUSE_OK: if (next_pressed_) ContinueText(); break;
            case TS_PAUSE_TIME: UpdatePauseTime(); break;
            case TS_OVERFLOW: if (next_pressed_) ScrollOneLine(); break;
            case TS_NEXT_PAGE: if (next_pressed_) ShowNextPage(); break;
            case TS_DONE: break;
        }
    }
    next_pressed_ = false;
    next_repeated_ = false;
    UiWidget::Update();
}

void UiTextArea::UpdateTimer(){
    int time = Timer::getSingleton().GetGameTimer();
    if (time > 5999) time = 5999; // 99:59
    if (time == timer_time_) return;
    timer_time_ = time;
    Ogre::String time_string = Ogre::StringUtil::format("%2d", timer_time_ / 60);
    time_string += (timer_time_ & 1) ? ":" : ";";
    time_string += Ogre::StringUtil::format("%2d", timer_time_ % 60);
    SetVariable(UiTextParser::TIMER_VARIABLE, time_string);
}

void UiTextArea::UpdateShowText(){
    if (text_print_speed_ == -1) text_limit_ = max_letters_;
    else{
        // Holding the button makes the text print faster.
        const float time = Timer::getSingleton().GetGameTimeDelta() * 30;
        if (next_pressed_ == true || next_repeated_ == true)
            text_print_speed_mod_ += (text_print_speed_mod_ >= 128) ? 0 : 1.0f * time;
        else text_print_speed_mod_ -= (text_print_speed_mod_ <= 1) ? 0 : 1.0f * time;
        float addition;
        float limit;
        if (text_print_speed_ < 128){
            addition = ((128 - text_print_speed_) / 32) + 2;
            limit = 1;
        }
        else{
            addition = 2;
            limit = ((text_print_speed_ - 128) / 32) + 1;
        }
        const float characters = (limit * text_print_speed_mod_ / 16 + addition) / limit;
        text_limit_ += time * characters;
    }
    update_transformation_ = true;
}

void UiTextArea::UpdateScrollText(){
    text_y_offset_
      -= font_->GetHeight() * Timer::getSingleton().GetGameTimeDelta() / text_scroll_time_;
    if (text_y_offset_ <= text_y_offset_target_){
        text_y_offset_ = text_y_offset_target_;
        ContinueText();
    }
    update_transformation_ = true;
}

void UiTextArea::UpdatePauseTime(){
    pause_time_ -= Timer::getSingleton().GetGameTimeDelta();
    if (pause_time_ <= 0) ContinueText();
}

void UiTextArea::ContinueText(){
    text_print_speed_mod_ = 1;
    text_state_ = TS_SHOW_TEXT;
}

void UiTextArea::ScrollOneLine(){
    text_y_offset_target_ = text_y_offset_ - font_->GetHeight();
    text_state_ = TS_SCROLL_TEXT;
}

void UiTextArea::ShowNextPage(){
    RemoveSpritesFromText(next_page_start_);
    text_.erase(text_.begin(), text_.begin() + next_page_start_);
    text_limit_ = 0;
    text_y_offset_ = 0;
    text_y_offset_target_ = 0;
    ContinueText();
}

void UiTextArea::Render(){
    // Scripts that run after the widgets update (like the battle) can change the text or the
    // position of a widget in the same frame it is drawn, so the geometry is refreshed here.
    if (update_transformation_ == true && visible_ == true) UpdateTransformation();
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

void UiTextArea::PutNumber(const std::string& number, const int digits){
    if (digits <= 0) return;
    int run_end = -1;
    for (size_t i = 0; i < text_.size();){
        if (text_[i].char_code == ' '){
            size_t j = i;
            while (j < text_.size() && text_[j].char_code == ' ') ++ j;
            if (static_cast<int>(j - i) >= digits) run_end = static_cast<int>(j);
            i = j;
        }
        else ++ i;
    }
    if (run_end < 0) return;
    std::string padded = number;
    while (static_cast<int>(padded.size()) < digits) padded = " " + padded;
    if (static_cast<int>(padded.size()) > digits) padded = padded.substr(padded.size() - digits);
    for (int k = 0; k < digits; ++ k) text_[run_end - digits + k].char_code = padded[k];
    update_transformation_ = true;
}

void UiTextArea::SetText(const char* text){
    // Unlike the other ways of setting the text, the one already shown is not cleared: what has
    // been printed is not printed again.
    text_.clear();
    TiXmlDocument doc;
    const Ogre::UTFString xml_text = "<container>" + Ogre::UTFString(text) + "</container>";
    doc.Parse(xml_text.asUTF8_c_str(), 0, TIXML_ENCODING_UTF8);
    if (doc.Error()){
        LOG_ERROR(
          "Can't parse text '" + Ogre::String(text) + "'. TinyXml Error: " + doc.ErrorDesc()
        );
    }
    // What could be read of an invalid text is shown.
    ShowText(doc.RootElement());
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
    if (text == nullptr){
        LOG_ERROR("Text pointer is NULL.");
        return;
    }
    // Reload font if language was changed.
    if (font_ != nullptr){
        const Ogre::String language = font_->GetLanguage();
        if (language != "" && TextHandler::getSingleton().GetLanguage() != language)
            SetFont(font_->GetName());
    }
    TextClear();
    ShowText(text);
}

void UiTextArea::ShowText(TiXmlNode* node){
    UiTextParser::Sources sources;
    sources.variable = [this](const Ogre::String& name){return GetVariable(name);};
    sources.character_name = [](int id){return TextHandler::getSingleton().GetCharacterName(id);};
    sources.party_name = [](int position){
        return TextHandler::getSingleton().GetPartyCharacterName(position);
    };
    sources.text = [](const Ogre::String& name){return TextHandler::getSingleton().GetText(name);};
    sources.image = [this](const Ogre::String& name, TextChar& character){
        return CreateImage(name, character);
    };
    UiTextParser parser(sources, text_);
    parser.Parse(node, colour_1_);
    if (parser.UsesTimer()) timer_ = true;
    text_state_ = TS_SHOW_TEXT;
    update_transformation_ = true;
    GrowVertexBuffer(text_.size());
    if (text_.size() > max_letters_){
        text_.clear();
        LOG_ERROR(
          "Max number of text reached in '" + path_name_ + "'. Can't render text from node. "
          + "Max number of letters is " + Ogre::StringConverter::toString(max_letters_) + "."
        );
    }
}

bool UiTextArea::CreateImage(const Ogre::String& name, TextChar& character){
    TiXmlNode* prototype = UiManager::getSingleton().GetPrototype("TextAreaSprite");
    if (prototype == nullptr) return false;
    for (TiXmlNode* node = prototype->FirstChild(); node != nullptr; node = node->NextSibling()){
        if (
          node->Type() != TiXmlNode::TINYXML_ELEMENT || node->ValueStr() != "sprite"
          || GetString(node, "name") != name
        ) continue;
        UiSprite* sprite = new UiSprite(name, name_ + "." + name, this);
        const Ogre::String image = GetString(node, "image");
        if (image != "") sprite->SetImage(image);
        const Ogre::String y = GetString(node, "y");
        if (y != ""){
            float percent = 0;
            ParsePercent(percent, character.sprite_y, y);
        }
        const Ogre::String width = GetString(node, "width");
        if (width != ""){
            float percent = 0;
            float pixels = 0;
            ParsePercent(percent, pixels, width);
            sprite->SetWidth(0, pixels);
        }
        const Ogre::String height = GetString(node, "height");
        if (height != ""){
            float percent = 0;
            float pixels = 0;
            ParsePercent(percent, pixels, height);
            sprite->SetHeight(percent, pixels);
        }
        sprite->SetVisible(false);
        AddChild(sprite);
        character.sprite = sprite;
        return true;
    }
    return false;
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
      "UiMaterials." + path_name_, "FONTS"
    );
    if (material_ == nullptr){
        material_ = Ogre::MaterialManager::getSingleton().create(
          "UiMaterials." + path_name_, "FONTS"
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
    if (font_ == nullptr){
        LOG_ERROR("Font for '" + path_name_ + "' if not set.");
        return;
    }
    float align_width = 0;
    if (text_align_ != LEFT){
        align_width = UiTextLayout::TextWidth(
          text_, [this](int code){return font_->GetCharData(code);}
        );
        if (text_align_ == CENTER) align_width /= 2;
    }
    const UiTextLayout::Area area{
      final_origin_, final_size_, final_scale_, screen_height_,
      padding_top_, padding_right_, padding_bottom_, padding_left_
    };
    UiTextLayout layout(area, align_width, text_y_offset_, font_->GetHeight());
    float* vertices = static_cast<float*>(vertex_buffer_->lock(Ogre::HardwareBuffer::HBL_NORMAL));
    render_operation_.vertexData->vertexCount = 0;
    unsigned int i = 0;
    for (; (i < text_limit_) && (i < text_.size()); ++ i){
        TextChar& character = text_[i];
        if (character.skip) continue;
        if (StopAtCommand(i)) break;
        if (character.sprite != nullptr){
            PlaceSprite(character, layout);
            continue;
        }
        const UiCharData data = font_->GetCharData(character.char_code);
        if (data.char_code == 10){ // LF (line feed): \n
            layout.NewLine();
            continue;
        }
        UiTextLayout::Box box;
        // The text goes below the area: the rest is not drawn.
        if (!layout.Place(data, box)){
            if (text_state_ != TS_SCROLL_TEXT) text_state_ = TS_OVERFLOW;
            break;
        }
        WriteCharacter(vertices, box, data, character.colour);
    }
    vertex_buffer_->unlock();
    if (i == text_.size()) text_state_ = TS_DONE;
}

bool UiTextArea::StopAtCommand(const unsigned int index){
    TextChar& character = text_[index];
    if (character.pause_ok) text_state_ = TS_PAUSE_OK;
    else if (character.pause_time != 0){
        pause_time_ = character.pause_time;
        text_state_ = TS_PAUSE_TIME;
    }
    else if (character.next_page){
        next_page_start_ = index + 1;
        text_state_ = TS_NEXT_PAGE;
    }
    else return false;
    // The command is done only once.
    character.skip = true;
    return true;
}

void UiTextArea::PlaceSprite(const TextChar& character, UiTextLayout& layout){
    float percent = 0;
    float width = 0;
    character.sprite->GetWidth(percent, width);
    float x = 0;
    float y = 0;
    layout.PlaceImage(width, x, y);
    character.sprite->SetX(0, x);
    character.sprite->SetY(0, character.sprite_y + y);
    character.sprite->SetVisible(true);
    character.sprite->UpdateGeometry();
}

void UiTextArea::WriteCharacter(
  float*& vertices, const UiTextLayout::Box& box, const UiCharData& data,
  const Ogre::ColourValue& colour
){
    const float width = font_->GetImageWidth();
    const float height = font_->GetImageHeight();
    const UiTextLayout::Box texture{
      static_cast<float>(data.x) / width, static_cast<float>(data.y) / height,
      static_cast<float>(data.x + data.width) / width,
      static_cast<float>(data.y + data.height) / height
    };
    const std::array<Ogre::Vector2, 4> corners = UiTextLayout::ScreenCorners(
      box, final_translate_, final_rotation_, Ogre::Vector2(screen_width_, screen_height_)
    );
    UiTextLayout::WriteQuad(vertices, corners, texture, final_z_, colour);
    render_operation_.vertexData->vertexCount += 6;
}

void UiTextArea::GrowVertexBuffer(const unsigned int letters){
    const unsigned int LIMIT = 8192;
    if (letters <= max_letters_ || letters > LIMIT || render_operation_.vertexData == nullptr) return;
    unsigned int size = max_letters_ > 0 ? max_letters_ : 1024;
    while (size < letters) size *= 2;
    DestroyVertexBuffer();
    CreateVertexBuffer(size);
}

void UiTextArea::CreateVertexBuffer(const unsigned int letters){
    max_letters_ = letters;
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
