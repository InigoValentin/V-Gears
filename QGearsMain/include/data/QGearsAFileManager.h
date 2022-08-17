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

#include <OgreResourceManager.h>
#include "QGearsPrerequisites.h"
#include "QGearsAFile.h"

namespace QGears{

    /**
     * A manager for A files.
     */
    class _QGearsExport AFileManager :
      public Ogre::ResourceManager, public Ogre::Singleton<AFileManager>
    {

        public:

            /**
             * Constructor.
             */
            AFileManager();

            /**
             * Destructor.
             */
            virtual ~AFileManager();

            /**
             * Retrieves a singleton to the manager.
             */
            static AFileManager& GetSingleton();

            /**
             * Retrieves a pointer to the manager singleton.
             */
            static AFileManager* GetSingletonPtr();

        protected:

            /**
             * Loads the manager.
             *
             * @param name[in] The unique name of the manager.
             * @param handle[in] @todo Understand and document.
             * @param group[in] The name of the resource group to which this
             * resource belong.
             * @param is_manual[in] True if the resource is manually loaded,
             * false otherwise.
             * @param loader[in] Pointer to a ManualResourceLoader
             * implementation which will be called when the Resource wishes to
             * load (should be supplied if is_manual is set to true). It can be
             * null, but the Resource will never be able to reload if anything
             * ever causes it to unload. Therefore provision of a proper
             * ManualResourceLoader instance is strongly recommended.
             * @param create_params[in] Unused.
             */
            virtual Ogre::Resource *createImpl(
              const Ogre::String &name, Ogre::ResourceHandle handle,
              const Ogre::String &group, bool is_manual,
              Ogre::ManualResourceLoader *loader,
              const Ogre::NameValuePairList *create_params
            ) override final;

    };
}
