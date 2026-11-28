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

namespace Ogre{
    class OverlaySystem;
    class RenderWindow;
}

namespace VGears{

    /**
     * What the engine and the modules need from the application.
     *
     * The application implements it, so the rest of the code doesn't depend on it. There is at
     * most one instance, which registers itself when it is created.
     */
    class ApplicationInterface{

        public:

            /**
             * Retrieves the application.
             *
             * @return The application, or nullptr if there isn't any.
             */
            static ApplicationInterface* Get(){return Instance();}

            /**
             * Retrieves the window of the game.
             *
             * @return The render window.
             */
            virtual Ogre::RenderWindow* getRenderWindow() = 0;

            /**
             * Retrieves the system that draws the Ogre overlays.
             *
             * @return The overlay system, or nullptr if there isn't any.
             */
            virtual Ogre::OverlaySystem* GetOverlaySystem() = 0;

        protected:

            /**
             * Constructor. Registers the instance.
             */
            ApplicationInterface(){Instance() = this;}

            /**
             * Destructor. Unregisters the instance.
             */
            virtual ~ApplicationInterface(){if (Instance() == this) Instance() = nullptr;}

        private:

            /**
             * Holds the registered instance.
             *
             * @return Reference to the instance pointer.
             */
            static ApplicationInterface*& Instance(){
                static ApplicationInterface* instance = nullptr;
                return instance;
            }
    };
}
