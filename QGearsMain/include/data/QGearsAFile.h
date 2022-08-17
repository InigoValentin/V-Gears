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

#include <OgreSkeleton.h>
#include <Ogre.h>
#include "common/TypeDefine.h"
#include "common/QGearsResource.h"

namespace QGears{

    /**
     * Handles A files.
     *
     * A files are files with skeleton animation data.
     */
    class AFile : public Resource{

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
            AFile(
              Ogre::ResourceManager *creator, const String &name,
              Ogre::ResourceHandle handle, const String &group,
              bool is_manual = false, Ogre::ManualResourceLoader *loader = NULL
            );

            /**
             * Destructor.
             */
            virtual ~AFile();

            /**
             * Duration of a frame.
             */
            static const Ogre::Real FRAME_DURATION;

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

            /**
             * Adds an animation to an skeleton.
             *
             * @parma skeleton[in|out] Skeleton to add the animation to.
             * @param name[in] Animation name.
             */
            void AddTo(Ogre::SkeletonPtr skeleton, const String &name) const;

            typedef std::vector<Ogre::Vector3> BoneRotationList;

            /**
             * A frame in an animation.
             */
            struct Frame{

                /**
                 * The rotation of the whole skeleton in the frame.
                 */
                Ogre::Vector3 root_rotation;

                /**
                 * The translation of the whole skeleton in the frame.
                 */
                Ogre::Vector3 root_translation;

                /**
                 * The list of individual bone rotations in the frame.
                 */
                BoneRotationList bone_rotations;
            };

            typedef std::vector<Frame> FrameList;

            /**
             * Retrieves the list of frames in the file.
             *
             * @return The list of frames.
             */
            FrameList& GetFrames(){return frames_;}

            /**
             * Sets the number of bones.
             *
             * @param bone_count[in] The number of bones.
             */
            void SetBoneCount(const uint32 bone_count);

        protected:

            /**
             * Loads the file.
             */
            virtual void loadImpl() override final;

            /**
             * Unloads the file.
             */
            virtual void unloadImpl() override final;

            /**
             * Calculates the size of the file.
             *
             * @return The size of the file.
             * @todo Units?
             */
            size_t CalculateSize() const;

            /**
             * Sets the rotation for a frame.
             *
             * @param key_frame[in] The frame.
             * @param rotation[in] The rotation for the frame
             */
            void SetFrameRotation(
              Ogre::TransformKeyFrame *key_frame,
              const Ogre::Vector3 &rotation
            ) const;

        private:

            /**
             * The number of bones.
             */
            uint32 bone_count_;

            /**
             * The list of frames.
             */
            FrameList frames_;
    };

    typedef Ogre::SharedPtr<AFile> AFilePtr;
}

