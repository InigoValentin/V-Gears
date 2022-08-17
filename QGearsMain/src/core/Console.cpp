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
#include <OgreRenderWindow.h>
#include "common/QGearsApplication.h"
#include "core/ConfigCmdManager.h"
#include "core/ConfigVarManager.h"
#include "core/Console.h"
#include "core/DebugDraw.h"
#include "core/Logger.h"
#include "core/ScriptManager.h"
#include "core/Timer.h"
#include "core/Utilites.h"


template<>Console *Ogre::Singleton<Console>::msSingleton = nullptr;

ConfigVar cv_console_notification(
  "console_notification", "Draw console strings even when console is hided",
  "false"
);


Console::Console():
  to_visible_(false),
  visible_(false),
  height_(0),
  max_output_line_(128),
  display_line_(0),
  input_line_(""),
  cursor_position_(0),
  cursor_blink_time_(0),
  history_line_cycle_index_(-1),
  max_history_size_(32),
  auto_completition_line_(0)
{

    Ogre::FontPtr font
      = Ogre::FontManager::getSingleton().getByName("CourierNew");
    if (font != nullptr)
        letter_width_ = static_cast<int>(font->getGlyphAspectRatio('_') * 16);
    else
        LOG_ERROR("Console::frameStarted: Font for console not found.");

    // Calculate width and height of console depending on size of application
    Ogre::RenderTarget *render_target(
      QGears::Application::getSingleton().getRenderWindow()
    );
    console_width_ = render_target->getWidth();
    console_height_ = static_cast<int>(render_target->getHeight() / 2.5f);
    line_width_ = (console_width_ - 20) / 8;
    LOG_TRIVIAL(
      "Created console width " + Ogre::StringConverter::toString(console_width_)
      + ", height " + Ogre::StringConverter::toString(console_height_)
    );

    // Add as frame and log listener
    Ogre::LogManager::getSingleton().getDefaultLog()->addListener(this);
    LoadHistory();
}

Console::~Console(){
    // Remove as listener
    Ogre::LogManager::getSingleton().getDefaultLog()->removeListener(this);
    SaveHistory();
}

void Console::LoadHistory(){
    LOG_TRIVIAL("Loading console_history.txt ...");
    std::ifstream file("console_history.txt");
    std::string historyLine;
    while (std::getline(file, historyLine)) AddToHistory(historyLine);
    std::reverse(history_.begin(), history_.end());
}

void Console::SaveHistory(){
    std::ofstream file("console_history.txt");
    if (!file.is_open()){
        LOG_ERROR("Failed to open console_history.txt for writing");
        return;
    }
    for (auto& historyLine : history_) file << historyLine << "\r\n";
}

void Console::AddToHistory(const Ogre::String& history){
    if (history_.size() >= max_history_size_) history_.pop_back();
    history_.push_front(history);
    history_line_cycle_index_ = -1;
}

