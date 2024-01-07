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

#include <OgreSingleton.h>
#include "Event.h"
#include "Manager.h"

/**
 * The world map manager.
 */
class WorldMapManager : public Manager, public Ogre::Singleton<WorldMapManager>{

    public:

        /**
         * Constructor.
         */
        WorldMapManager();

        /**
         * Destructor.
         */
        virtual ~WorldMapManager();

        /**
         * Handles an input event.
         *
         * @param[in] event Event to handle.
         */
        void Input(const VGears::Event& event) override;

        /**
         * Updates the world map manager  with debug information.
         *
         * It's automatically called from {@see Update}.
         */
        void UpdateDebug() override;

        /**
         * Handles resizing events
         */
        void OnResize() override;

        /**
         * Clears all field information in the field manager.
         *
         * Does nothing.
         */
        void ClearField() override;

        /**
         * Clears all battle information in the battle manager.
         *
         * Does nothing
         */
        void ClearBattle() override;

        /**
         * Clears all world map information in the world map manager.
         */
        void ClearWorld() override;

        /**
         * Adds a terrain block.
         *
         * @param[in] index Block index.
         * @param[in] mesh Path to the mesh file.
         * @param[in] pos Terrain position (x, y, z).
         */
        void AddTerrain(
          const unsigned int id, const Ogre::String mesh, const Ogre::Vector3 pos
        );

    private:

        /**
         * Updates the manager while in the field.
         *
         * It does nothing
         */
        void UpdateField() override;

        /**
         * Updates the manager during a battle.
         */
        void UpdateBattle() override;

        /**
         * Updates manager while in the world map.
         *
         * It does nothing
         */
        void UpdateWorld() override;

        /**
         * The scene node.
         */
        Ogre::SceneNode* scene_node_;

};
