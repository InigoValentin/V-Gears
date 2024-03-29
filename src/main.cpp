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
#include <Overlay/OgreOverlaySystem.h>
#include <Overlay/OgreOverlayManager.h>
#include <Overlay/OgreFontManager.h>
#include <OIS/OIS.h>
#include "VGearsGameState.h"
#include "common/VGearsApplication.h"
#include "core/AudioManager.h"
#include "core/BattleManager.h"
#include "core/CameraManager.h"
#include "core/ConfigCmdHandler.h"
#include "core/ConfigFile.h"
#include "core/ConfigVarHandler.h"
#include "core/Console.h"
#include "core/DebugDraw.h"
#include "core/EntityManager.h"
#include "core/GameFrameListener.h"
#include "core/InputManager.h"
#include "core/Logger.h"
#include "core/SavemapHandler.h"
#include "core/ScriptManager.h"
#include "core/Timer.h"
#include "core/UiManager.h"
#include "core/particles/ParticleSystemManager.h"
#include "core/DialogsManager.h"
#include "core/TextHandler.h"
#include "data/VGearsLZSFLevelFileManager.h"
#include "data/VGearsLGPArchiveFactory.h"
#include "data/worldmap/WorldmapFileManager.h"
#include "modules/worldmap/WorldmapModule.h"

/**
 * Main application function
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv List of command line arguments.
 * @return 0 on sucess, an error code on error.
 */
int main(int argc, char *argv[]){
    try{
        VGears::Application app(argc, argv);
        if (!app.initOgre()) return 0;
        Ogre::Root *root(app.getRoot());
        Ogre::RenderWindow *window(app.getRenderWindow());
        Ogre::SceneManager *scene_manager(nullptr);

        auto timer = std::make_unique<Timer>();
        auto particle_system_manager = std::make_unique<ParticleSystemManager>();

        // Set scene camera and viewport for other modules.
        // Create this before initialize particle because some of them use
        // scene to create themselves.
        scene_manager = root->createSceneManager(Ogre::ST_GENERIC, "Scene");
        // TODO: Why is this used twice?
        scene_manager->setAmbientLight(Ogre::ColourValue(1, 1, 1));
        scene_manager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
        Ogre::Light *directionalLight(scene_manager->createLight("directionalLight"));
        directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
        directionalLight->setDiffuseColour(Ogre::ColourValue(0.5, 0.5, 0.5));
        directionalLight->setSpecularColour(Ogre::ColourValue(0.0, 0.0, 0.0));
        directionalLight->setDirection(Ogre::Vector3(0, 1, 0));
        // auto fontManager = std::make_unique<Ogre::FontManager>();
        //VGears::MapFileManager* worldManager = new VGears::MapFileManager();

        // Initialize resources.
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/system/", "FileSystem"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/audio/", "FileSystem", "AUDIO"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/fields/", "FileSystem", "FIELDS", true
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/fonts/", "FileSystem", "FONTS"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/gamedata/", "FileSystem", "GAMEDATA"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/images/", "FileSystem", "IMAGES", true
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/models/fields/", "FileSystem", "FIELD_MODELS"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/models/battle/", "FileSystem", "BATTLE_MODELS", true
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/screens/", "FileSystem", "SCREENS"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/scripts/", "FileSystem", "SCRIPTS"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/system/", "FileSystem", "SYSTEM"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/texts/", "FileSystem", "TEXTS"
        );
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
          "./data/wm", "FileSystem", "WORLD_MAP"
        );
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        // Initialize it before console because it may use it
        auto config_var_manager = std::make_unique<ConfigVarHandler>();
        auto config_cmd_manager = std::make_unique<ConfigCmdHandler>();
        auto debug_draw = std::make_unique<DebugDraw>();

        // Initialize before GameFrameListener, but after ConfigCmdManager
        auto input_manager = std::make_unique<InputManager>();

        auto audio_manager = std::make_unique<AudioManager>();

        auto savemap_manager = std::make_unique<SavemapHandler>();

        // Create this earlier than DisplayFrameListener cause it can fire
        // events there
        auto camera_manager = std::make_unique<CameraManager>();
        auto text_manager = std::make_unique<TextHandler>();
        auto ui_manager = std::make_unique<UiManager>();
        auto dialogs_manager = std::make_unique<DialogsManager>();
        auto entity_manager = std::make_unique<EntityManager>();
        auto battle_manager = std::make_unique<BattleManager>();
        auto console = std::make_unique<Console>();
        auto worldMapModule = std::make_unique<VGears::WorldmapModule>();

        // Initialize after game managers because it attaches them to script.
        auto script_manager = std::make_unique<ScriptManager>();

        // Set base listener for usual game modules.
        auto frame_listener = std::make_unique<GameFrameListener>(window);
        root->addFrameListener(frame_listener.get());

        // Execute the configuration file to locad values.
        ConfigFile config;
        config.Execute("./data/config/config.cfg");

        // Initialize UID and run it's scripts.
        ui_manager->Initialise();
        dialogs_manager->Initialise();

        // Run application loop
        VGears::g_ApplicationState = VGears::G_GAME;
        root->startRendering();

        // System modules
        // Thes must be removed first cause this can fire events to console.
        root->removeFrameListener(frame_listener.get());

        // Must be destroyed before the script manager.
        entity_manager.reset();
        battle_manager.reset();
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
