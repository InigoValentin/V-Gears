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

#include <OgreResourceGroupManager.h>
#include <OgreLogManager.h>
#include "data/VGearsBackgroundFile.h"
#include "data/VGearsBackgroundFileSerializer.h"

namespace VGears{

    const String BackgroundFile::RESOURCE_TYPE("VGearsBackgroundFile");

    BackgroundFile::BackgroundFile(
      Ogre::ResourceManager *creator, const String &name,
      Ogre::ResourceHandle handle, const String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader
   ) : Ogre::Resource(creator, name, handle, group, is_manual, loader)
    {createParamDictionary(RESOURCE_TYPE);}

    BackgroundFile::~BackgroundFile(){unload();}

    void BackgroundFile::loadImpl(){
        BackgroundFileSerializer serializer;
        Ogre::DataStreamPtr stream(
          Ogre::ResourceGroupManager::getSingleton().openResource(
            mName, mGroup, true, this
         )
       );
        serializer.ImportBackgroundFile(stream, this);
    }

    void BackgroundFile::unloadImpl(){
        for (auto& layer : layers_) memset(&layer, 0, sizeof(layer));
        for (auto& pal : palette_) memset(&pal, 0, sizeof(pal));
        for (size_t i(PAGE_COUNT); i --;){
            pages_[i].enabled = false;
            pages_[i].unknown_02 = 0;
            pages_[i].value_size = 0;
            pages_[i].data.clear();
        }
    }

    size_t BackgroundFile::CalculateSize() const{
        size_t data_size(0);
        for (size_t i(LAYER_COUNT); i-- ;)
            data_size += CalculateSize(layers_[i]);
        for (size_t i(PAGE_COUNT); i-- ;) data_size += CalculateSize(pages_[i]);
        return data_size;
    }

    size_t BackgroundFile::CalculateSize(const Layer &layer) const{
        return
          sizeof(layer.enabled)
          + sizeof(layer.width)
          + sizeof(layer.height)
          + sizeof(layer.unknown_06)
          + sizeof(layer.unknown_08)
          + sizeof(layer.unknown_0E)
          + (layer.sprites.size() * sizeof(layer.sprites[0]));
    }

    size_t BackgroundFile::CalculateSize(const Page &page) const{
        return
          sizeof(page.enabled)
          + sizeof(page.unknown_02)
          + sizeof(page.value_size)
          + page.data.size();
    }

    void BackgroundFile::AddAllSprites(SpritePtrList& sprites){
        for (size_t i(0); i < LAYER_COUNT; ++ i){
            if (layers_[i].enabled){
                for (auto& it : layers_[i].sprites){
                    SpriteData* ptr = &it;
                    sprites.push_back(ptr);
                }
            }
        }
    }

