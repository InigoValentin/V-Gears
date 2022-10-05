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
#include "VGearsRSDFile.h"

namespace VGears{

    /**
     * A manager for RDS files.
     */
    class _VGearsExport RSDFileManager :
      public Ogre::ResourceManager, public Ogre::Singleton<RSDFileManager>
    {
        public:

            /**
             * Constructor.
             */
            RSDFileManager();

            /**
             * Destructor.
             */
            virtual ~RSDFileManager();

            /**
             * Retrieves a singleton to the manager.
             */
            static RSDFileManager& GetSingleton();

            /**
             * Retrieves a pointer to the manager singleton.
             */
            static RSDFileManager* GetSingletonPtr();

            /**
             * Parses the file script and adds the resource.
             *
             * @param[in] stream The contents of the file.
             * @param[in] group_name Name of the group to add the resource to.
             */
            virtual void ParseScript(
              Ogre::DataStreamPtr &stream, const String &group_name
            );

        protected:

            /**
             * Loads the manager.
             *
             * @param[in] name The unique name of the manager.
             * @param[in] handle @todo Understand and document.
             * @param[in] group The name of the resource group to which this
             * resource belong.
             * @param[in] is_manual True if the resource is manually loaded,
             * false otherwise.
             * @param[in] loader Pointer to a ManualResourceLoader
             * implementation which will be called when the Resource wishes to
             * load (should be supplied if is_manual is set to true). It can be
             * null, but the Resource will never be able to reload if anything
             * ever causes it to unload. Therefore provision of a proper
             * ManualResourceLoader instance is strongly recommended.
             * @param[in] create_params Unused.
             */
            Ogre::Resource *createImpl(
              const Ogre::String &name, Ogre::ResourceHandle handle,
              const Ogre::String &group, bool is_manual,
              Ogre::ManualResourceLoader *loader,
              const Ogre::NameValuePairList *create_params
            );

    };
}
