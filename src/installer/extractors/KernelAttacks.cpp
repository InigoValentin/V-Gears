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
#include <iostream>
#include <memory>
#include "installer/extractors/KernelDataInstaller.h"
#include "installer/extractors/KernelDataParts.h"
#include "installer/extractors/KernelTextList.h"

using namespace KernelData;

int KernelDataInstaller::ReadCommands(){

    // Empty the command list
    commands_.clear();

    File command_file = kernel_.ExtractGZip(KERNEL_COMMAND_DATA);
    KernelTextList names(kernel_.ExtractGZip(KERNEL_COMMAND_NAMES));
    KernelTextList descriptions(kernel_.ExtractGZip(KERNEL_COMMAND_DESCRIPTIONS));
    int command_count = 0;

    // There are exactly 32 commands. Some of them are blank, so there is not really a
    // reliable exit condition.
    for (int i = 0; i < 32; i ++){

        CommandData data;

        data.name = names.NextName();
        data.description = descriptions.NextDescription();

        // Read data from the command       m, data section.
        data.cursor_action = command_file.readU8();
        data.target_raw = command_file.readU8();
        data.unknown = command_file.readU16LE();
        data.camera_single = command_file.readU16LE();
        data.camera_multiple = command_file.readU16LE();

        // Derive data from the read data.
        data.id = i;
        if (data.target_raw == (data.target_raw | 0x01)) data.target.selection_enabled = true;
        else data.target.selection_enabled = false;
        if (data.target_raw == (data.target_raw | 0x02)) data.target.default_enemy = true;
        else data.target.default_enemy = false;
        // Default multiple is inverted
        if (data.target_raw == (data.target_raw | 0x04)) data.target.default_multiple = false;
        else data.target.default_multiple = true;
        if (data.target_raw == (data.target_raw | 0x08)) data.target.toggle_multiple = true;
        else data.target.toggle_multiple = false;
        if (data.target_raw == (data.target_raw | 0x10)) data.target.fixed_row = true;
        else data.target.fixed_row = false;
        if (data.target_raw == (data.target_raw | 0x20)) data.target.short_range = true;
        else data.target.short_range = false;
        if (data.target_raw == (data.target_raw | 0x40)) data.target.all_rows = true;
        else data.target.all_rows = false;
        if (data.target_raw == (data.target_raw | 0x80)) data.target.random = true;
        else data.target.random = false;
        switch (data.cursor_action){
            case NONE: data.menu = "Battle.Menu.NONE"; break;
            case MAGIC: data.menu = "Battle.Menu.MAGIC"; break;
            case SUMMON: data.menu = "Battle.Menu.SUMMON"; break;
            case ITEM: data.menu = "Battle.Menu.ITEM"; break;
            case E_SKILL: data.menu = "Battle.Menu.E_SKILL"; break;
            case THROW: data.menu = "Battle.Menu.THROW"; break;
            case LIMIT: data.menu = "Battle.Menu.LIMIT"; break;
            case TARGET: data.menu = "Battle.Menu.TARGET"; break;
            case W_MAGIC: data.menu = "Battle.Menu.W_MAGIC"; break;
            case W_SUMMON: data.menu = "Battle.Menu.W_SUMMON"; break;
            case W_ITEM: data.menu = "Battle.Menu.W_ITEM"; break;
            case COIN: data.menu = "Battle.Menu.COIN"; break;
            default: data.menu = "Battle.Menu.NONE";
        }

        // Add to the list of commands.
        if (data.name != ""){
            command_names_[i] = data.name;
            commands_.push_back(data);
            command_count ++;
        }
    }

    return command_count;
}

void KernelDataInstaller::WriteCommands(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (CommandData command : commands_){
        file << "Game.Commands[" << command.id << "] = {\n"
          << "    name = \"" << command.name << "\",\n"
          << "    description = \"" << command.description << "\",\n"
          << "    menu = " << command.menu << ",\n"
          << "    camera = {single = " << command.camera_single << ", "
          << "multiple = " << command.camera_multiple << "}\n}\n";
    }
    file.close();
}

