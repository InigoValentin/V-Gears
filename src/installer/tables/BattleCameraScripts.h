/*
 * Copyright (C) 2026 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <map>
#include <ostream>
#include <string>
#include <vector>
#include "ExeEffectCameras.h"

/**
 * Reads the camera scripts of the battles (camdat0.bin, camdat1.bin and camdat2.bin of data/battle).
 *
 * The files have four pointers (the PSX memory address 0x801a0000 plus the offset in the file). The
 * first points to a list of scripts for the position of the camera, indexed by (camera id * 3 +
 * variant), and the second to the same for the point it looks at. The camera ids are the ones of
 * the attacks (camera_single and camera_multiple) and of the enemy attacks. Several entries share
 * the same script.
 *
 * Scripts are lists of bytes: an opcode (0xd5 to 0xff, other values do nothing) followed by its
 * operands (bytes and 16 bit values). The lengths of the operands for each opcode of the two kinds
 * of scripts were found in the interpreter of ff7.exe (0x5c2341 for the position, 0x5c3f45 for the
 * point it looks at). A script ends with 0xff, or with 0xfe followed by 0xc0 (loop).
 *
 * The third pointer is the list of the scripts of the camera of the victory (the camera id -3 in the
 * game): three for the position, one for each variant, and then three for the point it looks at.
 */
namespace BattleCameraScripts{

    /**
     * A file: for each kind of script, the list of the entries (the key of the script it uses), and
     * the scripts by the offset in the file.
     */
    struct Set{
        std::vector<uint32_t> position;
        std::vector<uint32_t> look;
        std::vector<uint32_t> victory_position;
        std::vector<uint32_t> victory_look;
        std::map<uint32_t, std::vector<uint8_t>> scripts;
    };

    /**
     * Operands of the opcodes of the position scripts (-1 if it isn't an opcode).
     */
    inline int PositionLength(const uint8_t op){
        switch (op){
            case 0xd5: return 2;
            case 0xd6: return 0;
            case 0xd7: return 2;
            case 0xd8: return 9;
            case 0xd9: case 0xda: case 0xdb: case 0xdc: return 0;
            case 0xdd: case 0xde: return 1;
            case 0xdf: return 0;
            case 0xe0: return 2;
            case 0xe1: return 0;
            case 0xe2: return 1;
            case 0xe3: return 9;
            case 0xe4: case 0xe5: return 8;
            case 0xe6: return 7;
            case 0xe7: case 0xe9: return 8;
            case 0xeb: return 9;
            case 0xef: return 8;
            case 0xf0: return 7;
            case 0xf1: return 0;
            case 0xf2: case 0xf3: return 5;
            case 0xf4: return 0;
            case 0xf5: return 1;
            case 0xf7: return 7;
            case 0xf8: return 12;
            case 0xf9: return 6;
            case 0xfe: case 0xff: return 0;
            default: return -1;
        }
    }

    /**
     * Operands of the opcodes of the look at scripts (-1 if it isn't an opcode).
     */
    inline int LookLength(const uint8_t op){
        switch (op){
            case 0xd8: return 9;
            case 0xd9: case 0xdb: case 0xdc: return 0;
            case 0xdd: case 0xde: return 1;
            case 0xdf: return 0;
            case 0xe0: return 2;
            case 0xe1: return 0;
            case 0xe2: return 1;
            case 0xe3: return 9;
            case 0xe4: case 0xe5: return 8;
            case 0xe6: return 7;
            case 0xe8: case 0xea: return 8;
            case 0xec: return 9;
            case 0xf0: return 8;
            case 0xf4: return 0;
            case 0xf5: return 1;
            case 0xf8: case 0xf9: return 7;
            case 0xfa: return 6;
            case 0xfe: case 0xff: return 0;
            default: return -1;
        }
    }

    inline uint32_t ReadU32(const std::vector<uint8_t>& d, const size_t o){
        uint32_t v = 0;
        if (o + 4 <= d.size()) std::memcpy(&v, &d[o], 4);
        return v;
    }

