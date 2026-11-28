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

#include <fstream>
#include "installer/extractors/KernelDataInstaller.h"
#include "installer/extractors/KernelDataParts.h"
#include "installer/extractors/KernelTextList.h"

using namespace KernelData;

int KernelDataInstaller::ReadMateria(){

    // Empty the item list
    materia_.clear();

    File materia_file = kernel_.ExtractGZip(KERNEL_MATERIA_DATA);
    KernelTextList names(kernel_.ExtractGZip(KERNEL_MATERIA_NAMES));
    KernelTextList descriptions(kernel_.ExtractGZip(KERNEL_MATERIA_DESCRIPTIONS));
    int materia_count = 0;

    for (int i = 0; i < 96; i ++){ // There are exactly 96.

        MateriaData data;

        data.name = names.NextName();
        data.description = descriptions.NextDescription();

        // Read data from the materia data section.
        for (int i = 0; i < 4; i ++) data.level_up_ap[i] = materia_file.readU16LE();
        data.stat_raw = materia_file.readU8();
        data.status_raw // WARNING: Status has only three bytes.
          = materia_file.readU8() + (materia_file.readU8() << 8) + (materia_file.readU8() << 16);
        data.element = materia_file.readU8();
        data.type_raw = materia_file.readU8();
        for (int i = 0; i < 6; i ++) data.attribute[i] = materia_file.readU8();

        // If the materia doesn't have a name, don't bother processing the rest of the data,
        // it's not going to be added to the list.
        if ("" == data.name) continue;

        // Derive data from the read data.
        data.id = i;
        // Status. 3 bytes, so only the first 24 can be present.
        ReadStatuses(data.status_raw, data.status);
        SetMateriaType(data);
        SetMateriaStats(data);

        // Add to the list of materia.
        materia_.push_back(data);
        materia_count ++;
    }

    return materia_count;
}

void KernelDataInstaller::SetMateriaType(MateriaData& data){
    // Limit type to materia color.
    data.type = data.type_raw & 15; // Lower nybble.
    data.sub_type = data.type_raw & 15; // Upper nyble.
    switch (data.type){
        case 0x09:
        case 0x0A:
            data.type = static_cast<int>(MATERIA_TYPE::MAGIC);
            break;
        case 0x05:
            data.type = static_cast<int>(MATERIA_TYPE::SUPPORT);
            break;
        case 0x02:
        case 0x03:
        case 0x06:
        case 0x07:
        case 0x08:
            data.type = static_cast<int>(MATERIA_TYPE::COMMAND);
            break;
        case 0x0B:
        case 0x0C:
            data.type = static_cast<int>(MATERIA_TYPE::SUMMON);
            break;
        default:
            data.type = static_cast<int>(MATERIA_TYPE::INDEPENDENT);
    }
}

void KernelDataInstaller::SetMateriaStats(MateriaData& data){
    // The stats of each value of the raw byte, from 0x01 on: str, vit, mag, spr, dex, lck, hp and
    // mp. Other values change nothing.
    static const int STATS[16][8] = {
      { -2,  -1,   2,   1,   0,   0,  -5,   5}, // 0x01
      { -4,  -2,   4,   2,   0,   0, -10,  10}, // 0x02
      {  0,   0,   0,   0,   2,  -2,   0,   0}, // 0x03
      { -1,  -1,   1,   1,   0,   0,   0,   0}, // 0x04
      {  1,   1,  -1,  -1,   0,   0,   0,   0}, // 0x05
      {  0,   1,   0,   0,   0,   0,   0,   0}, // 0x06
      {  0,   0,   0,   0,   0,   1,   0,   0}, // 0x07
      {  0,   0,   0,   0,   0,  -1,   0,   0}, // 0x08
      {  0,   0,   0,   0,  -2,   0,   0,   0}, // 0x09
      {  0,   0,   0,   0,   2,   0,   0,   0}, // 0x0A
      { -1,   0,   1,   0,   0,   0,  -2,   2}, // 0x0B
      {  0,   0,   1,   0,   0,   0,  -2,   2}, // 0x0C
      {  0,   0,   1,   1,   0,   0,  -5,   5}, // 0x0D
      {  0,   0,   2,   2,   0,   0, -10,  10}, // 0x0E
      {  0,   0,   4,   4,   0,   0, -10,  15}, // 0x0F
      {  0,   0,   8,   8,   0,   0, -10,  20}  // 0x10
    };
    const bool changes = data.stat_raw >= 1 && data.stat_raw <= 16;
    static const int NONE_CHANGED[8] = {};
    const int* values = changes ? STATS[data.stat_raw - 1] : NONE_CHANGED;
    data.stats.change = changes;
    data.stats.str = values[0];
    data.stats.vit = values[1];
    data.stats.mag = values[2];
    data.stats.spr = values[3];
    data.stats.dex = values[4];
    data.stats.lck = values[5];
    data.stats.hp = values[6];
    data.stats.mp = values[7];
}

