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

#include <iostream>
#include <OgreFontManager.h>
#include <OgreOverlay.h>
#include <Overlay/OgreOverlayManager.h>
#include "installer/common/OgreBase.h"

Ogre::Root* root;

Ogre::RenderWindow* window;

std::vector<Ogre::Entity*> entities;

Ogre::Camera *camera;

DisplayFrameListener *frame_listener;

void InitializeOgreBase(const Ogre::String& name){
    std::cout << "[OGRE] Initializing..." << std::endl;
    Ogre::LogManager* log_manager = new Ogre::LogManager();
    log_manager->createLog("v-gears.log", true, true);
    log_manager->getDefaultLog()->setLogDetail((Ogre::LoggingLevel)3);
    Ogre::String ressource_cfg("");
    Ogre::String plugins_cfg("");
    Ogre::String dyn_lib_ext("");
    Ogre::String render_system("");
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    dyn_lib_ext = ".dll";
    std::cout << "[OGRE] Initializing LIBW32" << std::endl;
#else // Assume Linux for now
    dyn_lib_ext = ".so";
    std::cout << "[OGRE] Initializing LIBLINUX" << std::endl;
#endif
#ifdef NDEBUG
    ressource_cfg = "resources.cfg";
    plugins_cfg = "plugins.cfg";
    render_system = "./RenderSystem_GL" + dyn_lib_ext;
#else
    std::cout << "[OGRE] Initializing in debug mode." << std::endl;
    ressource_cfg = "resources_d.cfg";
    plugins_cfg = "plugins_d.cfg";
    render_system = "./RenderSystem_GL_d" + dyn_lib_ext;
#endif
    // Init root early.
    root = new Ogre::Root(plugins_cfg);
    std::cout << "[InitializeOgreBase] 2" << std::endl;
    // Set up resources.
    // Load resource paths from config file.
    Ogre::ConfigFile cf;
    cf.load(ressource_cfg);
    // Go through all sections and settings in the file.
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    Ogre::String secName, typeName, archName;
    Ogre::ResourceGroupManager &res_gm(Ogre::ResourceGroupManager::getSingleton());
    while (seci.hasMoreElements()){
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i){
            typeName = i->first;
            archName = i->second;
            res_gm.addResourceLocation(archName, typeName, secName, true);
        }
    }

    // Configure.
    // Show the configuration dialog and initialize the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    // TODO: This gives a compilation error, don't show the dialog until fixed.
    //if (!root->restoreConfig() && !root->showConfigDialog()){
    //    root->setRenderSystem(root->getAvailableRenderers()[ 0 ]);
    //}*/
    root->setRenderSystem(root->getAvailableRenderers()[ 0 ]);
    root->initialise(false);
    Ogre::NameValuePairList misc;
    misc["title"] = name;
    window = root->createRenderWindow("VGearsWindow", 800, 600, false, &misc);
    // Initialize resources in the current launch directory.
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./", "FileSystem", "General");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
      "./exported", "FileSystem", "General"
    );
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    Ogre::SceneManager* scene_manager;
    Ogre::Viewport* viewport;
    frame_listener = new DisplayFrameListener(window);
    root->addFrameListener(frame_listener);
    scene_manager = root->createSceneManager(Ogre::ST_GENERIC, "Scene");
    scene_manager->clearScene();
    scene_manager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
    Ogre::Light* directional_light = scene_manager->createLight("directionalLight");
    directional_light->setType(Ogre::Light::LT_DIRECTIONAL);
    directional_light->setDiffuseColour(Ogre::ColourValue(0.5, 0.5, 0.5));
    directional_light->setSpecularColour(Ogre::ColourValue(0.5, 0.5, 0.5));
    directional_light->getParentSceneNode()->setDirection(Ogre::Vector3(0, 0, -1));
    camera = scene_manager->createCamera("Camera");
    camera->setNearClipDistance(0.01f);
    camera->getParentSceneNode()->setPosition(0, 5, 10);
    camera->getParentSceneNode()->lookAt(Ogre::Vector3(0, 0, 0), Ogre::Node::TS_LOCAL);
    viewport = window->addViewport(camera);
    viewport->setBackgroundColour(Ogre::ColourValue(0.0f, 0.4f, 0.0f));
    camera->setAspectRatio(
      Ogre::Real(viewport->getActualWidth()) / Ogre::Real(viewport->getActualHeight())
    );
    LOGGER = new Logger("game.log");
}

void DeinitializeOgreBase(){
    delete LOGGER;
    delete root;
    delete frame_listener;
}
