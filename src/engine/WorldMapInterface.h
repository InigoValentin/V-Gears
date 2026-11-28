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

#include <OgreString.h>

namespace VGears{

    /**
     * What the engine and the other modules need from the world map.
     *
     * The world map module implements it, so the rest of the code doesn't depend on the module.
     * There is at most one instance, which registers itself when it is created.
     */
    class WorldMapInterface{

        public:

            /**
             * Retrieves the world map.
             *
             * @return The world map, or nullptr if there isn't any.
             */
            static WorldMapInterface* Get(){return Instance();}

            /**
             * Hides the world map and stops it.
             */
            virtual void Suspend() = 0;

            /**
             * Hides the world map and stops it while a battle is on.
             */
            virtual void PauseForBattle() = 0;

            /**
             * Shows the world map again after a battle.
             */
            virtual void ResumeAfterBattle() = 0;

            /**
             * Retrieves the type of the terrain under an entity.
             *
             * @param[in] entity_name Name of the entity.
             * @return Name of the terrain type, or an empty string if it is unknown.
             */
            virtual Ogre::String GetTerrainTypeUnderEntity(const char* entity_name) const = 0;

            /**
             * Retrieves the terrain triangle under an entity.
             *
             * @param[in] entity_name Name of the entity.
             * @return Index of the triangle, or -1 if it is unknown.
             */
            virtual int GetTerrainTriangleUnderEntity(const char* entity_name) const = 0;

        protected:

            /**
             * Constructor. Registers the instance.
             */
            WorldMapInterface(){Instance() = this;}

            /**
             * Destructor. Unregisters the instance.
             */
            virtual ~WorldMapInterface(){if (Instance() == this) Instance() = nullptr;}

        private:

            /**
             * Holds the registered instance.
             *
             * @return Reference to the instance pointer.
             */
            static WorldMapInterface*& Instance(){
                static WorldMapInterface* instance = nullptr;
                return instance;
            }
    };
}
