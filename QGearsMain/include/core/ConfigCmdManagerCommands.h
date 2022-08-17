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

#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreStringConverter.h>
#include "Console.h"
#include "ConfigCmdManager.h"
#include "ConfigVarManager.h"
#include "EntityManager.h"
#include "Logger.h"
#include "XmlMapFile.h"
#include "XmlMapsFile.h"
#include "QGearsGameState.h"
#include "common/QGearsApplication.h"

/**
 * Command to quit the application.
 *
 * @param params[in] Command parameters. Ignored.
 */
void CmdQuit(const Ogre::StringVector& params){
    QGears::g_ApplicationState = QGears::G_EXIT;
}

/**
 * Command to print to console.
 *
 * @param params[in] Command parameters. All of them will be concatenated and
 * printed. If none are supplied, a command usage text will be printed instead.
 */
void CmdEcho(const Ogre::StringVector& params){
    if (params.size() < 1){
        Console::getSingleton().AddTextToOutput(
          "Usage: /echo <string to output>"
        );
        return;
    }
    Ogre::String text = "";
    for (size_t i = 1; i < params.size(); ++ i){
        if (i != 1)text += " ";
        text += params[i];
    }
    Console::getSingleton().AddTextToOutput(text + "\n");
}

/**
 * Searches variables in the variable list and prints them.
 *
 * @param params[in] Command parameters. The first one is the command name. If
 * no more are passed, all variables will be printed. If another parameter is
 * passed, the variables with that name (if any) will be printed. If more than
 * two parameter are passed, a command usage string will be printed instead.
 */
void CmdConfigVarList(const Ogre::StringVector& params){
    if (params.size() > 2){
        Console::getSingleton().AddTextToOutput(
          "Usage: /config_var_list [search string]"
       );
        return;
    }

    int number = 0;
    int num_vars = ConfigVarManager::getSingleton().GetConfigVarNumber();
    for (int i = 0; i < num_vars; ++ i){
        ConfigVar* var = ConfigVarManager::getSingleton().GetConfigVar(i);
        Ogre::String name = var->GetName();

        if (params.size() > 1){
            int found = name.find(params[1]);
            if (found == 0){
                Console::getSingleton().AddTextToOutput(
                  var->GetName() + " = \"" + var->GetS() + "\""
               );
                ++number;
            }
        }
        else{
            Console::getSingleton().AddTextToOutput(
              var->GetName() + " = \"" + var->GetS() + "\""
           );
            ++number;
        }
    }
    Console::getSingleton().AddTextToOutput(
      Ogre::StringConverter::toString(number) + " config variables.\n"
   );
}

/**
 * Searches the command list and prints the comands.
 *
 * @param params[in] Command parameters. The first one is the command name. If
 * no more is passed, all commands will be printed. If another parameter is
 * passed, the command with that name (if any) will be printed. If more than
 * two parameter are passed, a command usage string will be printed instead.
 */
void CmdConfigCmdList(const Ogre::StringVector& params){
    if (params.size() > 2){
        Console::getSingleton().AddTextToOutput(
          "Usage: /config_cmd_list [search string]"
       );
        return;
    }
    int number = 0;
    int num_cmds = ConfigCmdManager::getSingleton().GetConfigCmdNumber();
    for (int i = 0; i < num_cmds; ++ i){
        ConfigCmd* cmd = ConfigCmdManager::getSingleton().GetConfigCmd(i);
        Ogre::String name = cmd->GetName();
        if (params.size() > 1){
            int found = name.find(params[1]);
            if (found == 0){
                Console::getSingleton().AddTextToOutput(cmd->GetName());
                ++number;
            }
        }
        else{
            Console::getSingleton().AddTextToOutput(cmd->GetName());
            ++number;
        }
    }
    Console::getSingleton().AddTextToOutput(
      Ogre::StringConverter::toString(number) + " config commands.\n"
   );
}

