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

#include <array>
#include <OgreResource.h>
#include <OgreImage.h>
#include <Ogre.h>
#include "common/TypeDefine.h"
#include "data/QGearsPaletteFile.h"

namespace QGears{

    // TODO inherit from QGears Resource.

    /**
     * A background file representation.
     */
    class BackgroundFile : public Ogre::Resource{

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
            BackgroundFile(
              Ogre::ResourceManager *creator, const String &name,
              Ogre::ResourceHandle handle, const String &group,
              bool is_manual = false, Ogre::ManualResourceLoader *loader = NULL
            );

            /**
             * Destructor.
             */
            virtual ~BackgroundFile();

            /**
             * The type of resource.
             */
            static const String RESOURCE_TYPE;

            enum {

                /**
                 * Width of the background page, in pixels.
                 *
                 * Same as {@see PAGE_DATAH_EIGHT}.
                 */
                PAGE_DATA_WIDTH = 256,

                /**
                 * Height of the background page, in pixels.
                 *
                 * Same as {@see PAGE_DATA_WIDTH}.
                 */
                PAGE_DATA_HEIGHT = PAGE_DATA_WIDTH,

                /**
                 * Size of the background page, in pixels.
                 *
                 * Same as {@see PAGE_DATA_WIDTH} * {@see PAGE_DATA_HEIGHT}.
                 */
                PAGE_DATA_SIZE = PAGE_DATA_WIDTH * PAGE_DATA_HEIGHT,

                /**
                 * Width of sprites, in pixels.
                 *
                 * Same as {@see SPRITE_HEIGHT}.
                 */
                SPRITE_WIDTH = 16,

                /**
                 * Height of sprites, in pixels.
                 *
                 * Same as {@see SPRITE_WIDTH}.
                 */
                SPRITE_HEIGHT = SPRITE_WIDTH,

                /**
                 * Size of sprites, in pixels.
                 *
                 * Same as {@see SPRITE_WIDTH} * {@see SPRITE_HEIGHT}.
                 */
                SPRITE_PIXEL_COUNT = SPRITE_WIDTH * SPRITE_HEIGHT,

                /**
                 * Number of layers in a background.
                 */
                LAYER_COUNT = 4,

                /**
                 * Number of palettes in a background.
                 */
                PALETTE_ENTRY_COUNT = 20,

                /**
                 * Number of pages in a background.
                 *
                 * @todo What are pages?
                 */
                PAGE_COUNT = 42
            };

            /**
             * Data for sprites.
             */
            struct SpriteData{

                /**
                 * Pixel destination.
                 */
                Pixel dst;

                /**
                 * Unused data.
                 */
                uint16 unknown_04[2];

                /**
                 * Pixel source.
                 */
                Pixel src;

                /**
                 * Pixel alternate source.
                 *
                 * Used for special effects pages, when data_page2 != 0, it
                 * must be used instead of {@see src}.
                 */
                Pixel src2;

                /**
                 * Sprite width.
                 */
                uint16 width;

                /**
                 * Sprite height.
                 */
                uint16 height;

                /**
                 * Palete page for the sprite
                 */
                uint16 palette_page;

                /**
                 * Sprite depth (Z-index).
                 */
                uint16 depth;

                /**
                 * Animation ID of the sprite.
                 */
                uint8 animation_id;

                /**
                 * Animation frame of the sprite.
                 */
                uint8 animation_frame;

                /**
                 * Indicates if the sprite uses color blending.
                 */
                bool has_blending[2];

                /**
                 * The type of blending used.
                 */
                uint16 blending;

                /**
                 * The page with the sprite data.
                 */
                uint16 data_page;

                /**
                 * Page with alternate sprite data.
                 *
                 * Used for special effects pages. When data_page2 != 0, it
                 * must be used instead of {@see data_page}.
                 */
                uint16 data_page2;

