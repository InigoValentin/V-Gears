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

#include <boost/algorithm/string.hpp>
#include "VGearsUtility.h"
#include "Characters.h"

namespace VGears{

    Utility::Utility(int argc, char *argv[]) :
      Application(argc, argv), frame_listener_(NULL), scene_manager_(NULL),
      camera_(NULL), viewport_(NULL)
    {}

    Utility::~Utility(){}

    Ogre::Camera* Utility::GetCamera(){return camera_;}

    void Utility::InitComponents(){
        Application::initComponents();
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        Ogre::Root *root(getRoot());
        scene_manager_ = root->createSceneManager(Ogre::ST_GENERIC, "Scene");
        scene_manager_->clearScene();
        scene_manager_->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
        Ogre::Light* directional_light = scene_manager_->createLight("directional_light");
        directional_light->setType(Ogre::Light::LT_DIRECTIONAL);
        directional_light->setDiffuseColour(Ogre::ColourValue(0.5, 0.5, 0.5));
        directional_light->setSpecularColour(Ogre::ColourValue(0.5, 0.5, 0.5));
        directional_light->setDirection(Ogre::Vector3(0, 0, -1));
        camera_ = scene_manager_->createCamera("Camera");
        camera_->setNearClipDistance(0.01f);
        camera_->setPosition(0, 5, 10);
        camera_->lookAt(0, 0, 0);
        Ogre::RenderWindow *window(getRenderWindow());
        viewport_ = window->addViewport(camera_);
        viewport_->setBackgroundColour(Ogre::ColourValue(0.0f, 0.4f, 0.0f));
        camera_->setAspectRatio(
          Ogre::Real(viewport_->getActualWidth()) / Ogre::Real(viewport_->getActualHeight())
        );
        frame_listener_ = new DisplayFrameListener(window);
        frame_listener_->setCamera(camera_);
        root->addFrameListener(frame_listener_);
    }

    void Utility::DestroyComponents(){
        delete frame_listener_;
        frame_listener_ = NULL;
        Application::destroyComponents();
    }

    std::string Utility::DecodeString(const u8* bytes, const unsigned int size){
        std::string decoded("");
        for (int i = 0; i < size; i ++)
            if (bytes[i] > 0x00 && bytes[i] < 0XFF) decoded += ENGLISH_CHARS[bytes[i]];
        boost::algorithm::trim(decoded);
        return decoded;
    }

}
