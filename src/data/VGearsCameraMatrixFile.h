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

#include <OgreMatrix3.h>
#include "common/VGearsResource.h"

namespace VGears{

    /**
     * Handles camera matrix files.
     */
    class CameraMatrixFile : public Resource{

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
            CameraMatrixFile(
              Ogre::ResourceManager *creator, const String &name,
              Ogre::ResourceHandle handle, const String &group,
              bool is_manual = false, Ogre::ManualResourceLoader *loader = NULL
           );

            /**
             * Destructor.
             */
            virtual ~CameraMatrixFile();

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

            /**
             * Retrieves the camera matrix in the file.
             *
             * @return The camera matrix.
             */
            const Ogre::Matrix3& GetMatrix() const;

            /**
             * Sets the camera matrix.
             *
             * @param[in] matrix The camera matrix.
             */
            void SetMatrix(const Ogre::Matrix3& matrix);

            /**
             * Retrieves the camera position.
             *
             * @return The camera position.
             */
            const Ogre::Vector3& GetPosition() const;

            /**
             * Sets the camera position.
             *
             * @param[in] position The camera position.
             */
            void SetPosition(const Ogre::Vector3& position);

            /**
             * Retrieves the camera offset.
             *
             * @return The camera offset, as a {@see Pixel}.
             */
            const Pixel& GetOffset() const;

            /**
             * Sets the camera offset.
             *
             * @param[in] offset The camera offset, as a {@see Pixel}.
             */
            void SetOffset(const Pixel& offset);

            /**
             * Counts the ???
             *
             * @return The number of ???
             * @todo What does this count?
             */
            const size_t& GetCount() const;

            /**
             * Sets the number of ???
             *
             * @param[in] count The number of ???
             * @todo What is this number?
             */
            void SetCount(const size_t count);

            /**
             * Retrieves the focal length.
             *
             * @return The focal length.
             */
            const Ogre::Real& GetFocalLength() const;

            /**
             * Sets the focal length.
             *
             * @param[in] focal_length The focal length.
             */
            void SetFocalLength(const Ogre::Real focal_length);

            /**
             * Retrieves the camera orientation.
             *
             * @return The camera orientation.
             */
            Ogre::Quaternion GetOrientation() const;

            /**
             * Retrieves the camera field of view.
             *
             * @return The field of view.
             */
            Ogre::Radian GetFov(Ogre::Real width) const;

        protected:

            /**
             * Loads the file.
             */
            virtual void loadImpl(void);

            /**
             * Unloads the file.
             */
            virtual void unloadImpl(void);

            /**
             * Calculates the file size
             *
             * @return Always 0.
             */
            virtual size_t CalculateSize(void) const;

        private:

            /**
             * The camera matrix.
             */
            Ogre::Matrix3 matrix_;

            /**
             * The position of the camera.
             */
            Ogre::Vector3 position_;

            /**
             * The camera offset.
             */
            Pixel offset_;

            /**
             * The focal length.
             */
            Ogre::Real focal_length_;

            /**
             * The number of @todo what?
             */
            size_t count_;
    };

    typedef Ogre::SharedPtr<CameraMatrixFile> CameraMatrixFilePtr;
}