void Console::Input(const QGears::Event& event){
    if (visible_ != true){
        // Add console
        if (event.type == QGears::ET_KEY_PRESS && event.param1 == OIS::KC_GRAVE)
            SetToVisible();
        return;
    }

    // input command
    else if (
      event.type == QGears::ET_KEY_PRESS && (
        event.param1 == OIS::KC_RETURN || event.param1 == OIS::KC_NUMPADENTER
      ) && input_line_.size()
    ){
        if (auto_completition_.size() > 0)
            input_line_ += auto_completition_[auto_completition_line_];
        AddTextToOutput(input_line_);
        AddInputToHistory();
        // Backslashed text are console commands, otherwise - script commands
        if ('\\' == input_line_[0] || '/' == input_line_[0]){
            if (input_line_.size() > 1){
                // Remove backslash
                input_line_.erase(0, 1);
                ExecuteCommand(input_line_);
            }
        }
        else ExecuteScript();
        input_line_.clear();
        cursor_position_ = 0;
        ResetAutoCompletion();
    }
    else if (event.type == QGears::ET_KEY_PRESS && event.param1 == OIS::KC_TAB)
        CompleteInput();
    // Remove console
    else if (
      event.type == QGears::ET_KEY_PRESS && event.param1 == OIS::KC_GRAVE
    ){
        SetToHide();
    }
    // History up
    else if (
      (
        event.type == QGears::ET_KEY_PRESS
        || event.type == QGears::ET_KEY_REPEAT_WAIT
      ) && event.param1 == OIS::KC_UP
    ){
        if (history_line_cycle_index_ < (int)history_.size() - 1){
            ++ history_line_cycle_index_;
            SetInputLineFromHistory();
        }
    }
    // History down
    else if (
      (
        event.type == QGears::ET_KEY_PRESS
        || event.type == QGears::ET_KEY_REPEAT_WAIT
      ) && event.param1 == OIS::KC_DOWN
    ){
        if (history_line_cycle_index_ > 0){
            -- history_line_cycle_index_;
            SetInputLineFromHistory();
        }
    }
    // Scroll display to previous row
    else if (event.type == QGears::ET_MOUSE_SCROLL && event.param1 > 0){
        if (display_line_ > 0) display_line_ -= 1;
    }
    // Scroll display to next row
    else if (event.type == QGears::ET_MOUSE_SCROLL && event.param1 < 0){
        if (display_line_ < output_line_.size()) display_line_ += 1;

    }
    // Scroll display to previous row
    else if (
      (
        event.type == QGears::ET_KEY_PRESS
        || event.type == QGears::ET_KEY_REPEAT_WAIT
      ) && event.param1 == OIS::KC_PGUP
    ){
        if (display_line_ > 0) display_line_ -= 1;

    }
    // Scroll display to next row
    else if (
      (
        event.type == QGears::ET_KEY_PRESS
        || event.type == QGears::ET_KEY_REPEAT_WAIT
      ) && event.param1 == OIS::KC_PGDOWN
    ){
        if (display_line_ < output_line_.size()) display_line_ += 1;
    }
    // Delete character after cursor
    else if (
      (
        event.type == QGears::ET_KEY_PRESS
        || event.type == QGears::ET_KEY_REPEAT_WAIT
      ) && event.param1 == OIS::KC_DELETE
    ){
        if (auto_completition_.size() > 0) ResetAutoCompletion();
        else{
            if (input_line_.size() > 0 && cursor_position_ < input_line_.size())
                input_line_.erase(cursor_position_, 1);
        }
    }
    // Delete character before cursor
    else if (
      (
        event.type == QGears::ET_KEY_PRESS
        || event.type == QGears::ET_KEY_REPEAT_WAIT
      ) && event.param1 == OIS::KC_BACK
    ){
        if (auto_completition_.size() > 0) ResetAutoCompletion();
        else{
            if (cursor_position_ > 0){
                input_line_.erase(cursor_position_ - 1, 1);
                -- cursor_position_;
            }
        }
    }
    // Move cursor to left
    else if (
      (
        event.type == QGears::ET_KEY_PRESS
        || event.type == QGears::ET_KEY_REPEAT_WAIT
      ) && event.param1 == OIS::KC_LEFT
    ){
        if (auto_completition_.size() > 0){
            input_line_ += auto_completition_[auto_completition_line_];
            ResetAutoCompletion();
        }
        if (cursor_position_ > 0) -- cursor_position_;
    }
    // Move cursor to right
    else if (
      (
        event.type == QGears::ET_KEY_PRESS
        || event.type == QGears::ET_KEY_REPEAT_WAIT
      ) && event.param1 == OIS::KC_RIGHT
    ){
        if (auto_completition_.size() > 0){
            input_line_ += auto_completition_[auto_completition_line_];
            ResetAutoCompletion();
        }
        if (cursor_position_ <  input_line_.size()) ++ cursor_position_;
    }
    else if (
      event.type == QGears::ET_KEY_PRESS && event.param1 == OIS::KC_ESCAPE
    ){
        input_line_.clear();
        cursor_position_ = 0;
        ResetAutoCompletion();
    }
    // Move cursor to start of string
    else if (
      event.type == QGears::ET_KEY_PRESS && event.param1 == OIS::KC_HOME
    ){
        cursor_position_ = 0;
        if (auto_completition_.size() > 0){
            input_line_ += auto_completition_[auto_completition_line_];
            ResetAutoCompletion();
        }
    }
    // Move cursor to end of string
    else if (event.type == QGears::ET_KEY_PRESS && event.param1 == OIS::KC_END){
        if (auto_completition_.size() > 0){
            input_line_ += auto_completition_[auto_completition_line_];
            ResetAutoCompletion();
        }
        cursor_position_ = input_line_.size();
    }
    // Input Ascii character
    else if (
      (
        event.type == QGears::ET_KEY_PRESS
        || event.type == QGears::ET_KEY_REPEAT_WAIT
      ) && input_line_.size() < line_width_
    ){
        char legalchars[]
          = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890"
            "~!@#$%^&*()-_=+?{[]}|\\;:'\"<>,./? ";
        char txt = TranslateNumpad(event);
        for (unsigned int c = 0; c < sizeof(legalchars) - 1; ++ c){
            if (legalchars[c] == txt){
                std::string::iterator i
                  = input_line_.begin() + cursor_position_;
                input_line_.insert(i, txt);
                ++ cursor_position_;
                ResetAutoCompletion();
                break;
            }
        }
    }
}

