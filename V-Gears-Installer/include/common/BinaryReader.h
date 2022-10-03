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

#include <fstream> 
#include <sstream>
#include <iterator>

#include "decompiler/DecompilerException.h"

/**
 * Reader for binary files.
 */
class BinaryReader{

    public:

        /**
         * Reads a file, from start to end.
         *
         * @oaram file_name[in] Path and name of the file to read.
         * @return Read bytes.
         * @throws std::runtime_error if the file can't be opened for reading.
         */
        static std::vector<unsigned char> ReadAll(std::string file_name){
            std::ifstream file(file_name.c_str(), std::ios::binary | std::ios::ate);
            if (file.is_open()){
                size_t file_size_in_bytes = size_t(file.tellg());
                std::vector<unsigned char> file_contents(file_size_in_bytes);
                file.seekg(0, std::ios::beg);
                file.read(reinterpret_cast<char*>(file_contents.data()), file_contents.size());
                return file_contents;
            }
            else throw std::runtime_error("Can't open file");
        }

        /**
         * Constructor.
         *
         * @param data[in] Bytes for the reader
         */
        BinaryReader(std::vector<unsigned char>&& data){
            size_ = data.size();
            std::copy(data.begin(), data.end(), std::ostream_iterator<unsigned char>(stream_));
            stream_.seekg(std::ios::beg);
        }

        /**
         * Retrieves the size of the data in the reader.
         *
         * @return The size of the data in bytes.
         */
        size_t GetSize() const{return size_;}

        /**
         * Moves the stream cursor.
         *
         * @param position[in] Position (offset) of the cursor.
         * @throws DecompilerException if the position is invalid.
         */
        void Seek(unsigned int position){
            if (!stream_.seekg(position)) throw DecompilerException();
        }

        /**
         * Retrieves the current stream cursor position.
         *
         * @return position[in] Position (offset) of the cursor.
         */
        unsigned int GetPosition(){return static_cast<unsigned int>(stream_.tellg());}

        /**
         * Reads 32 bits of data as an unsigned integer.
         *
         * It advances the stream cursor.
         *
         * @return Read bits.
         * @throws DecompilerException if there is not enough data.
         */
        unsigned int ReadU32(){return InternalRead<unsigned int>();}

        /**
         * Reads 32 bits of data as a signed integer.
         *
         * It advances the stream cursor.
         *
         * @return Read bits.
         * @throws DecompilerException if there is not enough data.
         */
        signed int ReadS32(){return InternalRead<signed int>();}

        /**
         * Reads 16 bits of data as an unsigned integer.
         *
         * It advances the stream cursor.
         *
         * @return Read bits.
         * @throws DecompilerException if there is not enough data.
         */
        signed short int ReadS16(){return InternalRead<signed short int>();}

        /**
         * Reads 16 bits of data as a signed integer.
         *
         * It advances the stream cursor.
         *
         * @return Read bits.
         * @throws DecompilerException if there is not enough data.
         */
        unsigned short int ReadU16(){return InternalRead<unsigned short int>();}

        /**
         * Reads 8 bits of data as an unsigned integer.
         *
         * It advances the stream cursor.
         *
         * @return Read bits.
         * @throws DecompilerException if there is not enough data.
         */
        unsigned char ReadU8(){return InternalRead<unsigned char>();}

        /**
         * Reads 8 bits of data as a signed integer.
         *
         * It advances the stream cursor.
         *
         * @return Read bits.
         * @throws DecompilerException if there is not enough data.
         */
        signed char ReadS8(){return InternalRead<signed char>();}

    private:

        /**
         * Reads T bits of data as a signed integer.
         *
         * It advances the stream cursor.
         *
         * @return Read bits.
         * @throws DecompilerException if there is not enough data.
         */
        template<class T> T InternalRead(){
            T r = {};
            if (!stream_.read(reinterpret_cast<char*>(&r), sizeof(r)))
                throw DecompilerException();
            return r;
        }

        /**
         * The data stream.
         */
        std::stringstream stream_;

        /**
         * The size of the data, in bytes.
         */
        size_t size_ = 0;
};
