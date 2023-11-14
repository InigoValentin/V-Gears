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

namespace Lzs{

    /**
     * Decompresses LZS data.
     *
     * @param[in] compressed Compressed data.
     * @return Decompressed data. An empty vector if data is not valid lzs.
     */
    inline std::vector<unsigned char> Decompress(const std::vector<unsigned char>& compressed){
        if (compressed.size() < 4){
            std::cout << "Error decompressing LZS data of size " << compressed.size() << std::endl;
            return std::vector<unsigned char>(0);
        }
        const unsigned int input_buffer_size = static_cast<unsigned int>(compressed.size());
        const unsigned int input_length =
          (((compressed[0] & 0xFF) << 0) |
          ((compressed[1] & 0xFF) << 8) |
          ((compressed[2] & 0xFF) << 16) |
          ((compressed[3] & 0xFF) << 24)) + 4;
        if (input_length != input_buffer_size){
            std::cerr << "Error decompressing LZS data of invalid size " << input_length
              << " | " << input_buffer_size << std::endl;
            return std::vector<unsigned char>(0);
        }
        unsigned int extract_size = (input_buffer_size + 255) & ~255;
        std::vector<unsigned char> extract_buffer(extract_size);
        unsigned int input_offset = 4;
        unsigned int output_offset = 0;
        unsigned char control_byte = 0;
        unsigned char control_bit = 0;
        while (input_offset < input_buffer_size){
            if (control_bit == 0){
                control_byte = compressed[input_offset ++];
                control_bit = 8;
            }
            if (control_byte & 1){
                extract_buffer[output_offset++] = compressed[input_offset ++];
                if (output_offset == extract_size){
                    extract_size += 256;
                    extract_buffer.resize(extract_size);
                }
            }
            else{
                const unsigned char reference1 = compressed[input_offset ++];
                const unsigned char reference2 = compressed[input_offset ++];
                const unsigned short int reference_offset = reference1 | ((reference2 & 0xF0) << 4);
                const unsigned char reference_length = (reference2 & 0xF) + 3;
                int real_offset = output_offset - ((output_offset - 18 - reference_offset) & 0xFFF);
                for (int j = 0; j < reference_length; ++ j){
                    if (real_offset < 0) extract_buffer[output_offset ++] = 0;
                    else extract_buffer[output_offset ++] = extract_buffer[real_offset];
                    if (output_offset == extract_size) {
                        extract_size += 256;
                        extract_buffer.resize(extract_size);
                    }
                    ++ real_offset;
                }
            }
            control_byte >>= 1;
            control_bit --;
        }
        // Truncate to exact size.
        extract_buffer.resize(output_offset);
        return extract_buffer;
    }
}
