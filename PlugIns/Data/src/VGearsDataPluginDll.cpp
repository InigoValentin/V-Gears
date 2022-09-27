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

#include <OgreRoot.h>
#include "VGearsDataPlugin.h"

#ifndef VGEARS_STATIC_LIB

namespace VGears{

    DataPlugin* data_plugin(NULL);

    extern "C" void _VGearsExport dllStartPlugin( void ){
        data_plugin = new DataPlugin();
        // Register.
        Ogre::Root::getSingleton().installPlugin(data_plugin);
    }
    extern "C" void _VGearsExport dllStopPlugin( void ){
        Ogre::Root::getSingleton().uninstallPlugin(data_plugin);
        delete data_plugin;
        data_plugin = NULL;
    }
}

#endif
