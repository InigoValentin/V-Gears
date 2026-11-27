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

#include <cstdlib>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <OgreConfigFile.h>
#include <OgreArchiveManager.h>
#include <OgreLogManager.h>
#include <OgreConfigDialog.h>
#include "app/Application.h"
#include "common/DataDirectory.h"
#include "common/OgrePlugins.h"
#include "engine/GameState.h"
#include "formats/model/AFileManager.h"
#include "formats/field/BackgroundFileManager.h"
#include "formats/field/CameraMatrixFileManager.h"
#include "formats/model/HRCFileManager.h"
#include "formats/field/LZSFLevelFileManager.h"
#include "formats/field/PaletteFileManager.h"
#include "formats/model/PFileManager.h"
#include "formats/model/RSDFileManager.h"
#include "formats/texture/TexCodec.h"
#include "formats/field/Background2DFileManager.h"
#include "formats/field/WalkmeshFileManager.h"
#include "formats/model/FF7ModelListFileManager.h"
#include "formats/lgp/LGPArchiveFactory.h"
#include "formats/field/TriggersFile.h"
#include "formats/field/MapListFile.h"
#include "Version.h"
#include "common/InstallStamp.h"
#include <OgreControllerManager.h>
#include <OgreDefaultHardwareBufferManager.h>
#include <OgreMaterialManager.h>
#include <OgreMeshManager.h>

/**
 * Application singleton.
 */
template<> VGears::Application* Ogre::Singleton<VGears::Application>::msSingleton = nullptr;

/**
 * Directory with the Ogre plugins, found when V-Gears was built. Set by CMake (VGEARS_OGRE_PLUGIN_DIR).
 */
