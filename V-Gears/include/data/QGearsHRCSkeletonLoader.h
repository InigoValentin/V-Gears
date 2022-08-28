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
#include "common/TypeDefine.h"

namespace QGears{
    class HRCFile;

    /**
     * A loader for skeletons.
     */
    class HRCSkeletonLoader : public Ogre::ManualResourceLoader{

        public:

            /**
             * Constructor.
             *
             * @param hrc_file[in] The HRC file to load from.
             */
            explicit HRCSkeletonLoader(HRCFile &hrc_file);

            /**
             * Destructor.
             */
            virtual ~HRCSkeletonLoader();

            /**
             * Loads a resource.
             *
             * @param resource[in] The resource to load.
             */
            virtual void loadResource( Ogre::Resource *resource );

        protected:

            /**
             * The name of the root bone.
             */
            static const String ROOT_BONE_NAME;

            /**
             * The root orientation
             */
            static const Ogre::Quaternion ROOT_ORIENTATION;

            /**
             * Creates a base orientation for the skeleton.
             */
            static Ogre::Quaternion CreateRootOrientation();

        private:

            HRCFile  &hrc_file_;
    };
}
