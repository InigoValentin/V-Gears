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

#include "QGearsFLevelFile.h"

namespace QGears{

    /**
     * Handles LZS compressed flevel files.
     */
    class LZSFLevelFile : public FLevelFile{

        public:

            /**
             * Constructor.
             *
             * @param creator[in] Pointer to the ResourceManager that is
             * creating this resource.
             * @param name[in] The unique name of the resource.
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
             */
            LZSFLevelFile(
              Ogre::ResourceManager *creator, const String &name,
              Ogre::ResourceHandle handle, const String &group,
              bool is_manual = false, Ogre::ManualResourceLoader *loader = NULL
            );

            /**
             * Destructor.
             */
            virtual ~LZSFLevelFile();

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

        protected:

            /**
             * Checks the resource type.
             *
             * @return The resource type.
             */
            virtual const String& getResourceType() const;

            /**
             * Opens the resource.
             */
            virtual Ogre::DataStreamPtr openResource() override;

    };
}
