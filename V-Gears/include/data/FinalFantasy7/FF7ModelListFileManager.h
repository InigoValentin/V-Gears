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
#include "VGearsPrerequisites.h"
#include "FF7ModelListFile.h"

namespace VGears{

    namespace FF7{

        /**
         * A manager for model list files.
         */
        class _VGearsExport ModelListFileManager :
          public Ogre::ResourceManager,
          public Ogre::Singleton<ModelListFileManager>
        {
            public:

                /**
                 * Constructor.
                 */
                ModelListFileManager();

                /**
                 * Destructor.
                 */
                virtual ~ModelListFileManager();

                /**
                 * Retrieves a singleton to the manager.
                 */
                static ModelListFileManager& GetSingleton();

                /**
                 * Retrieves a pointer to the manager singleton.
                 */
                static ModelListFileManager *GetSingletonPtr();

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
                Ogre::Resource *createImpl(
                  const Ogre::String &name, Ogre::ResourceHandle handle,
                  const Ogre::String &group, bool is_manual,
                  Ogre::ManualResourceLoader *loader,
                  const Ogre::NameValuePairList *create_params
                );
        };
    }
}
