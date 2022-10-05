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
#include "Logger.h"
#include "XmlTextsFile.h"

/**
 * Sets the language for texts.
 *
 * @param[in] params Command parameters. Exactly three are required. The first
 * one is the command name and it's not evaluated here. The second one must be
 * the language to set. If less than three parameter are passed, a usage string
 * will be printed instead, and nothing will be done.
 */
void CmdSetLanguage(const Ogre::StringVector& params){
    if (params.size() < 2){
        Console::getSingleton().AddTextToOutput(
          "Usage: /set_language <language>"
        );
        return;
    }
    TextManager::getSingleton().SetLanguage(params[1]);
    LOG_TRIVIAL("Set game language to \"" + params[1] + "\".");
}

/**
 * Loads a list of every available language identifiers.
 *
 * To do so, it uses the folders in data/texts.
 *
 * @param[out] complete_params The list of languages will be saved here.
 */
void CmdSetLanguageCompletition(Ogre::StringVector& complete_params){
    XmlTextsFile texts( "./data/texts.xml" );
    texts.GetAvailableLanguages(complete_params);
}

void TextManager::InitCmd(){
    ConfigCmdManager::getSingleton().AddCommand(
      "set_language", "Change language of texts and dialogs", "",
      CmdSetLanguage, CmdSetLanguageCompletition
    );
}
