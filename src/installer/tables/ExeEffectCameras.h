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

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

/**
 * Reads the camera scripts of the battle effects from ff7.exe.
 *
 * Each effect that moves the camera has its own copy of a small script interpreter in the code of
 * the game, and a pair of scripts (for one target and for several) in the data. The copies are
 * found by the code of their opcode decoder, the function that registers each one by the push of
 * its address, and the scripts by the calls to that function: the address of the script is the
 * last thing pushed before the call.
 *
 * A script is a list of 16 bit words. The opcode is the low 6 bits of a word, and bits 0x40 and
 * 0x80 are the stream it affects (0x40 the camera, 0x80 the point it looks at). The opcodes with
 * the number of words that follow them are: 0 wait (1), 1 set (3), 2 move (4), 3 orbit (4),
 * 4 accelerated move (4), 5 eased orbit (5), 6 (2), 7 end (0), 8 (1) and 9 wait (1). A word of -1
 * ends the script. Found by reading the code of the interpreter of the Shiva effect (0x58e57d in
 * the 1998 build).
 *
 * Only scripts that are made of valid opcodes until their end are kept, so other versions of the
 * executable don't give garbage, just fewer scripts (or none).
 */
namespace ExeEffectCameras{

    /**
     * Sections of the executable.
     */
    struct Section{
        uint32_t address; ///< Address in memory.
        uint32_t size; ///< Size in memory.
        uint32_t file_offset; ///< Position in the file.
        uint32_t file_size; ///< Size in the file.
        bool code; ///< True if it has code.
    };

    /**
     * Reads a value, little endian.
     */
    template<typename T> inline T Read(const std::vector<uint8_t>& data, const size_t offset){
        T value = 0;
        if (offset + sizeof(T) <= data.size()) std::memcpy(&value, &data[offset], sizeof(T));
        return value;
    }

    /**
     * Gets the position in the file of an address in memory.
     *
     * @return -1 if the address is not in any section.
     */
    inline int64_t FileOffset(const std::vector<Section>& sections, const uint32_t address){
        for (const Section& s : sections){
            if (address >= s.address && address < s.address + s.file_size){
                return static_cast<int64_t>(s.file_offset) + (address - s.address);
            }
        }
        return -1;
    }

    /**
     * Number of words after each opcode.
     *
     * @return -1 for the opcodes that don't exist.
     */
    inline int OpcodeArguments(const int opcode){
        static const int arguments[10] = {1, 3, 4, 4, 4, 5, 2, 0, 1, 1};
        return (opcode >= 0 && opcode < 10) ? arguments[opcode] : -1;
    }

    /**
     * Reads a script.
     *
     * @param[in] data The executable.
     * @param[in] offset Position of the script in the file.
     * @return The words of the script, empty if it isn't a valid one.
     */
    inline std::vector<int16_t> ReadScript(const std::vector<uint8_t>& data, const size_t offset){
        std::vector<int16_t> words;
        size_t position = offset;
        while (words.size() < 1024 && position + 2 <= data.size()){
            const int16_t word = Read<int16_t>(data, position);
            if (word == -1){
                words.push_back(word);
                return words;
            }
            const int opcode = word & 0x3f;
            const int stream = word & 0xc0;
            const int arguments = OpcodeArguments(opcode);
            // Ending, and the opcodes that don't depend on a stream, don't have one.
            const bool no_stream = opcode == 6 || opcode == 7 || opcode == 8;
            if (arguments < 0 || (stream != 0x40 && stream != 0x80 && !no_stream)) return {};
            if (position + 2 * (1 + arguments) > data.size()) return {};
            for (int i = 0; i <= arguments; ++ i) words.push_back(Read<int16_t>(data, position + 2 * i));
            position += 2 * (1 + arguments);
            if (opcode == 7) return words;
        }
        return {};
    }

    /**
     * Finds the start of the function that has an address.
     *
     * @return 0 if there is no function start near.
     */
    inline uint32_t FunctionStart(
      const std::vector<uint8_t>& data, const Section& code, const uint32_t address
    ){
        size_t index = address - code.address;
        for (size_t steps = 0; steps < 0x4000 && index >= 4; ++ steps, -- index){
            const size_t o = code.file_offset + index;
            if (data[o] == 0x55 && data[o + 1] == 0x8b && data[o + 2] == 0xec
              && (data[o - 1] == 0xc3 || data[o - 1] == 0xcc || data[o - 1] == 0x90)){
                return code.address + static_cast<uint32_t>(index);
            }
        }
        return 0;
    }

    /**
     * The executable loaded in memory: its bytes, sections and the first one with code.
     */
    struct Image{
        std::vector<uint8_t> data;
        std::vector<Section> sections;
        Section code = {0, 0, 0, 0, false};
    };

