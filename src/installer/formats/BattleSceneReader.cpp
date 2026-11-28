/*
 * V-Gears
 * Copyright (C) 2022 V-Gears Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BattleSceneReader.h"

namespace{

    /**
     * Offset of the AI scripts in a scene.
     */
    const u32 AI_OFFSET = 0x0E80;

    /**
     * Reads a coordinate of an enemy.
     *
     * It's stored as a 16 bit value in units 255 times bigger. Values over 0x7F9B are negative.
     */
    int ReadCoordinate(File& file){
        int value = file.readU16LE();
        if (value > 0x7F9B) value -= 0xFFFF;
        return value / 0xFF;
    }

    /**
     * Reads a little endian 16 bit value.
     */
    u16 ReadU16(const std::vector<u8>& bytes, const u32 offset){
        return static_cast<u16>(bytes[offset] | (bytes[offset + 1] << 8));
    }

    void ReadSetups(File& file, BattleSceneData& data){
        for (int s = 0; s < 4; s ++){
            data.setup[s].location = file.readU16LE();
            data.setup[s].next_battle = file.readU16LE();
            data.setup[s].escape_counter = file.readU16LE();
            data.setup[s].unused = file.readU16LE();
            for (int a = 0; a < 4; a ++) data.setup[s].next_arena_candidates[a] = file.readU16LE();
            data.setup[s].flags = file.readU16LE();
            data.setup[s].layout = file.readU8();
            data.setup[s].camera = file.readU8();
        }
    }

    void ReadCameras(File& file, BattleSceneData& data){
        for (int c = 0; c < 4; c ++){
            for (int p = 0; p < 3; p ++){
                // The position of the camera and the point it looks at, signed 16 bit values in the units of
                // the original game. They are kept as they are, and converted when the formations are built.
                data.camera[c].camera[p].x = static_cast<int16_t>(file.readU16LE());
                data.camera[c].camera[p].y = static_cast<int16_t>(file.readU16LE());
                data.camera[c].camera[p].z = static_cast<int16_t>(file.readU16LE());
                data.camera[c].camera[p].d_x = static_cast<int16_t>(file.readU16LE());
                data.camera[c].camera[p].d_y = static_cast<int16_t>(file.readU16LE());
                data.camera[c].camera[p].d_z = static_cast<int16_t>(file.readU16LE());
            }
            for (int p = 0; p < 12; p ++) data.camera[c].unused[p] = file.readU8();
        }
    }

    void ReadFormations(File& file, BattleSceneData& data){
        for (int f = 0; f < 4; f ++){
            for (int e = 0; e < 6; e ++){
                data.formation[f][e].id = file.readU16LE();
                data.formation[f][e].x = ReadCoordinate(file);
                // Y and Z are inverted in original data.
                data.formation[f][e].z = ReadCoordinate(file);
                data.formation[f][e].y = ReadCoordinate(file);
                data.formation[f][e].row = file.readU16LE();
                data.formation[f][e].cover_flags = file.readU16LE();
                data.formation[f][e].flags = file.readU32LE();
            }
        }
    }

    void ReadEnemies(File& file, BattleSceneData& data){
        for (int e = 0; e < 3; e ++){
            for (int n = 0; n < 32; n ++) data.enemy_data[e].name[n] = file.readU8();
            data.enemy_data[e].level = file.readU8();
            data.enemy_data[e].spd = file.readU8();
            data.enemy_data[e].lck = file.readU8();
            data.enemy_data[e].eva = file.readU8();
            data.enemy_data[e].str = file.readU8();
            data.enemy_data[e].def = file.readU8();
            data.enemy_data[e].mag = file.readU8();
            data.enemy_data[e].mdef = file.readU8();
            for (int l = 0; l < 8; l ++) data.enemy_data[e].elements[l] = file.readU8();
            for (int l = 0; l < 8; l ++) data.enemy_data[e].element_rates[l] = file.readU8();
            for (int a = 0; a < 16; a ++) data.enemy_data[e].animation_index[a] = file.readU8();
            for (int a = 0; a < 16; a ++) data.enemy_data[e].attacks[a] = file.readU16LE();
            for (int a = 0; a < 16; a ++) data.enemy_data[e].attack_camera[a] = file.readU16LE();
            for (int i = 0; i < 4; i ++) data.enemy_data[e].item_rate[i] = file.readU8();
            for (int i = 0; i < 4; i ++) data.enemy_data[e].item[i] = file.readU16LE();
            for (int m = 0; m < 3; m ++) data.enemy_data[e].maniputale_attacks[m] = file.readU16LE();
            data.enemy_data[e].unknown_0 = file.readU16LE();
            data.enemy_data[e].mp = file.readU16LE();
            data.enemy_data[e].ap = file.readU16LE();
            data.enemy_data[e].morph = file.readU16LE();
            data.enemy_data[e].back_damage = file.readU8();
            data.enemy_data[e].unused = file.readU8();
            data.enemy_data[e].hp = file.readU32LE();
            data.enemy_data[e].exp = file.readU32LE();
            data.enemy_data[e].money = file.readU32LE();
            data.enemy_data[e].immunities = file.readU32LE();
            data.enemy_data[e].unknown_1 = file.readU32LE();
        }
    }

    void ReadAttacks(File& file, BattleSceneData& data){
        for (int a = 0; a < 32; a ++){
            data.attack_data[a].accuracy = file.readU8();
            data.attack_data[a].impact_effect = file.readU8();
            data.attack_data[a].hurt_anim = file.readU8();
            data.attack_data[a].unknown = file.readU8();
            data.attack_data[a].mp = file.readU16LE();
            data.attack_data[a].sound = file.readU16LE();
            data.attack_data[a].camera_single = file.readU16LE();
            data.attack_data[a].camera_multiple = file.readU16LE();
            data.attack_data[a].target = file.readU8();
            data.attack_data[a].effect = file.readU8();
            data.attack_data[a].damage = file.readU8();
            data.attack_data[a].power = file.readU8();
            data.attack_data[a].condition = file.readU8();
            data.attack_data[a].status_change = file.readU8();
            data.attack_data[a].additional_effects = file.readU8();
            data.attack_data[a].additional_effects_mod = file.readU8();
            data.attack_data[a].status = file.readU32LE();
            data.attack_data[a].element = file.readU16LE();
            data.attack_data[a].special = file.readU16LE();
        }
        for (int a = 0; a < 32; a ++) data.attack_ids[a] = file.readU16LE();
        for (int a = 0; a < 32; a ++) for (int b = 0; b < 32; b ++) data.attack_names[a][b] = file.readU8();
    }

    /**
     * Reads the AI scripts of an enemy.
     *
     * An enemy block has 16 offsets (0xFFFF for slots without a script) followed by the scripts,
     * which end with the opcode 0x73.
     *
     * @param[in] bytes The scene.
     * @param[in] start Where the block of the enemy starts.
     * @param[in] end Where it ends.
     * @param[in,out] scripts The scripts of the enemy.
     */
    void ReadEnemyScripts(
      const std::vector<u8>& bytes, const u32 start, const u32 end,
      std::vector<std::vector<u8>>& scripts
    ){
        const u32 size = static_cast<u32>(bytes.size());
        if (start + 32 > size || end > size) return;
        u16 entries[16];
        for (int k = 0; k < 16; k ++) entries[k] = ReadU16(bytes, start + 2 * k);
        for (int k = 0; k < 16; k ++){
            if (entries[k] == 0xFFFF) continue;
            // A script ends where the next one starts, or at the end of the block.
            u32 script_end = end - start;
            for (int n = 0; n < 16; n ++){
                if (entries[n] != 0xFFFF && entries[n] > entries[k] && entries[n] < script_end)
                    script_end = entries[n];
            }
            if (entries[k] >= script_end || start + script_end > size) continue;
            std::vector<u8> script(
              bytes.begin() + start + entries[k], bytes.begin() + start + script_end
            );
            // Padding at the end.
            while (!script.empty() && script.back() == 0xFF) script.pop_back();
            scripts[k] = script;
        }
    }
}

