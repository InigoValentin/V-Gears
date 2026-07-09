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
#include <memory>
#include <string>

/**
 * Information about a Final Fantasy VII disk image.
 */
struct DiskImage{

    public:

        /**
         * The platform for which the disk image is for.
         */
        enum Platform{

            /** The disk image is for PC. */
            PLATFORM_PC,
            /** The disk image is for PS1. */
            PLATFORM_PS1,
            /** The disk image is for an unknown platform. */
            PLATFORM_UNKNOWN
        };

        /**
         * The region for which the disk image is for.
         */
        enum Region{
            /** The disk image is for Japan. */
            REGION_JAPAN,
            /** The disk image is for North America. */
            REGION_NORTH_AMERICA,
            /** The disk image is for Europe. */
            REGION_EUROPE,
            /** The disk image is for an unknown region. */
            REGION_UNKNOWN
        };

        /**
         * The language for which the disk image is in.
         */
        enum Language{
            /** The disk image is in Japanese. */
            LANGUAGE_JAPANESE,
            /** The disk image is in English. */
            LANGUAGE_ENGLISH,
            /** The disk image is in French. */
            LANGUAGE_FRENCH,
            /** The disk image is in Spanish. */
            LANGUAGE_SPANISH,
            /** The disk image is in German. */
            LANGUAGE_GERMAN,
            /** The disk image is in Italian. */
            LANGUAGE_ITALIAN,
            /** The disk image is for an unknown language. */
            LANGUAGE_UNKNOWN
        };

        /**
         * The disk for which the disk image corresponds to.
         */
        enum Disk{
            /** The disk image is for the install disk (PC only). */
            DISK_INSTALL,
            /** The disk image is for disk 1. */
            DISK_1,
            /** The disk image is for disk 2. */
            DISK_2,
            /** The disk image is for disk 3. */
            DISK_3,
            /** The disk image is for disk 4. */
            DISK_4,
            /** The disk image is for an unknown disk. */
            DISK_UNKNOWN
        };

        /**
         * Constructs a new disk image.
         */
        DiskImage();

        /**
         * Constructs a new disk image from an ISO path.
         *
         * @param path The path to the ISO file.
         */
        DiskImage(std::string path);

        /**
         * Destroys the disk image.
         */
        ~DiskImage();

        /**
         * Gets the path to the disk image file.
         *
         * @return The path to the disk image file.
         */
        std::string const getPath();

        /**
         * Gets the human readable ID of the disk image.
         *
         * @return The ID of the disk image.
         */
        std::string const getId();

        /**
         * Gets the platform for which the disk image is intended.
         *
         * @return The platform of the disk image.
         */
        Platform const getPlatform();

        /**
         * Gets the region for which the disk image is intended.
         *
         * @return The region of the disk image.
         */
        Region const getRegion();

        /**
         * Gets the language for which the disk image is intended.
         *
         * @return The language of the disk image.
         */
        Language const getLanguage();

        /**
         * Gets the disk for which the disk image is intended.
         *
         * @return The disk of the disk image.
         */
        Disk const getDisk();

        /**
         * Checks if the disk image is a valid Final Fantasy VII release.
         *
         * @return True if the disk image is valid, false otherwise.
         */
        bool const isValid();

        /**
         * Gets the error message for the disk image.
         *
         * @return The error message.
         */
        std::string const getErrorMessage();

        /**
         * Checks if the disk image is supported and tested in V-Gears.
         *
         * @return True if the disk image is supported, false otherwise.
         */
        bool const isSupported();

        /**
         * Gets a warning in case the disk image is not fully supported.
         *
         * @return The warning message.
         */
        std::string const getWarningMessage();

        /**
         * Gets the path where the content of the disk image has been extracted.
         * 
         * @return The path to the extracted content, null if the content has
         * not been extracted yet.
         */
        std::string const getContentPath();

        /**
         * Extracts the disk image file to the specified output path. The files
         * will be extracted to the "original_data" subdirectory of the output
         * path.
         *
         * @param installation_path The v-gears installation directory.
         * @return True if the extraction was successful, false otherwise.
         */
        bool extractImage(std::string installation_path);

        /**
         * Finds a file in the extracted content of the disk image.
         *
         * @param file_path The relative path to the file within the extracted
         * content.
         * @return A pointer to the full path of the matched file, or nullptr
         * if no matching file exists. If the image has not been extracted yet,
         * this function will always return nullptr.
         */
        std::unique_ptr<std::string> fileExists(std::string file_path);

    private:

        /**
         * The path to the image file.
         */
        std::string path;

        /**
         * A human readable ID of the disk image.
         */
        std::string id;

        /**
         * The platform for which the disk image is intended.
         */
        Platform platform;

        /**
         * The region for which the disk image is intended.
         */
        Region region;

        /**
         * The language for which the disk image is intended.
         */
        Language language;

        /**
         * The disk for which the disk image is intended.
         */
        Disk disk;

        /**
         * Whether the disk image is valid.
         */
        bool valid;

        /**
         * Whether the disk image is supported.
         */
        bool supported;

        /**
         * The error message for the disk image. Empty if no error.
         */
        std::string error_message;

        /**
         * The warning message for the disk image. Empty if no warning.
         */
        std::string warning_message;

        /**
         * The path where the content has been extracted.
         */
        std::string content_path;

        /**
         * Whether the content has been extracted.
         */
        bool is_extracted;
};