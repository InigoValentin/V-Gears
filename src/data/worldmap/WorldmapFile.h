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

#include "common/VGearsResource.h"

namespace VGears {

    /**
     * Handles a world map file.
     */
    class WorldmapFile : public Resource{

        public:

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

            /**
             * Constructor.
             *
             * @param[in] creator Pointer to the ResourceManager that is
             * creating this resource.
             * @param[in] name The unique name of the resource.
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
             */
            WorldmapFile(
              Ogre::ResourceManager* creator, const Ogre::String& name,
              Ogre::ResourceHandle handle, const Ogre::String& group,
              bool is_manual = false, Ogre::ManualResourceLoader* loader = 0
            );

            /**
             * Destructor.
             */
            virtual ~WorldmapFile();

        private:

            /**
             * Loads the map file.
             */
            virtual void loadImpl() override final;

            /**
             * Unloads the map file.
             */
            virtual void unloadImpl() override final;
    };

    typedef Ogre::SharedPtr<WorldmapFile> MapFilePtr;

}