    /**
     * Reads the script that starts at an offset.
     *
     * @return False if it doesn't end before the end of the file.
     */
    inline bool ReadScript(
      const std::vector<uint8_t>& d, const uint32_t start, const bool position, std::vector<uint8_t>& out
    ){
        out.clear();
        size_t o = start;
        while (o < d.size()){
            const uint8_t op = d[o];
            const int n = position ? PositionLength(op) : LookLength(op);
            if (n < 0){
                // Bytes that aren't opcodes do nothing.
                out.push_back(op);
                ++ o;
                continue;
            }
            if (o + 1 + n > d.size()) return false;
            for (int i = 0; i <= n; ++ i) out.push_back(d[o + i]);
            o += 1 + n;
            if (op == 0xff) return true;
            if (op == 0xfe && o < d.size() && d[o] == 0xc0){
                out.push_back(0xc0);
                return true;
            }
        }
        return false;
    }

    /**
     * Reads a file.
     *
     * @return False if it isn't a valid file.
     */
    inline bool ReadFile(const std::string& path, Set& set){
        set = Set();
        std::ifstream file(path, std::ios::binary);
        if (!file) return false;
        const std::vector<uint8_t> d((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        if (d.size() < 0x20) return false;
        const uint32_t base = 0x801a0000;
        const uint32_t first = ReadU32(d, 0) - base;
        const uint32_t second = ReadU32(d, 4) - base;
        if (first >= d.size() || second >= d.size() || second <= first) return false;
        const size_t count = (second - first) / 4;
        for (int kind = 0; kind < 2; ++ kind){
            const bool position = kind == 0;
            const uint32_t table = position ? first : second;
            std::vector<uint32_t>& list = position ? set.position : set.look;
            for (size_t i = 0; i < count; ++ i){
                const uint32_t offset = ReadU32(d, table + 4 * i) - base;
                if (offset >= d.size()) return false;
                list.push_back(offset);
                // The key has the kind, because the opcodes are different.
                std::vector<uint8_t> script;
                if (!ReadScript(d, offset, position, script)) return false;
                const uint32_t key = offset * 2 + (position ? 0 : 1);
                set.scripts[key] = script;
                list.back() = key;
            }
        }
        const uint32_t victory = ReadU32(d, 8) - base;
        if (victory >= d.size()) return false;
        for (int i = 0; i < 6; ++ i){
            const bool position = i < 3;
            const uint32_t offset = ReadU32(d, victory + 4 * i) - base;
            std::vector<uint8_t> script;
            if (offset >= d.size() || !ReadScript(d, offset, position, script)) return false;
            const uint32_t key = offset * 2 + (position ? 0 : 1);
            set.scripts[key] = script;
            (position ? set.victory_position : set.victory_look).push_back(key);
        }
        return true;
    }

    /**
     * Reads the scripts of the cameras of the start of the battles (the camera id -2 in the game) from
     * ff7.exe. There are two lists of pointers, one for the position and one for the point it looks
     * at, indexed by the camera byte of the formation (read at 0x5c2406 and 0x5c400a in the 1998
     * build). An entry that isn't a script has the key 0.
     *
     * @return False if the file couldn't be read or the scripts aren't there.
     */
    inline bool ReadIntro(const std::string& path, Set& set){
        set = Set();
        ExeEffectCameras::Image image;
        if (!ExeEffectCameras::LoadImage(path, image)) return false;
        const uint32_t tables[2] = {0x8f70d0, 0x8f7270};
        const size_t count = (tables[1] - tables[0]) / 4;
        size_t valid = 0;
        for (int kind = 0; kind < 2; ++ kind){
            const bool position = kind == 0;
            std::vector<uint32_t>& list = position ? set.position : set.look;
            const int64_t table = ExeEffectCameras::FileOffset(image.sections, tables[kind]);
            if (table < 0) return false;
            for (size_t i = 0; i < count; ++ i){
                list.push_back(0);
                const uint32_t address = ReadU32(image.data, table + 4 * i);
                const int64_t offset = ExeEffectCameras::FileOffset(image.sections, address);
                std::vector<uint8_t> script;
                // They start with an opcode, and none is long.
                if (offset < 0 || image.data[offset] < 0xd5
                  || !ReadScript(image.data, static_cast<uint32_t>(offset), position, script)
                  || script.size() > 512) continue;
                const uint32_t key = static_cast<uint32_t>(offset) * 2 + (position ? 0 : 1);
                set.scripts[key] = script;
                list.back() = key;
                ++ valid;
            }
        }
        // Another version of the executable has other things there.
        return valid >= count;
    }

    /**
     * Writes the cameras of the start of the battles, after {@see WriteLua}. Game.BattleCameras.intro
     * has the lists "position" and "look" (index = camera of the formation + 1) and the scripts.
     */
    inline void WriteIntroLua(std::ostream& out, const Set& set){
        const std::string name = "Game.BattleCameras.intro";
        out << "-- Cameras of the start of the battles, read from ff7.exe.\n"
            << name << " = {position = {}, look = {}, scripts = {}}\n";
        for (int kind = 0; kind < 2; ++ kind){
            const std::vector<uint32_t>& list = kind == 0 ? set.position : set.look;
            out << name << "." << (kind == 0 ? "position" : "look") << " = {";
            for (size_t i = 0; i < list.size(); ++ i){
                if (i % 16 == 0) out << "\n    ";
                out << list[i] << ", ";
            }
            out << "\n}\n";
        }
        for (const auto& script : set.scripts){
            out << name << ".scripts[" << script.first << "] = {";
            for (size_t i = 0; i < script.second.size(); ++ i){
                if (i % 24 == 0) out << "\n    ";
                out << static_cast<int>(script.second[i]) << ", ";
            }
            out << "\n}\n";
        }
    }

    /**
     * Writes the sets to a Lua file. Game.BattleCameras[file] has the list "position" and "look"
     * (index = camera id * 3 + variant + 1, the value is the name of a script), and the table
     * "scripts" with the bytes of each one.
     */
    inline void WriteLua(std::ostream& out, const std::map<int, Set>& sets){
        out << "-- Camera scripts of the battles, read from camdat0.bin, camdat1.bin and camdat2.bin.\n"
            << "-- Game.BattleCameras[file].position[camera id * 3 + variant + 1] is the name of a script in\n"
            << "-- Game.BattleCameras[file].scripts (the bytes of the script), the same for .look (the point\n"
            << "-- the camera looks at). .victory_position[variant + 1] and .victory_look are the ones of the\n"
            << "-- camera of the victory. See engine/scripts/battle/battle_camera.lua for the format.\n"
            << "Game.BattleCameras = Game.BattleCameras or {}\n";
        for (const auto& item : sets){
            const Set& set = item.second;
            const std::string name = "Game.BattleCameras[" + std::to_string(item.first) + "]";
            out << name << " = {position = {}, look = {}, scripts = {}}\n";
            for (int kind = 0; kind < 2; ++ kind){
                const std::vector<uint32_t>& list = kind == 0 ? set.position : set.look;
                out << name << "." << (kind == 0 ? "position" : "look") << " = {";
                for (size_t i = 0; i < list.size(); ++ i){
                    if (i % 16 == 0) out << "\n    ";
                    out << list[i] << ", ";
                }
                out << "\n}\n";
            }
            for (int kind = 0; kind < 2; ++ kind){
                out << name << "." << (kind == 0 ? "victory_position" : "victory_look") << " = {";
                for (const uint32_t key : kind == 0 ? set.victory_position : set.victory_look) out << key << ", ";
                out << "}\n";
            }
            for (const auto& script : set.scripts){
                out << name << ".scripts[" << script.first << "] = {";
                for (size_t i = 0; i < script.second.size(); ++ i){
                    if (i % 24 == 0) out << "\n    ";
                    out << static_cast<int>(script.second[i]) << ", ";
                }
                out << "\n}\n";
            }
        }
    }

}