int KernelDataInstaller::ReadAttacks(){

    // Empty the item list
    attacks_.clear();

    File attacks_file = kernel_.ExtractGZip(KERNEL_ATTACK_DATA);
    KernelTextList names(kernel_.ExtractGZip(KERNEL_ATTACK_NAMES));
    KernelTextList descriptions(kernel_.ExtractGZip(KERNEL_ATTACK_DESCRIPTIONS));
    int attack_count = 0;

    limit_attacks_.clear();

    // Limit data, from ff7.exe (file offset of the table at 0x910d94, in the .data section).
    const int LIMIT_TABLE_OFFSET = 0x910d94 - 0x7b0000 + 0x3ae000;
    std::unique_ptr<File> exe_file;
    try{
        exe_file.reset(new File(exe_path_));
    }
    catch (...){
        std::cerr << "[WARNING] Can't read the data of the limits from ff7.exe." << std::endl;
    }

    // The kernel has data for 128 attacks, but names and descriptions for 256. The last 128 are the
    // limit break techniques, which the characters refer to by ID (128 and up).
    for (int i = 0; i < 256; i ++){

        AttackData data;

        data.name = names.NextName();
        data.description = descriptions.NextDescription();

        // Limit techniques only have name and description. Both start with control codes, drop
        // everything before the first letter.
        // The data of the limits that the characters use (128 to 183) is not in the kernel: it is a table of
        // attack records, with the same format, in the data of ff7.exe (0x910d94, indexed by the limit ID minus
        // 128; found from the code that ff7.exe uses to load the limits of a character, 0x5c9524).
        File* source = &attacks_file;
        bool limit_from_exe = false;
        if (i >= 128 && i <= 183 && exe_file != nullptr){
            exe_file->SetOffset(LIMIT_TABLE_OFFSET + (i - 128) * 0x1c);
            source = exe_file.get();
            limit_from_exe = true;
        }
        if (i >= 128){
            for (std::string* text : {&data.name, &data.description}){
                size_t first = text->find_first_of(
                  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
                if (first == std::string::npos) text->clear();
                else text->erase(0, first);
            }
            data.id = i;
            if (!limit_from_exe){
                limit_attacks_.push_back(data);
                continue;
            }
        }

        // Read data from the item data section.
        data.accuracy = source->readU8();
        data.impact_effect = source->readU8();
        data.hurt_anim = source->readU8();
        data.unknown = source->readU8();
        data.mp = source->readU16LE();
        data.sound = source->readU16LE();
        data.camera_single = source->readU16LE();
        data.camera_multiple = source->readU16LE();
        data.target_raw = source->readU8();
        data.effect = source->readU8();
        data.damage_raw = source->readU8();
        data.power = source->readU8();
        data.condition_raw = source->readU8();
        data.status_change_raw = source->readU8();
        data.additional_effects_raw = source->readU8();
        data.additional_effects_mod_raw = source->readU8();
        data.status_raw = source->readU32LE();
        data.element_raw = source->readU16LE();
        data.special_raw = source->readU16LE();

        // Derive data from the read data.
        data.id = i;
        ReadTarget(data.target_raw, data.target);
        data.damage_formula = data.damage_raw >> 4; // Upper nybble.
        data.damage_modifier = data.damage_raw & 15; // Lower nybble.
        data.restore_type = ReadRestoreType(data.condition_raw);
        ReadStatusEffect(data.status_change_raw, data.status_raw, data.status);
        ReadElements(data.element_raw, data.elements);

        // Add to the list of items.
        attacks_.push_back(data);
        attack_names_[i] = data.name;
        attack_count ++;
    }

    return attack_count;
}

void KernelDataInstaller::WriteAttacks(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (AttackData attack : attacks_){
        file << "Game.Attacks[" << attack.id << "] = {\n"
          << "    name = \"" << attack.name << "\",\n"
          << "    description = \"" << attack.description << "\",\n"
          << "    accuracy = " << static_cast<int>(attack.accuracy) << ",\n"
          << "    impact_effect = " << static_cast<int>(attack.impact_effect) << ",\n"
          << "    hurt_anim = " << static_cast<int>(attack.hurt_anim) << ",\n"
          << "    mp = " << attack.mp << ",\n"
          << "    sound = " << attack.sound << ",\n"
          << "    camera = {single = " << attack.camera_single << ", "
          << "multiple = " << attack.camera_multiple << "},\n"
          << "    effect = " << static_cast<int>(attack.effect) << ",\n"
          << "    dmg_formula = " << static_cast<int>(attack.damage_formula) << ",\n"
          << "    dmg_modifier = " << static_cast<int>(attack.damage_modifier) << ",\n"
          << "    power = " << static_cast<int>(attack.power) << ",\n"
          << "    additional = {effect = " << static_cast<int>(attack.additional_effects_raw)
          << ", modifier = " << static_cast<int>(attack.additional_effects_mod_raw) << "},\n"
          << "    restore = " << attack.restore_type << ",\n";
        WriteTarget(file, attack.target);
        file << "    elements = {";
        for (int element : attack.elements) file << element << ", ";
        file << "},\n    status = {";
        // Status effects: ID (bit position), mode (0 inflict, 1 cure, 2 toggle) and chance out of 63.
        for (int status : attack.status.status){
            file << "{status = " << status << ", mode = " << attack.status.mode
              << ", chance = " << attack.status.chance << "}, ";
        }
        file << "}\n}\n";
    }
    // Limit techniques: only name and description are known.
    for (AttackData attack : limit_attacks_){
        file << "Game.Attacks[" << attack.id << "] = {\n"
          << "    name = \"" << attack.name << "\",\n"
          << "    description = \"" << attack.description << "\"\n"
          << "}\n";
    }
    file.close();
}

int KernelDataInstaller::ReadSummonNames(){

    // Empty the summon list
    summon_names_.clear();

    KernelTextList names(kernel_.ExtractGZip(KERNEL_SUMMON_ATTACK_NAMES));
    int summon_count = 0;

    for (int i = 0; i < 16; i ++){ // There are exactly 16.

        SummonNameData data;

        data.name = names.NextName();

        // If the key item doesn't have a name, it means all key items have been read.
        // break the loop.
        if ("" == data.name) break;

        // Add to the list of items.
        data.id = i;
        summon_names_.push_back(data);
        summon_count ++;
    }

    return summon_count;
}

void KernelDataInstaller::WriteSummonNames(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (SummonNameData summon : summon_names_)
        file << "Game.Summon_Names[" << summon.id << "] = {\"" << summon.name << "\"}\n";
    file.close();
}
