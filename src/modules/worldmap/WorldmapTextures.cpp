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

#include "modules/worldmap/WorldmapTextures.h"
#include <memory>
#include <OgreException.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreMaterialManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include "modules/worldmap/WorldmapData.h"
#include "formats/texture/TexFile.h"

namespace VGears {

    using namespace Ogre;

    float ToTextureCoordU(uint8 coord, uint16 id){
        return (float(coord) - float(WORLD_MAP_TEXTURES[id].u)) / float(WORLD_MAP_TEXTURES[id].width);
        //return (float(WORLD_MAP_TEXTURES[id].u) + float(coord)) / 256.0f;
    }

    float ToTextureCoordV(uint8 coord, uint16 id){
        return (float(coord) - float(WORLD_MAP_TEXTURES[id].v)) / float(WORLD_MAP_TEXTURES[id].height);
        //return (float(WORLD_MAP_TEXTURES[id].v) + float(coord)) / 640.0f;
    }

    struct ImageDataDeleter{

      void operator()(Ogre::ImageCodec::ImageData* image_data) const{
        OGRE_DELETE image_data;
      }
    };

    TexturePtr LoadWorldMapTexture(const std::string& name){
        const std::string texture_name = name + ".tex";
        Ogre::DataStreamPtr stream
          = ResourceGroupManager::getSingleton().openResource(
              texture_name, WORLD_MAP_RESOURCE_GROUP, nullptr, true
            );
        VGears::TexFile tex_file;
        tex_file.Read(stream);

        Ogre::MemoryDataStreamPtr pixel_data(tex_file.GetPixelData());
        std::unique_ptr<Ogre::ImageCodec::ImageData, ImageDataDeleter> image_data(
          tex_file.GetImageData()
        );
        TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(
          std::string("WorldMapTex_") + name,
          ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
          image_data->width, image_data->height, 0, image_data->format, TU_STATIC
        );
        texture->getBuffer()->blitFromMemory(Ogre::PixelBox(
          image_data->width, image_data->height, 1, image_data->format,
          pixel_data->getPtr()
        ));
        return texture;
    }


    /**
     * Animated textures (water, shores, rivers, waterfalls, swamps). The original game has sets of
     * frames named by the first one plus a number (ff7.exe 0x95b880: fall1-8, riv_l1-4, ...). The
     * triangles use the first frame, and the material of it shows the frames in turn.
     */
    struct AnimatedTextureSet{
        const char* prefix;
        int frames;
    };

    static const AnimatedTextureSet ANIMATED_TEXTURES[] = {
        {"we_s", 8}, {"fall", 8}, {"uf", 4}, {"lst", 4}, {"sst", 4}, {"rm", 4}, {"ds", 4}, {"sh", 4},
        {"ssee", 4}, {"riv_l", 4}, {"ds_s", 4}, {"sh_s", 4}, {"sse_s", 4}, {"swp", 4}, {"dsee", 4},
        {"shlm_", 4}, {"des0", 4}, {"edes0", 4}, {"rivs", 4}
    };

    /** Seconds that each frame of the animated textures lasts (the original timing is not known). */
    static const float ANIMATED_TEXTURE_FRAME_SECONDS = 0.125f;

    void AnimateWorldMapTextures(const float delta_time){
        static float elapsed = 0;
        static long last_step = -1;
        elapsed += delta_time;
        const long step = static_cast<long>(elapsed / ANIMATED_TEXTURE_FRAME_SECONDS);
        if (step == last_step) return;
        last_step = step;
        for (const AnimatedTextureSet& set : ANIMATED_TEXTURES){
            MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(
              std::string("WorldMap_") + set.prefix + "1"
            );
            if (material.isNull()) continue;
            const std::string frame_name = std::string(set.prefix) + std::to_string(step % set.frames + 1);
            const std::string texture_name = std::string("WorldMapTex_") + frame_name;
            try{
                if (!Ogre::TextureManager::getSingleton().resourceExists(texture_name)){
                    if (!ResourceGroupManager::getSingleton().resourceExists(
                      WORLD_MAP_RESOURCE_GROUP, frame_name + ".tex"
                    )) continue;
                    LoadWorldMapTexture(frame_name);
                }
                material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(texture_name);
            }
            catch (const Ogre::Exception&){}
        }
    }

    MaterialPtr LoadTextureMaterial(uint16 texture_id){
        const std::string material_name
          = std::string("WorldMap_") + WORLD_MAP_TEXTURES[texture_id].name;
        MaterialPtr material
          = Ogre::MaterialManager::getSingleton().getByName(material_name);
        // The textures and materials are kept when a map is replaced, and the maps share them.
        if (material.isNull()){
            TexturePtr texture = LoadWorldMapTexture(WORLD_MAP_TEXTURES[texture_id].name);
            material = Ogre::MaterialManager::getSingleton().create(
              material_name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
            );
            Pass* pass = material->getTechnique(0)->getPass(0);
            pass->createTextureUnitState(texture->getName());
            pass->setTextureFiltering(TFO_NONE);
            pass->setLightingEnabled(false);
        }
        return material;
    }

} // namespace VGears
