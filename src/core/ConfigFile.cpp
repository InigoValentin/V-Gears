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

#include "core/ConfigFile.h"
#include "core/Logger.h"
#include "core/Utilites.h"
#include "ConfigCmdHandler.h"

void ConfigFile::Execute(const Ogre::String& name){
    // Open the configuration file
    std::ifstream fp;
    // Always open in binary mode
    fp.open(name.c_str(), std::ios::in | std::ios::binary);
    if (!fp) LOG_WARNING("Config file\"" + name + "\" not found!");
    else{
        Ogre::String command = "";
        while (true){
            char c = fp.get(); // get character from file
            if (fp.good() == false) break;
            if (c == '\r' || (c == '\n' && command.size() == 0)){}
            else if (c == '\n' && command.size() > 0){
                Ogre::StringVector params = StringTokenise(command);

                if (params.size() > 0){
                    // handle command
                    ConfigCmd* cmd
                      = ConfigCmdHandler::getSingleton().Find(params[0]);
                    if (cmd != nullptr) cmd->GetHandler()(params);
                    else
                        LOG_ERROR("Can't find command \"" + params[0] + "\".");
                }

                command = "";
            }
            // use only ascii characters
            else if (c >= 0) command += c;
        }

        if (command.size() > 0){
            Ogre::StringVector params = StringTokenise(command);

            if (params.size() > 0){
                ConfigCmd* cmd
                  = ConfigCmdHandler::getSingleton().Find(params[0]);

                if(cmd != nullptr) cmd->GetHandler()(params);
                else LOG_ERROR("Can't find command \"" + params[0] + "\".");
            }
        }
        fp.close(); // close file
    }
}
