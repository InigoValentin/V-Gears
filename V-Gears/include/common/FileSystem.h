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

/**
 * Static class which allows filesystem manipulation
 */
class FileSystem{

    public:

        /**
         * Destructor.
         */
        FileSystem() = delete;

        /**
         * Retrieves a file size.
         *
         * @param path[in] Path to the file.
         * @return File size, in bytes. 0 on error.
         */
        static unsigned int GetFileSize(const Ogre::String& path);

        /**
         * Reads a file contents and saves them to a buffer.
         *
         * @param path[in] Path to the file to read.
         * @param buffer[out] The buffer to populate with file data.
         * @param start[in] Offset at which to start reading.
         * @param lenght[in] Size of the data to read, in bytes.
         * @return True if the file was successfully read, false on error.
         */
        static bool ReadFile(
          const Ogre::String &path, void* buffer,
          const unsigned int start, const unsigned int length
        );

        /**
         * Writes the contents of a buffer to a file.
         *
         * The file must exists, and the new data will be appended at the end.
         *
         * @param path[in] Path to the file to write.
         * @param buffer[in] Buffer with the data.
         * @param length[in] Size of the data to write, in bytes.
         * @return True if the file was successfully written, false on error.
         */
        static bool WriteFile(
          const Ogre::String &path, const void* buffer,
          const unsigned int length
        );

        /**
         * Writes the contents of a buffer to new a file.
         *
         * If the file exists, it will be deleted before adding the new data.
         *
         * @param path[in] Path to the file to write.
         * @param buffer[in] Buffer with the data.
         * @param length[in] Size of the data to write, in bytes.
         * @return True if the file was successfully written, false on error.
         */
        static bool WriteNewFile(
          const Ogre::String &path, const void* buffer,
          const unsigned int length
        );

        /**
         * Deletes a file.
         *
         * @param path Path to the file to delete.
         * @return True if the file was successfully deleted, false on error.
         */
        static bool RemoveFile(const Ogre::String &path);
};