/**
 * Sets the value of a configuration value.
 *
 * @param params[in] Command parameters. The first one is the command name. The
 * second one is a variable name. The third one is optional and is a value for
 * the variable. If a value is supplied, the variable will be given that value.
 * If not, the variable will be reset to it's default value. If there is no
 * variable by that name, nothing will be done. In any case, a feddback will be
 * printed to console. If less than two or more than three parameters are
 * passed, a usage text wil be printed and nothing will be done.
 */
void CmdSetConfigVar(const Ogre::StringVector& params)
{
    if (params.size() < 2 || params.size() > 3){
        Console::getSingleton().AddTextToOutput(
          "Usage: /set <config variable> [value]"
       );
        return;
    }
    Ogre::String name = params[1];
    ConfigVar* cvar = ConfigVarManager::getSingleton().Find(name);
    if (cvar == NULL){
        LOG_ERROR("Config variable \"" + name + "\" not found.");
        return;
    }
    if (params.size() == 3){
        cvar->SetS(params[2]);
        Console* console = Console::getSingletonPtr();
        if (console != NULL)
            LOG_TRIVIAL(params[1] + " changed to \"" + params[2] + "\".");
    }
    else{
        // Reset to default
        cvar->SetS(cvar->GetDefaultValue());
        LOG_TRIVIAL(
          params[1] + " changed to default \""
          + cvar->GetDefaultValue() + "\"."
       );
    }
}

/**
 * Changes the value of a configuration value conditionally.
 *
 * @param params[in] Command parameters. The first one is the command name. The
 * next ones are possible values for the variables. If the value of the
 * variable is the current one, the next one will be assigned. Once the value
 * is changed once, no more steps will be taken and the function will return.
 * If the last provided value is the current value of the variable, it will not
 * be changed.
 */
void CmdToggleConfigVar(const Ogre::StringVector& params){
    if (params.size() < 4){
        Console::getSingleton().AddTextToOutput(
          "Usage: /toggle <config variable> [value1] [value2] ...");
        return;
    }
    Ogre::String name = params[1];
    ConfigVar* cvar = ConfigVarManager::getSingleton().Find(name);
    if (cvar == NULL){
        LOG_ERROR("Config variable \"" + name + "\" not found.");
        return;
    }

    // Sequentially trigger values
    int number_of_values = params.size() - 2;
    Ogre::String value = cvar->GetS();
    int i = 0;
    for (; i < number_of_values; ++ i){
        if (value == params[i + 2]){
            ++ i;
            break;
        }
    }
    if (i == number_of_values) i = 0;
    cvar->SetS(params[i + 2]);
}

/**
 * Increments the value of a configuration variable.
 *
 * @param params[in] Command parameters. Exactly five must be provided. The
 * first one is the command name. The second one is the variable to increment.
 * The third one is the minimum value the variable will take. The fourth one is
 * the maximum value the variable will take. The fifth value is the increment
 * to apply to the variable. The variable in the second parameter will be
 * incremented by the value in the fith one, but it will be capped between the
 * third and fourth one. If there are more or less than five parameters, a
 * usage text will be printed and nothing will be done.
 */
void CmdIncrementConfigVar(const Ogre::StringVector& params){
    if (params.size() != 5){
        Console::getSingleton().AddTextToOutput(
          "Usage: /increment <cvar name> [value min] [value max] [step]"
       );
        return;
    }
    Ogre::String name = params[1];
    ConfigVar* cvar = ConfigVarManager::getSingleton().Find(name);
    if (cvar == NULL){
        LOG_ERROR("Config variable \"" + name + "\" not found.");
        return;
    }
    float start_value = Ogre::StringConverter::parseReal(params[2]);
    float end_value = Ogre::StringConverter::parseReal(params[3]);
    float step = Ogre::StringConverter::parseReal(params[4]);

    float new_value = cvar->GetF() + step;
    if (new_value > end_value) new_value = end_value;
    else if (new_value < start_value) new_value = start_value;
    cvar->SetF(new_value);
}

