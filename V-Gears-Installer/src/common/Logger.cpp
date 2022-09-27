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
#include "common/Logger.h"
#include "common/FileSystem.h"

Logger* LOGGER = NULL;

Ogre::String HexToString(int value, unsigned short width, char fill){
    std::stringstream stream;
    stream.width(width);
    stream.fill(fill);
    stream.setf(std::ios::hex, std::ios::basefield);
    stream << value;
    return stream.str();
}

Ogre::String BoolToString(bool value){
    return Ogre::StringConverter::toString(value);
}

Ogre::String IntToString(int value){
    return Ogre::StringConverter::toString(value);
}

Ogre::String FloatToString(float value){
    return Ogre::StringConverter::toString(value);
}

Logger::Logger(const Ogre::String& log_file_name):
    log_file_(log_file_name){
    FileSystem::RemoveFile(log_file_);
}

Logger::~Logger(){}

void Logger::Log(const Ogre::String& text){
    FileSystem::WriteFile(log_file_, text.c_str(), text.size());
}

void Logger::LogW(const Ogre::UTFString& text){
    FileSystem::WriteFile(log_file_, text.c_str(), text.size() * 2);
}

void Logger::Log(std::vector< unsigned char >& text){
    unsigned char* temp = new unsigned char[ text.size() ];
    for (unsigned int i = 0; i < text.size(); ++ i) temp[i] = text[i];
    FileSystem::WriteFile(log_file_, temp, text.size());
    delete[] temp;
}