#ifndef VGEARS_OGRE_PLUGIN_DIR
#define VGEARS_OGRE_PLUGIN_DIR ""
#endif

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
    const char* Application::CLI_DATA_DIR("data-dir");
    const char* Application::CLI_DATA_DIR_DESCRIPTION(
      "Directory with the game data and configuration, where saves and logs are written. "
      "Defaults to $VGEARS_DATA_DIR, the current directory if it has the V-Gears files, or ~/.v-gears"
    );

    Application::Application(int argc, char *argv[]): _argc(argc), _argv(argv){}

    Application::~Application(){destroyComponents();}

    bool Application::initOgre(bool hideWindow){
        if(_initialized) return false;

        _initialized = true;

        if (_argc && _argv){
            if (!processCommandLine(_argc, _argv)) return false;
        }

        // No plugins file is passed to Ogre: the plugins are looked for in several places, see loadPlugins.
        _root = std::make_unique<Ogre::Root>("", config_filename_, log_filename_);
        loadPlugins();
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
            //
            _render_window = _root->createRenderWindow(getWindowTitle(), 1, 1, false, &list);
        }
        else{
            _render_window = _root->initialise(false, getWindowTitle());
            // TODO: Get actual resolution from config
            _render_window = _root->createRenderWindow(getWindowTitle(), 1024, 768, false);
        }
        registerArchiveFactories();
        loadResourcesConfig();
        initComponents();
        return true;
    }

    bool Application::initOgreHeadless(){
        if (_initialized) return false;
        _initialized = true;
        _headless = true;
        // Ogre writes its messages to the log file only, not to the terminal: the programs that convert data have
        // their own output. Created before the root, which then doesn't create its own log.
        if (Ogre::LogManager::getSingletonPtr() == nullptr){
            _log_manager = std::make_unique<Ogre::LogManager>();
            _log_manager->createLog(log_filename_, true, false, false);
        }
        // No plugins file is passed to Ogre: the plugins are looked for in several places, see loadPlugins.
        _root = std::make_unique<Ogre::Root>("", config_filename_, log_filename_);
        loadPlugins(true);
        _overlay_system = std::make_unique<Ogre::OverlaySystem>();
        Ogre::LogManager::getSingleton().getDefaultLog()->setLogDetail(Ogre::LL_BOREME);
        // Without a render system nobody creates the hardware buffer manager, and meshes need one.
        _buffer_manager = std::make_unique<Ogre::DefaultHardwareBufferManager>();
        // What Ogre::Root::initialise and the render system do, except what needs a graphics driver: the
        // controllers (used by animated materials), a texture manager for the textures named by the materials,
        // the default material that new materials copy, and the built-in meshes.
        if (Ogre::ControllerManager::getSingletonPtr() == nullptr)
            _controller_manager = std::make_unique<Ogre::ControllerManager>();
        _texture_manager = std::make_unique<Ogre::DefaultTextureManager>();
        Ogre::MaterialManager::getSingleton().initialise();
        Ogre::MeshManager::getSingleton()._initialise();
        registerArchiveFactories();
        loadResourcesConfig();
        initComponents();
        return true;
    }

    void Application::loadPlugins(bool skip_render_systems){
        const bfs::path plugins_file(bfs::absolute(bfs::path(plugins_filename_)));
        OgrePlugins::Config config;
        if (!OgrePlugins::ReadConfig(plugins_file, config))
            throw std::runtime_error("The Ogre plugins file can't be read: " + plugins_file.string());
        const std::vector<bfs::path> directories(OgrePlugins::SearchDirectories(
          plugins_file.parent_path(), config.folder,
          DataDirectory::ExecutableDirectory(_argc > 0 && _argv ? _argv[0] : nullptr), VGEARS_OGRE_PLUGIN_DIR
        ));
        const String ogre_version(
          std::to_string(OGRE_VERSION_MAJOR) + "." + std::to_string(OGRE_VERSION_MINOR) + "."
          + std::to_string(OGRE_VERSION_PATCH)
        );
        for (const String& name : config.plugins){
            if (skip_render_systems && name.compare(0, 13, "RenderSystem_") == 0){
                Ogre::LogManager::getSingleton().logMessage("Not loading Ogre plugin " + name + ": no render system");
                continue;
            }
            const bfs::path file(OgrePlugins::Find(name, directories, ogre_version));
            if (file.empty()){
                String searched;
                for (const bfs::path& directory : directories) searched += "\n  " + directory.string();
                throw std::runtime_error(
                  "The Ogre plugin " + name + " (listed in " + plugins_file.string() + ") was not found in:"
                  + searched
                );
            }
            Ogre::LogManager::getSingleton().logMessage("Loading Ogre plugin " + file.string());
            try{
                _root->loadPlugin(file.string());
            }
            catch (const Ogre::Exception& e){
                // The exception can point to strings of the plugin library, which is unloaded when the root is
                // destroyed, before the exception is caught in main. Copy the message now.
                throw std::runtime_error(
                  "The Ogre plugin " + file.string() + " could not be loaded: " + e.getFullDescription()
                );
            }
        }
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
                res_mgr_->addResourceLocation(archive_name, archive_type, section_name, true);
                ++ it;
            }
        }
    }

    Ogre::Root* Application::getRoot(){return _root.get();}

    Ogre::RenderWindow* Application::getRenderWindow(){return _render_window;}

    String Application::getWindowTitle() const {return VGEARS_VERSION_SIGNATURE;}

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
            (  CLI_DATA_DIR
              , bpo::value< String >()
              , CLI_DATA_DIR_DESCRIPTION
           )
            ;
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(generic).run(), vm);
        if (vm.count(CLI_HELP)){
            std::cout << "Usage: " << self_stem << "\n" << generic << "\n";
            return false;
        }

        bpo::notify(vm);

        // The game changes into the data directory, so the files given in the command line are made absolute first.
        // The default ones are left relative, to be found in the data directory.
        const bfs::path start_directory(bfs::current_path());
        auto file_option = [&](const char* name) -> String{
            const String value(vm[name].as<String>());
            if (vm[name].defaulted() || value.empty()) return value;
            return bfs::absolute(bfs::path(value), start_directory).lexically_normal().string();
        };
        config_filename_ = file_option(CLI_CONFIG_FILE);
        log_filename_ = file_option(CLI_LOG_FILE);
        plugins_filename_ = file_option(CLI_PLUGINS_FILE);
        resources_filename_ = file_option(CLI_RESOURCES_FILE);

        String requested_directory;
        if (vm.count(CLI_DATA_DIR)) requested_directory = vm[CLI_DATA_DIR].as<String>();
        else if (const char* env = std::getenv(DataDirectory::ENV_DATA_DIR)) requested_directory = env;
        const DataDirectory::Resolution data_directory(DataDirectory::Resolve(
          requested_directory, start_directory, DataDirectory::DefaultDirectory(),
          DataDirectory::TemplateCandidates(DataDirectory::ExecutableDirectory(argv[0]))
        ));
        if (!data_directory.error.empty()){
            std::cerr << "V-Gears can't start: " << data_directory.error << std::endl;
            return false;
        }
        if (data_directory.seeded){
            std::cout << "Copied " << data_directory.copied_files << " V-Gears files from "
              << data_directory.template_directory.string() << " to " << data_directory.directory.string()
              << ". Run v-gears-installer to add the data of the original game." << std::endl;
        }
        boost::system::error_code ec;
        bfs::current_path(data_directory.directory, ec);
        if (ec){
            std::cerr << "V-Gears can't start: can't use the data directory " << data_directory.directory.string()
              << ": " << ec.message() << std::endl;
            return false;
        }
        std::cout << "Data directory: " << data_directory.directory.string() << std::endl;
        // The record of the installations says if the game data must be installed again.
        bool recorded = false;
        for (const std::string& problem : InstallStamp::Problems(data_directory.directory, &recorded))
            std::cout << "Warning: " << problem << std::endl;
        if (!recorded && bfs::exists(data_directory.directory / "data" / "fields" / "_fields.xml", ec)){
            std::cout << "Warning: the game data was installed by an older version of v-gears-installer, and may be "
              "outdated. Install it again with v-gears-installer." << std::endl;
        }
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
