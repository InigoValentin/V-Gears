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
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <OgreConfigFile.h>
#include <OgreArchiveManager.h>
#include <OgreLogManager.h>
#include <OgreConfigDialog.h>
#include "common/VGearsApplication.h"
#include "VGearsGameState.h"
#include "data/VGearsAFileManager.h"
#include "data/VGearsBackgroundFileManager.h"
#include "data/VGearsCameraMatrixFileManager.h"
#include "data/VGearsHRCFileManager.h"
#include "data/VGearsLZSFLevelFileManager.h"
#include "data/VGearsPaletteFileManager.h"
#include "data/VGearsPFileManager.h"
#include "data/VGearsRSDFileManager.h"
#include "data/VGearsTexCodec.h"
#include "map/VGearsBackground2DFileManager.h"
#include "map/VGearsWalkmeshFileManager.h"
#include "data/FinalFantasy7/FF7ModelListFileManager.h"
#include "data/VGearsLGPArchiveFactory.h"
#include "data/VGearsTriggersFile.h"
#include "data/VGearsMapListFile.h"
#include "vgears_version.h"

template<> VGears::Application
  *Ogre::Singleton<VGears::Application>::msSingleton = nullptr;

namespace VGears{

    namespace bfs = boost::filesystem;
    namespace bpo = boost::program_options;
    typedef Ogre::ConfigFile::SettingsMultiMap SettingsMultiMap;

    const char* Application::CLI_SECTION_GENERIC("Generic options");
    const char* Application::CLI_HELP("help");
    const char* Application::CLI_HELP_DESCRIPTION("This help message");
    const char* Application::CLI_CONFIG_FILE("config-file");
    const char* Application::CLI_CONFIG_FILE_DESCRIPTION(
      "Path to Ogre config file containing selected rendersystem and resolution"
    );
    const char* Application::CLI_LOG_FILE("log-file");
    const char* Application::CLI_LOG_FILE_DESCRIPTION("Path to log file");
    const char* Application::CLI_PLUGINS_FILE("plugins-file");
    const char* Application::CLI_PLUGINS_FILE_DESCRIPTION(
      "Path to config file containing available plugins"
    );
    const char* Application::CLI_RESOURCES_FILE("resources-file");
    const char* Application::CLI_RESOURCES_FILE_DESCRIPTION(
      "path to config file containing available resources"
    );

    Application::Application(int argc, char *argv[]): _argc(argc), _argv(argv){}

    Application::~Application(){destroyComponents();}

    bool Application::initOgre(bool hideWindow){
        if(_initialized) return false;

        _initialized = true;

        if (_argc && _argv){
            if (!processCommandLine(_argc, _argv)) return false;
        }

        _root = std::make_unique<Ogre::Root>(
          plugins_filename_, config_filename_, log_filename_
        );
        _overlay_system = std::make_unique<Ogre::OverlaySystem>();

        Ogre::Log* default_log(
          Ogre::LogManager::getSingleton().getDefaultLog()
        );
        assert(default_log);
        default_log->setLogDetail(Ogre::LL_BOREME);

        // configure
        // Show the configuration dialog and initialise the system
        // You can skip this and use root.restoreConfig() to load configuration
        // settings if you were sure there are valid ones saved in ogre.cfg
        // IVV ERROR COMPILING
        if(/* !_root->restoreConfig() &&*/ !_root->showConfigDialog(NULL)){
            return false;
        }

        if (hideWindow){
            Ogre::NameValuePairList list;
            list.emplace("hidden", "true");
            _root->initialise(false, getWindowTitle());
            _render_window = _root->createRenderWindow(getWindowTitle(), 1, 1, false, &list);
        }
        else{
            _render_window = _root->initialise(true, getWindowTitle());
        }
        registerArchiveFactories();
        loadResourcesConfig();
        initComponents();
        return true;
    }

