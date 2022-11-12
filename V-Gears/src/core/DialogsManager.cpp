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

#include "core/DialogsManager.h"
#include "core/ConfigVar.h"
#include "core/DebugDraw.h"
#include "core/Logger.h"
#include "core/TextManager.h"

/**
 * Dialog manager singleton.
 */
template<>DialogsManager *Ogre::Singleton< DialogsManager >::msSingleton = NULL;

ConfigVar cv_debug_message("debug_message", "Draw message debug", "false");

Ogre::String message_state_string[5] = {
  "Closed", "Show Window", "Show Text", "Opened", "Hide Window"
};



DialogsManager::DialogsManager():
  next_pressed_(false),
  next_repeated_(false),
  up_pressed_(false),
  down_pressed_(false),
  limit_area_(NULL)
{LOG_TRIVIAL("DialogsManager created.");}

DialogsManager::~DialogsManager(){
    for (unsigned int i = 0; i < messages_.size(); ++i) delete messages_[i];
    LOG_TRIVIAL("DialogsManager destroyed.");
}

void DialogsManager::Initialise(){
    UiWidget* g_widget = UiManager::getSingletonPtr()->GetWidget("Dialog");
    if (g_widget == NULL){
        LOG_ERROR("Can't create DialogsManager because Widget 'Dialog' not founded.");
        return;
    }
    limit_area_ = g_widget->GetChild("LimitArea");
    if (limit_area_ == NULL){
        LOG_ERROR(
          "Can't create DialogsManager because Widget 'LimitArea' not founded in Widget 'Dialog'."
        );
        return;
    }
    unsigned int num = limit_area_->GetNumberOfChildren();
    for (unsigned int i = 0; i < num; ++i){
        UiWidget* widget = limit_area_->GetChild(i);
        UiTextArea* text_area = static_cast<UiTextArea*>(widget->GetChild("TextArea"));
        if (text_area == NULL){
            LOG_ERROR(
              "Can't create dialog '" +widget->GetName()
              + "' because because text_area with name 'TextArea' is missing."
            );
            continue;
        }
        MessageData* data = new MessageData();
        data->widget = widget;
        data->window = widget->GetChild("Window");
        data->text_area = text_area;
        //data->text_area->SetTextPrintSpeed(256);
        data->text_area->SetTextPrintSpeed(25);
        //data->text_area->SetTextScrollTime(0.267f);
        data->text_area->SetTextScrollTime(0.1f);
        data->cursor = widget->GetChild("Cursor");
        if (data->cursor != NULL) data->cursor->GetY(data->cursor_percent_y, data->cursor_y);
        messages_.push_back(data);
    }
    LOG_TRIVIAL("DialogsManager initialized.");
}

void DialogsManager::Input(const VGears::Event& input){
    if (input.type == VGears::ET_KEY_PRESS && input.event == "message_ok") next_pressed_ = true;
    else if (input.type == VGears::ET_KEY_REPEAT && input.event == "message_ok")
        next_repeated_ = true;
    else if (input.type == VGears::ET_KEY_PRESS && input.event == "message_up") up_pressed_ = true;
    else if (input.type == VGears::ET_KEY_PRESS && input.event == "message_down")
        down_pressed_ = true;
}