char Console::TranslateNumpad(const QGears::Event& event){
    switch (static_cast<int>(event.param1)){
        case OIS::KC_NUMPAD0: return '0';
        case OIS::KC_NUMPAD1: return '1';
        case OIS::KC_NUMPAD2: return '2';
        case OIS::KC_NUMPAD3: return '3';
        case OIS::KC_NUMPAD4: return '4';
        case OIS::KC_NUMPAD5: return '5';
        case OIS::KC_NUMPAD6: return '6';
        case OIS::KC_NUMPAD7: return '7';
        case OIS::KC_NUMPAD8: return '8';
        case OIS::KC_NUMPAD9: return '9';
        case OIS::KC_DECIMAL: return '.';
        case OIS::KC_ADD: return '+';
        case OIS::KC_SUBTRACT: return '-';
        case OIS::KC_MULTIPLY: return '*';
        case OIS::KC_DIVIDE: return '/';
    }
    return static_cast<char>(event.param2);
}

void Console::Update(){
    float delta_time = Timer::getSingleton().GetSystemTimeDelta();
    if (to_visible_ == true && height_ < console_height_){
        // TODO: Convert this to a constant.
        height_ += delta_time * 1500;
        if (height_ >= console_height_)
            height_ = static_cast<float>(console_height_);
    }
    else if (to_visible_ == false && height_ > 0){
        height_ -= delta_time * 1500;
        if (height_ <= 0){
            height_ = 0;
            visible_ = false;
        }
    }
    if (visible_ == true) UpdateDraw();
    else if (cv_console_notification.GetB() == true) UpdateNotification();
}