    void Application::loadResourcesConfig(){

        // Load resource paths from config file
        Ogre::ConfigFile config_file;
        config_file.load(resources_filename_);

        // Go through all sections & settings in the file
        Ogre::ConfigFile::SectionIterator section(config_file.getSectionIterator());

        Ogre::String section_name, archive_type, archive_name;
        res_mgr_ = (&Ogre::ResourceGroupManager::getSingleton());
        while(section.hasMoreElements()){
            section_name = section.peekNextKey();
            SettingsMultiMap *settings(section.getNext());
            SettingsMultiMap::const_iterator it(settings->begin());
            SettingsMultiMap::const_iterator end(settings->end());
            while(it != end){
                archive_type = it->first;
                archive_name = it->second;
                res_mgr_->addResourceLocation(
                  archive_name, archive_type, section_name, true
                );
                ++ it;
            }
        }
    }

    Ogre::Root* Application::getRoot(){return _root.get();}

    Ogre::RenderWindow* Application::getRenderWindow(){return _render_window;}

    const String& Application::getResourcesFilename(){
        return resources_filename_;
    }

    String Application::getWindowTitle() const {return QG_VERSION_NAME;}

    bool Application::processCommandLine( int argc, char *argv[]){
        bfs::path self(argv[0]);
        const String self_stem(self.stem().string());
        const String config_filename(self_stem + EXT_CONFIG);
        const String log_filename(self_stem + EXT_LOG);

#ifdef  NDEBUG
        const String plugins_filename("plugins" + EXT_CONFIG);
        const String resources_filename("resources" + EXT_CONFIG);
#else
        const String plugins_filename("plugins_d" + EXT_CONFIG);
        const String resources_filename("resources_d" + EXT_CONFIG);
#endif

        bpo::options_description generic(CLI_SECTION_GENERIC);
        generic.add_options()
            (CLI_HELP, CLI_HELP_DESCRIPTION)
            (  CLI_CONFIG_FILE
              , bpo::value< String >()->default_value(config_filename)
              , CLI_CONFIG_FILE_DESCRIPTION
           )
            (  CLI_LOG_FILE
              , bpo::value< String >()->default_value(log_filename)
              , CLI_LOG_FILE_DESCRIPTION
           )
            (  CLI_PLUGINS_FILE
              , bpo::value< String >()->default_value(plugins_filename)
              , CLI_PLUGINS_FILE_DESCRIPTION
           )
            (  CLI_RESOURCES_FILE
              , bpo::value< String >()->default_value(resources_filename)
              , CLI_RESOURCES_FILE_DESCRIPTION
           )
            ;
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(generic).run(), vm);
        if (vm.count(CLI_HELP)){
            std::cout << "Usage: " << self_stem << "\n" << generic << "\n";
            return false;
        }

        bpo::notify(vm);
        config_filename_ = vm[CLI_CONFIG_FILE].as<String>();
        log_filename_ = vm[CLI_LOG_FILE].as<String>();
        plugins_filename_ = vm[CLI_PLUGINS_FILE].as<String>();
        resources_filename_ = vm[CLI_RESOURCES_FILE].as<String>();

        return true;
    }

    void Application::registerArchiveFactories(){
        Ogre::ArchiveManager::getSingleton().addArchiveFactory(
          new LGPArchiveFactory()
        );
    }

    void Application::initComponents(){
        VGears::TexCodec::install();
        VGears::TexCodec::initialise();
        createResourceManagers();
    }

    void Application::destroyComponents(){
        destroyResourceManagers();
        VGears::TexCodec::shutdown();
        VGears::TexCodec::uninstall();
    }

    void Application::createResourceManagers(){
        assert(_resource_managers.empty());
        _resource_managers.emplace_back(
          std::make_shared<VGears::CameraMatrixFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::PaletteFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::WalkmeshFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::ModelListFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::HRCFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::RSDFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::AFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::PFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::LZSFLevelFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::BackgroundFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::Background2DFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::TriggersFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<VGears::MapListFileManager>()
        );
    }

    void Application::destroyResourceManagers(){_resource_managers.clear();}

}
