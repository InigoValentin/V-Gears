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
#include "VGearsHRCFile.h"

namespace VGears{

    /**
     * A loader for skeleton meshes.
     */
    class HRCMeshLoader : public Ogre::ManualResourceLoader{

        public:

            /**
             * Constructor.
             *
             * @param[in] hrc_file The HRC file to load from.
             */
            explicit HRCMeshLoader(HRCFile &hrc_file);

            /**
             * Destructor.
             */
            virtual ~HRCMeshLoader();

            /**
             * Loads a resource.
             *
             * @param[in] resource The resource to load.
             */
            virtual void loadResource(Ogre::Resource *resource);

        protected:

            /**
             * Loads a bone.
             *
             * @param[in,out] mesh The mesh to add the bone to.
             * @param[in] bone The bone to add.
             * @param[in] path Path to the file with the bone info.
             */
            virtual void LoadBone(
              Ogre::Mesh *mesh, const HRCFile::Bone &bone, const String &path
            );

        private:

            /**
             * The HRC file.
             */
            HRCFile  &hrc_file_;
    };
}
