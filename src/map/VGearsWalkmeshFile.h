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
#include "core/Walkmesh.h"

namespace VGears{

    /**
     * Handles walkmesh files.
     */
    class WalkmeshFile : public Resource{

        public:

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
            WalkmeshFile(
              Ogre::ResourceManager* creator, const String &name,
              Ogre::ResourceHandle handle, const String& group,
              bool is_manual = false,
              Ogre::ManualResourceLoader* loader = NULL
            );

            /**
            * Destructor.
            */
            virtual ~WalkmeshFile();

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

            typedef ::WalkmeshTriangle Triangle;

            typedef std::vector<Triangle> TriangleList;

            /**
             * Retrieves the list of triangles.
             */
            virtual TriangleList& GetTriangles();

        protected:

            /**
             * Loads the file.
             */
            virtual void loadImpl() override;

            /**
             * Unloads the file.
             */
            virtual void unloadImpl() override;

            /**
             * Calculates the size of the palette.
             *
             * @return The size of the palette.
             * @todo Units?
             */
            virtual size_t calculateSize() const;

        private:
            TriangleList triangles_;
    };

    typedef Ogre::SharedPtr<WalkmeshFile> WalkmeshFilePtr;
}
