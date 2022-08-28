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

#include "QGearsDataPlugin.h"
#include "data/QGearsTexCodec.h"

using Ogre::String;

namespace QGears {

    const String DataPlugin::ms_plugin_name(
      "Plugin for Various FFVII File Formats"
    );

    DataPlugin::DataPlugin(){}

    const String& DataPlugin::getName() const{return ms_plugin_name;}

    void DataPlugin::install(){TexCodec::install();}

    void DataPlugin::initialise(){
        TexCodec::initialise();
        p_manager_           = new PFileManager();
        hrc_manager_         = new HRCFileManager();
        rsd_manager_         = new RSDFileManager();
        //palette_manager_     = new PaletteFileManager();
        //background_manager_  = new BackgroundFileManager();
    }

    void DataPlugin::shutdown(){
        TexCodec::shutdown();
        delete p_manager_;
        delete hrc_manager_;
        delete rsd_manager_;
        //delete palette_manager_;
        //delete background_manager_;
    }

    void DataPlugin::uninstall(){TexCodec::uninstall();}
}
