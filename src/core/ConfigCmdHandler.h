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

#include <OgreSingleton.h>
#include <vector>
#include <memory>
#include "ConfigCmd.h"

/**
 * A handler for configuration commands.
 */
class ConfigCmdHandler : public Ogre::Singleton<ConfigCmdHandler>{

    public:

        /**
         * Constructor.
         */
        ConfigCmdHandler();

        /**
         * Destructor.
         */
        ~ConfigCmdHandler();

        /**
         * Adds a command to the manager.
         *
         * @param[in] name Command name.
         * @param[in] description Command description.
         * @param[in] params_description Command parameters description.
         * @param[in] params Command parameter list.
         * @param[in] completion Command completion list.
         */
        void AddCommand(
          const Ogre::String& name, const Ogre::String& description,
          const Ogre::String& params_description, ConfigCmdParams params,
          ConfigCmdCompletion completion
        );

        /**
         * Executes a command.
         *
         * @param[in] cmd_string The command, in string format.
         */
        void ExecuteString( const Ogre::String& cmd_string );

        /**
         * Finds a command by name.
         *
         * @param[in] name Name of the command.
         * @return The command by the name, or nullptr if there is none.
         */
        ConfigCmd* Find(const Ogre::String& name) const;

        /**
         * Counts the commands in the manager.
         *
         * @return The total number of commands
         */
        int GetConfigCmdNumber();

        /**
         * Retrieves a command by index.
         *
         * A command index is the position at which it was added to the manager.
         */
        ConfigCmd* GetConfigCmd(unsigned int i) const;

    private:

        /**
         * Forbidden copy constructor.
         *
         * @param[in] rhs Manager to not copy.
         */
        ConfigCmdHandler(const ConfigCmdHandler& rhs) = delete;

        /**
         * Forbidden copy constructor.
         *
         * @param[in] rhs Manager to not copy.
         */
        ConfigCmdHandler operator =(const ConfigCmdHandler& rhs) = delete;

        /**
         * Initializes the command.
         *
         * Must be called from the constructor.
         */
        void InitCmd();

        /**
         * List of command in the manager.
         */
        std::vector<std::unique_ptr<ConfigCmd>> commands_;
};

