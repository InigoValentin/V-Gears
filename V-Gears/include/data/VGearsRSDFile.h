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
#include <OgreMaterial.h>
#include "common/TypeDefine.h"

namespace VGears{

    /**
     * Handles RDS files.
     *
     * RSD files are product of the original PlayStation Psy-Q 3D development
     * libraries. They are often exported by 3D modelers when converting from
     * an LWO or DXF file to something more understandable by the PSX. They are
     * in ascii, making it easy to edit by hand. When a 3D editor does an
     * export, four text files are actually created, .rsd, .ply, .mat, and
     * .grp. These can then be "compiled" into a binary .rsd file for the PSX.
     * In the PC version of Final Fantasy 7, the text .rsd files are used while
     * the other files were "compiled" into Polygon(.p) format.
     */
    class RSDFile : public Ogre::Resource{

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
            RSDFile(
              Ogre::ResourceManager* creator, const String &name,
              Ogre::ResourceHandle handle, const String& group,
              bool is_manual = false,
              Ogre::ManualResourceLoader* loader = nullptr
            );

            /**
             * Destructor.
             */
            virtual ~RSDFile();

            /**
             * The resource type.
             */
            static const String RESOURCE_TYPE;

            typedef std::vector<String> TextureNameList;

            typedef std::vector<Ogre::MaterialPtr> MaterialList;

            /**
             * Sets the name of the polygon.
             *
             * @param polygon_name[in] The name for the polygon.
             */
            virtual void SetPolygonName(const String &polygon_name);

            /**
             * Sets the material type.
             *
             * @param material_name[in] The material name.
             */
            virtual void SetMaterialName(const String &material_name);

            /**
             * Sets the group name.
             *
             * @param group_name[in] The name for the group.
             */
            virtual void SetGroupName(const String &group_name);

            /**
             * Retrieves the polygon name.
             *
             * @return The polygon name.
             */
            virtual const String& GetPolygonName() const;

            /**
             * Retrieves the material name.
             *
             * @return The matyerial name.
             */
            virtual const String& GetMaterialName() const;

            /**
             * Retrieves the group name.
             *
             * @return The group name.
             */
            virtual const String& GetGroupName() const;

            /**
             * Counts the texture names.
             *
             * @return The number of texture names.
             */
            virtual size_t GetTextureNameCount() const;

            /**
             * Retrieves the list of texture names.
             *
             * @return The list of texture names.
             */
            virtual TextureNameList& GetTextureNames();

            /**
             * Retrieves the material base.
             *
             * @return The material base name.
             */
            virtual String GetMaterialBaseName() const;

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
            virtual size_t  calculateSize() const;

            /**
             * Adds a texture.
             *
             * @param pass[in|out] Render pas options.
             * @param index[in] Index of the texture to add
             */
            virtual void AddTexture(Ogre::Pass *pass, const size_t index) const;

        private:

            /**
             * The polygon names.
             */
            String polygon_name_;

            /**
             * The material type.
             */
            String material_name_;

            /**
             * The group name.
             */
            String group_name_;

            /**
             * The texture name.
             */
            TextureNameList texture_names_;

            /**
             * The lsit of materials
             */
            MaterialList materials_;
    };

    typedef Ogre::SharedPtr<RSDFile> RSDFilePtr;
}
