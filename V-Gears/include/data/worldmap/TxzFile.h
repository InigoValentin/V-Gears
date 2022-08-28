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

#include "common/QGearsResource.h"

namespace QGears {

    /**
     * Handles TXZ files.
     *
     * TXZ files contains texture data used in the world map.
     */
    class TxzFile : public Resource{

        public:

            /**
             * The resource type.
             */
            static const String RESOURCE_TYPE;

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
            TxzFile(
              Ogre::ResourceManager* creator, const Ogre::String& name,
              Ogre::ResourceHandle handle, const Ogre::String& group,
              bool isManual = false, Ogre::ManualResourceLoader* loader = 0
            );

            /**
             * Destructor.
             */
            virtual ~TxzFile();

        private:

            /**
             * Loads the file.
             */
            virtual void loadImpl() override final;

            /**
             * Unloads the file.
             */
            virtual void unloadImpl() override final;
    };

    typedef Ogre::SharedPtr<TxzFile> TxzFilePtr;

}
