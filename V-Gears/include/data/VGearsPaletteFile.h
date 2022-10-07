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

#include <OgreColourValue.h>
#include <OgreResource.h>
#include "common/TypeDefine.h"

namespace VGears{

    /**
     * Handles colour palette files.
     */
    class PaletteFile : public Ogre::Resource{

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
            PaletteFile(
              Ogre::ResourceManager* creator, const String &name,
              Ogre::ResourceHandle handle, const String& group,
              bool is_manual = false,
              Ogre::ManualResourceLoader* loader = nullptr
           );

            /**
            * Destructor.
            */
            virtual ~PaletteFile();

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

            typedef Ogre::ColourValue Color;

            typedef std::vector<Color> Page;

            typedef std::vector<Page> PageList;

            /**
             * Retrieevs the list of pages.
             *
             * @return The list of pages.
             */
            virtual PageList& GetPages(){return pages_;}

            /**
             * Retrieves a page.
             *
             * @param[in] index Page index.
             * @return The page at the specified index.
             */
            virtual const Page& GetPage(size_t index) const{
                return pages_.at(index);
            }

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
            virtual size_t calculateSize() const override;

        private:

            /**
             * The list of palette pages.
             */
            PageList pages_;
    };

    typedef Ogre::SharedPtr<PaletteFile> PaletteFilePtr;
}
