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

#include <OgreResource.h>

namespace QGears{

    class FLevelFile;

    /**
     * A flevel texture loader.
     */
    class FLevelTextureLoader : public Ogre::ManualResourceLoader{

        public:

            /**
             * Constructor.
             *
             * @param flevel_file[in] The file to load textures from.
             */
            explicit FLevelTextureLoader(FLevelFile &flevel_file);

            /**
             * Destructor.
             */
            virtual ~FLevelTextureLoader();

            /**
             * Loads a resource.
             *
             * @param resource[in] the resource to load.
             */
            virtual void loadResource(Ogre::Resource *resource);

        private:

            /**
             * The flevel file.
             */
            FLevelFile &flevel_file_;

    };
}
