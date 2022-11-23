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

#include "common/VGearsApplication.h"
#include "common/OgreBase.h"

namespace VGears{

    /**
     * Provides utilities for the application.
     */
    class Utility : public Application{

        public:

            /**
             * Constructor.
             *
             * @param[in] argc Number of arguments passed to the application.
             * @param[in] argv List of arguments passed to the application.
             */
            Utility(int argc, char *argv[]);

            /**
             * Destructor.
             */
            virtual ~Utility();

            /**
             * Retrieves the camera.
             *
             * @return The camera.
             */
            virtual Ogre::Camera* GetCamera(void);

        protected:

            /**
             * Initializes components used by the utilities.
             */
            virtual void InitComponents();

            /**
             * Initializes components used by the utilities.
             */
            virtual void DestroyComponents();

        private:

            /**
             * Constructor.
             */
            Utility();

            /**
             * The frame listener.
             */
            DisplayFrameListener *frame_listener_;

            /**
             * The application scene manager.
             */
            Ogre::SceneManager *scene_manager_;

            /**
             * The engine camera.
             */
            Ogre::Camera *camera_;

            /**
             * The engine viewport.
             */
            Ogre::Viewport *viewport_;
        };
}

