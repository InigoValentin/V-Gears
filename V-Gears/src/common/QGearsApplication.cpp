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
#include "common/QGearsApplication.h"
#include "QGearsGameState.h"
#include "data/QGearsAFileManager.h"
#include "data/QGearsBackgroundFileManager.h"
#include "data/QGearsCameraMatrixFileManager.h"
#include "data/QGearsHRCFileManager.h"
#include "data/QGearsLZSFLevelFileManager.h"
#include "data/QGearsPaletteFileManager.h"
#include "data/QGearsPFileManager.h"
#include "data/QGearsRSDFileManager.h"
#include "data/QGearsTexCodec.h"
#include "map/QGearsBackground2DFileManager.h"
#include "map/QGearsWalkmeshFileManager.h"
#include "data/FinalFantasy7/FF7ModelListFileManager.h"
#include "data/QGearsLGPArchiveFactory.h"
#include "data/QGearsTriggersFile.h"
#include "data/QGearsMapListFile.h"
#include "common/make_unique.h"
#include "vgears_version.h"

template<> QGears::Application
  *Ogre::Singleton<QGears::Application>::msSingleton = nullptr;

namespace QGears{

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
        QGears::TexCodec::install();
        QGears::TexCodec::initialise();
        createResourceManagers();
    }

    void Application::destroyComponents(){
        destroyResourceManagers();
        QGears::TexCodec::shutdown();
        QGears::TexCodec::uninstall();
    }

    void Application::createResourceManagers(){
        assert(_resource_managers.empty());
        _resource_managers.emplace_back(
          std::make_shared<QGears::CameraMatrixFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::PaletteFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::WalkmeshFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::FF7::ModelListFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::HRCFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::RSDFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::AFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::PFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::LZSFLevelFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::BackgroundFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::Background2DFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::TriggersFileManager>()
        );
        _resource_managers.emplace_back(
          std::make_shared<QGears::MapListFileManager>()
        );
    }

    void Application::destroyResourceManagers(){_resource_managers.clear();}

}