                /**
                 * Color depth.
                 *
                 * Do not use. Use texture page depth instead.
                 */
                uint16 colour_depth;

                /**
                 * Sprite source.
                 *
                 * For PC use only.
                 * z = unknown, x = srcX / 16 * 625000, y = srcY / 16 * 625000
                 */
                Ogre::Vector3 src_big;
            };

            typedef std::vector<SpriteData> SpriteList;

            typedef std::vector<SpriteData*> SpritePtrList;

            /**
             * A layer of a background.
             */
            struct Layer{

                /**
                 * Indicates if the layer is used.
                 */
                bool enabled;

                /**
                 * Layer width.
                 */
                uint16 width;

                /**
                 * Layer height.
                 */
                uint16 height;
                /**
                 * Unknown.
                 *
                 * uint16 sprite_count only read in serializer. Unused in layer
                 * 0.
                 */
                uint16 unknown_06;

                /**
                 * Unknown.
                 *
                 * Used in layers 1, 2 and 3
                 */
                uint16 unknown_08[3];

                /**
                 * Unknown.
                 *
                 * Used in layers 2 and 3
                 */
                uint16 unknown_0E[4];

                /**
                 * List of sprites in the layer.
                 */
                SpriteList sprites;
            };

            typedef PaletteFile::Color Color;

            typedef std::vector<uint8> Buffer;

            typedef std::vector<Color> Colors;

            /**
             * A background page/
             */
            struct Page{

                /**
                 * Indicates if the page is used.
                 */
                bool enabled;

                /**
                 * Unknown data.
                 */
                uint16 unknown_02;

                /**
                 * @todo Understand and document.
                 */
                uint16 value_size;

                // //uint8 if value_size == 1, uint16 if value_size == 2
                // uint8 data[PAGE_DATA_WIDTH][PAGE_DATA_HEIGHT];

                /**
                 * Data buffer.
                 */
                Buffer data;

                /**
                 * Background color list.
                 */
                Colors colors;
            };

            /**
             * Retrieves the background layers.
             *
             * @return The list of layers.
             */
            std::array<Layer, LAYER_COUNT>& GetLayers(void) {return layers_;}

            /**
             * Retrieves the background palette.
             *
             * @return The palette.
             */
            std::array<uint8, PALETTE_ENTRY_COUNT>& GetPalette(void) {
                return palette_;
            }

            /**
             * Retrieves the background pages.
             *
             * @return The background pages.
             */
            std::array<Page, PAGE_COUNT>& GetPages(void){return pages_;}

            /**
             * Creates an image from a palette.
             *
             * @param palette[in] The color palette for the image.
             * @return The image.
             */
            Ogre::Image* CreateImage(const PaletteFilePtr &palette);

            /**
             * Adds all sprites to the background.
             *
             * @param sprites[in] The list of sprites to add.
             */
            void AddAllSprites(SpritePtrList& sprites) ;

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
             * Calculates the size of the background.
             *
             * It includes all layers and pages.
             *
             * @return The total size of the background, in bytes.
             */
            virtual size_t CalculateSize() const;

            /**
             * Calculates the size of a background layer.
             *
             * @param layer[in] The layer to calculate size from.
             * @return The total size of the background layer, in bytes.
             */
            virtual size_t CalculateSize(const Layer &layer) const;

            /**
             * Calculates the size of a background page.
             *
             * @param page[in] The layer to calculate size from.
             * @return The total size of the background page, in bytes.
             */
            virtual size_t CalculateSize(const Page &page) const;

        private:

            /**
             * The list of layers.
             */
            std::array<Layer, LAYER_COUNT> layers_;

            /**
             * The color palette.
             */
            std::array<uint8, PALETTE_ENTRY_COUNT> palette_;

            /**
             * The background pages.
             */
            std::array<Page, PAGE_COUNT> pages_;
    };

    typedef Ogre::SharedPtr<BackgroundFile> BackgroundFilePtr;
}
