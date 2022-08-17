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

#include "ConfigCmdManager.h"
#include "Console.h"

/**
 * Runs a script string.
 *
 * @param params[in] Script parameters. Exactly three are required. The first
 * one is the command name and it's not evaluated here. The second one must be
 * the line to execute. If less than three parameter are passed, a usage string
 * will be printed instead, and nothing will be done.
 */
void CmdScriptRunString(const Ogre::StringVector& params){
    if(params.size() < 2){
        Console::getSingleton().AddTextToOutput(
          "Usage: /script_run_string <string>"
        );
        return;
    }
    ScriptManager::getSingleton().RunString(params[1]);
}

/**
 * Runs a script string.
 *
 * @param params[in] Script parameters. Exactly three are required. The first
 * one is the command name and it's not evaluated here. The second one must be
 * the path to the file to execute. If less than three parameter are passed, a
 * usage string will be printed instead, and nothing will be done.
 */
void CmdScriptRunFile(const Ogre::StringVector& params){
    if(params.size() < 2){
        Console::getSingleton().AddTextToOutput(
          "Usage: /script_run_file <file name>"
        );
        return;
    }
    ScriptManager::getSingleton().RunFile(params[1]);
}

void ScriptManager::InitCmd(){
    ConfigCmdManager::getSingleton().AddCommand(
      "script_run_string", "Run script string", "", CmdScriptRunString, NULL
    );
    ConfigCmdManager::getSingleton().AddCommand(
      "script_run_file", "Run script file", "", CmdScriptRunFile, NULL
    );
}