void KernelDataInstaller::WriteMateria(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (MateriaData materia : materia_){
        file << "Game.Materia[" << materia.id << "] = {\n"
          << "    price = " << prices_[materia.id + 320] << ",\n"
          << "    name = \"" << materia.name << "\",\n"
          << "    description = \"" << materia.description << "\",\n"
          << "    type = " << materia.type << ",\n"


        // TODO Remove from here...
          << "    sub_type = " << (int)materia.sub_type << ",\n"
          << "    type_raw = " << (int)materia.type_raw << ",\n"
          << "    type_raw_upp = " << (int)(materia.type_raw >> 4) << ",\n"
          << "    type_raw_low = " << (int)(materia.type_raw & 15) << ",\n"
          << "    attributtes = {";
        for (int i = 0; i < 6; i ++)
            file << (int) materia.attribute[i] << ", ";
        file << "},\n"
        // TODO ... until here


          << "    levels_ap = {[1] = 0, ";
        for (int i = 0; i < 4; i ++){
            if (materia.level_up_ap[i] < 65535)
                file << "[" << (i + 2) << "] = " << (materia.level_up_ap[i] * 100) << ", ";
        }
        file
          << "},\n"
          << "    status = {";
        for (int s : materia.status) file << s << ", ";
        file
          << "},\n"
          << "    elements = {";
        if (materia.element != 0xFF) file << static_cast<int>(materia.element);
        file << "},\n    stats = {";
        if (materia.stats.str != 0) file << "\n        str = " << materia.stats.str << ",";
        if (materia.stats.vit != 0) file << "\n        vit = " << materia.stats.vit << ",";
        if (materia.stats.mag != 0) file << "\n        mag = " << materia.stats.mag << ",";
        if (materia.stats.spr != 0) file << "\n        spr = " << materia.stats.spr << ",";
        if (materia.stats.dex != 0) file << "\n        dex = " << materia.stats.dex << ",";
        if (materia.stats.lck != 0) file << "\n        lck = " << materia.stats.lck << ",";
        if (materia.stats.hp != 0) file << "\n        hp = " << materia.stats.hp << ",";
        if (materia.stats.mp != 0) file << "\n        mp = " << materia.stats.mp << ",";
        if (materia.stats.change) file << "\n    },\n";
        else file << "},\n";

        // Type-dependant
        switch (static_cast<MATERIA_TYPE>(materia.type)){
            case MATERIA_TYPE::MAGIC: WriteMagicMateria(file, materia); break;
            case MATERIA_TYPE::SUMMON: WriteSummonMateria(file, materia); break;
            case MATERIA_TYPE::COMMAND: WriteCommandMateria(file, materia); break;
            case MATERIA_TYPE::SUPPORT: WriteSupportMateria(file, materia); break;
            case MATERIA_TYPE::INDEPENDENT: WriteIndependentMateria(file, materia); break;
        }
        file << "}\n";
    }
    file.close();
}

void KernelDataInstaller::WriteMagicMateria(std::ostream& file, const MateriaData& materia){
    file << "    magic = {\n";
    for (int a = 0; a < 6; a ++){
        if (materia.attribute[a] != 0xFF){
            file << "        [" << a + 1 << "] = " << static_cast<int>(materia.attribute[a])
              << ", -- " << attack_names_[static_cast<int>(materia.attribute[a])] << "\n";
        }
    }
    file << "    }\n";
}

void KernelDataInstaller::WriteSummonMateria(std::ostream& file, const MateriaData& materia){
    file << "    summon = " << static_cast<int>(materia.attribute[0]) << ", -- "
      << attack_names_[static_cast<int>(materia.attribute[0])] << "\n";
    file << "    times = {";
    for (int a = 1; a < 6; a ++)
        file << "[" << a << "] = " << static_cast<int>(materia.attribute[a]) << ", ";
    file << "}\n";
}

void KernelDataInstaller::WriteCommandMateria(std::ostream& file, const MateriaData& materia){
    switch (materia.sub_type){
        case 0x02: // Command, replace attack.
            WriteMateriaCommandList(file, materia);
            file << "    },\n    command_replace = 1 -- Attack\n";
            break;
        case 0x03: // Command, W commands.
            WriteMateriaCommandList(file, materia);
            file << "    },\n    command_replace = "
              << static_cast<int>(materia.attribute[0] - 19) << " -- "
              << command_names_[static_cast<int>(materia.attribute[0] - 19)] << "\n";
            break;
        case 0x06: // Normal command.
        case 0x07: // E.Skill.
            WriteMateriaCommandList(file, materia);
            file << "    }\n";
            break;
        case 0x08: // Master command.
            // TODO
            break;
    }
}

