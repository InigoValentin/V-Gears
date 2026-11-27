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
#include <OgreHardwareBufferManager.h>
#include <OgreLogManager.h>
#include <OgreControllerManager.h>
#include <OgreTextureManager.h>
#include <memory>
#include "common/TypeDefine.h"
#include "engine/ApplicationInterface.h"

namespace VGears{

    /**
     * The V-Gears main applicacion
     */
    class Application : public Ogre::Singleton<Application>, public ApplicationInterface{

        public:

            /**
             * Constructor.
             *
             * @param[in] pluginsFileName Path to the plugins file.
             * @param[in] resourcesFile Path to the resources file.
             * @param[in] logFileName Path to the log file.
             */
            Application(
              Ogre::String pluginsFileName, Ogre::String resourcesFile, Ogre::String logFileName
            ): plugins_filename_(pluginsFileName),
               resources_filename_(resourcesFile), log_filename_(logFileName)
            {}

            /**
             * Constructor.
             *
             * @param[in] argc Number of command line arguments.
             * @param[in] argv List of command line arguments.
             */
            Application(int argc, char *argv[]);

            /**
             * Destructor.
             */
            virtual ~Application();

            /**
             * Initializes the Ogre rendering system.
             *
             * @param[in] hideWindow True to not show the main window, false to
             * show it.
             * @return True if the system was succesfully initiated, false
             * if there were errors or it was already initialized.
             */
            bool initOgre(bool hideWindow = false);

            /**
             * Initializes Ogre without a render system, for programs that only convert data.
             *
             * No window is created, so no display or graphics driver is needed. The render system plugins in the
             * plugins file are not loaded, and meshes and skeletons are kept in system memory (see
             * Ogre::DefaultHardwareBufferManager). Materials can name textures, but no texture is created (see
             * Ogre::DefaultTextureManager). Nothing can be rendered.
             *
             * @return True if the system was succesfully initiated, false if there were errors or it was already
             * initialized.
             */
            bool initOgreHeadless();

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
            Ogre::RenderWindow* getRenderWindow() override;

            /**
             * The overlay system (it must be added to a scene manager as a render queue listener to draw overlays).
             */
            Ogre::OverlaySystem* GetOverlaySystem() override{return _overlay_system.get();}

            /**
             * Retrieves the Ogre resource manager.
             *
             * @return Pointer to the Ogre resource manager.
             */
            Ogre::ResourceGroupManager* ResMgr() {return res_mgr_;}

            /**
             * Re-finalizes singleton registration after full object construction.
             * 
             * Call this method immediately after constructing the Application
             * to ensure the singleton pointer is properly registered after the
             * object's vptr is fully initialized (avoids UBSAN vptr warnings).
             * 
             * @param[in] instance Pointer to the Application instance.
             */
            static void FinalizeSingletonRegistration(Application* instance) {
                msSingleton = instance;
            }

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
             * It also decides the data directory, copies the V-Gears files there if it has none (see
             * {@see DataDirectory::Resolve}) and makes it the working directory.
             *
             * @param[in] argc Number of command line arguments.
             * @param[in] argv List of command line arguments.
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
             * Loads the Ogre plugins listed in the plugins file.
             *
             * Each plugin is looked for in the directories given by {@see OgrePlugins::SearchDirectories}.
             *
             * @param[in] skip_render_systems True to not load the render system plugins.
             * @throws std::runtime_error If the plugins file can't be read or a plugin is not found.
             */
            void loadPlugins(bool skip_render_systems = false);

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
             * String for the command line arguments.
             */
            static const char* CLI_DATA_DIR;

            /**
             * String for the command line help text.
             */
            static const char* CLI_DATA_DIR_DESCRIPTION;

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
             * True if Ogre was initialized without a render system.
             */
            bool _headless = false;

            /**
             * Hardware buffer manager used without a render system, that keeps the buffers in system memory.
             *
             * It is declared before the root so it is destroyed after it: the meshes, destroyed with the root,
             * notify the buffer manager when their buffers are released.
             */
            std::unique_ptr<Ogre::HardwareBufferManager> _buffer_manager;

            /**
             * Log manager created without a render system, so Ogre's messages only go to the log file.
             *
             * The root uses it instead of creating its own. Declared before the root so it is destroyed after it.
             */
            std::unique_ptr<Ogre::LogManager> _log_manager;

            /**
             * Controller manager used without a render system, if the root didn't create one.
             *
             * Declared before the root for the same reason as the buffer manager.
             */
            std::unique_ptr<Ogre::ControllerManager> _controller_manager;

            /**
             * Ogre main component.
             */
            std::unique_ptr<Ogre::Root> _root;

            /**
             * Texture manager used without a render system. Materials can name textures, but no texture is created.
             *
             * Declared after the root so it is destroyed before it, as the texture manager of a render system is.
             */
            std::unique_ptr<Ogre::TextureManager> _texture_manager;

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
