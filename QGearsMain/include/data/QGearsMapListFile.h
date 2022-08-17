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

#include <OgreResourceManager.h>
#include "QGearsPrerequisites.h"
#include "common/QGearsResource.h"
#include "QGearsSerializer.h"

namespace QGears{

    /**
     * A manager for map list files.
     */
    class MapListFileManager :
      public Ogre::ResourceManager, public Ogre::Singleton<MapListFileManager>
    {

        // TODO: Move to it's own file?

        public:

            /**
             * Constructor.
             */
            MapListFileManager();

            /**
             * Destructor.
             */
            virtual ~MapListFileManager();

            /**
             * Retrieves a singleton to the manager.
             */
            static MapListFileManager& GetSingleton();

            /**
             * Retrieves a pointer to the manager singleton.
             */
            static MapListFileManager* GetSingletonPtr();

        protected:

            /**
             * Loads the manager.
             *
             * @param name[in] The unique name of the manager.
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
             * @param create_params[in] Unused.
             */
            virtual Ogre::Resource *createImpl(
              const Ogre::String &name, Ogre::ResourceHandle handle,
              const Ogre::String &group, bool is_manual,
              Ogre::ManualResourceLoader *loader,
              const Ogre::NameValuePairList *create_params
            ) override;
        };

    /**
     * Handles map list files.
     */
    class MapListFile : public Resource{

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
            MapListFile(
              Ogre::ResourceManager* creator, const String &name,
              Ogre::ResourceHandle handle, const String& group,
              bool is_manual = false,
              Ogre::ManualResourceLoader* loader = nullptr
            );

            /**
             * Destructor.
             */
            virtual ~MapListFile();

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

            /**
             * Retrieves the list of maps.
             *
             * @return The list of maps.
             */
            const std::vector<std::string> GetMapList() const{return map_list_;}

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
             * Calculates the size of the map list.
             *
             * @return The size of the map list.
             * @todo Units?
             */
            virtual size_t calculateSize(void) const override;

        private:

            friend class MapListFileSerializer;

            /**
             * The list of maps.
             */
            std::vector<std::string> map_list_;
    };

    /**
     * Handles the serialization of map list files.
     */
    class MapListFileSerializer : public Serializer{

        // TODO: Move to it's own file?

        public:

            /**
             * Constructor.
             */
            MapListFileSerializer() = default;

            /**
             * Imports a map list file.
             *
             * @param stream[in] The contents of the map list file.
             * @param dest[out] The formed map list file.
             */
            void ImportMapListFile(
              Ogre::DataStreamPtr &stream, MapListFile& dest
            );
    };

    typedef Ogre::SharedPtr<MapListFile> MapListFilePtr;
}
