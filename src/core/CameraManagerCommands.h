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

#include <OgreStringConverter.h>
#include "CameraManager.h"
#include "ConfigCmdHandler.h"
#include "Console.h"

void CommandCameraFree(const Ogre::StringVector& params){
    if (params.size() < 1){
        Console::getSingleton().AddTextToOutput("Usage: /camera_free <enable>");
        return;
    }
    CameraManager::getSingleton().SetCameraFree(
      Ogre::StringConverter::parseBool(params[1])
    );
}

void CommandCameraFreeCompletition( Ogre::StringVector& complete_params ){
    complete_params.push_back( "true" );
    complete_params.push_back( "false" );
}

void CameraManager::InitCommands(){
    ConfigCmdHandler::getSingleton().AddCommand(
      "camera_free", "Enable or disable free camera", "",
      CommandCameraFree, CommandCameraFreeCompletition
    );
}
