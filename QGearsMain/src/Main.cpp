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
#include <OgreConfigFile.h>
#include <OgreArchiveManager.h>
#include <OIS.h>
#include <OgreFontManager.h>
#include <OgreOverlayManager.h>
#include <OgreOverlaySystem.h>
#include "QGearsGameState.h"
#ifdef VGears_SOUND
#include "core/AudioManager.h"
#endif
#include "common/QGearsApplication.h"
#include "core/CameraManager.h"
#include "core/ConfigCmdManager.h"
#include "core/ConfigFile.h"
#include "core/ConfigVarManager.h"
#include "core/Console.h"
#include "core/DebugDraw.h"
#include "core/EntityManager.h"
#include "../include/core/GameFrameListener.h"
#include "core/InputManager.h"
#include "core/Logger.h"
#include "core/ScriptManager.h"
#include "core/Timer.h"
#include "core/UiManager.h"
#include "core/particles/ParticleSystemManager.h"
#include "core/TextManager.h"
#include "core/DialogsManager.h"
#include "data/QGearsLZSFLevelFileManager.h"
#include "common/make_unique.h"
#include "modules/worldmap/WorldMapModule.h"
#include "data/worldmap/MapFileManager.h"
#include "data/QGearsLGPArchiveFactory.h"

/**
 * Main application function
 *
 * @param argc Number of command line arguments.
 * @param argv List of command line arguments.
 * @return 0 on sucess, an error code on error.
 */
int main(int argc, char *argv[]){
    try{
        std::cout << "V-Gears Init" << std::endl;
        QGears::Application app(argc, argv);
        if (!app.initOgre()) return 0;
        Ogre::Root *root(app.getRoot());
        Ogre::RenderWindow *window(app.getRenderWindow());
        Ogre::SceneManager *scene_manager(nullptr);

        auto timer = std::make_unique<Timer>();
        auto particle_system_manager =
          std::make_unique<ParticleSystemManager>();

        // Set scene camera and viewport for other modules.
        // Create this before initialize particle because some of them use
        // scene to create themselves.
        scene_manager = root->createSceneManager(Ogre::ST_GENERIC, "Scene");
        // TODO: Why is this used twice?
        scene_manager->setAmbientLight(Ogre::ColourValue(1, 1, 1));
        scene_manager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
        Ogre::Light *directionalLight(
          scene_manager->createLight("directionalLight")
        );
        directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
        directionalLight->setDiffuseColour(Ogre::ColourValue(0.5, 0.5, 0.5));
        directionalLight->setSpecularColour(Ogre::ColourValue(0.0, 0.0, 0.0));
        directionalLight->setDirection(Ogre::Vector3(0, 1, 0));
        // auto fontManager = std::make_unique<Ogre::FontManager>();
        QGears::MapFileManager* worldManager = new QGears::MapFileManager();

        // Initialize resources.
        // TODO: Use correct file location in the end, now is OK for testing
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          ".", "FileSystem"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/wm", "FileSystem", "TEST"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/wm/world_us.lgp",
          QGears::LGPArchiveFactory::ARCHIVE_TYPE, "TEST"
        );
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        // Initialize it before console because it may use it
        auto config_var_manager = std::make_unique<ConfigVarManager>();
        auto config_cmd_manager = std::make_unique<ConfigCmdManager>();
        auto debug_draw = std::make_unique<DebugDraw>();

        // Initialize before GameFrameListener, but after ConfigCmdManager
        auto input_manager = std::make_unique<InputManager>();


#ifdef VGears_SOUND
        //auto audio_manager = std::make_unique<AudioManager>();
        //audio_manager->MusicPlay( "loop1" );
#endif

        // Create this earlier than DisplayFrameListener cause it can fire
        // events there
        auto camera_manager = std::make_unique<CameraManager>();
        auto text_manager = std::make_unique<TextManager>();
        auto ui_manager = std::make_unique<UiManager>();
        auto dialogs_manager = std::make_unique<DialogsManager>();
        auto entity_manager = std::make_unique<EntityManager>();
        auto console = std::make_unique<Console>();
        auto worldMapModule = std::make_unique<QGears::WorldMapModule>();

        // Initialize after game managers because it attaches them to script.
        auto script_manager = std::make_unique<ScriptManager>();

        // Set base listener for usual game modules.
        auto frame_listener = std::make_unique<GameFrameListener>(window);
        root->addFrameListener(frame_listener.get());

        // Execute the configuration file to locad values.
        ConfigFile config;
        config.Execute("./data/config.cfg");


        // Initialize UID and run it's scripts.
        ui_manager->Initialise();
        dialogs_manager->Initialise();

        // Run application loop
        QGears::g_ApplicationState = QGears::G_GAME;
        root->startRendering();

        // System modules
        // Thes must be removed first cause this can fire events to console.
        root->removeFrameListener(frame_listener.get());

        // Must be destroyed before the script manager.
        entity_manager.reset();
        ui_manager.reset();
        script_manager.reset();
    }
    catch (const std::runtime_error& ex){
        std::cout << "std::runtime_error thrown: " << ex.what() << std::endl;
    }
    catch (const Ogre::Exception& ex){
        std::cout << "Ogre::Exception thrown: " << ex.what() << std::endl;
    }
    return 0;
}
