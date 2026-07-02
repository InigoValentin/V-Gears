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
#include <string>

/**
 * Information about Final Fantasy VII releases.
 */
struct Release{

    public:

        /**
         * The platform for which the release is intended.
         */
        enum Platform{

            /** The release is for PC. */
            PLATFORM_PC,
            /** The release is for PS1. */
            PLATFORM_PS1,
            /** The release is for an unknown platform. */
            PLATFORM_UNKNOWN
        };

        /**
         * The region for which the release is intended.
         */
        enum Region{
            /** The release is for Japan. */
            REGION_JAPAN,
            /** The release is for North America. */
            REGION_NORTH_AMERICA,
            /** The release is for Europe. */
            REGION_EUROPE,
            /** The release is for an unknown region. */
            REGION_UNKNOWN
        };

        /**
         * The language for which the release is intended.
         */
        enum Language{
            /** The release is in Japanese. */
            LANGUAGE_JAPANESE,
            /** The release is in English. */
            LANGUAGE_ENGLISH,
            /** The release is in French. */
            LANGUAGE_FRENCH,
            /** The release is in Spanish. */
            LANGUAGE_SPANISH,
            /** The release is in German. */
            LANGUAGE_GERMAN,
            /** The release is in Italian. */
            LANGUAGE_ITALIAN,
            /** The release is for an unknown language. */
            LANGUAGE_UNKNOWN
        };

        /**
         * The disk for which the release is intended.
         */
        enum Disk{
            /** The release is for the install disk (PC only). */
            DISK_INSTALL,
            /** The release is for disk 1. */
            DISK_1,
            /** The release is for disk 2. */
            DISK_2,
            /** The release is for disk 3. */
            DISK_3,
            /** The release is for disk 4. */
            DISK_4,
            /** The release is for an unknown disk. */
            DISK_UNKNOWN
        };

        /**
         * Constructs a new release.
         */
        Release();

        /**
         * Constructs a new release from an ISO path.
         *
         * @param iso_path The path to the ISO file.
         */
        Release(std::string iso_path);

        /**
         * Destroys the release.
         */
        ~Release();

        std::string getIsoPath() const { return iso_path; }

        /**
         * Gets the human readable ID of the release.
         *
         * @return The ID of the release.
         */
        std::string const getId();

        /**
         * Gets the platform for which the release is intended.
         *
         * @return The platform of the release.
         */
        Platform const getPlatform();

        /**
         * Gets the region for which the release is intended.
         *
         * @return The region of the release.
         */
        Region const getRegion();

        /**
         * Gets the language for which the release is intended.
         *
         * @return The language of the release.
         */
        Language const getLanguage();

        /**
         * Gets the disk for which the release is intended.
         *
         * @return The disk of the release.
         */
        Disk const getDisk();

        /**
         * Checks if the release is a valid Final Fantasy VII release.
         *
         * @return True if the release is valid, false otherwise.
         */
        bool const isValid();

        /**
         * Gets the error message for the release.
         *
         * @return The error message.
         */
        std::string const getErrorMessage();

        /**
         * Checks if the release is supported and tested in V-Gears.
         *
         * @return True if the release is supported, false otherwise.
         */
        bool const isSupported();

        /**
         * Gets a warning in case the release is not fully supported.
         *
         * @return The warning message.
         */
        std::string const getWarningMessage();

        /**
         * Gets the path where the content has been extracted.
         * 
         * @return The path to the extracted content, null if the content has
         * not been extracted yet.
         */
        std::string const getContentPath();

        /**
         * Extracts the ISO file to the specified output path. The files will
         * be extracted to the "original_data" subdirectory of the output path.
         *
         * @param installation_path The v-gears installation directory.
         * @return True if the extraction was successful, false otherwise.
         */
        bool extractIso(std::string installation_path);

    private:

        /**
         * The path to the ISO file.
         */
        std::string iso_path;

        /**
         * A human readable ID of the release.
         */
        std::string id;

        /**
         * The platform for which the release is intended.
         */
        Platform platform;

        /**
         * The region for which the release is intended.
         */
        Region region;

        /**
         * The language for which the release is intended.
         */
        Language language;

        /**
         * The disk for which the release is intended.
         */
        Disk disk;

        /**
         * Whether the release is valid.
         */
        bool valid;

        /**
         * Whether the release is supported.
         */
        bool supported;

        /**
         * The error message for the release. Empty if no error.
         */
        std::string error_message;

        /**
         * The warning message for the release. Empty if no warning.
         */
        std::string warning_message;

        /**
         * The path where the content has been extracted.
         */
        std::string content_path;
};