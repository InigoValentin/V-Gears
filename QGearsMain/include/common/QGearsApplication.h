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

#pragma once

#include <OgreRoot.h>
#include <OgreOverlaySystem.h>
#include <memory>
#include "TypeDefine.h"

/**
 * The V-Gears main applicacion
 */
namespace QGears{

    class Application : public Ogre::Singleton<Application>{

        public:

            /**
             * Constructor.
             *
             * @param pluginsFileName[in] Path to the plugins file.
             * @param resourcesFile[in] Path to the resources file.
             * @param logFileName[in] Path to the log file.
             */
            Application(
              Ogre::String pluginsFileName,
              Ogre::String resourcesFile, Ogre::String logFileName
            ): plugins_filename_(pluginsFileName),
               resources_filename_(resourcesFile), log_filename_(logFileName)
            {}

            /**
             * Constructor.
             *
             * @param argc[in] Number of command line arguments.
             * @param argv[in] List of command line arguments.
             */
            Application(int argc, char *argv[]);

            /**
             * Destructor.
             */
            virtual ~Application();

            /**
             * Initializes the Ogre rendering system.
             *
             * @param hideWindow[in] True to not show the main window, false to
             * show it.
             * @return True if the system was succesfully initiated, false
             * if there were errors or it was already initialized.
             */
            bool initOgre(bool hideWindow = false);

            /**
             * Retrieves the Ogre system main component.
             *
             * @return Pointer to the Ogre main component.
             */
            Ogre::Root* getRoot(void);

            /**
             * Retrieves the Ogre main window.
             *
             * @return Pointer to the Ogre window.
             */
            Ogre::RenderWindow* getRenderWindow(void);

            /**
             * Retrieves the Ogre resources file name.
             *
             * @return Ogre resource configuration file name.
             */
            const String& getResourcesFilename(void);

            /**
             * Retrieves the Ogre resource manager.
             *
             * @return Pointer to the Ogre resource manager.
             */
            Ogre::ResourceGroupManager* ResMgr() {return res_mgr_;}

        protected:

            /**
             * Retrieves the main window title.
             *
             * @return Th windo title.
             */
            String getWindowTitle(void) const;

            /**
             * Processes command line arguments.
             *
             * @param argc[in] Number of command line arguments.
             * @param argv[in] List of command line arguments.
             * @return True if all arguments were successfully processed, false
             * on error
             */
            bool processCommandLine(int argc, char *argv[]);

            /**
             * Registers an {@see LGPArchiveFactory} with the Ogre archive
             * manager
             */
            void registerArchiveFactories(void);

            /**
             * Loads the resource configuration from  the file.
             */
            void loadResourcesConfig(void);

            /**
             * Initializes components.
             *
             * Initializes the TexCodec component and the resource manager.
             */
            void initComponents(void);

            /**
             * Unloads components.
             *
             * Unoads the TexCodec component and the resource manager.
             */
            void destroyComponents(void);

            /**
             * Initializes the resource manager.
             */
            void createResourceManagers(void);

            /**
             * Unloads the resource manager.
             */
            void destroyResourceManagers(void);

        private:

            /**
             * List of resource managers.
             */
            typedef std::vector<std::shared_ptr<Ogre::ResourceManager>>
              ResourceManagerVector;

            /**
             * String for the command line help text.
             */
            static const char* CLI_SECTION_GENERIC;

            /**
             * String for the command line arguments.
             */
            static const char* CLI_HELP;

            /**
             * String for the command line help text.
             */
            static const char* CLI_HELP_DESCRIPTION;

            /**
             * String for the command line arguments.
             */
            static const char* CLI_CONFIG_FILE;

            /**
             * String for the command line help text.
             */
            static const char* CLI_CONFIG_FILE_DESCRIPTION;

            /**
             * String for the command line arguments.
             */
            static const char* CLI_LOG_FILE;

            /**
             * String for the command line help text.
             */
            static const char* CLI_LOG_FILE_DESCRIPTION;

            /**
             * String for the command line arguments.
             */
            static const char* CLI_PLUGINS_FILE;

            /**
             * String for the command line help text.
             */
            static const char* CLI_PLUGINS_FILE_DESCRIPTION;

            /**
             * String for the command line arguments.
             */
            static const char* CLI_RESOURCES_FILE;

            /**
             * String for the command line help text.
             */
            static const char* CLI_RESOURCES_FILE_DESCRIPTION;

            /**
             * Number of arguments assed by command line.
             */
            int _argc = 0;

            /**
             * List of arguments passed by command line.
             */
            char **_argv = nullptr;

            /**
             * Path to the configuration file.
             */
            String config_filename_;

            /**
             * Path to the log file.
             */
            String log_filename_;

            /**
             * Path to the plugin configuration file.
             */
            String plugins_filename_;

            /**
             * Path to the resource configuration file.
             */
            String resources_filename_;

            /**
             * Flag for initialization status.
             */
            bool _initialized = false;

            /**
             * Ogre main component..
             */
            std::unique_ptr<Ogre::Root> _root;

            /**
             *The ogre overlay system.
             */
            std::unique_ptr<Ogre::OverlaySystem> _overlay_system;

            /**
             * The Ogre main window.
             */
            Ogre::RenderWindow* _render_window = nullptr; // Not owned

            /**
             * List of Ogre resource managers.
             */
            ResourceManagerVector _resource_managers;

            /**
             * Ogre resource group manager.
             */
            Ogre::ResourceGroupManager* res_mgr_ = nullptr; // Not owned
    };
}
