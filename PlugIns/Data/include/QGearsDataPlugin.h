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

#include <OgrePlugin.h>

#include "data/QGearsBackgroundFileManager.h"
#include "data/QGearsPaletteFileManager.h"
#include "data/QGearsPFileManager.h"
#include "data/QGearsHRCFileManager.h"
#include "data/QGearsRSDFileManager.h"
#include "data/QGearsTexCodec.h"

namespace QGears {

    /**
     * Plugin instance for various FFVII Data Formats
     */
    class DataPlugin : public Ogre::Plugin{

        public:

            DataPlugin();

            /**
             * Retrieves the name fo the plugin.
             */
            const Ogre::String& getName() const;

            /**
             * Perform the plugin initial installation sequence.
             */
            void install();

            /**
             * Perform any tasks the plugin needs on system initialization.
             */
            void initialise();

            /**
             * Perform any tasks the plugin needs on system shut down.
             */
            void shutdown();

            /**
             * Perform the final plugin uninstallation sequence.
             */
            void uninstall();

            /**
             * The plugin name.
             */
            static const Ogre::String ms_plugin_name;

        private:

            /**
             * The .P file manager.
             */
            PFileManager *p_manager_;

            /**
             * The HRC file manager.
             */
            HRCFileManager *hrc_manager_;

            /**
             * The RSD file manager.
             */
            RSDFileManager *rsd_manager_;

            /**
             * The palette file manager.
             */
            PaletteFileManager *palette_manager_;

            /**
             * The background file manager.
             */
            BackgroundFileManager *background_manager_;

    };
}