void Console::UpdateDraw(){
    float delta_time = Timer::getSingleton().GetSystemTimeDelta();
    DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.LEFT);
    DEBUG_DRAW.SetScreenSpace(true);
    DEBUG_DRAW.SetColour(Ogre::ColourValue(0.05f, 0.06f, 0.2f, 0.95f));
    DEBUG_DRAW.SetZ(-0.5f);
    DEBUG_DRAW.Quad(
      0.0f, 0.0f, static_cast<float>(console_width_),
      0.0f, static_cast<float>(console_width_), height_, 0.0f, height_
    );
    DEBUG_DRAW.SetColour(Ogre::ColourValue(0.18f, 0.22f, 0.74f, 0.95f));
    DEBUG_DRAW.SetZ(-0.6f);
    DEBUG_DRAW.Line(0.0f, height_, static_cast<float>(console_width_), height_);
    int row = 1;
    int rows = (console_height_ - 30) / 16;
    int y = static_cast<int>(-console_height_ + height_);
    int empty = rows - display_line_;
    if (empty > 0) y += empty * 16;
    std::list<OutputLine>::iterator i;
    for (i = output_line_.begin(); i != output_line_.end(); ++ i, ++ row){
        if (row > (int)display_line_ - rows && row <= (int)display_line_){
            DEBUG_DRAW.SetColour((*i).colour);
            DEBUG_DRAW.Text(5.0f, static_cast<float>(y), (*i).text);
            y += 16;
        }
    }
    if (display_line_ != output_line_.size()){
        Ogre::String temp = "";
        for (unsigned int i = 0; i < line_width_; ++ i) temp += "^";
        DEBUG_DRAW.SetColour(Ogre::ColourValue(1.0f, 0.0f, 0.0f, 1.0f));
        DEBUG_DRAW.Text(5.0f, static_cast<float>(y), temp);
    }
    cursor_blink_time_ += delta_time;
    if ((((int) (cursor_blink_time_ * 1000)) >> 8) & 1){
        DEBUG_DRAW.SetColour(Ogre::ColourValue(0.88f, 0.88f, 0.88f, 1));
        DEBUG_DRAW.Text(
          static_cast<float>(12 + cursor_position_ * letter_width_),
          static_cast<float>(-19 + height_), "_"
        );
    }
    if (auto_completition_.size() > 0){
        DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 1, 1, 1));
        DEBUG_DRAW.Text(12, -20 + height_, input_line_);
        DEBUG_DRAW.SetColour(Ogre::ColourValue(0, 1, 0, 1));
        DEBUG_DRAW.Text(
          static_cast<float>(12 + (input_line_.size() * letter_width_)),
          static_cast<float>(-20 + height_),
          auto_completition_[auto_completition_line_]
        );
    }
    else{
        DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 1, 1, 1));
        DEBUG_DRAW.Text(12, -20 + height_, input_line_);
    }
    DEBUG_DRAW.SetZ(0);
}

void Console::UpdateNotification(){
    DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.LEFT);
    DEBUG_DRAW.SetScreenSpace(true);
    DEBUG_DRAW.SetZ(-0.6f);
    std::list< OutputLine >::reverse_iterator i;
    int y = (output_line_.size() > 10) ? 160 : output_line_.size() * 16;
    int line = 0;
    float max_time = 3.0f;
    for (
      i = output_line_.rbegin(); i != output_line_.rend() && line < 10; ++ i
    ){
        float time = Timer::getSingleton().GetSystemTimeTotal() - (*i).time;
        if (time < max_time){
            Ogre::ColourValue colour = (*i).colour;
            colour.a = (max_time - time) / max_time;
            DEBUG_DRAW.SetColour(colour);
            DEBUG_DRAW.Text(5.0f, static_cast<float>(y), (*i).text);
            y -= 16;
            ++line;
        }
    }
}

void Console::OnResize(){
    // Calculate width and height of console depending on size of application
    Ogre::RenderTarget *render_target(
      QGears::Application::getSingleton().getRenderWindow()
    );
    console_width_ = render_target->getWidth();
    console_height_ = static_cast<int>(render_target->getHeight() / 2.5f);
    line_width_ = (console_width_ - 20) / 8;
    LOG_TRIVIAL(
      "Resized console width to "
      + Ogre::StringConverter::toString(console_width_) + ", height to "
      + Ogre::StringConverter::toString(console_height_)
    );
    // Update height of already opened console
    height_ = (height_ > console_height_) ? console_height_ : height_;
}

void Console::SetToVisible(){
    to_visible_ = true;
    visible_ = true;
}

void Console::SetToHide(){to_visible_ = false;}

bool Console::IsVisible() const{return visible_;}

