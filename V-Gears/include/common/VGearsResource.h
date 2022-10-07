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

#include <OgreDataStream.h>
#include <OgreResource.h>
#include <OgreResourceGroupManager.h>
#include "common/TypeDefine.h"
#include "VGearsPrerequisites.h"

namespace VGears{

    /**
     * A resource.
     */
    class _VGearsExport Resource : public Ogre::Resource{

        public:

            /**
             * Constructor.
             *
             * Initializes a resource.
             *
             * @param[in] creator The resource manager that handles the
             * resource creation.
             * @param[in] name A name for the resource.
             * @param[in] handle Ogre object that will handle the resource.
             * @param[in] group Name of the group to include the resource in.
             * @param[in] manual Indicates if the resource is a {@see
             * VGearsManualObject}.
             * @param[in] loader Ogre resource loader for the resource.
             */
            Resource(
              Ogre::ResourceManager *creator, const String &name, Ogre::ResourceHandle handle,
              const String &group, bool manual, Ogre::ManualResourceLoader *loader
            ): Ogre::Resource(creator, name, handle, group, manual, loader){}

            /**
             * Destructor.
             */
            virtual ~Resource() = default;

        protected:

            /**
             * Opens the resource.
             *
             * @return A pointer to the resource.
             */
            virtual Ogre::DataStreamPtr openResource(){
                // Last parameter to true to throw an exception.
                // Change when ready.
                return Ogre::ResourceGroupManager::getSingleton().openResource(
                  mName, mGroup, this, false
                );
            }

    };
}

