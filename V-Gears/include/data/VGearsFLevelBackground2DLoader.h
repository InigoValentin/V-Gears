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
#include "data/VGearsBackgroundFile.h"
#include "map/VGearsBackground2DFile.h"

namespace VGears{

    class FLevelFile;

    /**
     * Handles 2D background loading.
     */
    class FLevelBackground2DLoader : public Ogre::ManualResourceLoader{

        public:

            /**
             * Constructor.
             *
             * @param flevel_file[in] Level file.
             */
            explicit FLevelBackground2DLoader(FLevelFile& flevel_file);

            /**
             * Destructor.
             */
            virtual ~FLevelBackground2DLoader();

            /**
             * Loads a resource.
             *
             * @param resource[in] The resource to load.
             */
            virtual void loadResource(Ogre::Resource *resource);

        protected:

            typedef Background2DFile::TileList TileList;

            typedef BackgroundFile::Layer Layer;

            typedef BackgroundFile::SpriteData SpriteData;

            typedef BackgroundFile::SpriteList SpriteList;

        private:

            /**
             * The flevel file.
             */
            FLevelFile  &flevel_file_;
    };
}
