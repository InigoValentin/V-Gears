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

#include <OgreStringConverter.h>
#include "Console.h"
#include "ConfigCmdManager.h"
#include "ConfigVarManager.h"
#include "Logger.h"
#include "Utilites.h"

/**
 * Parses a list of keys to retrieve their keycodes.
 *
 * @param string[in] Every character in this string will be processed, and the
 * correspondant keycodes will be added to 'key_codes'.
 * @param key_codes[out] Every keycode assigned to a character in 'string' will
 * be added here, even repeated ones.
 * @return True if there were no errors, false if at least one of the
 * characters in 'string' doesn't have an associated keycode. Every case wil be
 * logged.
 */
bool ParseKeys(const Ogre::String& string, ButtonList& key_codes){
    Ogre::StringVector keys = Ogre::StringUtil::split(string, "+");

    for (unsigned int i = 0; i < keys.size(); ++ i)
        key_codes.push_back(StringToKey(keys[i]));

    bool fail = false;
    for (unsigned int i = 0; i < key_codes.size(); ++ i){
        if (key_codes[i] == OIS::KC_UNASSIGNED){
            LOG_ERROR(
              "Failed to parse key string \"" + string
              + "\". Can't recognize key " + Ogre::StringConverter::toString(i)
            );
            fail = true;
        }
    }
    return fail;
}

/**
 * Binds a key combination to a command.
 *
 * @param params[in] Binding parameters. Exactly three are required. The first
 * one is the binding command name and it's not evaluated here. The second one
 * must be one or more keycodes. The third one is the command string. If more
 * or less than three parameter are passed, a usage string will be printed
 * instead, and nothing will be done.
 * @todo How are keycodes separated? spaces or '+'?
 */
void CmdBind(const Ogre::StringVector& params){
    if (params.size() != 3){
        Console::getSingleton().AddTextToOutput(
          "Usage: /bind <key1>+[key2]+[key3] \"<command line>\""
        );
        return;
    }
    ButtonList key_codes;
    if (ParseKeys(params[1], key_codes) == false){
        Ogre::StringVector params_cmd = StringTokenise(params[2]);

        // Handle command
        ConfigCmd* cmd = ConfigCmdManager::getSingleton().Find(params_cmd[0]);
        if (cmd != NULL){
            InputManager::getSingleton().BindCommand(
              cmd, params_cmd, key_codes
            );
            LOG_TRIVIAL(
              "Bind \"" + params[1] + "\" to command \"" + params[2] + "\"."
            );
        }
        else{
            LOG_ERROR(
              "Can't find command \"" + params_cmd[0]
              + "\" in bind command \"" + params[2] + "\"."
            );
        }
    }
}

/**
 * Binds a key combination to a game event.
 *
 * @param params[in] Binding parameters. Exactly three are required. The first
 * one is the binding command name and it's not evaluated here. The second one
 * must be one or more keycodes. The third one is the game event string. If
 * more or less than three parameter are passed, a usage string will be printed
 * instead, and nothing will be done.
 * @todo How are keycodes separated? spaces or '+'?
 */
void CmdBindGameEvent(const Ogre::StringVector& params){
    if (params.size() != 3){
        Console::getSingleton().AddTextToOutput(
          "Usage: /game_bind <key1>+[key2]+[key3] \"<game event>\""
        );
        return;
    }
    ButtonList key_codes;
    if (ParseKeys(params[1], key_codes) == false){
        InputManager::getSingleton().BindGameEvent(params[2], key_codes);
        LOG_TRIVIAL(
          "Bind \"" + params[1] + "\" to game event \"" + params[2] + "\"."
        );
    }
}

// TODO: Move this to InpuManager.cpp?
void InputManager::InitCmd(){
    ConfigCmdManager::getSingleton().AddCommand(
      "bind", "Bind command to keys", "", CmdBind, NULL
    );
    ConfigCmdManager::getSingleton().AddCommand(
      "bind_game_event", "Bind game event to keys", "", CmdBindGameEvent, NULL
    );
}
