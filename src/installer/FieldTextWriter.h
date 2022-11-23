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

#include <vector>
#include <memory>
#include "common/TypeDefine.h"
#include "common/Logger.h"

/**
 * Handles the
 */
class FieldTextWriter{

    public:

        /**
         * Initializes the text file.
         *
         * Writes the  XML text section opening tag.
         *
         * @param[in] file_name Path to the file to write te text to.
         */
        void Begin(std::string file_name);

        /**
         * Writes text to the XML file.
         *
         * @param[in] script_section_buffer The text data to write.
         * @param[in] field_name The field of the text.
         * @param[in] english True if the language to write is english.
         */
        void Write(
          const std::vector<u8>& script_section_buffer, std::string field_name, bool english = true
        );

        /**
         * Finalizes the text file.
         *
         * Writes the  XML text section closing tag.
         */
        void End();

    private:

        /**
         * Adds text.
         *
         * @param[in] text The text to add.
         */
        void AddText(std::vector<unsigned char>& text);

        /**
         * Reads 16 bytes of data from the data (little endian).
         *
         * @param[in] offset Offset at which to read.
         * @return 16 bytes of data at the specified offset.
         * @throws std::out_of_range If the offset is out of the data.
         */
        u16 GetU16LE(u32 offset);

        /**
         * Reads 8 bytes of data from the data.
         *
         * @param[in] offset Offset at which to read.
         * @return 8 bytes of data at the specified offset.
         * @throws std::out_of_range If the offset is out of the data.
         */
        u8 GetU8(u32 offset);

        /**
         * The logger used to write text.
         */
        std::unique_ptr<Logger> logger_;

        /**
         * The data being read.
         */
        std::vector<u8> data_;

        /**
         * Indicates if the text XML tag has been opened and not closed.
         */
        bool tag_open_ = false;
};