void DialogsManager::Update(){
    for (unsigned int i = 0; i < messages_.size(); ++ i){
        switch(messages_[i]->state){
            case MS_SHOW_WINDOW:
                {
                    if (
                      (messages_[i]->window == NULL)
                      || (messages_[i]->window->GetCurrentAnimationName() == Ogre::BLANKSTRING)
                    ){
                        messages_[i]->text_area->PlayAnimation("Show", UiAnimation::ONCE, 0, -1);
                        messages_[i]->text_area->SetVisible(true);
                        messages_[i]->state = MS_SHOW_TEXT;
                    }
                }
                break;
            case MS_SHOW_TEXT:{
                if (AutoCloseCheck(i) == true) break;
                if (messages_[i]->clickable == true){
                    if (next_pressed_ == true) messages_[i]->text_area->InputPressed();
                    if (next_repeated_ == true) messages_[i]->text_area->InputRepeated();
                }

                if (messages_[i]->text_area->GetTextState() == TS_DONE){
                    messages_[i]->state = MS_OPENED;
                    if (messages_[i]->cursor != NULL && messages_[i]->show_cursor == true){
                        messages_[i]->cursor->SetY(
                          messages_[i]->cursor_percent_y,
                          messages_[i]->cursor_y + messages_[i]->cursor_row_current
                            * messages_[i]->text_area->GetFont()->GetHeight()
                        );
                        messages_[i]->cursor->SetVisible(true);
                    }
                }
            }
                break;
            case MS_OPENED:
                {
                    if (messages_[i]->clickable == true && next_pressed_ == true){
                        messages_[i]->auto_close = true;
                        if ((messages_[i]->cursor != NULL) && (messages_[i]->show_cursor == true)){
                            messages_[i]->show_cursor = false;
                            messages_[i]->cursor_row_selected = messages_[i]->cursor_row_current;
                            messages_[i]->cursor->SetVisible(false);
                        }
                    }
                    if (AutoCloseCheck(i) == true) break;
                    if (
                      (messages_[i]->cursor != NULL)
                      && (messages_[i]->show_cursor == true)
                      && (messages_[i]->clickable == true)
                    ){
                        if (up_pressed_ == true) messages_[i]->cursor_row_current -= 1;
                        else if (down_pressed_ == true) messages_[i]->cursor_row_current += 1;
                        messages_[i]->cursor_row_current =
                          (messages_[i]->cursor_row_current < messages_[i]->cursor_row_first)
                            ? messages_[i]->cursor_row_last : messages_[i]->cursor_row_current;
                        messages_[i]->cursor_row_current =
                          (messages_[i]->cursor_row_current > messages_[i]->cursor_row_last)
                            ? messages_[i]->cursor_row_first : messages_[i]->cursor_row_current;
                        messages_[i]->cursor->SetY(
                          messages_[i]->cursor_percent_y,
                          messages_[i]->cursor_y + messages_[i]->cursor_row_current
                            * messages_[i]->text_area->GetFont()->GetHeight()
                        );
                    }
                }
                break;
            case MS_HIDE_WINDOW:{
                    if (
                      (messages_[i]->window == NULL)
                      || (messages_[i]->window->GetCurrentAnimationName() == Ogre::BLANKSTRING)
                    ){
                        HideMessage(i);
                    }
                }
            break;
        }
    }

    if (cv_debug_message.GetB() == true){
        DEBUG_DRAW.SetColour(Ogre::ColourValue::White);
        DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.LEFT);
        DEBUG_DRAW.SetScreenSpace(true);
        int y = 34;
        for (unsigned int i = 0; i < messages_.size(); ++ i){
            Ogre::String caption;
            caption += "Message " + Ogre::StringConverter::toString(i) + ": "
              + message_state_string[messages_[i]->state];

            if (messages_[i]->state == MS_SHOW_TEXT){
                caption += " ("
                  + Ogre::StringConverter::toString(int(messages_[i]->text_area->GetTextLimit()))
                  + "/" + Ogre::StringConverter::toString(messages_[i]->text_area->GetTextSize());

                switch(messages_[i]->text_area->GetTextState()){
                    case TS_PAUSE_OK: caption += " pause ok"; break;
                    case TS_PAUSE_TIME:
                        caption += " pause time " + Ogre::StringConverter::toString(
                          int(messages_[i]->text_area->GetPauseTime())
                        );
                        break;
                    case TS_OVERFLOW: caption += " overflow"; break;
                    case TS_NEXT_PAGE: caption += " next page"; break;
                    case TS_SCROLL_TEXT: caption += " scroll"; break;
                }
                caption += ")";
            }
            DEBUG_DRAW.Text(10, y, caption);
            y += 16;
        }
    }
    next_pressed_ = false;
    next_repeated_ = false;
    up_pressed_ = false;
    down_pressed_ = false;
}

void DialogsManager::Clear(){for (unsigned int i = 0; i < messages_.size(); ++ i) HideMessage(i);}

void DialogsManager::OpenDialog(const char* d_name, int x, int y, int w, int h){
    int id = GetMessageId(d_name);
    if (id == -1){
        LOG_TRIVIAL("dialog_open: dialog '" + Ogre::String(d_name) + "' doesn't exist.");
        return;
    }
    MessageData* data = messages_[id];
    if (data->state != MS_CLOSED){
        LOG_TRIVIAL(
          "dialog_open: dialog '" + Ogre::String(d_name) + "' already created. Close it first."
        );
        return;
    }
    data->x = x;
    data->y = y;
    data->w = w;
    data->h = h;
    LOG_TRIVIAL(
      "dialog_open: dialog(" + std::to_string(x) + "," + std::to_string(y) + ","
      + std::to_string(w) + "," + std::to_string(h) + ")"
    );
}

void DialogsManager::SetText(const char* d_name, const char* text){
    int id = GetMessageId(d_name);
    if (id == -1){
        LOG_TRIVIAL("[SCRIPT] show_text: dialog '" + Ogre::String(d_name) + "' doesn't exist.");
        return;
    }
    MessageData* data = messages_[id];
    // XML data can change dialog w/h VS what is in the lua script
    float width = 0.0f;
    float height = 0.0f;
    TiXmlNode* xmlText = TextManager::getSingleton().GetDialog(text, width, height);
    if (xmlText != NULL){
        ShowMessage(
          id, data->x, data->y,
          width == 0.0f ? data->w : width,
          height == 0.0f ? data->h : height
        );
        messages_[id]->text_area->SetText(xmlText);
    }
    else{
        ShowMessage(id, data->x, data->y, data->w, data->h);
        messages_[id]->text_area->SetText(std::string("[ERROR string not found:] ") + text);
    }
}