/**
 * Configures the log level.
 *
 * @param params[in] Command parameters. Exactly two must be provided. The
 * first one is the command name. The second one is the log level. Accepted
 * values are 1 (only errors), 2 (errors and warnings) and 3 (all). If there
 * are more or less than five parameters, a usage text will be printed and
 * nothing will be done.
 */
void CmdSetLogLevel(const Ogre::StringVector& params){
    if (params.size() != 2){
        Console::getSingleton().AddTextToOutput(
          "Usage: /log_level "
          "<level: 1 - only errors, 2 - errors and warnings, 3 - all>"
        );
        return;
    }

    int value = Ogre::StringConverter::parseInt(params[1]);
    if (value > 0 && value < 4){
        Ogre::LogManager::getSingletonPtr()->getDefaultLog()->setLogDetail(
          (Ogre::LoggingLevel) value
        );
        switch(value){
            case 1:
                Console::getSingleton().AddTextToOutput(
                  "Logger level changed to \"only errors\".\n"
                );
                break;
            case 2:
                Console::getSingleton().AddTextToOutput(
                  "Logger level changed to \"errors and warnings\".\n"
                );
                break;
            case 3:
                Console::getSingleton().AddTextToOutput(
                  "Logger level changed to \"all\".\n"
                );
                break;
        }
    }
    else{
        Console::getSingleton().AddTextToOutput(
          "Logger level can't be changed. Value \""
          + params[1] + "\" isn't supported.\n"
        );
    }
}

/**
 * Changes the game map.
 *
 * @param params[in] Command parameters. Exactly two must be provided. The
 * first one is the command name. The second one is the map ID. If there
 * are more or less than five parameters, a usage text will be printed and
 * nothing will be done.
 */
void CmdMap(const Ogre::StringVector& params){
    if (params.size() != 2){
        Console::getSingleton().AddTextToOutput("Usage: /map [map_id]");
        return;
    }
    EntityManager::getSingleton().Clear();
    XmlMapsFile xml("./data/maps.xml");
    Ogre::String file_name = xml.GetMapFileNameByName(params[1]);
    XmlMapFile xml_map("./data/" + file_name);
    xml_map.LoadMap();
}

/**
 * Loads a list of map names.
 *
 * @param complete_params[in] The map names will be loaded here.
 */
void CmdMapCompletion(Ogre::StringVector& complete_params){
    XmlMapsFile xml("./data/maps.xml");
    xml.GetMapNames(complete_params);
}

/**
 * Sets the resolution and full screen mode.
 *
 * @param params[in] Command parameters. Three or four must be provided. The
 * first one is the command name. The second one is the resolution width. The
 * third one is the resolution height. The fourth one is optional and can be
 * used to toggle the full screen. "true", "yes" or 1 will set the game in full
 * screen mode. Anything else will set it to windowed mode.
 */
void CmdResolution(const Ogre::StringVector& params){
    if (params.size() < 3){
        Console::getSingleton().AddTextToOutput(
          "Usage: /resolution <width> <height> [full screen]"
        );
        return;
    }
    Ogre::RenderWindow* window
      = QGears::Application::getSingleton().getRenderWindow();
    if (params.size() >= 4){
        window->setFullscreen(
          Ogre::StringConverter::parseBool(params[3]),
          Ogre::StringConverter::parseInt(params[1]),
          Ogre::StringConverter::parseInt(params[2])
        );
    }
    else{
        window->resize(
          Ogre::StringConverter::parseInt(params[1]),
          Ogre::StringConverter::parseInt(params[2])
        );
    }
}

/**
 * Loads a list of resolution modes.
 *
 * A resolution mode is represented by a string with the format "[w] [h] [f]",
 * where [w] is the resolution width, in pixels, [h] is the resolution height,
 * in pixels and [f] is the full screen state (0 for windowed mode, 1 for full
 * screen)
 *
 * @param complete_params[in] The resolution modes will be loaded here.
 */