void Console::AddTextToOutput(
  const Ogre::String& text, const Ogre::ColourValue& colour
){
    // Go through line and add it to output correctly
    const char* str = text.c_str();
    Ogre::String output_line;
    unsigned int string_size = 0;
    bool indent = false;
    unsigned int c = 0;
    for (; c < text.size(); ++ c){
        // Add space at start of string if we want indent
        if (string_size == 0 && indent == true){
            output_line.push_back(' ');
            ++ string_size;
        }
        if (str[c] != '\n'){
            output_line.push_back(str[c]);
            ++ string_size;
        }
        if (
          str[c] == '\n' || string_size >= line_width_ || c >= text.size() - 1
        ){
            // If string is larger than output size than add indent
            indent = (string_size >= line_width_) ? true : false;
            if (output_line_.size() >= max_output_line_)
                output_line_.pop_front();
            if (output_line_.size() == display_line_) ++ display_line_;
            OutputLine line;
            line.text = output_line;
            line.colour = colour;
            line.time = Timer::getSingleton().GetSystemTimeTotal();
            output_line_.push_back(line);
            output_line.clear();
            string_size = 0;
        }
    }
    // Add one more string if text ended with \n
    if (text.size() == 0 || str[text.size() - 1] == '\n'){
        if (output_line_.size() >= max_output_line_) output_line_.pop_front();
        if (output_line_.size() == display_line_) ++ display_line_;
        OutputLine line;
        line.text = "";
        line.colour = colour;
        line.time = Timer::getSingleton().GetSystemTimeTotal();
        output_line_.push_back(line);
    }
}

void Console::ExecuteCommand(const Ogre::String& command){
    bool handled = false;
    Ogre::StringVector params = StringTokenise(command);

    // Is it cvar?
    ConfigVar* cvar = ConfigVarManager::getSingleton().Find(params[0]);
    if (cvar != nullptr){
        handled = true;
        if (params.size() > 1){
            cvar->SetS(params[1]);
            AddTextToOutput(params[0] + " changed to \"" + params[1] + "\".\n");
        }
        else{
            AddTextToOutput(
              params[0] + " = \"" + cvar->GetS() + "\".\n" + " default:\""
              + cvar->GetDefaultValue() + "\".\n" + " description: "
              + cvar->GetDescription() + ".\n"
            );
        }
    }
    if (handled == false){
        // Handle command
        ConfigCmd* cmd = ConfigCmdManager::getSingleton().Find(params[0]);
        if (cmd != nullptr){
            cmd->GetHandler()(params);
            return;
        }
        else AddTextToOutput("Can't find command \"" + params[0] + "\".\n");
    }
}

void Console::ExecuteScript(){
    ScriptManager::getSingleton().RunString(input_line_);
}

