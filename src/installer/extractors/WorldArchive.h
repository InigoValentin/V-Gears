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
#include <vector>
#include "common/File.h"
#include "common/TypeDefine.h"
#include "formats/lgp/LGPArchive.h"

/**
 * The archive with the data of the world map (data/wm/world_us.lgp).
 */
class WorldArchive{

    public:

        /**
         * Constructor. Reads the list of files of the archive.
         *
         * @param[in] path Path of the archive.
         */
        explicit WorldArchive(const std::string& path);

        /**
         * Retrieves the entries of the files of the archive.
         *
         * @return The entries, in the order of the archive.
         */
        const VGears::LGPArchive::FileList& Entries() const;

        /**
         * Checks if the contents of a file are in the archive.
         *
         * @param[in] entry Entry of the file.
         * @return False if the entry goes past the end of the archive.
         */
        bool Contains(const VGears::LGPArchive::FileEntry& entry) const;

        /**
         * Opens a file of the archive.
         *
         * @param[in] entry Entry of the file.
         * @return The file.
         */
        File Open(const VGears::LGPArchive::FileEntry& entry);

        /**
         * Reads a file of the archive.
         *
         * @param[in] entry Entry of the file.
         * @return The contents of the file.
         */
        std::vector<u8> Read(const VGears::LGPArchive::FileEntry& entry);

        /**
         * Retrieves the name of a file in lower case.
         *
         * @param[in] entry Entry of the file.
         * @return The name.
         */
        static std::string LowerName(const VGears::LGPArchive::FileEntry& entry);

    private:

        /**
         * The archive, as a file.
         */
        File file_;

        /**
         * The entries of the archive.
         */
        VGears::LGPArchive::FileList entries_;
};