void CmdResolutionCompletition(Ogre::StringVector& complete_params){
    complete_params.push_back("640 480 0");
    complete_params.push_back("640 480 1");
    complete_params.push_back("800 600 0");
    complete_params.push_back("800 600 1");
    complete_params.push_back("1024 768 0");
    complete_params.push_back("1024 768 1");
    complete_params.push_back("1280 720 0");
    complete_params.push_back("1280 720 1");
    complete_params.push_back("1280 1024 0");
    complete_params.push_back("1280 1024 1");
}

/**
 * Saves a screenshot of the current game window.
 *
 * @param params Command parameters. Unused.
 */
void CmdScreenshot(const Ogre::StringVector& params){
    Ogre::RenderWindow* window
      = QGears::Application::getSingleton().getRenderWindow();
    Ogre::String ret
      = window->writeContentsToTimestampedFile("screenshot_", ".tga");
    Console::getSingleton().AddTextToOutput("Screenshot " + ret + " saved.");
}



/*
void CmdViewer(const Ogre::StringVector& params){
    if (params.size() == 2 || params.size() > 3){
        Console::getSingleton().AddTextToOutput(
          "Usage: /viewer [type_of_thing_to_view] [path_to_resource]"
        );
        return;
    }
    ModuleManager::getSingleton().RunViewer();
    if (params.size() == 3){
        ViewerModule* module
          = (ViewerModule*)(ModuleManager::getSingleton().GetTopModule());

        if (params[1] == "walkmesh") module->SetWalkmeshToLoad(params[2]);
        else if (params[1] == "model") module->SetModelToLoad(params[2]);
        else{
            LOG_ERROR(
              "Unsupported type \"" + params[1] + "\" in viewer command."
            );
        }
    }
}

void CmdViewerCompletion(Ogre::StringVector& complete_params){
    // models
    complete_params.push_back("model");
    Ogre::FileInfoListPtr resources
      = Ogre::ResourceGroupManager::getSingleton().listResourceFileInfo("Game");
    Ogre::FileInfoList resource_names = *resources;

    Ogre::FileInfoList::iterator i = resource_names.begin();
    for (; i != resource_names.end(); ++ i){
        Ogre::String name;
        Ogre::String ext;
        Ogre::StringUtil::splitBaseFilename(i->filename, name, ext);
        if (ext == "mesh") complete_params.push_back("model " + i->filename);
    }

    // walkmeshes
    complete_params.push_back("walkmesh");
    XmlMapsFile xml("./data/game_data/maps.xml");
    Ogre::StringVector tmp;
    xml.GetMapNames(tmp);
    for (int i = 0; i < tmp.size(); ++i)
        complete_params.push_back("walkmesh " + tmp[i]);
}
*/

/**
 * Initializes all available commands.
 */
void ConfigCmdManager::InitCmd(){
    AddCommand("quit", "Stops application and quit", "", CmdQuit, NULL);
    AddCommand("echo", "Print command parameters", "", CmdEcho, NULL);
    AddCommand(
      "config_var_list", "List of registered config variables",
      "[<filter substring>]", CmdConfigVarList, NULL
    );
    AddCommand(
      "config_cmd_list", "List of registered config commands",
      "[<filter substring>]", CmdConfigCmdList, NULL
    );
    AddCommand(
      "set", "Set cvar value", "<cvar name> [value]", CmdSetConfigVar, NULL
    );
    AddCommand(
      "toggle", "Toggle cvar value",
      "<cvar name> [value1] [value2] ...", CmdToggleConfigVar, NULL
    );
    AddCommand(
      "increment",
      "Increment cvar value", "<cvar name> [value min] [value max] [step]",
      CmdIncrementConfigVar, NULL
    );
    AddCommand(
      "set_log_level", "Set log messages level", "", CmdSetLogLevel, NULL
    );
    AddCommand("map", "Run game module", "", CmdMap, CmdMapCompletion);
    AddCommand(
      "resolution", "Change resolution", "",
      CmdResolution, CmdResolutionCompletition
    );
    AddCommand(
      "screenshot", "Capture current screen content", "", CmdScreenshot, NULL
    );
    //AddCommand(
    //  "viewer", "Run viewer module", "", CmdViewer, CmdViewerCompletion
    //);
}