void Console::CompleteInput(){
    bool add_slash = false;
    if (auto_completition_.size() == 0){
        // remove backslash
        if (
          input_line_.size() > 0
          && ('\\' == input_line_[0]
          || '/' == input_line_[0])
        ){
            input_line_.erase(0, 1);
        }
        size_t input_size = input_line_.size();
        Ogre::StringVector params = StringTokenise(input_line_);
        if (params.size() == 0){
            // Add cvars
            int num_vars
              = ConfigVarManager::getSingleton().GetConfigVarNumber();
            for (int i = 0; i < num_vars; ++ i){
                auto_completition_.push_back(
                  ConfigVarManager::getSingleton().GetConfigVar(i)->GetName()
                );
            }
            // Add commands
            int num_cmds
              = ConfigCmdManager::getSingleton().GetConfigCmdNumber();
            for (int i = 0; i < num_cmds; ++i){
                auto_completition_.push_back(
                  ConfigCmdManager::getSingleton().GetConfigCmd(i)->GetName()
                );
            }
            add_slash = true;
        }
        else if (params.size() == 1){
            input_line_ = params[0];
            // add Cvars
            int num_vars
              = ConfigVarManager::getSingleton().GetConfigVarNumber();
            for (int i = 0; i < num_vars; ++ i){
                Ogre::String name =
                  ConfigVarManager::getSingleton().GetConfigVar(i)->GetName();
                unsigned int pos = name.find(input_line_);
                if (pos == 0){
                    add_slash = true;
                    if (input_size != name.size()){
                        Ogre::String part
                          = name.substr(input_size, name.size() - input_size);
                        auto_completition_.push_back(part);
                    }
                }
            }
            // Add commands
            int num_cmds
              = ConfigCmdManager::getSingleton().GetConfigCmdNumber();
            for (int i = 0; i < num_cmds; ++ i){
                Ogre::String name
                  = ConfigCmdManager::getSingleton().GetConfigCmd(i)->GetName();
                int pos = name.find(input_line_);
                if (pos == 0){
                    add_slash = true;
                    if (name != params[0]){
                        Ogre::String part
                          = name.substr(input_size, name.size() - input_size);
                        auto_completition_.push_back(part);
                    }
                    else if (
                      ConfigCmdManager::getSingleton().GetConfigCmd(i)
                      ->GetCompletion()
                      != nullptr
                    ){
                        input_line_ += " ";
                        ConfigCmdManager::getSingleton().GetConfigCmd(i)->
                          GetCompletion()(auto_completition_);
                    }
                }
            }
        }
        else if (params.size() > 1){
            Ogre::String all_params = params[1];
            for (size_t i = 2; i < params.size(); ++ i)
                all_params += " " + params[i];
            // Add commands arguments
            ConfigCmd* cmd = ConfigCmdManager::getSingleton().Find(params[0]);
            if (cmd != nullptr){
                add_slash = true;
                if (cmd->GetCompletion() != nullptr){
                    Ogre::StringVector full_complete;
                    cmd->GetCompletion()(full_complete);
                    if (full_complete.size() > 0){
                        std::sort(full_complete.begin(), full_complete.end());
                        for (size_t i = 0 ; i < full_complete.size(); ++ i){
                            int pos = full_complete[i].find(all_params);
                            if (pos == 0 && all_params != full_complete[i]){
                                Ogre::String part = full_complete[i].substr(
                                  all_params.size(),
                                  full_complete[i].size() - all_params.size()
                                );
                                auto_completition_.push_back(part);
                            }
                        }
                    }
                }
            }
            input_line_ = params[0] + " " + all_params;
        }

        // If at least one match was found
        if (add_slash == true) input_line_ = "/" + input_line_;
        cursor_position_ = input_line_.size();

        // Sort list
        std::sort(auto_completition_.begin(), auto_completition_.end());
        auto_completition_line_ = 0;

        for (size_t i = 0; i < auto_completition_.size(); ++ i)
            AddTextToOutput(" " + input_line_ + auto_completition_[i]);
        if (auto_completition_.size() > 0) AddTextToOutput("\n");
    }
    else{
        if (auto_completition_line_ < auto_completition_.size() - 1)
            ++auto_completition_line_;
        else auto_completition_line_ = 0;
    }
}

void Console::ResetAutoCompletion(){
    auto_completition_.clear();
    auto_completition_line_ = 0;
}

void Console::AddInputToHistory(){
    AddToHistory(input_line_);
}

void Console::SetInputLineFromHistory(){
    ResetAutoCompletion();
    std::list<Ogre::String>::iterator i = history_.begin();
    for (int count = 0; i != history_.end(); ++i, ++ count){
        if (count == history_line_cycle_index_){
            input_line_ = (*i);
            cursor_position_ = input_line_.size();
            break;
        }
    }
}

void Console::messageLogged(
  const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug,
  const Ogre::String& logName, bool& skipThisMessage
){
    Ogre::ColourValue colour = Ogre::ColourValue::White;
    switch((int)lml){
        case 2: colour = Ogre::ColourValue(1, 1, 0, 1); break;
        case 3: colour = Ogre::ColourValue(1, 0, 0, 1); break;
    }
    AddTextToOutput(message, colour);
}
