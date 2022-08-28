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

#include <OgreLogManager.h>
#include <OgreString.h>
#include <OgreStringConverter.h>
#include <string>

/**
 * Prints an error log message.
 *
 * @param message[in] Message to print.
 */
#define LOG_ERROR(message) Ogre::LogManager::getSingleton().logMessage( \
  "[ERROR] " + Ogre::String(__FILE__) + " " \
  + Ogre::StringConverter::toString(__LINE__) + ": " + message, \
  Ogre::LML_CRITICAL \
)

/**
 * Prints  awarning log message.
 *
 * @param message[in] Message to print.
 */
#define LOG_WARNING(message) Ogre::LogManager::getSingleton().logMessage( \
  "[WARNING] " + Ogre::String(__FILE__) + " " \
  + Ogre::StringConverter::toString(__LINE__) + ": " + message, \
  Ogre::LML_NORMAL \
)

/**
 * Prints a trivial log message.
 *
 * @param message[in] Message to print.
 */
#define LOG_TRIVIAL(message) Ogre::LogManager::getSingleton().logMessage( \
  message, Ogre::LML_TRIVIAL \
)

/**
 * Prints a log message to the console with normal priority.
 *
 * @param message[in] Message to print.
 */
#define LOG_CONSOLE(message) Ogre::LogManager::getSingleton().logMessage( \
  message, Ogre::LML_NORMAL \
)

// TODO: Remove microsoft tools and leave only the generic?
#ifdef _MSC_VER

/**
 * Prints an explicit debug message to the log (Microsoft only).
 *
 * It includes the filename, line, and function name. The priority is critical
 *
 * @param message[in] Message to print.
 */
#define LOG_DEBUG_EX(message) Ogre::LogManager::getSingleton().logMessage( \
  "[DEBUG] (" +  __FILE__ + " " + Ogre::StringConverter::toString(__LINE__) \
  + ")(" + std::string(__FUNCTION__) + "): " + message, \
  Ogre::LML_CRITICAL \
  )

/**
 * Prints a debug message to the log (Microsoft only).
 *
 * It includes the function name. The priority is critical
 *
 * @param message[in] Message to print.
 */
#define LOG_DEBUG(message) Ogre::LogManager::getSingleton().logMessage(\
  "[DEBUG] (" + std::string(__FUNCTION__) + "): " + message, \
  Ogre::LML_CRITICAL \
)
#else

/**
 * Prints an explicit debug message to the log.
 *
 * It includes the filename, line, and function name. The priority is critical
 *
 * @param message[in] Message to print.
 */
#define LOG_DEBUG_EX(message) Ogre::LogManager::getSingleton().logMessage( \
  "[DEBUG] (" +  __FILE__ + " " + Ogre::StringConverter::toString(__LINE__) \
  + ")(" + std::string(__PRETTY_FUNCTION__) + "): " + message, \
  Ogre::LML_CRITICAL \
)

/**
 * Prints a debug message to the log.
 *
 * It includes the function name. The priority is critical
 *
 * @param message[in] Message to print.
 */
#define LOG_DEBUG(message) Ogre::LogManager::getSingleton().logMessage( \
  "[DEBUG] (" + std::string(__PRETTY_FUNCTION__) + "): " + message, \
  Ogre::LML_CRITICAL \
)
#endif