namespace BattleSceneReader{

    BattleSceneData Read(File file){
        // Value initialized: the parts a short file doesn't have are zeroes.
        BattleSceneData data{};
        file.SetOffset(0);
        for (int e = 0; e < 3; e ++) data.enemy[e] = file.readU16LE();
        data.unused = file.readU16LE();
        ReadSetups(file, data);
        ReadCameras(file, data);
        ReadFormations(file, data);
        ReadEnemies(file, data);
        ReadAttacks(file, data);
        const u32 size = file.GetFileSize();
        std::vector<u8> bytes(size);
        if (size > 0) file.GetFileBuffer(bytes.data(), 0, size);
        ReadAiScripts(bytes, data);
        return data;
    }

    void ReadAiScripts(const std::vector<u8>& bytes, BattleSceneData& data){
        for (int e = 0; e < 3; e ++) data.ai_scripts[e].assign(16, std::vector<u8>());
        // The AI block is at a fixed offset, and starts with an offset for each of the 3 enemies
        // of the scene (0xFFFF if the enemy has none).
        const u32 size = static_cast<u32>(bytes.size());
        if (size <= AI_OFFSET + 6) return;
        u16 blocks[3];
        for (int e = 0; e < 3; e ++) blocks[e] = ReadU16(bytes, AI_OFFSET + 2 * e);
        for (int e = 0; e < 3; e ++){
            if (blocks[e] == 0xFFFF) continue;
            // The block ends where the next enemy block starts, or at the end of the file.
            u32 end = size;
            for (int n = e + 1; n < 3; n ++){
                if (blocks[n] != 0xFFFF){
                    end = AI_OFFSET + blocks[n];
                    break;
                }
            }
            ReadEnemyScripts(bytes, AI_OFFSET + blocks[e], end, data.ai_scripts[e]);
        }
    }
}
