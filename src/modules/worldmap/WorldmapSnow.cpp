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

#include "modules/worldmap/WorldmapSnow.h"
#include <algorithm>
#include <cmath>
#include <OgreException.h>
#include <OgreLogManager.h>
#include <OgreMaterialManager.h>
#include <OgreMath.h>
#include <OgreOverlaySystem.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include <Overlay/OgreOverlay.h>
#include <Overlay/OgreOverlayContainer.h>
#include <Overlay/OgreOverlayManager.h>
#include <Overlay/OgrePanelOverlayElement.h>
#include "engine/ApplicationInterface.h"
#include "engine/entity/EntityManager.h"
#include "modules/worldmap/WorldmapConstants.h"
#include "modules/worldmap/WorldmapTextures.h"

namespace VGears {

    using namespace Ogre;

    /// Snow map (WM3): the camera turns by itself after this many seconds (random in the range) ...
    static const float SNOW_CAMERA_MIN_WAIT = 6.0f;
    static const float SNOW_CAMERA_MAX_WAIT = 12.0f;
    /// ... by this many degrees (random in the range, to any side), at the speed of the manual rotation.
    static const float SNOW_CAMERA_MIN_DEGREES = 30.0f;
    static const float SNOW_CAMERA_MAX_DEGREES = 160.0f;

    /// Snow map (WM3): how many poles can exist at the same time.
    static const size_t SNOW_MAX_POLES = 4;

    /**
     * The blizzard of WM3: two layers of the unused textures snow4 (cloud mist) and snow5 (dark blotches)
     * drift over the screen at different speeds. How the original draws it is not known.
     */
    struct SnowLayer{
        const char* texture;
        float alpha;
        float scroll_u;
        float scroll_v;
        float tiles_x;
        float tiles_y;
    };

    static const SnowLayer SNOW_LAYERS[] = {
        // The same texture three times, with other size, speed and direction (the opacities add up).
        {"snow4", 0.28f, 0.030f, 0.150f, 0.6f, 0.45f},
        {"snow4", 0.22f, -0.070f, 0.240f, 0.5f, 0.4f},
        {"snow4", 0.18f, 0.105f, 0.345f, 0.8f, 0.6f}
        // HIDDEN: {"snow5", 0.30f, -0.015f, 0.090f, 1.0f, 0.75f}
    };

    void WorldmapSnow::SetVisible(const bool visible){
        Ogre::OverlayManager& manager = Ogre::OverlayManager::getSingleton();
        Ogre::Overlay* overlay = manager.getByName("WorldMapSnow");
        if (overlay == nullptr){
            if (!visible) return;
            // The engine doesn't draw the overlays of Ogre unless its system listens to the scene.
            Ogre::OverlaySystem* overlay_system = VGears::ApplicationInterface::Get()->GetOverlaySystem();
            if (overlay_system != nullptr){
                Root::getSingleton().getSceneManager("Scene")->addRenderQueueListener(overlay_system);
            }
            overlay = manager.create("WorldMapSnow");
            overlay->setZOrder(100);
            int index = 0;
            for (const SnowLayer& layer : SNOW_LAYERS){
                // The same texture can be in several layers: the materials and panels are by index.
                const std::string name = std::string("WorldMapSnow_") + std::to_string(index) + "_" + layer.texture;
                try{
                    if (!Ogre::TextureManager::getSingleton().resourceExists(std::string("WorldMapTex_") + layer.texture)){
                        LoadWorldMapTexture(layer.texture);
                    }
                }
                catch (const Ogre::Exception& exception){
                    LogManager::getSingleton().logMessage(
                      "Failed to load the snow texture " + std::string(layer.texture) + ": " + exception.getDescription()
                    );
                    continue;
                }
                MaterialPtr material = Ogre::MaterialManager::getSingleton().create(
                  name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
                );
                Pass* pass = material->getTechnique(0)->getPass(0);
                pass->setLightingEnabled(false);
                pass->setDepthCheckEnabled(false);
                pass->setDepthWriteEnabled(false);
                pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
                Ogre::TextureUnitState* unit = pass->createTextureUnitState(std::string("WorldMapTex_") + layer.texture);
                unit->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
                unit->setAlphaOperation(Ogre::LBX_MODULATE, Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL, 1.0f, layer.alpha);
                unit->setScrollAnimation(layer.scroll_u, layer.scroll_v);
                Ogre::PanelOverlayElement* panel = static_cast<Ogre::PanelOverlayElement*>(
                  manager.createOverlayElement("Panel", name + "_panel")
                );
                panel->setMetricsMode(Ogre::GMM_RELATIVE);
                panel->setPosition(0.0f, 0.0f);
                panel->setDimensions(1.0f, 1.0f);
                panel->setMaterialName(name);
                panel->setTiling(layer.tiles_x, layer.tiles_y);
                overlay->add2D(panel);
                ++index;
            }
        }
        if (visible) overlay->show();
        else overlay->hide();
        LogManager::getSingleton().logMessage(
          std::string("World map snow overlay ") + (visible ? "shown" : "hidden") + ", layers: "
          + std::to_string(overlay->get2DElements().size())
        );
    }

    void WorldmapSnow::Reset(){
        snow_poles_.clear();
        snow_pole_counter_ = 0;
        snow_camera_remaining_ = 0.0f;
        snow_camera_timer_ = Ogre::Math::RangeRandom(SNOW_CAMERA_MIN_WAIT, SNOW_CAMERA_MAX_WAIT);
    }

    float WorldmapSnow::UpdateCamera(const float delta_time){
        if (snow_camera_remaining_ == 0.0f){
            snow_camera_timer_ -= delta_time;
            if (snow_camera_timer_ > 0.0f) return 0.0f;
            const float degrees = Ogre::Math::RangeRandom(SNOW_CAMERA_MIN_DEGREES, SNOW_CAMERA_MAX_DEGREES);
            const float direction = Ogre::Math::UnitRandom() < 0.5f ? -1.0f : 1.0f;
            snow_camera_remaining_ = direction * Ogre::Math::DegreesToRadians(degrees);
            snow_camera_timer_ = Ogre::Math::RangeRandom(SNOW_CAMERA_MIN_WAIT, SNOW_CAMERA_MAX_WAIT);
        }
        const float step = std::min(std::abs(snow_camera_remaining_), CAMERA_ROTATION_SPEED * delta_time);
        const float signed_step = snow_camera_remaining_ < 0.0f ? -step : step;
        snow_camera_remaining_ -= signed_step;
        if (std::abs(snow_camera_remaining_) < 0.0001f) snow_camera_remaining_ = 0.0f;
        return signed_step;
    }

    void WorldmapSnow::PlacePole(const PoleCreator& create){
        // The oldest pole goes away when there is no room for another one.
        while (snow_poles_.size() >= SNOW_MAX_POLES){
            EntityManager::getSingleton().RemoveWorldMapEntity(snow_poles_.front());
            snow_poles_.pop_front();
        }
        const std::string name = "snowpole_" + std::to_string(snow_pole_counter_++);
        create(name);
        if (EntityManager::getSingleton().GetEntity("WorldMap" + name) != nullptr){
            snow_poles_.push_back("WorldMap" + name);
        }
    }

} // namespace VGears