void KernelDataInstaller::WriteMateriaCommandList(
  std::ostream& file, const MateriaData& materia
){
    file << "    command = {\n";
    for (int a = 0; a < 6; a ++){
        if (materia.attribute[a] != 0xFF){
            file << "        [" << a + 1 << "] = " << static_cast<int>(materia.attribute[a])
              << ", -- " << command_names_[static_cast<int>(materia.attribute[a])] << "\n";
        }
    }
}

void KernelDataInstaller::WriteSupportMateria(std::ostream& file, const MateriaData& materia){
    file << "    link = {\n        id = ";
    switch (materia.attribute[0]){
        case 0x51: file << "Materia.LINK.ALL,\n"; break;
        case 0x54: file << "Materia.LINK.COMMAND_COUNTER,\n"; break;
        case 0x55: file << "Materia.LINK.MAGIC_COUNTER,\n"; break;
        case 0x56: file << "Materia.LINK.SNEAK_ATTACK,\n"; break;
        case 0x57: file << "Materia.LINK.FINAL_ATTACK,\n"; break;
        case 0x58: file << "Materia.LINK.MP_TURBO,\n"; break;
        case 0x59: file << "Materia.LINK.MP_ABSORB,\n"; break;
        case 0x5A: file << "Materia.LINK.HP_ABSORB,\n"; break;
        case 0x5C: file << "Materia.LINK.ADDED_CUT,\n"; break;
        case 0x5D: file << "Materia.LINK.STEAL_AS_WELL,\n"; break;
        case 0x5E: file << "Materia.LINK.ELEMENTAL,\n"; break;
        case 0x5F: file << "Materia.LINK.ADDED_EFFECT,\n"; break;
        case 0x63: file << "Materia.LINK.QUADRA_MAGIC,\n"; break;
        default: file << "nil,\n"; break;
    }
    file << "        param = {";
    for (int a = 1; a < 6; a ++){
        if (materia.attribute[a] != 0xFF){
            file << "[" << a << "] = " << static_cast<int>(materia.attribute[a]) << ", ";
        }
    }
    file << "}\n    }\n";
}

void KernelDataInstaller::WriteIndependentMateria(std::ostream& file, const MateriaData& materia){
    file << "    ability = {\n        id = ";
    switch (materia.sub_type){
        case 0x00: // Character effects
            switch (materia.attribute[0]){
                case 0x02: file << "Materia.ABILITY.MAG_PLUS,\n"; break;
                case 0x04: file << "Materia.ABILITY.DEX_PLUS,\n"; break;
                case 0x05: file << "Materia.ABILITY.LCK_PLUS,\n"; break;
                case 0x08: file << "Materia.ABILITY.HP_PLUS,\n"; break;
                case 0x09: file << "Materia.ABILITY.MP_PLUS,\n"; break;
                case 0x0A: file << "Materia.ABILITY.EXP_PLUS,\n"; break;
                case 0x0B: file << "Materia.ABILITY.COVER,\n"; break;
                case 0x0C: file << "Materia.ABILITY.UNDERWATER,\n"; break;
                case 0x50: file << "Materia.ABILITY.LONG_RANGE,\n"; break;
                case 0x53: file << "Materia.ABILITY.COUNTER_ATTACK,\n"; break;
                case 0x62: file << "Materia.ABILITY.HP_MP,\n"; break;
                default: file << "nil,\n"; break;
            }
            break;
        case 0x01: // Party effects
            switch (materia.attribute[0]){
                case 0x00: file << "Materia.ABILITY.MONEY_PLUS,\n"; break;
                case 0x01: // Enemy Away / Enemy lure / Chocobo lure
                    if (materia.id == 7) file << "Materia.ABILITY.ENCOUNTER_DOWN,\n";
                    else if (materia.id == 8) file << "Materia.ABILITY.ENCOUNTER_UP,\n";
                    else if (materia.id == 9) file << "Materia.ABILITY.ENCOUNTER_CHOCOBO,\n";
                    else file << "nil,\n";
                    break;
                case 0x03: file << "Materia.ABILITY.PREEMPTIVE,\n"; break;
                default: file << "nil,\n"; break;
            }
            break;
        case 0x04:
            file << "Materia.ABILITY.MEGA_ALL,\n";
            break;
        default: file << "nil,\n"; break;
    }
    file << "        param = {";
    for (int a = 1; a < 6; a ++){
        if (materia.attribute[a] != 0xFF){
            file << "[" << a << "] = " << static_cast<int>(materia.attribute[a]) << ", ";
        }
    }
    file << "}\n    }\n";
}