    Ogre::Image* BackgroundFile::CreateImage(const PaletteFilePtr &palette){
        assert(palette != nullptr);
        SpritePtrList sprites;
        AddAllSprites(sprites);
        size_t sprite_count(sprites.size());
        size_t width(1024);
        size_t row_pitch(width);
        size_t height(width);
        size_t pixel_count(width * height);
        row_pitch = width;
        Ogre::PixelFormat format(Ogre::PF_A8R8G8B8);
        size_t memory_size(
          Ogre::PixelUtil::getMemorySize(width, height, 1, format)
        );
        Ogre::MemoryDataStream *buffer(new Ogre::MemoryDataStream(memory_size));
        uint32 *color(reinterpret_cast<uint32*>(buffer->getPtr()));

        /*
        Ogre::LogManager::getSingleton().stream() << "Image Size: " << width
          << " x " << height << " sprite_count " << sprite_count;
        */
        int dst_x(0), dst_y(0);
        int dst_x_16(0), dst_y_16(0);
        int dst_x_32(0), dst_y_32(0);
        int dst_n_16(0);

        for (auto it(sprites.begin()); it != sprites.end(); ++ it){
            SpriteData& sprite = **it;
            const Page& data_page(pages_[sprite.data_page]);
            const Pixel& src(sprite.src);
            if (!data_page.enabled){
                Ogre::LogManager::getSingleton().stream()
                  << "Error: referencing an disabled data page";
            }
            // Position sprites in 32x32 blocks.
            if (sprite.width == 32){
                dst_x = dst_x_32;
                dst_y = dst_y_32;
                dst_x_32 += 32;
                if (dst_x_32 == width){
                    dst_y_32 += 32;
                    dst_x_32 = 0;
                }
            }
            else if (sprite.width == 16){
                // Start new 16x16*4 block
                if (dst_n_16 == 0){
                    dst_x_16 = dst_x_32;
                    dst_y_16 = dst_y_32;
                    dst_x_32 += 32;
                    if (dst_x_32 == width){
                        dst_y_32 += 32;
                        dst_x_32 = 0;
                    }
                }
                dst_x = dst_x_16;
                dst_y = dst_y_16;
                ++ dst_n_16;
                if (dst_n_16 == 1 || dst_n_16 == 3) dst_x_16 += 16;
                else if (dst_n_16 == 2){
                    dst_x_16 -= 16;
                    dst_y_16 += 16;
                }
                else if (dst_n_16 == 4) dst_n_16 = 0;
            }
            else{
                Ogre::LogManager::getSingleton().stream()
                  << "Error: sprite data with invalid size";
            }

            if (data_page.value_size == 2){
                for (uint16 y((*it)->height); y --;){
                    for (uint16 x((*it)->width); x--;){
                        size_t data_index(
                          (src.y + y) * PAGE_DATA_WIDTH + src.x + x
                        );
                        if (data_index >= data_page.colors.size()){
                            Ogre::LogManager::getSingleton().stream()
                              << "Error: data page Index out of Bounds "
                              << data_index;
                        }
                        Color colour(data_page.colors.at(data_index));
                        data_index = (dst_y + y) * row_pitch + dst_x + x;
                        if (data_index >= pixel_count){
                            Ogre::LogManager::getSingleton().stream()
                              << "Error: writing Pixel out of Bounds "
                              << data_index << " " << (dst_x + x) << " x "
                              << dst_y + y;
                        }
                        color[data_index] = colour.getAsARGB();
                    }
                }
            }
            else if (data_page.value_size == 1){
                if (sprite.palette_page >= palette->GetPages().size()){
                    // fr_e substrative blending hack.
                    if (
                      mName.compare("fr_e.background") == 0
                      && sprite.has_blending
                      && palette->GetPages().size() == 2
                    ){
                        // Repair palette id.
                        sprite.palette_page = sprite.data_page > 16;
                        sprite.blending = 2; // Substrative blending.
                    }
                    // lastmap hack
                    else if (
                      mName.compare("lastmap.background") == 0
                      && sprite.has_blending
                      && palette->GetPages().size() >= 7
                    ){
                        sprite.palette_page = 6; // Repair palette id
                        sprite.blending = 2; // Substrative blending
                    }
                    else{
                        Ogre::LogManager::getSingleton().stream()
                          << "Error: palette page Index out of Bounds "
                          << sprite.palette_page;
                    }
                }
                const PaletteFile::Page& palette_page(
                  palette->GetPage(sprite.palette_page)
                );
                bool first_color_hidden(false);
                if (sprite.palette_page < PALETTE_ENTRY_COUNT){
                    first_color_hidden = palette_[sprite.palette_page] > 0;
                }
                for (uint16 y(sprite.height); y --;){
                    for (uint16 x(sprite.width); x --;){
                        size_t data_index(
                          (src.y + y) * PAGE_DATA_WIDTH + src.x + x
                        );
                        if (data_index >= data_page.data.size()){
                            Ogre::LogManager::getSingleton().stream()
                              << "Error: data page Index out of Bounds "
                              << data_index;
                        }
                        uint8 index(data_page.data.at(data_index));
                        if (index >= palette_page.size()){
                            Ogre::LogManager::getSingleton().stream()
                              << "Error: palette page Index out of Bounds "
                              << index;
                        }
                        data_index = (dst_y + y) * row_pitch + dst_x + x;
                        if (data_index >= pixel_count){
                            Ogre::LogManager::getSingleton().stream()
                              << "Error: writing Pixel out of Bounds "
                              << data_index << " " << (dst_x + x) << " x "
                              << dst_y + y;
                        }
                        Color colour(palette_page[index]);
                        // Strange PC version behavior: empty colors are
                        // replaced by the first color and the first color is
                        // hidden by a flag.
                        if (colour == Color::ZERO) colour = palette_page[0];
                        if (index == 0 && first_color_hidden) colour.a = 0;
                        else if (
                          colour.a
                          && (colour.r != 0 || colour.g != 0 || colour.b != 0)
                        ){
                            colour.a = 0.5;
                        }
                        else colour.a = 1;
                        color[data_index] = colour.getAsARGB();
                    }
                }
                
            }
            // Source in the texture atlas is where it's just copied to.
            sprite.src.x = dst_x;
            sprite.src.y = dst_y;
        }
        Ogre::DataStreamPtr stream(buffer);
        Ogre::Image *image(new Ogre::Image());
        image->loadRawData(stream, width, height, format);
        return image;
    }

}
