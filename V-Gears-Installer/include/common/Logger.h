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
#include <OgreUTFString.h>

/**
 * Converts an hexadecimal value to a string.
 *
 * @param value[in] The value to convert.
 * @param width[in] Word width.
 * @param fill[in] Filling character for word alignment.
 * @return String representation of the value.
 */
Ogre::String HexToString(int value, unsigned short width, char fill);

/**
 * Converts a boolean value to a string.
 *
 * @param value[in] The value to convert.
 * @return String representation of the value. 'true' if the value is true,
 * 'false' otherwise.
 */
Ogre::String BoolToString(bool value);

/**
 * Converts an integer value to a string.
 *
 * @param value[in] The value to convert.
 * @return String representation of the value.
 */
Ogre::String IntToString(int value);

/**
 * Converts a real value to a string.
 *
 * @param value[in] The value to convert.
 * @return String representation of the value.
 */
Ogre::String FloatToString(float value);

/**
 * Application logger utility.
 */
class Logger{

    public:

        /**
         * Constructor.
         *
         * @param log_file_name[in] Path to the log file.
         */
        explicit Logger(const Ogre::String& log_file_name);

        /**
         * Destructor.
         */
        virtual ~Logger();

        /**
         * Logs a message.
         *
         * @param text[in] Message to log.
         */
        void Log(const Ogre::String& text);

        /**
         * Logs a message.
         *
         * @param text[in] Message to log.
         */
        void LogW(const Ogre::UTFString& text);

        /**
         * Logs messages.
         *
         * @param text[in] Messages to log.
         */
        void Log(std::vector<unsigned char>& text);

    private:

        /**
         * PAth to the log file.
         */
        Ogre::String log_file_;
};

/**
 * Application logger.
 *
 * To be used application-wide.
 */
extern Logger *LOGGER;
