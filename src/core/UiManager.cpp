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
    XmlFontsFile fonts("./data/fonts.xml");
    fonts.LoadFonts();
    XmlScreensFile screens("./data/screens.xml");
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

void UiManager::renderQueueStarted(
  Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation
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

void UiManager::UpdateBattle(){}

void UiManager::UpdateWorld(){}