    /**
     * Reads the executable and its sections.
     *
     * @param[in] path Path to ff7.exe.
     * @param[out] image The executable.
     * @return False if the file couldn't be read or it isn't a 32 bit executable.
     */
    inline bool LoadImage(const std::string& path, Image& image){
        image = Image();
        std::ifstream file(path, std::ios::binary);
        if (!file) return false;
        image.data.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        const std::vector<uint8_t>& data = image.data;
        if (data.size() < 0x100 || data[0] != 'M' || data[1] != 'Z') return false;
        const uint32_t pe = Read<uint32_t>(data, 0x3c);
        if (Read<uint32_t>(data, pe) != 0x00004550 || Read<uint16_t>(data, pe + 4) != 0x14c) return false;
        const uint16_t section_count = Read<uint16_t>(data, pe + 6);
        const uint16_t optional_size = Read<uint16_t>(data, pe + 20);
        const uint32_t image_base = Read<uint32_t>(data, pe + 24 + 28);
        for (uint16_t i = 0; i < section_count; ++ i){
            const size_t h = pe + 24 + optional_size + 40 * i;
            Section s;
            s.size = Read<uint32_t>(data, h + 8);
            s.address = image_base + Read<uint32_t>(data, h + 12);
            s.file_size = Read<uint32_t>(data, h + 16);
            s.file_offset = Read<uint32_t>(data, h + 20);
            s.code = (Read<uint32_t>(data, h + 36) & 0x20) != 0;
            if (s.file_offset + s.file_size > data.size()) return false;
            image.sections.push_back(s);
            if (s.code && image.code.file_size == 0) image.code = s;
        }
        return image.code.file_size >= 0x1000;
    }

    /**
     * Extracts the camera scripts of the effects.
     *
     * @param[in] path Path to ff7.exe.
     * @param[out] scripts The scripts, by the address in memory they have in the executable.
     * @return False if the file couldn't be read or it isn't a 32 bit executable.
     */
    inline bool Extract(const std::string& path, std::map<uint32_t, std::vector<int16_t>>& scripts){
        scripts.clear();
        Image image;
        if (!LoadImage(path, image)) return false;
        const std::vector<uint8_t>& data = image.data;
        const std::vector<Section>& sections = image.sections;
        const Section& code = image.code;

        // Signature of the opcode decoder of the interpreter:
        //   movsx ecx, word [ebp-X] ; and cl, 0x3f ; mov edx, [Y] ; mov [edx], cx
        static const uint8_t signature[] = {
          0x0f, 0xbf, 0x8d, 0, 0, 0, 0, 0x80, 0xe1, 0x3f, 0x8b, 0x15, 0, 0, 0, 0, 0x66, 0x89, 0x0a
        };
        static const bool wildcard[] = {
          false, false, false, true, true, true, true, false, false, false, false, false,
          true, true, true, true, false, false, false
        };
        const size_t signature_size = sizeof(signature);

        // All the calls, to find the ones to the functions that register the interpreters.
        struct Call{uint32_t address; uint32_t target;};
        std::vector<Call> calls;
        for (size_t i = 5; i + 5 <= code.file_size; ++ i){
            if (data[code.file_offset + i] != 0xe8) continue;
            const int32_t relative = Read<int32_t>(data, code.file_offset + i + 1);
            const uint32_t address = code.address + static_cast<uint32_t>(i);
            calls.push_back({address, address + 5 + static_cast<uint32_t>(relative)});
        }

        for (size_t i = 0; i + signature_size <= code.file_size; ++ i){
            bool match = true;
            for (size_t k = 0; k < signature_size && match; ++ k){
                if (!wildcard[k] && data[code.file_offset + i + k] != signature[k]) match = false;
            }
            if (!match) continue;
            const uint32_t interpreter = FunctionStart(data, code, code.address + static_cast<uint32_t>(i));
            if (interpreter == 0) continue;
            // The function that registers the interpreter pushes its address.
            for (size_t j = 0; j + 5 <= code.file_size; ++ j){
                if (data[code.file_offset + j] != 0x68 || Read<uint32_t>(data, code.file_offset + j + 1) != interpreter) continue;
                const uint32_t registration = FunctionStart(data, code, code.address + static_cast<uint32_t>(j));
                if (registration == 0) continue;
                // Its calls have the address of the script as the last argument pushed.
                for (const Call& call : calls){
                    if (call.target != registration) continue;
                    const size_t o = code.file_offset + (call.address - code.address);
                    if (data[o - 5] != 0x68) continue;
                    const uint32_t script = Read<uint32_t>(data, o - 4);
                    const int64_t offset = FileOffset(sections, script);
                    if (offset < 0 || scripts.count(script) > 0) continue;
                    const std::vector<int16_t> words = ReadScript(data, static_cast<size_t>(offset));
                    if (!words.empty()) scripts[script] = words;
                }
            }
        }
        return true;
    }

}
