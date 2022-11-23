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
#include <OgreMesh.h>
#include <OgreSkeleton.h>
#include "common/TypeDefine.h"

namespace VGears{

    class HRCMeshLoader;

    class HRCSkeletonLoader;

    /**
     * Handles HRC files.
     *
     * HRC files describe bone hierarchy in an skeleton.
     */
    class HRCFile : public Ogre::Resource{

        public:

            /**
             * Scale down constant.
             */
            static const float DOWN_SCALER;

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
            HRCFile(
              Ogre::ResourceManager *creator, const String &name,
              Ogre::ResourceHandle handle, const String &group,
              bool is_manual = false, Ogre::ManualResourceLoader *loader = NULL
            );

            /**
             * Destructor.
             */
            virtual ~HRCFile();

            typedef std::vector<String> RSDNameList;

            /**
             * A bone in a skeleton.
             */
            struct Bone{

                /**
                 * Bone name.
                 */
                String name;

                /**
                 * Parent bone name.
                 */
                String parent;

                /**
                 * Bone length
                 *
                 * @todo In pixels?
                 */
                Ogre::Real length;

                /**
                 * List of RDS file assigned to the bone.
                 *
                 * RSD are Resource Data Files.
                 */
                RSDNameList rsd_names;
            };

            typedef std::vector<Bone> BoneList;

            /**
             * Sets a name for the skelenton.
             *
             * @param[in] name The name for the skeleton.
             */
            virtual void SetSkeletonName(const String& name);

            /**
             * Retrieves the skeleton name.
             *
             * @return The skeleton name.
             */
            virtual const String& GetSkeletonName() const{
                return skeleton_name_;
            }

            /**
             * Retrieves the skeleton file name.
             *
             * @return The skeleton file name.
             */
            virtual String GetSkeletonFileName() const;

            /**
             * Retrieves the file name of the mesh assigned to the skeleton.
             *
             * @return The mesh file name.
             */
            virtual String GetMeshFileName() const;

            /**
             * Retrieves the list of bones in the skeleton.
             *
             * @return The list of bones.
             */
            virtual BoneList& GetBones(){return bones_;}

            /**
             * Retrieves the list of bones in the skeleton.
             *
             * @return The list of bones.
             */
            virtual const BoneList& GetBones() const{return bones_;}

            /**
             * Retrieves the skeleton.
             *
             * @return The skeleton.
             */
            virtual Ogre::SkeletonPtr GetSkeleton() const;

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

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
             * Calculates the size of the skeleton.
             *
             * @return The size of the skeleton.
             * @todo Units?
             */
            virtual size_t CalculateSize() const;

            /**
             * Calculates the size of a bone.
             *
             * @param[in] bone The bone whose size to calculate.
             * @return The size of the bone.
             * @todo Units?
             */
            virtual size_t CalculateSize(const Bone &bone) const;

        private:

            /**
             * The skeleton name.
             */
            String skeleton_name_;

            /**
             * The bones in the skeleton.
             */
            BoneList bones_;

            /**
             * The mesh loader.
             */
            HRCMeshLoader *mesh_loader_;

            /**
             * The skeleton loader.
             */
            HRCSkeletonLoader *skeleton_loader_;

            /**
             * The skeleton.
             */
            Ogre::SkeletonPtr skeleton_;

            /**
             * The mesh assigned to the skeleton.
             */
            Ogre::MeshPtr mesh_;
    };

    typedef Ogre::SharedPtr<HRCFile> HRCFilePtr;
}
