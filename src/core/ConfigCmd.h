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

#include <OgreString.h>
#include <OgreStringVector.h>

class ConfigCmdManager;

typedef void (*ConfigCmdHandler)(const Ogre::StringVector& params);

typedef void (*ConfigCmdCompletion)(Ogre::StringVector& complete_params);


/**
 * A configuration command.
 */
class ConfigCmd{

    friend class ConfigCmdManager;

    public:

        /**
         * Constructor.
         *
         * It should only be created only by ConfigCmdManager.
         *
         * @param[in] name Command name.
         * @param[in] description Command description.
         * @param[in] params_description Command parameters description.
         * @param[in] handler Command handler.
         * @param[in] completion Command completion.
         */
        ConfigCmd(
          const Ogre::String& name, const Ogre::String& description,
          const Ogre::String& params_description, ConfigCmdHandler handler,
          ConfigCmdCompletion completion
        );

        /**
         * Retrieves the command name.
         *
         * @return The command name.
         */
        const Ogre::String& GetName() const;

        /**
         * Retrieves the command description.
         *
         * @return The command description.
         */
        const Ogre::String& GetDescription() const;

        /**
         * Retrieves the command parameter description.
         *
         * @return The command parameter description.
         */
        const Ogre::String& GetParamsDescription() const;

        /**
         * Retrieves the command parameter description.
         *
         * @return The command parameter description.
         */
        ConfigCmdHandler GetHandler() const;

        /**
         * Checks the command completion
         *
         * @return The command completion.
         */
        ConfigCmdCompletion GetCompletion() const;

    private:

        /**
         * Forbidden copy constructor.
         *
         * @param[in] rhs Command to not copy.
         */
        ConfigCmd(const ConfigCmd& rhs) = delete;

        /**
         * Forbidden copy constructor.
         *
         * @param[in] rhs Command to not copy.
         */
        ConfigCmd& operator =(const ConfigCmd& rhs) = delete;

        /**
         * The command name.
         */
        Ogre::String name_;

        /**
         * The command description.
         */
        Ogre::String description_;

        /**
         * The command parameters description.
         */
        Ogre::String params_description_;

        /**
         * The command handler.
         */
        ConfigCmdHandler handler_;

        /**
         * The command completion.
         */
        ConfigCmdCompletion completion_;
};