int DialogsManager::Sync(const char* d_name){
    int id = GetMessageId(d_name);
    if (id == -1){
        LOG_TRIVIAL("sync: dialog '" + Ogre::String(d_name) + "' doesn't exist.");
        return 1;
    }

    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    messages_[id]->sync.push_back(script);
    return -1;
}

void DialogsManager::Hide(const char* d_name){
    int id = GetMessageId(d_name);
    if (id == -1){
        LOG_TRIVIAL("hide: dialog '" + Ogre::String(d_name) + "' doesn't exist.");
        return;
    }
    if (messages_[id]->state == MS_CLOSED){
        LOG_TRIVIAL("hide: dialog '" + Ogre::String(d_name) + "' already closed. Open it first.");
        return;
    }
    messages_[id]->auto_close = true;
}

void DialogsManager::SetVariable(const char* d_name, const char* name, const char* value){
    int id = GetMessageId(d_name);
    if (id == -1){
        LOG_TRIVIAL("set_variable: dialog '" + Ogre::String(d_name) + "' doesn't exist.");
        return;
    }
    messages_[id]->text_area->SetVariable(name, value);
}

void DialogsManager::SetClickable(const char* d_name, const bool clickable){
    int id = GetMessageId(d_name);
    if (id == -1){
        LOG_TRIVIAL("set_clickable: dialog '" + Ogre::String(d_name) + "' doesn't exist.");
        return;
    }
    messages_[id]->clickable = clickable;
}

void DialogsManager::SetCursor(const char* d_name, const int first_row, const int last_row){
    int id = GetMessageId(d_name);
    if (id == -1){
        LOG_TRIVIAL("set_cursor: dialog '" + Ogre::String(d_name) + "' doesn't exist.");
        return;
    }
    if (messages_[id]->state != MS_CLOSED){
        LOG_TRIVIAL(
          "set_cursor: dialog '" + Ogre::String(d_name) + "' already shown. Close it first."
        );
        return;
    }
    messages_[id]->show_cursor = true;
    messages_[id]->cursor_row_current = first_row;
    messages_[id]->cursor_row_first = first_row;
    messages_[id]->cursor_row_last = last_row;
}

int DialogsManager::GetCursor(const char* d_name) const{
    int id = GetMessageId(d_name);
    if (id == -1){
        LOG_TRIVIAL("get_cursor: dialog '" + Ogre::String(d_name) + "' doesn't exist.");
        return 0;
    }
    return messages_[id]->cursor_row_selected;
}

void DialogsManager::ShowMessage(
  const int id, const int x, const int y, const int width, const int height
){
    // Add 5 pixels, to compensate character width operation roundowns.
    int extra_width = width + 5;
    // Handle error when dialog window cross border of limit area
    float la_percent_width, la_width;
    limit_area_->GetWidth(la_percent_width, la_width);
    float move_back_x = 0;
    if (x + extra_width > la_width) move_back_x = x + extra_width - la_width;
    float la_percent_height, la_height;
    limit_area_->GetWidth(la_percent_height, la_height);
    float move_back_y = 0;
    if (y + height > la_height) move_back_y = y + height - la_height;
    messages_[id]->widget->SetX(0, x - move_back_x);
    messages_[id]->widget->SetY(0, y - move_back_y);
    messages_[id]->widget->SetWidth(0, extra_width);
    messages_[id]->widget->SetHeight(0, height);
    messages_[id]->widget->SetVisible(true);
    messages_[id]->auto_close = false;
    if (messages_[id]->window != NULL && messages_[id]->show_window == true){
        messages_[id]->window->SetVisible(true);
        messages_[id]->window->PlayAnimation("Show", UiAnimation::ONCE, 0, -1);
    }
    messages_[id]->state = MS_SHOW_WINDOW;
}

void DialogsManager::HideMessage(const int id){
    messages_[id]->auto_close = false;
    messages_[id]->state = MS_CLOSED;
    if (messages_[id]->window != NULL) messages_[id]->window->SetVisible(false);
    if (messages_[id]->cursor != NULL) messages_[id]->cursor->SetVisible(false);
    messages_[id]->text_area->SetVisible(false);
    messages_[id]->widget->SetVisible(false);
    for (unsigned int i = 0; i < messages_[id]->sync.size(); ++ i)
        ScriptManager::getSingleton().ContinueScriptExecution(messages_[id]->sync[i]);
    messages_[id]->sync.clear();
}

int DialogsManager::GetMessageId(const char* d_name) const{
    for (unsigned int i = 0; i < messages_.size(); ++ i)
        if (messages_[i]->widget->GetName() == Ogre::String(d_name)) return i;
    return -1;
}

bool DialogsManager::AutoCloseCheck(const unsigned int id){
    if (messages_[id]->auto_close == true){
        messages_[id]->text_area->PlayAnimation("Hide", UiAnimation::ONCE, 0, -1);
        if (messages_[id]->window != NULL)
            messages_[id]->window->PlayAnimation("Hide", UiAnimation::ONCE, 0, -1);
        messages_[id]->state = MS_HIDE_WINDOW;
        return true;
    }
    return false;
}
