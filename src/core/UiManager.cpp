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


#include <OgreRoot.h>
#include <OgreStringVector.h>
#include <iostream>
#include <cctype>
#include <algorithm>
#include "core/Logger.h"
#include "core/ScriptManager.h"
#include "core/UiManager.h"
#include "core/Utilites.h"
#include "core/XmlFontsFile.h"
#include "core/XmlPrototypesFile.h"
#include "core/XmlScreensFile.h"
#include "core/XmlTextsFile.h"
#include "TextHandler.h"

/**
 * UI manager singleton.
 */
template<>UiManager *Ogre::Singleton<UiManager>::msSingleton = nullptr;

UiManager::UiManager(){
    Ogre::Root::getSingleton().getSceneManager("Scene")->addRenderQueueListener(this);
}

UiManager::~UiManager(){
    Ogre::Root::getSingleton().getSceneManager("Scene")->removeRenderQueueListener(this);
    for (size_t i = 0; i < fonts_.size(); ++ i) delete fonts_[i];
    for (size_t i = 0; i < widgets_.size(); ++ i) delete widgets_[i];
}

void UiManager::Initialise(){
    XmlFontsFile fonts("./data/fonts/_fonts.xml");
    fonts.LoadFonts();
    XmlScreensFile screens("./data/screens/_screens.xml");
    screens.LoadScreens();
}

void UiManager::Input(const VGears::Event& event){}

void UiManager::UpdateDebug(){}

void UiManager::ClearField(){}

void UiManager::ClearBattle(){}

void UiManager::ClearWorld(){}

void UiManager::OnResize(){
    for (unsigned int i = 0; i < widgets_.size(); ++ i) widgets_[i]->OnResize();
}

void UiManager::AddFont(UiFont* font){fonts_.push_back(font);}

/**
 * Turns a string to lowercase.
 *
 * @param[in] s String to convert.
 * @return Lowercase string.
 */
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){return std::tolower(c);});
    return s;
}

UiFont* UiManager::GetFont(const Ogre::String& name){
    Ogre::String language = TextHandler::getSingleton().GetLanguage();

    for (unsigned int i = 0; i < fonts_.size(); ++ i){
        if (fonts_[ i ]->GetName() == name){
            Ogre::String f_lang = fonts_[ i ]->GetLanguage();
            if ((f_lang == "") || (toLower(f_lang) == toLower(language))) return fonts_[i];
        }
    }
    return NULL;
}

void UiManager::AddPrototype(const Ogre::String& name, TiXmlNode* prototype){
    UiPrototype ui_prototype;
    ui_prototype.name = name;
    ui_prototype.node = prototype;
    prototypes_.push_back(ui_prototype);
}

TiXmlNode* UiManager::GetPrototype(const Ogre::String& name) const{
    for (unsigned int i = 0; i < prototypes_.size(); ++ i)
        if (prototypes_[ i ].name == name) return prototypes_[ i ].node;
    return NULL;
}

void UiManager::AddWidget(UiWidget* widget){widgets_.push_back(widget);}

UiWidget* UiManager::GetWidget(const Ogre::String& name){
    // Get the real table by name.
    Ogre::StringVector table_path = StringTokenise(name, ".");
    UiWidget* widget = NULL;
    if (table_path.size() > 0){
        for (unsigned int i = 0; i < widgets_.size(); ++ i){
            if (widgets_[ i ]->GetName() == table_path[0]){
                widget = widgets_[i];
                for (unsigned int j = 1; (j < table_path.size()) && (widget != NULL); ++ j)
                    widget = widget->GetChild(table_path[j]);
            }
        }
    }
    return widget;
}

UiWidget* UiManager::ScriptGetWidget(const char* name){return GetWidget(Ogre::String(name));}

void UiManager::SetWindowColours(
  const float r1, const float g1, const float b1, const float r2, const float g2, const float b2,
  const float r3, const float g3, const float b3, const float r4, const float g4, const float b4
){
    const float colours[12] = {r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4};
    for (int i = 0; i < 12; ++ i) window_colours_[i] = colours[i];
    window_colours_set_ = true;
    for (unsigned int i = 0; i < widgets_.size(); ++ i) ApplyWindowColours(widgets_[i]);
}

void UiManager::ApplyWindowColours(UiWidget* widget){
    if (widget == nullptr || window_colours_set_ == false) return;
    if (widget->IsWindowBackground()){
        widget->SetColours(
          window_colours_[0], window_colours_[1], window_colours_[2],
          window_colours_[3], window_colours_[4], window_colours_[5],
          window_colours_[6], window_colours_[7], window_colours_[8],
          window_colours_[9], window_colours_[10], window_colours_[11]
        );
    }
    for (unsigned int i = 0; i < widget->GetNumberOfChildren(); ++ i){
        ApplyWindowColours(widget->GetChild(i));
    }
}

void UiManager::renderQueueStarted(
  Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation
){
    // The widgets are drawn when the queue ends. See renderQueueEnded.
}

void UiManager::renderQueueEnded(
  Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation
){
    if (queueGroupId == Ogre::RENDER_QUEUE_OVERLAY){
        Ogre::Root::getSingletonPtr()->getRenderSystem()->clearFrameBuffer(Ogre::FBT_DEPTH);
        for (unsigned int i = 0; i < widgets_.size(); ++ i) widgets_[i]->Render();
    }
}

void UiManager::UpdateField(){
    // Update all ui scripts
    ScriptManager::getSingleton().Update(ScriptManager::UI);
    for (unsigned int i = 0; i < widgets_.size(); ++ i) widgets_[i]->Update();
}

void UiManager::UpdateBattle(){UpdateField();}

void UiManager::UpdateWorld(){UpdateField();}
