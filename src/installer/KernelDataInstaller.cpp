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


#include <tinyxml.h>
#include <fstream>
#include <iostream>
#include "KernelDataInstaller.h"
#include "common/FinalFantasy7/FF7NameLookup.h"
#include <boost/algorithm/string/replace.hpp>

KernelDataInstaller::KernelDataInstaller(std::string path):
  kernel_(path + "data/kernel/KERNEL.BIN"), exe_path_(path + "ff7.exe"){
    for (int i = 0; i < 416; i ++) prices_[i] = 50;
}

KernelDataInstaller::~KernelDataInstaller(){}

void KernelDataInstaller::ReadPrices(){
    File exe_file(exe_path_);
    exe_file.SetOffset(0x00514518); // Items, weapons, armors, accessories start here
    for (int i = 0; i < 320; i ++) prices_[i] = exe_file.readU32LE();
    exe_file.SetOffset(0x00514B18); // Materias start here
    for (int i = 320; i < 416; i ++) prices_[i] = exe_file.readU32LE();
}

int KernelDataInstaller::ReadCommands(){

    // Empty the item list
    commands_.clear();

    File command_file = kernel_.ExtractGZip(KERNEL_COMMAND_DATA);
    File command_name_file = kernel_.ExtractGZip(KERNEL_COMMAND_NAMES);
    File command_desc_file = kernel_.ExtractGZip(KERNEL_COMMAND_DESCRIPTIONS);
    int command_count = 0;

    // There are exactly 32 commands. Some of them are blank, so there is not really a
    // reliable exit condition.
    for (int i = 0; i < 32; i ++){

        CommandData data;
        int name_offset;
        int desc_offset;

        // From the current name file offset, read 16 bytes.
        // This is the pointer to the offset for the item name.
        // Start reading there until 0xFF is found.
        data.name = "";
        u8 ch = 1;
        name_offset = command_name_file.readU16LE();
        while (ch != 0xFF){
            ch = command_name_file.GetU8(name_offset);
            if (ch != 0xFF) data.name += ENGLISH_CHARS[ch];
            name_offset ++;
        }
        boost::replace_all(data.name, "\"", "'");

        // For descriptions, do the same as for names.
        data.description = "";
        ch = 1;
        desc_offset = command_desc_file.readU16LE();
        while (ch != 0xFF){
            ch = command_desc_file.GetU8(desc_offset);
            if (ch != 0xFF){
                // Warning: If the char is 0xF9, special function!
                // the next byte (in bits, aaoooooo) means to read aa data at offset oooooo.
                // This is to compress information in the kernel.
                if (ch == 0xF9){
                    // Get how much to read, two upper bits of the next byte.
                    // Multiply by 2 and add 4. That's just how it is.
                    u8 comp_size = (command_desc_file.GetU8(desc_offset + 1) >> 6) * 2 + 4;
                    // Get new offset, lower six bits of the next byte.
                    // The offset goes backwards from the position of the 0xF9 byte.
                    u8 comp_offset = command_desc_file.GetU8(desc_offset + 1) & 63;

                    for (int c = 0; c < comp_size; c ++){
                        data.description += ENGLISH_CHARS[
                          command_desc_file.GetU8(desc_offset - 1 - comp_offset + c)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }
        boost::replace_all(data.description, "\"", "'");

        // Read data from the item data section.
        data.cursor_action = command_file.readU8();
        data.target_raw = command_file.readU8();
        data.camera_single = command_file.readU16LE();
        data.camera_multiple = command_file.readU16LE();

        // Derive data from the read data.
        data.id = i;
        if (data.target_raw == (data.target_raw | 0x01)) data.target.selection_enabled = true;
        else data.target.selection_enabled = false;
        if (data.target_raw == (data.target_raw | 0x02)) data.target.default_enemy = true;
        else data.target.default_enemy = false;
        if (data.target_raw == (data.target_raw | 0x04)) data.target.default_multiple = true;
        else data.target.default_multiple = false;
        if (data.target_raw == (data.target_raw | 0x08)) data.target.toggle_multiple = true;
        else data.target.toggle_multiple = false;
        if (data.target_raw == (data.target_raw | 0x10)) data.target.fixed_row = true;
        else data.target.fixed_row = false;
        if (data.target_raw == (data.target_raw | 0x20)) data.target.short_range = true;
        else data.target.short_range = false;
        if (data.target_raw == (data.target_raw | 0x40)) data.target.default_enemy = true;
        else data.target.all_rows = false;
        if (data.target_raw == (data.target_raw | 0x80)) data.target.random = true;
        else data.target.random = false;

        // Add to the list of items.
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
          << "    camera = {single = " << command.camera_single << ", "
          << "multiple = " << command.camera_multiple << "}\n}\n";
    }
    file.close();
}

int KernelDataInstaller::ReadAttacks(){

    // Empty the item list
    attacks_.clear();

    File attacks_file = kernel_.ExtractGZip(KERNEL_ATTACK_DATA);
    File attacks_name_file = kernel_.ExtractGZip(KERNEL_ATTACK_NAMES);
    File attacks_desc_file = kernel_.ExtractGZip(KERNEL_ATTACK_DESCRIPTIONS);
    int attack_count = 0;

    for (int i = 0; i < 128; i ++){

        AttackData data;
        int name_offset;
        int desc_offset;

        // From the current name file offset, read 16 bytes.
        // This is the pointer to the offset for the item name.
        // Start reading there until 0xFF is found.
        data.name = "";
        u8 ch = 1;
        name_offset = attacks_name_file.readU16LE();
        while (ch != 0xFF){
            ch = attacks_name_file.GetU8(name_offset);
            if (ch != 0xFF) data.name += ENGLISH_CHARS[ch];
            name_offset ++;
        }
        boost::replace_all(data.name, "\"", "'");

        // For descriptions, do the same as for names.
        data.description = "";
        ch = 1;
        desc_offset = attacks_desc_file.readU16LE();
        while (ch != 0xFF){
            ch = attacks_desc_file.GetU8(desc_offset);
            if (ch != 0xFF){
                // Warning: If the char is 0xF9, special function!
                // the next byte (in bits, aaoooooo) means to read aa data at offset oooooo.
                // This is to compress information in the kernel.
                if (ch == 0xF9){
                    // Get how much to read, two upper bits of the next byte.
                    // Multiply by 2 and add 4. That's just how it is.
                    u8 comp_size = (attacks_desc_file.GetU8(desc_offset + 1) >> 6) * 2 + 4;
                    // Get new offset, lower six bits of the next byte.
                    // The offset goes backwards from the position of the 0xF9 byte.
                    u8 comp_offset = attacks_desc_file.GetU8(desc_offset + 1) & 63;

                    for (int c = 0; c < comp_size; c ++){
                        data.description += ENGLISH_CHARS[
                          attacks_desc_file.GetU8(desc_offset - 1 - comp_offset + c)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }
        boost::replace_all(data.description, "\"", "'");

        // Read data from the item data section.
        data.accuracy = attacks_file.readU8();
        data.impact_effect = attacks_file.readU8();
        data.hurt_anim = attacks_file.readU8();
        data.unknown = attacks_file.readU8();
        data.mp = attacks_file.readU16LE();
        data.sound = attacks_file.readU16LE();
        data.camera_single = attacks_file.readU16LE();
        data.camera_multiple = attacks_file.readU16LE();
        data.target_raw = attacks_file.readU8();
        data.effect = attacks_file.readU8();
        data.damage_raw = attacks_file.readU8();
        data.power = attacks_file.readU8();
        data.condition_raw = attacks_file.readU8();
        data.status_change_raw = attacks_file.readU8();
        data.additional_effects_raw = attacks_file.readU8();
        data.additional_effects_mod_raw = attacks_file.readU8();
        data.status_raw = attacks_file.readU32LE();
        data.element_raw = attacks_file.readU16LE();
        data.special_raw = attacks_file.readU16LE();

        // Derive data from the read data.
        data.id = i;
        if (data.target_raw == (data.target_raw | 0x01)) data.target.selection_enabled = true;
        else data.target.selection_enabled = false;
        if (data.target_raw == (data.target_raw | 0x02)) data.target.default_enemy = true;
        else data.target.default_enemy = false;
        if (data.target_raw == (data.target_raw | 0x04)) data.target.default_multiple = true;
        else data.target.default_multiple = false;
        if (data.target_raw == (data.target_raw | 0x08)) data.target.toggle_multiple = true;
        else data.target.toggle_multiple = false;
        if (data.target_raw == (data.target_raw | 0x10)) data.target.fixed_row = true;
        else data.target.fixed_row = false;
        if (data.target_raw == (data.target_raw | 0x20)) data.target.short_range = true;
        else data.target.short_range = false;
        if (data.target_raw == (data.target_raw | 0x40)) data.target.default_enemy = true;
        else data.target.all_rows = false;
        if (data.target_raw == (data.target_raw | 0x80)) data.target.random = true;
        else data.target.random = false;
        data.damage_formula = data.damage_raw >> 4; // Upper nybble.
        data.damage_modifier = data.damage_raw & 15; // Lower nybble.
        data.status.chance = 63 - (data.status_change_raw ^ 63); // Out of 63;
        if (data.status_change_raw == (data.status_change_raw | 0x00000080))
            data.status.mode = TOGGLE;
        else if (data.status_change_raw == (data.status_change_raw | 0x00000040))
            data.status.mode = CURE;
        else data.status.mode = INFLICT;
        switch (data.condition_raw){
            case 0: data.restore_type = RESTORE_HP; break;
            case 1: data.restore_type = RESTORE_MP; break;
            case 2: data.restore_type = RESTORE_STATUS; break;
            default: data.restore_type = RESTORE_NONE;
        }
        // In KERNEL.BIN, items that don't induce statuses actually have all statuses marked, and
        // a chance of 255. If it's the case, skip statuses altogether.
        if (data.status.chance <= 64){
            // TODO: Im fairly sure that this can be done with a single-line loop...
            // ... but I don't know how
            if (data.status_raw == (data.status_raw | 0x00000001))
                data.status.status.push_back(DEATH);
            if (data.status_raw == (data.status_raw | 0x00000002))
                data.status.status.push_back(NEAR_DEATH);
            if (data.status_raw == (data.status_raw | 0x00000004))
                data.status.status.push_back(SLEEP);
            if (data.status_raw == (data.status_raw | 0x00000008))
                data.status.status.push_back(POISONED);
            if (data.status_raw == (data.status_raw | 0x00000010))
                data.status.status.push_back(SADNESS);
            if (data.status_raw == (data.status_raw | 0x00000020))
                data.status.status.push_back(FURY);
            if (data.status_raw == (data.status_raw | 0x00000040))
                data.status.status.push_back(CONFU);
            if (data.status_raw == (data.status_raw | 0x00000080))
                data.status.status.push_back(SILENCE);
            if (data.status_raw == (data.status_raw | 0x00000100))
                data.status.status.push_back(HASTE);
            if (data.status_raw == (data.status_raw | 0x00000200))
                data.status.status.push_back(SLOW);
            if (data.status_raw == (data.status_raw | 0x00000400))
                data.status.status.push_back(STOP);
            if (data.status_raw == (data.status_raw | 0x00000800))
                data.status.status.push_back(FROG);
            if (data.status_raw == (data.status_raw | 0x00001000))
                data.status.status.push_back(SMALL);
            if (data.status_raw == (data.status_raw | 0x00002000))
                data.status.status.push_back(SLOW_NUMB);
            if (data.status_raw == (data.status_raw | 0x00004000))
                data.status.status.push_back(PETRIFY);
            if (data.status_raw == (data.status_raw | 0x00008000))
                data.status.status.push_back(REGEN);
            if (data.status_raw == (data.status_raw | 0x00010000))
                data.status.status.push_back(BARRIER);
            if (data.status_raw == (data.status_raw | 0x00020000))
                data.status.status.push_back(M_BARRIER);
            if (data.status_raw == (data.status_raw | 0x00040000))
                data.status.status.push_back(REFLECT);
            if (data.status_raw == (data.status_raw | 0x00080000))
                data.status.status.push_back(DUAL);
            if (data.status_raw == (data.status_raw | 0x00100000))
                data.status.status.push_back(SHIELD);
            if (data.status_raw == (data.status_raw | 0x00200000))
                data.status.status.push_back(D_SENTENCE);
            if (data.status_raw == (data.status_raw | 0x00400000))
                data.status.status.push_back(MANIPULATE);
            if (data.status_raw == (data.status_raw | 0x00800000))
                data.status.status.push_back(BERSERK);
            if (data.status_raw == (data.status_raw | 0x01000000))
                data.status.status.push_back(PEERLESS);
            if (data.status_raw == (data.status_raw | 0x02000000))
                data.status.status.push_back(PARALYSIS);
            if (data.status_raw == (data.status_raw | 0x04000000))
                data.status.status.push_back(DARKNESS);
            if (data.status_raw == (data.status_raw | 0x08000000))
                data.status.status.push_back(DUAL_DRAIN);
            if (data.status_raw == (data.status_raw | 0x10000000))
                data.status.status.push_back(DEATH_FORCE);
            if (data.status_raw == (data.status_raw | 0x20000000))
                data.status.status.push_back(RESIST);
            if (data.status_raw == (data.status_raw | 0x40000000))
                data.status.status.push_back(LUCKY_GIRL);
            if (data.status_raw == (data.status_raw | 0x80000000))
                data.status.status.push_back(IMPRISONED);
        }
        // Get elements
        if (data.element_raw == (data.element_raw | 0x0001)) data.elements.push_back(FIRE);
        if (data.element_raw == (data.element_raw | 0x0002)) data.elements.push_back(ICE);
        if (data.element_raw == (data.element_raw | 0x0004)) data.elements.push_back(BOLT);
        if (data.element_raw == (data.element_raw | 0x0008)) data.elements.push_back(EARTH);
        if (data.element_raw == (data.element_raw | 0x0010)) data.elements.push_back(POISON);
        if (data.element_raw == (data.element_raw | 0x0020)) data.elements.push_back(GRAVITY);
        if (data.element_raw == (data.element_raw | 0x0040)) data.elements.push_back(WATER);
        if (data.element_raw == (data.element_raw | 0x0080)) data.elements.push_back(WIND);
        if (data.element_raw == (data.element_raw | 0x0100)) data.elements.push_back(HOLY);
        if (data.element_raw == (data.element_raw | 0x0200)) data.elements.push_back(RESTORATIVE);
        if (data.element_raw == (data.element_raw | 0x0400)) data.elements.push_back(CUT);
        if (data.element_raw == (data.element_raw | 0x0800)) data.elements.push_back(HIT);
        if (data.element_raw == (data.element_raw | 0x1000)) data.elements.push_back(PUNCH);
        if (data.element_raw == (data.element_raw | 0x2000)) data.elements.push_back(SHOOT);
        if (data.element_raw == (data.element_raw | 0x4000)) data.elements.push_back(SHOUT);
        if (data.element_raw == (data.element_raw | 0x8000)) data.elements.push_back(HIDDEN);
        // In KERNEL.BIN, Non elemental item actually have all elements marked.
        // If that happens, clear them all.
        if (data.elements.size() == 16) data.elements.clear();


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
          << "    restore = " << attack.restore_type << ",\n"
          << "    target = {selectable = " << attack.target.selection_enabled
          << ", default_enemy = " << attack.target.default_enemy
          << ", default_multiple = " << attack.target.default_multiple
          << ", toggle_multiple = " << attack.target.toggle_multiple
          << ", fixed_row = " << attack.target.fixed_row
          << ", short_range = " << attack.target.short_range
          << ", all_rows = " << attack.target.all_rows
          << ", random = " << attack.target.random << "}\n}\n";
    }
    file.close();
}

int KernelDataInstaller::ReadCharacters(){

    // TODO: Do something about young Cloud and Sephiroth.

    // Empty the item list
    characters_.clear();

    File characters_file = kernel_.ExtractGZip(KERNEL_BATTLE_AND_GROWTH_DATA);
    int character_count = 0;

    for (int i = 0; i < 11; i ++){ // 11 characters

        CharacterData data;

        // Read data from the item data section.
        data.curve_str = characters_file.readU8();
        data.curve_vit = characters_file.readU8();
        data.curve_mag = characters_file.readU8();
        data.curve_spr = characters_file.readU8();
        data.curve_dex = characters_file.readU8();
        data.curve_lck = characters_file.readU8();
        data.curve_hp = characters_file.readU8();
        data.curve_mp = characters_file.readU8();
        data.curve_exp = characters_file.readU8();
        data.unused_0 = characters_file.readU8();
        data.initial_level = characters_file.readU8();
        data.unused_1 = characters_file.readU8();
        data.limit_1_1 = characters_file.readU8();
        data.limit_1_2 = characters_file.readU8();
        data.limit_1_3 = characters_file.readU8();
        data.limit_2_1 = characters_file.readU8();
        data.limit_2_2 = characters_file.readU8();
        data.limit_2_3 = characters_file.readU8();
        data.limit_3_1 = characters_file.readU8();
        data.limit_3_2 = characters_file.readU8();
        data.limit_3_3 = characters_file.readU8();
        data.limit_4_1 = characters_file.readU8();
        data.limit_4_2 = characters_file.readU8();
        data.limit_4_3 = characters_file.readU8();
        data.limit_2_kills = characters_file.readU16LE();
        data.limit_3_kills = characters_file.readU16LE();
        data.limit_1_2_uses = characters_file.readU16LE();
        data.limit_1_3_uses = characters_file.readU16LE();
        data.limit_2_2_uses = characters_file.readU16LE();
        data.limit_2_3_uses = characters_file.readU16LE();
        data.limit_3_2_uses = characters_file.readU16LE();
        data.limit_3_3_uses = characters_file.readU16LE();
        data.limit_1_div = characters_file.readU32LE();
        data.limit_2_div = characters_file.readU32LE();
        data.limit_3_div = characters_file.readU32LE();
        data.limit_4_div = characters_file.readU32LE();

        // Derive data from the read data.
        data.id = i;
        data.name = VGears::NameLookup::CharName(i);

        // Add to the list of characters.
        characters_.push_back(data);
        character_count ++;
    }

    return character_count;
}

void KernelDataInstaller::WriteCharacters(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (CharacterData character : characters_){
        file
          << "Game.Characters[" << character.id << "] = {\n"
          << "    id = " << character.id << ",\n"
          << "    name = \"" << character.name << "\",\n"
          << "    initial_level = \"" << static_cast<int>(character.initial_level) << "\",\n"
          << "    curves = {\n"
          << "        str = " << static_cast<int>(character.curve_str) << ",\n"
          << "        vit = " << static_cast<int>(character.curve_vit) << ",\n"
          << "        mag = " << static_cast<int>(character.curve_mag) << ",\n"
          << "        spr = " << static_cast<int>(character.curve_spr) << ",\n"
          << "        dex = " << static_cast<int>(character.curve_dex) << ",\n"
          << "        lck = " << static_cast<int>(character.curve_lck) << ",\n"
          << "        hp = " << static_cast<int>(character.curve_hp) << ",\n"
          << "        mp = " << static_cast<int>(character.curve_mp) << ",\n"
          << "        exp = " << static_cast<int>(character.curve_exp) << ",\n"
          << "    },\n"
          << "    limits = {\n"
          << "        [1] = {\n"
          << "            kills = 0,\n"
          << "            techniques = {\n"
          << "                [1] = {\n"
          << "                    command = " << static_cast<int>(character.limit_1_1) << ",\n"
          << "                    uses = 0\n"
          << "                },\n";
        if (character.limit_1_2 < 255){ // Some characters only have 1 limit per level.
            file
              << "                [2] = {\n"
              << "                    command = " << static_cast<int>(character.limit_1_2) << ",\n"
              << "                    uses = " << character.limit_1_2_uses << "\n"
              << "                },\n";
        }
        file
          << "            }\n"
          << "        },\n";
        // Limit level 2.
        if (character.limit_2_kills < 65535){
            file
              << "        [2] = {\n"
              << "            kills = " << character.limit_2_kills << ",\n"
              << "            techniques = {\n"
              << "                [1] = {\n"
              << "                    command = " << static_cast<int>(character.limit_2_1) << ",\n"
              << "                    uses = 0\n"
              << "                },\n";
            if (character.limit_2_2 < 255){ // Some characters only have 1 limit per level.
                file
                  << "                [2] = {\n"
                  << "                    command = "
                  << static_cast<int>(character.limit_2_2) << ",\n"
                  << "                    uses = " << character.limit_2_2_uses << "\n"
                  << "                },\n";
            }
            file
              << "            }\n"
              << "        },\n";
        }
        // Limit level 3.
        if (character.limit_3_kills < 65535){
            file
              << "        [3] = {\n"
              << "            kills = " << character.limit_3_kills << ",\n"
              << "            techniques = {\n"
              << "                [1] = {\n"
              << "                    command = " << static_cast<int>(character.limit_3_1) << ",\n"
              << "                    uses = 0\n"
              << "                },\n";
            if (character.limit_3_2 < 255){ // Some characters only have 1 limit per level.
                file
                  << "                [2] = {\n"
                  << "                    command = "
                  << static_cast<int>(character.limit_3_2) << ",\n"
                  << "                    uses = " << character.limit_3_2_uses << "\n"
                  << "                },\n";
            }
            file
              << "            }\n"
              << "        },\n";
        }
        // Limit level 4.
        if (character.limit_4_1 < 255){
            file
              << "        [4] = {\n"
              << "            kills = 0,\n"
              << "            techniques = {\n"
              << "                [1] = {\n"
              << "                    command = " << static_cast<int>(character.limit_4_1) << ",\n"
              << "                    uses = 0\n"
              << "                },\n"
              << "            }\n"
              << "        },\n";
        }
        file
          << "    }\n}\n";
    }
    file.close();
}

void KernelDataInstaller::ReadGrowth(){

    File growth_file = kernel_.ExtractGZip(KERNEL_BATTLE_AND_GROWTH_DATA);
    growth_file.SetOffset(0x01F8); //Skip character data.

    for (int i = 0; i < 12; i ++) growth_.bonus_stat[i] = growth_file.readU8();
    for (int i = 0; i < 12; i ++) growth_.bonus_hp[i] = growth_file.readU8();
    for (int i = 0; i < 12; i ++) growth_.bonus_mp[i] = growth_file.readU8();
    for (int i = 0; i < 64; i ++){
        growth_.curves[i].id = i;
        if (i < 37) growth_.curves[i].type = CURVE_PRIMARY;
        else if (i >=37 && i < 46) growth_.curves[i].type = CURVE_HP;
        else if (i >=46 && i < 55) growth_.curves[i].type = CURVE_MP;
        else growth_.curves[i].type = CURVE_EXP;
        for (int j = 0; j < 8; j ++){
            growth_.curves[i].gradient[j] = growth_file.readU8();
            growth_.curves[i].base[j] = growth_file.readU8();
        }
    }
}

void KernelDataInstaller::WriteGrowth(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    file << "Game.Growth.bonus.primary = {";
    for (int i = 0; i < 12; i ++)
        file << "[" << i << "] = " << static_cast<int>(growth_.bonus_stat[i]) << ", ";
    file << "}\nGame.Growth.bonus.hp = {";
    for (int i = 0; i < 12; i ++)
        file << "[" << i << "] = " << static_cast<int>(growth_.bonus_hp[i]) << ", ";
    file << "}\nGame.Growth.bonus.mp = {";
    for (int i = 0; i < 12; i ++)
        file << "[" << i << "] = " << static_cast<int>(growth_.bonus_mp[i]) << ", ";
    file << "}\n";
    for (int i = 0; i < 64; i ++){
        file << "Game.Growth.curves[" << i << "] = {\n";
        for (int j = 0; j < 8; j ++){
            file << "    [" << j << "] = {gradient = "
              << static_cast<int>(growth_.curves[i].gradient[j])
              << ", base = " << static_cast<int>(growth_.curves[i].gradient[j]) << "},\n";
        }
        file << "}\n";
    }

    file.close();
}

int KernelDataInstaller::ReadItems(){

    // Empty the item list
    items_.clear();

    File items_file = kernel_.ExtractGZip(KERNEL_ITEM_DATA);
    File items_name_file = kernel_.ExtractGZip(KERNEL_ITEM_NAMES);
    File items_desc_file = kernel_.ExtractGZip(KERNEL_ITEM_DESCRIPTIONS);
    int item_count = 0;

    for (int i = 0; i < 128; i ++){

        ItemData data;
        int name_offset;
        int desc_offset;

        // From the current name file offset, read 16 bytes.
        // This is the pointer to the offset for the item name.
        // Start reading there until 0xFF is found.
        data.name = "";
        u8 ch = 1;
        name_offset = items_name_file.readU16LE();
        while (ch != 0xFF){
            ch = items_name_file.GetU8(name_offset);
            if (ch != 0xFF) data.name += ENGLISH_CHARS[ch];
            name_offset ++;
        }
        boost::replace_all(data.name, "\"", "'");

        // For descriptions, do the same as for names.
        data.description = "";
        ch = 1;
        desc_offset = items_desc_file.readU16LE();
        while (ch != 0xFF){
            ch = items_desc_file.GetU8(desc_offset);
            if (ch != 0xFF){
                // Warning: If the char is 0xF9, special function!
                // the next byte (in bits, aaoooooo) means to read aa data at offset oooooo.
                // This is to compress information in the kernel.
                if (ch == 0xF9){
                    // Get how much to read, two upper bits of the next byte.
                    // Multiply by 2 and add 4. That's just how it is.
                    u8 comp_size = (items_desc_file.GetU8(desc_offset + 1) >> 6) * 2 + 4;
                    // Get new offset, lower six bits of the next byte.
                    // The offset goes backwards from the position of the 0xF9 byte.
                    u8 comp_offset = items_desc_file.GetU8(desc_offset + 1) & 63;

                    for (int c = 0; c < comp_size; c ++){
                        data.description += ENGLISH_CHARS[
                          items_desc_file.GetU8(desc_offset - 1 - comp_offset + c)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }
        boost::replace_all(data.description, "\"", "'");

        // If the item doesn't have a name, it means all items have been read.
        // break the loop.
        if ("" == data.name) break;

        // Read data from the item data section.
        data.unknown_0 = items_file.readU32LE();
        data.unknown_1 = items_file.readU32LE();
        data.camera = items_file.readU16LE();
        data.restrict_raw = items_file.readU16LE();
        data.target_raw = items_file.readU8();
        data.effect = items_file.readU8();
        data.damage_raw = items_file.readU8();
        data.power = items_file.readU8();
        data.condition_raw = items_file.readU8();
        data.status_change_raw = items_file.readU8();
        data.additional_effects_raw = items_file.readU8();
        data.additional_effects_mod_raw = items_file.readU8();
        data.status_raw = items_file.readU32LE();
        data.element_raw = items_file.readU16LE();
        data.special_raw = items_file.readU16LE();

        // Derive data from the read data.
        data.id = i;
        data.sellable = !(data.restrict_raw & (1 << 0));
        data.useable_battle = !(data.restrict_raw & (2 << 0));
        data.useable_menu = !(data.restrict_raw & (4 << 0));
        if (data.target_raw == (data.target_raw | 0x01)) data.target.selection_enabled = true;
        else data.target.selection_enabled = false;
        if (data.target_raw == (data.target_raw | 0x02)) data.target.default_enemy = true;
        else data.target.default_enemy = false;
        if (data.target_raw == (data.target_raw | 0x04)) data.target.default_multiple = true;
        else data.target.default_multiple = false;
        if (data.target_raw == (data.target_raw | 0x08)) data.target.toggle_multiple = true;
        else data.target.toggle_multiple = false;
        if (data.target_raw == (data.target_raw | 0x10)) data.target.fixed_row = true;
        else data.target.fixed_row = false;
        if (data.target_raw == (data.target_raw | 0x20)) data.target.short_range = true;
        else data.target.short_range = false;
        if (data.target_raw == (data.target_raw | 0x40)) data.target.default_enemy = true;
        else data.target.all_rows = false;
        if (data.target_raw == (data.target_raw | 0x80)) data.target.random = true;
        else data.target.random = false;
        data.damage_formula = data.damage_raw >> 4; // Upper nybble.
        data.damage_modifier = data.damage_raw & 15; // Lower nybble.
        data.status.chance = 63 - (data.status_change_raw ^ 63); // Out of 63;
        if (data.status_change_raw == (data.status_change_raw | 0x00000080))
            data.status.mode = TOGGLE;
        else if (data.status_change_raw == (data.status_change_raw | 0x00000040))
            data.status.mode = CURE;
        else data.status.mode = INFLICT;
        switch (data.condition_raw){
            case 0: data.restore_type = RESTORE_HP; break;
            case 1: data.restore_type = RESTORE_MP; break;
            case 2: data.restore_type = RESTORE_STATUS; break;
            default: data.restore_type = RESTORE_NONE;
        }


        // In KERNEL.BIN, items that don't induce statuses actually have all statuses marked, and
        // a chance of 255. If it's the case, skip statuses altogether.
        if (data.status.chance <= 64){
            // TODO: Im fairly sure that this can be done with a single-line loop...
            // ... but I don't know how
            if (data.status_raw == (data.status_raw | 0x00000001))
                data.status.status.push_back(DEATH);
            if (data.status_raw == (data.status_raw | 0x00000002))
                data.status.status.push_back(NEAR_DEATH);
            if (data.status_raw == (data.status_raw | 0x00000004))
                data.status.status.push_back(SLEEP);
            if (data.status_raw == (data.status_raw | 0x00000008))
                data.status.status.push_back(POISONED);
            if (data.status_raw == (data.status_raw | 0x00000010))
                data.status.status.push_back(SADNESS);
            if (data.status_raw == (data.status_raw | 0x00000020))
                data.status.status.push_back(FURY);
            if (data.status_raw == (data.status_raw | 0x00000040))
                data.status.status.push_back(CONFU);
            if (data.status_raw == (data.status_raw | 0x00000080))
                data.status.status.push_back(SILENCE);
            if (data.status_raw == (data.status_raw | 0x00000100))
                data.status.status.push_back(HASTE);
            if (data.status_raw == (data.status_raw | 0x00000200))
                data.status.status.push_back(SLOW);
            if (data.status_raw == (data.status_raw | 0x00000400))
                data.status.status.push_back(STOP);
            if (data.status_raw == (data.status_raw | 0x00000800))
                data.status.status.push_back(FROG);
            if (data.status_raw == (data.status_raw | 0x00001000))
                data.status.status.push_back(SMALL);
            if (data.status_raw == (data.status_raw | 0x00002000))
                data.status.status.push_back(SLOW_NUMB);
            if (data.status_raw == (data.status_raw | 0x00004000))
                data.status.status.push_back(PETRIFY);
            if (data.status_raw == (data.status_raw | 0x00008000))
                data.status.status.push_back(REGEN);
            if (data.status_raw == (data.status_raw | 0x00010000))
                data.status.status.push_back(BARRIER);
            if (data.status_raw == (data.status_raw | 0x00020000))
                data.status.status.push_back(M_BARRIER);
            if (data.status_raw == (data.status_raw | 0x00040000))
                data.status.status.push_back(REFLECT);
            if (data.status_raw == (data.status_raw | 0x00080000))
                data.status.status.push_back(DUAL);
            if (data.status_raw == (data.status_raw | 0x00100000))
                data.status.status.push_back(SHIELD);
            if (data.status_raw == (data.status_raw | 0x00200000))
                data.status.status.push_back(D_SENTENCE);
            if (data.status_raw == (data.status_raw | 0x00400000))
                data.status.status.push_back(MANIPULATE);
            if (data.status_raw == (data.status_raw | 0x00800000))
                data.status.status.push_back(BERSERK);
            if (data.status_raw == (data.status_raw | 0x01000000))
                data.status.status.push_back(PEERLESS);
            if (data.status_raw == (data.status_raw | 0x02000000))
                data.status.status.push_back(PARALYSIS);
            if (data.status_raw == (data.status_raw | 0x04000000))
                data.status.status.push_back(DARKNESS);
            if (data.status_raw == (data.status_raw | 0x08000000))
                data.status.status.push_back(DUAL_DRAIN);
            if (data.status_raw == (data.status_raw | 0x10000000))
                data.status.status.push_back(DEATH_FORCE);
            if (data.status_raw == (data.status_raw | 0x20000000))
                data.status.status.push_back(RESIST);
            if (data.status_raw == (data.status_raw | 0x40000000))
                data.status.status.push_back(LUCKY_GIRL);
            if (data.status_raw == (data.status_raw | 0x80000000))
                data.status.status.push_back(IMPRISONED);
        }

        // Get elements
        if (data.element_raw == (data.element_raw | 0x0001)) data.elements.push_back(FIRE);
        if (data.element_raw == (data.element_raw | 0x0002)) data.elements.push_back(ICE);
        if (data.element_raw == (data.element_raw | 0x0004)) data.elements.push_back(BOLT);
        if (data.element_raw == (data.element_raw | 0x0008)) data.elements.push_back(EARTH);
        if (data.element_raw == (data.element_raw | 0x0010)) data.elements.push_back(POISON);
        if (data.element_raw == (data.element_raw | 0x0020)) data.elements.push_back(GRAVITY);
        if (data.element_raw == (data.element_raw | 0x0040)) data.elements.push_back(WATER);
        if (data.element_raw == (data.element_raw | 0x0080)) data.elements.push_back(WIND);
        if (data.element_raw == (data.element_raw | 0x0100)) data.elements.push_back(HOLY);
        if (data.element_raw == (data.element_raw | 0x0200)) data.elements.push_back(RESTORATIVE);
        if (data.element_raw == (data.element_raw | 0x0400)) data.elements.push_back(CUT);
        if (data.element_raw == (data.element_raw | 0x0800)) data.elements.push_back(HIT);
        if (data.element_raw == (data.element_raw | 0x1000)) data.elements.push_back(PUNCH);
        if (data.element_raw == (data.element_raw | 0x2000)) data.elements.push_back(SHOOT);
        if (data.element_raw == (data.element_raw | 0x4000)) data.elements.push_back(SHOUT);
        if (data.element_raw == (data.element_raw | 0x8000)) data.elements.push_back(HIDDEN);
        // In KERNEL.BIN, Non elemental item actually have all elements marked.
        // If that happens, clear them all.
        if (data.elements.size() == 16) data.elements.clear();

        // Add to the list of items.
        items_.push_back(data);
        item_count ++;
    }

    return item_count;
}

void KernelDataInstaller::WriteItems(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (ItemData item : items_){
        file << "Game.Items[" << item.id << "] = {\n"
          << "    type = Inventory.ITEM_TYPE.ITEM,\n"
          << "    id = " << item.id << ",\n"
          << "    price = " << prices_[item.id] << ",\n"
          << "    name = \"" << item.name << "\",\n"
          << "    description = \"" << item.description << "\",\n"
          << "    camera = " << item.camera << ",\n"
          << "    sell = " << item.sellable << ",\n"
          << "    battle = " << item.useable_battle << ",\n"
          << "    menu = " << item.useable_menu << ",\n"
          << "    throw = 0,\n"
          << "    effect = " << static_cast<int>(item.effect) << ",\n"
          << "    dmg_formula = " << static_cast<int>(item.damage_formula) << ",\n"
          << "    dmg_modifier = " << static_cast<int>(item.damage_modifier) << ",\n"
          << "    power = " << static_cast<int>(item.power) << ",\n"
          << "    restore = " << item.restore_type << ",\n"
          << "    target = {selectable = " << item.target.selection_enabled
          << ", default_enemy = " << item.target.default_enemy
          << ", default_multiple = " << item.target.default_multiple
          << ", toggle_multiple = " << item.target.toggle_multiple
          << ", fixed_row = " << item.target.fixed_row
          << ", short_range = " << item.target.short_range
          << ", all_rows = " << item.target.all_rows
          << ", random = " << item.target.random << "},\n"
          << "    status = {";
        for (int s : item.status.status){
            file << "\n        {status = " << s << ", mode = " << item.status.mode
              << ", chance = " << item.status.chance << "},";
        }
        if (item.status.status.size() > 0) file << "\n    },\n";
        else file << "},\n";
        file << "    elements = {";
        for (int e : item.elements) file << e << ", ";
        file << "},\n}\n";


    }
    file.close();
}

int KernelDataInstaller::ReadWeapons(){

    // Empty the item list
    weapons_.clear();

    File weapons_file = kernel_.ExtractGZip(KERNEL_WEAPON_DATA);
    File weapons_name_file = kernel_.ExtractGZip(KERNEL_WEAPON_NAMES);
    File weapons_desc_file = kernel_.ExtractGZip(KERNEL_WEAPON_DESCRIPTIONS);
    int weapon_count = 0;

    for (int i = 0; i < 128; i ++){

        WeaponData data;
        int name_offset;
        int desc_offset;

        // From the current name file offset, read 16 bytes.
        // This is the pointer to the offset for the item name.
        // Start reading there until 0xFF is found.
        data.name = "";
        u8 ch = 1;
        name_offset = weapons_name_file.readU16LE();
        while (ch != 0xFF){
            ch = weapons_name_file.GetU8(name_offset);
            if (ch != 0xFF) data.name += ENGLISH_CHARS[ch];
            name_offset ++;
        }
        boost::replace_all(data.name, "\"", "'");

        // For descriptions, do the same as for names.
        data.description = "";
        ch = 1;
        desc_offset = weapons_desc_file.readU16LE();
        while (ch != 0xFF){
            ch = weapons_desc_file.GetU8(desc_offset);
            if (ch != 0xFF){
                // Warning: If the char is 0xF9, special function!
                // the next byte (in bits, aaoooooo) means to read aa data at offset oooooo.
                // This is to compress information in the kernel.
                if (ch == 0xF9){
                    // Get how much to read, two upper bits of the next byte.
                    // Multiply by 2 and add 4. That's just how it is.
                    u8 comp_size = (weapons_desc_file.GetU8(desc_offset + 1) >> 6) * 2 + 4;
                    // Get new offset, lower six bits of the next byte.
                    // The offset goes backwards from the position of the 0xF9 byte.
                    u8 comp_offset = weapons_desc_file.GetU8(desc_offset + 1) & 63;

                    for (int c = 0; c < comp_size; c ++){
                        data.description += ENGLISH_CHARS[
                          weapons_desc_file.GetU8(desc_offset - 1 - comp_offset + c)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }
        boost::replace_all(data.description, "\"", "'");

        // If the item doesn't have a name, it means all items have been read.
        // break the loop.
        if ("" == data.name) break;

        // Read data from the weapon data section.
        data.target_raw = weapons_file.readU8();
        data.unused_0 = weapons_file.readU8();
        data.damage_raw = weapons_file.readU8();
        data.unused_1 = weapons_file.readU8();
        data.power = weapons_file.readU8();
        data.status = weapons_file.readU8();
        data.growth = weapons_file.readU8();
        data.critical = weapons_file.readU8();
        data.accuracy = weapons_file.readU8();
        data.model_raw = weapons_file.readU8();
        data.unused_2 = weapons_file.readU8();
        data.high_sound = weapons_file.readU8();
        data.camera = weapons_file.readU16LE();
        data.equip_raw = weapons_file.readU16LE();
        data.element_raw = weapons_file.readU16LE();
        data.unused_3 = weapons_file.readU16LE();
        for (int s = 0; s < 4; s ++) data.stat_raw[s] = weapons_file.readU8();
        for (int s = 0; s < 4; s ++) data.stat_bonus_raw[s] = weapons_file.readU8();
        for (int s = 0; s < 8; s ++) data.slots_raw[s] = weapons_file.readU8();
        data.sound = weapons_file.readU8();
        data.sound_critical = weapons_file.readU8();
        data.sound_miss = weapons_file.readU8();
        data.effect = weapons_file.readU8();
        data.special_raw = weapons_file.readU16LE();
        data.restrict_raw = weapons_file.readU16LE();


        // Derive data from the read data.
        data.id = i;
        if (data.growth != 0 && data.growth != 2 && data.growth != 3) data.growth = 1;
        data.sellable = !(data.restrict_raw & (1 << 0));
        data.useable_battle = !(data.restrict_raw & (2 << 0));
        data.useable_menu = !(data.restrict_raw & (4 << 0));
        data.throwable = !(data.restrict_raw & (8 << 0));
        if (data.target_raw == (data.target_raw | 0x01)) data.target.selection_enabled = true;
        else data.target.selection_enabled = false;
        if (data.target_raw == (data.target_raw | 0x02)) data.target.default_enemy = true;
        else data.target.default_enemy = false;
        if (data.target_raw == (data.target_raw | 0x04)) data.target.default_multiple = true;
        else data.target.default_multiple = false;
        if (data.target_raw == (data.target_raw | 0x08)) data.target.toggle_multiple = true;
        else data.target.toggle_multiple = false;
        if (data.target_raw == (data.target_raw | 0x10)) data.target.fixed_row = true;
        else data.target.fixed_row = false;
        if (data.target_raw == (data.target_raw | 0x20)) data.target.short_range = true;
        else data.target.short_range = false;
        if (data.target_raw == (data.target_raw | 0x40)) data.target.default_enemy = true;
        else data.target.all_rows = false;
        if (data.target_raw == (data.target_raw | 0x80)) data.target.random = true;
        else data.target.random = false;
        data.damage_formula = data.damage_raw >> 4; // Upper nybble.
        data.damage_modifier = data.damage_raw & 15; // Lower nybble.
        data.animation_index = data.model_raw >> 4; // Upper nybble.
        data.model = data.model_raw & 15; // Lower nybble.
        if (data.equip_raw == (data.equip_raw | 0x0001)) data.equip.push_back(0);
        if (data.equip_raw == (data.equip_raw | 0x0002)) data.equip.push_back(1);
        if (data.equip_raw == (data.equip_raw | 0x0004)) data.equip.push_back(2);
        if (data.equip_raw == (data.equip_raw | 0x0008)) data.equip.push_back(3);
        if (data.equip_raw == (data.equip_raw | 0x0010)) data.equip.push_back(4);
        if (data.equip_raw == (data.equip_raw | 0x0020)) data.equip.push_back(5);
        if (data.equip_raw == (data.equip_raw | 0x0040)) data.equip.push_back(6);
        if (data.equip_raw == (data.equip_raw | 0x0080)) data.equip.push_back(7);
        if (data.equip_raw == (data.equip_raw | 0x0100)) data.equip.push_back(8);
        if (data.equip_raw == (data.equip_raw | 0x0200)) data.equip.push_back(9);
        if (data.equip_raw == (data.equip_raw | 0x0400)) data.equip.push_back(10);
        for (int s = 0; s < 4; s ++){
            if (data.stat_raw[s] < 0XFF && data.stat_bonus_raw[s] > 0){
                StatBonus stat_bonus;
                stat_bonus.stat = data.stat_raw[s];
                stat_bonus.bonus = data.stat_bonus_raw[s];
                data.stat_bonus.push_back(stat_bonus);
            }
        }
        for (int s = 0; s < 8; s ++){
            if (data.slots_raw[s] == 1 || data.slots_raw[s] == 5){
                data.materia_slots.push_back(SLOT_UNLINKED);
            }
            else if (
              data.slots_raw[s] == 2 || data.slots_raw[s] == 3
              || data.slots_raw[s] == 6 || data.slots_raw[s] == 7
            ){
                data.materia_slots.push_back(SLOT_LINKED);
            }
        }
        if (data.element_raw == (data.element_raw | 0x0001)) data.elements.push_back(FIRE);
        if (data.element_raw == (data.element_raw | 0x0002)) data.elements.push_back(ICE);
        if (data.element_raw == (data.element_raw | 0x0004)) data.elements.push_back(BOLT);
        if (data.element_raw == (data.element_raw | 0x0008)) data.elements.push_back(EARTH);
        if (data.element_raw == (data.element_raw | 0x0010)) data.elements.push_back(POISON);
        if (data.element_raw == (data.element_raw | 0x0020)) data.elements.push_back(GRAVITY);
        if (data.element_raw == (data.element_raw | 0x0040)) data.elements.push_back(WATER);
        if (data.element_raw == (data.element_raw | 0x0080)) data.elements.push_back(WIND);
        if (data.element_raw == (data.element_raw | 0x0100)) data.elements.push_back(HOLY);
        if (data.element_raw == (data.element_raw | 0x0200)) data.elements.push_back(RESTORATIVE);
        if (data.element_raw == (data.element_raw | 0x0400)) data.elements.push_back(CUT);
        if (data.element_raw == (data.element_raw | 0x0800)) data.elements.push_back(HIT);
        if (data.element_raw == (data.element_raw | 0x1000)) data.elements.push_back(PUNCH);
        if (data.element_raw == (data.element_raw | 0x2000)) data.elements.push_back(SHOOT);
        if (data.element_raw == (data.element_raw | 0x4000)) data.elements.push_back(SHOUT);
        if (data.element_raw == (data.element_raw | 0x8000)) data.elements.push_back(HIDDEN);
        // In KERNEL.BIN, Non elemental item actually have all elements marked.
        // If that happens, clear them all.
        if (data.elements.size() == 16) data.elements.clear();

        // Add to the list of weapons.
        weapons_.push_back(data);
        weapon_count ++;
    }

    return weapon_count;
}

void KernelDataInstaller::WriteWeapons(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (WeaponData weapon : weapons_){
        file << "Game.Items[" << weapon.id + 128 << "] = {\n"
          << "    type = Inventory.ITEM_TYPE.WEAPON,\n"
          << "    id = " << weapon.id + 128 << ",\n"
          << "    price = " << prices_[weapon.id + 128] << ",\n"
          << "    name = \"" << weapon.name << "\",\n"
          << "    description = \"" << weapon.description << "\",\n"
          << "    camera = " << weapon.camera << ",\n"
          << "    sell = " << weapon.sellable << ",\n"
          << "    battle = " << weapon.useable_battle << ",\n"
          << "    menu = " << weapon.useable_menu << ",\n"
          << "    throw = " << weapon.throwable << ",\n"
          << "    effect = " << static_cast<int>(weapon.effect) << ",\n"
          << "    dmg_formula = " << static_cast<int>(weapon.damage_formula) << ",\n"
          << "    dmg_modifier = " << static_cast<int>(weapon.damage_modifier) << ",\n"
          << "    power = " << static_cast<int>(weapon.power) << ",\n"
          << "    growth = " << static_cast<int>(weapon.growth) << ",\n"
          << "    accuracy = " << static_cast<int>(weapon.accuracy) << ",\n"
          << "    critical = " << static_cast<int>(weapon.critical) << ",\n"
          << "    model = " << static_cast<int>(weapon.model) << ",\n"
          << "    animation = " << static_cast<int>(weapon.animation_index) << ",\n"
          << "    target = {selectable = " << weapon.target.selection_enabled
          << ", default_enemy = " << weapon.target.default_enemy
          << ", default_multiple = " << weapon.target.default_multiple
          << ", toggle_multiple = " << weapon.target.toggle_multiple
          << ", fixed_row = " << weapon.target.fixed_row
          << ", short_range = " << weapon.target.short_range
          << ", all_rows = " << weapon.target.all_rows
          << ", random = " << weapon.target.random << "},\n"
          << "    elements = {";
        for (int s : weapon.elements) file << s << ", ";
        file << "},\n";
        file << "    slots = {";
        for (u8 slot : weapon.materia_slots) file << static_cast<int>(slot) << ", ";
        file << "},\n    stats = {";
        for (StatBonus s : weapon.stat_bonus){
            file << "\n        " << GetBasicStatName(s.stat) << " = "
              << static_cast<int>(s.stat) << ",";
        }
        if (weapon.stat_bonus.size() > 0) file << "\n    },\n";
        else file << "},\n";
        file << "    users = {";
        for (int s : weapon.equip) file << s << ", ";
        file << "}\n}\n";
    }
    file.close();
}

int KernelDataInstaller::ReadArmors(){

    // Empty the item list
    armors_.clear();

    File armor_file = kernel_.ExtractGZip(KERNEL_ARMOR_DATA);
    File armor_name_file = kernel_.ExtractGZip(KERNEL_ARMOR_NAMES);
    File armor_desc_file = kernel_.ExtractGZip(KERNEL_ARMOR_DESCRIPTIONS);
    int armor_count = 0;

    for (int i = 0; i < 32; i ++){ // There are exactly 32.

        ArmorData data;
        int name_offset;
        int desc_offset;

        // From the current name file offset, read 16 bytes.
        // This is the pointer to the offset for the item name.
        // Start reading there until 0xFF is found.
        data.name = "";
        u8 ch = 1;
        name_offset = armor_name_file.readU16LE();
        while (ch != 0xFF){
            ch = armor_name_file.GetU8(name_offset);
            if (ch != 0xFF) data.name += ENGLISH_CHARS[ch];
            name_offset ++;
        }
        boost::replace_all(data.name, "\"", "'");

        // For descriptions, do the same as for names.
        data.description = "";
        ch = 1;
        desc_offset = armor_desc_file.readU16LE();
        while (ch != 0xFF){
            ch = armor_desc_file.GetU8(desc_offset);
            if (ch != 0xFF){
                // Warning: If the char is 0xF9, special function!
                // the next byte (in bits, aaoooooo) means to read aa data at offset oooooo.
                // This is to compress information in the kernel.
                if (ch == 0xF9){
                    // Get how much to read, two upper bits of the next byte.
                    // Multiply by 2 and add 4. That's just how it is.
                    u8 comp_size = (armor_desc_file.GetU8(desc_offset + 1) >> 6) * 2 + 4;
                    // Get new offset, lower six bits of the next byte.
                    // The offset goes backwards from the position of the 0xF9 byte.
                    u8 comp_offset = armor_desc_file.GetU8(desc_offset + 1) & 63;

                    for (int c = 0; c < comp_size; c ++){
                        data.description += ENGLISH_CHARS[
                          armor_desc_file.GetU8(desc_offset - 1 - comp_offset + c)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }
        boost::replace_all(data.description, "\"", "'");

        // If the armor doesn't have a name, it means all items have been read.
        // break the loop.
        if ("" == data.name) break;

        // Read data from the armor data section.
        data.unknown_0 = armor_file.readU8();
        data.element_defense_mode = armor_file.readU8();
        data.defense = armor_file.readU8();
        data.m_defense = armor_file.readU8();
        data.evasion = armor_file.readU8();
        data.m_evasion = armor_file.readU8();
        data.status = armor_file.readU8();
        data.unknown_1 = armor_file.readU16LE();
        for (int s = 0; s < 8; s ++) data.slots_raw[s] = armor_file.readU8();
        data.growth = armor_file.readU8();
        data.equip_raw = armor_file.readU16LE();
        data.element_raw = armor_file.readU16LE();
        data.unknown_2 = armor_file.readU16LE();
        for (int s = 0; s < 4; s ++) data.stat_raw[s] = armor_file.readU8();
        for (int s = 0; s < 4; s ++) data.stat_bonus_raw[s] = armor_file.readU8();
        data.restrict_raw = armor_file.readU16LE();
        data.unknown_3 = armor_file.readU16LE();

        // Derive data from the read data.
        data.id = i;
        if (data.growth != 0 && data.growth != 2 && data.growth != 3) data.growth = 1;
        data.sellable = !(data.restrict_raw & (1 << 0));
        data.useable_battle = !(data.restrict_raw & (2 << 0));
        data.useable_menu = !(data.restrict_raw & (4 << 0));
        if (data.equip_raw == (data.equip_raw | 0x0001)) data.equip.push_back(0);
        if (data.equip_raw == (data.equip_raw | 0x0002)) data.equip.push_back(1);
        if (data.equip_raw == (data.equip_raw | 0x0004)) data.equip.push_back(2);
        if (data.equip_raw == (data.equip_raw | 0x0008)) data.equip.push_back(3);
        if (data.equip_raw == (data.equip_raw | 0x0010)) data.equip.push_back(4);
        if (data.equip_raw == (data.equip_raw | 0x0020)) data.equip.push_back(5);
        if (data.equip_raw == (data.equip_raw | 0x0040)) data.equip.push_back(6);
        if (data.equip_raw == (data.equip_raw | 0x0080)) data.equip.push_back(7);
        if (data.equip_raw == (data.equip_raw | 0x0100)) data.equip.push_back(8);
        if (data.equip_raw == (data.equip_raw | 0x0200)) data.equip.push_back(9);
        if (data.equip_raw == (data.equip_raw | 0x0400)) data.equip.push_back(10);
        for (int s = 0; s < 4; s ++){
            if (data.stat_raw[s] < 0XFF && data.stat_bonus_raw[s] > 0){
                StatBonus stat_bonus;
                stat_bonus.stat = data.stat_raw[s];
                stat_bonus.bonus = data.stat_bonus_raw[s];
                data.stat_bonus.push_back(stat_bonus);
            }
        }
        for (int i = 0; i < 8; i ++){
            if (data.slots_raw[i] == 1 || data.slots_raw[i] == 5){
                data.materia_slots.push_back(SLOT_UNLINKED);
            }
            else if (
              data.slots_raw[i] == 2 || data.slots_raw[i] == 3
              || data.slots_raw[i] == 6 || data.slots_raw[i] == 7
            ){
                data.materia_slots.push_back(SLOT_LINKED);
            }
        }
        // In KERNEL.BIN, items with no elemental defense actually do have all elements marked,
        // with mode 255. If, that's the case, consider that there are no elements.
        if (data.element_defense_mode != 0xFF){
            if (data.element_raw == (data.element_raw | 0x0001)) data.elements.push_back(FIRE);
            if (data.element_raw == (data.element_raw | 0x0002)) data.elements.push_back(ICE);
            if (data.element_raw == (data.element_raw | 0x0004)) data.elements.push_back(BOLT);
            if (data.element_raw == (data.element_raw | 0x0008)) data.elements.push_back(EARTH);
            if (data.element_raw == (data.element_raw | 0x0010)) data.elements.push_back(POISON);
            if (data.element_raw == (data.element_raw | 0x0020)) data.elements.push_back(GRAVITY);
            if (data.element_raw == (data.element_raw | 0x0040)) data.elements.push_back(WATER);
            if (data.element_raw == (data.element_raw | 0x0080)) data.elements.push_back(WIND);
            if (data.element_raw == (data.element_raw | 0x0100)) data.elements.push_back(HOLY);
            if (data.element_raw == (data.element_raw | 0x0200))
                data.elements.push_back(RESTORATIVE);
            if (data.element_raw == (data.element_raw | 0x0400)) data.elements.push_back(CUT);
            if (data.element_raw == (data.element_raw | 0x0800)) data.elements.push_back(HIT);
            if (data.element_raw == (data.element_raw | 0x1000)) data.elements.push_back(PUNCH);
            if (data.element_raw == (data.element_raw | 0x2000)) data.elements.push_back(SHOOT);
            if (data.element_raw == (data.element_raw | 0x4000)) data.elements.push_back(SHOUT);
            if (data.element_raw == (data.element_raw | 0x8000)) data.elements.push_back(HIDDEN);
        }
        // In KERNEL.BIN, Non elemental item actually have all elements marked.
        // If that happens, clear them all.
        if (data.elements.size() == 16) data.elements.clear();

        // Add to the list of weapons.
        armors_.push_back(data);
        armor_count ++;
    }

    return armor_count;
}

void KernelDataInstaller::WriteArmors(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (ArmorData armor : armors_){
        file << "Game.Items[" << armor.id + 256 << "] = {\n"
          << "    type = Inventory.ITEM_TYPE.ARMOR,\n"
          << "    id = " << armor.id + 256 << ",\n"
          << "    price = " << prices_[armor.id + 256] << ",\n"
          << "    name = \"" << armor.name << "\",\n"
          << "    description = \"" << armor.description << "\",\n"
          << "    sell = " << armor.sellable << ",\n"
          << "    battle = " << armor.useable_battle << ",\n"
          << "    menu = " << armor.useable_menu << ",\n"
          << "    throw = 0,\n"
          << "    defense = {physical = " << static_cast<int>(armor.defense) << ", magical = "
          << static_cast<int>(armor.m_defense) << "},\n"
          << "    evasion = {physical = " << static_cast<int>(armor.evasion) << ", magical = "
          << static_cast<int>(armor.m_evasion) << "},\n"
          << "    growth = " << static_cast<int>(armor.growth) << ",\n"
          << "    status = {";
        if (armor.status != 0XFF) file << armor.status;
        file
          << "},\n"
          << "    elements = {";
        for (int s : armor.elements){
            file << "\n        {id = " << s << ", mode = "
              << static_cast<int>(armor.element_defense_mode) << "},";
        }
        if (armor.elements.size() > 0) file << "\n    },\n";
        else file << "},\n";
        file << "    slots = {";
        for (u8 slot : armor.materia_slots) file << static_cast<int>(slot) << ", ";
        file << "},\n    stats = {";
        for (StatBonus s : armor.stat_bonus){
            file << "\n        " << GetBasicStatName(s.stat) << " = "
              << static_cast<int>(s.stat) << ",";
        }
        if (armor.stat_bonus.size() > 0) file << "\n    },\n";
        else file << "},\n";
        file << "    users = {";
        for (int s : armor.equip) file << s << ", ";
        file << "}\n}\n";
    }
    file.close();
}

int KernelDataInstaller::ReadAccessories(){

    // Empty the item list
    accessories_.clear();

    File accessory_file = kernel_.ExtractGZip(KERNEL_ACCESSORY_DATA);
    File accessory_name_file = kernel_.ExtractGZip(KERNEL_ACCESSORY_NAMES);
    File accessory_desc_file = kernel_.ExtractGZip(KERNEL_ACCESSORY_DESCRIPTIONS);
    int accessory_count = 0;

    for (int i = 0; i < 32; i ++){ // There are exactly 32.

        AccessoryData data;
        int name_offset;
        int desc_offset;

        // From the current name file offset, read 16 bytes.
        // This is the pointer to the offset for the item name.
        // Start reading there until 0xFF is found.
        data.name = "";
        u8 ch = 1;
        name_offset = accessory_name_file.readU16LE();
        while (ch != 0xFF){
            ch = accessory_name_file.GetU8(name_offset);
            if (ch != 0xFF) data.name += ENGLISH_CHARS[ch];
            name_offset ++;
        }
        boost::replace_all(data.name, "\"", "'");

        // For descriptions, do the same as for names.
        data.description = "";
        ch = 1;
        desc_offset = accessory_desc_file.readU16LE();
        while (ch != 0xFF){
            ch = accessory_desc_file.GetU8(desc_offset);
            if (ch != 0xFF){
                // Warning: If the char is 0xF9, special function!
                // the next byte (in bits, aaoooooo) means to read aa data at offset oooooo.
                // This is to compress information in the kernel.
                if (ch == 0xF9){
                    // Get how much to read, two upper bits of the next byte.
                    // Multiply by 2 and add 4. That's just how it is.
                    u8 comp_size = (accessory_desc_file.GetU8(desc_offset + 1) >> 6) * 2 + 4;
                    // Get new offset, lower six bits of the next byte.
                    // The offset goes backwards from the position of the 0xF9 byte.
                    u8 comp_offset = accessory_desc_file.GetU8(desc_offset + 1) & 63;

                    for (int c = 0; c < comp_size; c ++){
                        data.description += ENGLISH_CHARS[
                          accessory_desc_file.GetU8(desc_offset - 1 - comp_offset + c)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }
        boost::replace_all(data.description, "\"", "'");

        // If the item doesn't have a name, it means all items have been read.
        // break the loop.
        if ("" == data.name) break;

        // Read data from the accessory data section.
        for (int s = 0; s < 2; s ++) data.stat_raw[s] = accessory_file.readU8();
        for (int s = 0; s < 2; s ++) data.stat_bonus_raw[s] = accessory_file.readU8();
        data.element_defense_mode = accessory_file.readU8();
        data.effect = accessory_file.readU8();
        data.element_raw = accessory_file.readU16LE();
        data.status_raw = accessory_file.readU32LE();
        data.equip_raw = accessory_file.readU16LE();
        data.restrict_raw = accessory_file.readU16LE();

        // Derive data from the read data.
        data.id = i;
        data.sellable = !(data.restrict_raw & (1 << 0));
        data.useable_battle = !(data.restrict_raw & (2 << 0));
        data.useable_menu = !(data.restrict_raw & (4 << 0));
        if (data.status_raw == (data.status_raw | 0x00000001)) data.status.push_back(DEATH);
        if (data.status_raw == (data.status_raw | 0x00000002)) data.status.push_back(NEAR_DEATH);
        if (data.status_raw == (data.status_raw | 0x00000004)) data.status.push_back(SLEEP);
        if (data.status_raw == (data.status_raw | 0x00000008)) data.status.push_back(POISONED);
        if (data.status_raw == (data.status_raw | 0x00000010)) data.status.push_back(SADNESS);
        if (data.status_raw == (data.status_raw | 0x00000020)) data.status.push_back(FURY);
        if (data.status_raw == (data.status_raw | 0x00000040)) data.status.push_back(CONFU);
        if (data.status_raw == (data.status_raw | 0x00000080)) data.status.push_back(SILENCE);
        if (data.status_raw == (data.status_raw | 0x00000100)) data.status.push_back(HASTE);
        if (data.status_raw == (data.status_raw | 0x00000200)) data.status.push_back(SLOW);
        if (data.status_raw == (data.status_raw | 0x00000400)) data.status.push_back(STOP);
        if (data.status_raw == (data.status_raw | 0x00000800)) data.status.push_back(FROG);
        if (data.status_raw == (data.status_raw | 0x00001000)) data.status.push_back(SMALL);
        if (data.status_raw == (data.status_raw | 0x00002000)) data.status.push_back(SLOW_NUMB);
        if (data.status_raw == (data.status_raw | 0x00004000)) data.status.push_back(PETRIFY);
        if (data.status_raw == (data.status_raw | 0x00008000)) data.status.push_back(REGEN);
        if (data.status_raw == (data.status_raw | 0x00010000)) data.status.push_back(BARRIER);
        if (data.status_raw == (data.status_raw | 0x00020000)) data.status.push_back(M_BARRIER);
        if (data.status_raw == (data.status_raw | 0x00040000)) data.status.push_back(REFLECT);
        if (data.status_raw == (data.status_raw | 0x00080000)) data.status.push_back(DUAL);
        if (data.status_raw == (data.status_raw | 0x00100000)) data.status.push_back(SHIELD);
        if (data.status_raw == (data.status_raw | 0x00200000)) data.status.push_back(D_SENTENCE);
        if (data.status_raw == (data.status_raw | 0x00400000)) data.status.push_back(MANIPULATE);
        if (data.status_raw == (data.status_raw | 0x00800000)) data.status.push_back(BERSERK);
        if (data.status_raw == (data.status_raw | 0x01000000)) data.status.push_back(PEERLESS);
        if (data.status_raw == (data.status_raw | 0x02000000)) data.status.push_back(PARALYSIS);
        if (data.status_raw == (data.status_raw | 0x04000000)) data.status.push_back(DARKNESS);
        if (data.status_raw == (data.status_raw | 0x08000000)) data.status.push_back(DUAL_DRAIN);
        if (data.status_raw == (data.status_raw | 0x10000000)) data.status.push_back(DEATH_FORCE);
        if (data.status_raw == (data.status_raw | 0x20000000)) data.status.push_back(RESIST);
        if (data.status_raw == (data.status_raw | 0x40000000)) data.status.push_back(LUCKY_GIRL);
        if (data.status_raw == (data.status_raw | 0x80000000)) data.status.push_back(IMPRISONED);
        if (data.equip_raw == (data.equip_raw | 0x0001)) data.equip.push_back(0);
        if (data.equip_raw == (data.equip_raw | 0x0002)) data.equip.push_back(1);
        if (data.equip_raw == (data.equip_raw | 0x0004)) data.equip.push_back(2);
        if (data.equip_raw == (data.equip_raw | 0x0008)) data.equip.push_back(3);
        if (data.equip_raw == (data.equip_raw | 0x0010)) data.equip.push_back(4);
        if (data.equip_raw == (data.equip_raw | 0x0020)) data.equip.push_back(5);
        if (data.equip_raw == (data.equip_raw | 0x0040)) data.equip.push_back(6);
        if (data.equip_raw == (data.equip_raw | 0x0080)) data.equip.push_back(7);
        if (data.equip_raw == (data.equip_raw | 0x0100)) data.equip.push_back(8);
        if (data.equip_raw == (data.equip_raw | 0x0200)) data.equip.push_back(9);
        if (data.equip_raw == (data.equip_raw | 0x0400)) data.equip.push_back(10);
        for (int s = 0; s < 2; s ++){
            if (data.stat_raw[s] < 0XFF && data.stat_bonus_raw[s] > 0){
                StatBonus stat_bonus;
                stat_bonus.stat = data.stat_raw[s];
                stat_bonus.bonus = data.stat_bonus_raw[s];
                data.stat_bonus.push_back(stat_bonus);
            }
        }
        // In KERNEL.BIN, items with no elemental defense actually do have all elements marked,
        // with mode 255. If that's the case, consider that there are no elements.
        if (data.element_defense_mode != 0xFF){
            if (data.element_raw == (data.element_raw | 0x0001)) data.elements.push_back(FIRE);
            if (data.element_raw == (data.element_raw | 0x0002)) data.elements.push_back(ICE);
            if (data.element_raw == (data.element_raw | 0x0004)) data.elements.push_back(BOLT);
            if (data.element_raw == (data.element_raw | 0x0008)) data.elements.push_back(EARTH);
            if (data.element_raw == (data.element_raw | 0x0010)) data.elements.push_back(POISON);
            if (data.element_raw == (data.element_raw | 0x0020)) data.elements.push_back(GRAVITY);
            if (data.element_raw == (data.element_raw | 0x0040)) data.elements.push_back(WATER);
            if (data.element_raw == (data.element_raw | 0x0080)) data.elements.push_back(WIND);
            if (data.element_raw == (data.element_raw | 0x0100)) data.elements.push_back(HOLY);
            if (data.element_raw == (data.element_raw | 0x0200))
                data.elements.push_back(RESTORATIVE);
            if (data.element_raw == (data.element_raw | 0x0400)) data.elements.push_back(CUT);
            if (data.element_raw == (data.element_raw | 0x0800)) data.elements.push_back(HIT);
            if (data.element_raw == (data.element_raw | 0x1000)) data.elements.push_back(PUNCH);
            if (data.element_raw == (data.element_raw | 0x2000)) data.elements.push_back(SHOOT);
            if (data.element_raw == (data.element_raw | 0x4000)) data.elements.push_back(SHOUT);
            if (data.element_raw == (data.element_raw | 0x8000)) data.elements.push_back(HIDDEN);
        }
        // In KERNEL.BIN, Non elemental item actually have all elements marked.
        // If that happens, clear them all.
        if (data.elements.size() == 16) data.elements.clear();

        // Add to the list of weapons.
        accessories_.push_back(data);
        accessory_count ++;
    }

    return accessory_count;
}

void KernelDataInstaller::WriteAccessories(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (AccessoryData accessory : accessories_){
        file << "Game.Items[" << accessory.id + 287 << "] = {\n"
          << "    type = Inventory.ITEM_TYPE.ACCESSORY,\n"
          << "    id = " << accessory.id + 287 << ",\n"
          << "    price = " << prices_[accessory.id + 287] << ",\n"
          << "    name = \"" << accessory.name << "\",\n"
          << "    description = \"" << accessory.description << "\",\n"
          << "    sell = " << accessory.sellable << ",\n"
          << "    battle = " << accessory.useable_battle << ",\n"
          << "    menu = " << accessory.useable_menu << ",\n"
          << "    throw = 0,\n"
          << "    special_effect = {";
        if (accessory.effect != 0XFF) file << static_cast<int>(accessory.effect);
        file
          << "},\n"
          << "    status = {";
        for (int s : accessory.status) file << s << ", ";
        file
          << "},\n"
          << "    elements = {";
        for (int s : accessory.elements) file << s << ", ";
        file << "},\n    stats = {";
        for (StatBonus s : accessory.stat_bonus){
            file << "\n        " << GetBasicStatName(s.stat) << " = "
              << static_cast<int>(s.stat) << ",";
        }
        if (accessory.stat_bonus.size() > 0) file << "\n    },\n";
        else file << "},\n";
        file << "    users = {";
        for (int s : accessory.equip) file << s << ", ";
        file << "}\n}\n";
    }
    file.close();
}

int KernelDataInstaller::ReadMateria(){

    // Empty the item list
    materia_.clear();

    File materia_file = kernel_.ExtractGZip(KERNEL_MATERIA_DATA);
    File materia_name_file = kernel_.ExtractGZip(KERNEL_MATERIA_NAMES);
    File materia_desc_file = kernel_.ExtractGZip(KERNEL_MATERIA_DESCRIPTIONS);
    int materia_count = 0;

    for (int i = 0; i < 96; i ++){ // There are exactly 96.

        MateriaData data;
        int name_offset;
        int desc_offset;

        // From the current name file offset, read 16 bytes.
        // This is the pointer to the offset for the item name.
        // Start reading there until 0xFF is found.
        data.name = "";
        u8 ch = 1;
        name_offset = materia_name_file.readU16LE();
        while (ch != 0xFF){
            ch = materia_name_file.GetU8(name_offset);
            if (ch != 0xFF) data.name += ENGLISH_CHARS[ch];
            name_offset ++;
        }
        boost::replace_all(data.name, "\"", "'");

        // For descriptions, do the same as for names.
        data.description = "";
        ch = 1;
        desc_offset = materia_desc_file.readU16LE();
        while (ch != 0xFF){
            ch = materia_desc_file.GetU8(desc_offset);
            if (ch != 0xFF){
                // Warning: If the char is 0xF9, special function!
                // the next byte (in bits, aaoooooo) means to read aa data at offset oooooo.
                // This is to compress information in the kernel.
                if (ch == 0xF9){
                    // Get how much to read, two upper bits of the next byte.
                    // Multiply by 2 and add 4. That's just how it is.
                    u8 comp_size = (materia_desc_file.GetU8(desc_offset + 1) >> 6) * 2 + 4;
                    // Get new offset, lower six bits of the next byte.
                    // The offset goes backwards from the position of the 0xF9 byte.
                    u8 comp_offset = materia_desc_file.GetU8(desc_offset + 1) & 63;

                    for (int c = 0; c < comp_size; c ++){
                        data.description += ENGLISH_CHARS[
                          materia_desc_file.GetU8(desc_offset - 1 - comp_offset + c)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }
        boost::replace_all(data.description, "\"", "'");

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
        if (data.status_raw == (data.status_raw | 0x000001)) data.status.push_back(DEATH);
        if (data.status_raw == (data.status_raw | 0x000002)) data.status.push_back(NEAR_DEATH);
        if (data.status_raw == (data.status_raw | 0x000004)) data.status.push_back(SLEEP);
        if (data.status_raw == (data.status_raw | 0x000008)) data.status.push_back(POISONED);
        if (data.status_raw == (data.status_raw | 0x000010)) data.status.push_back(SADNESS);
        if (data.status_raw == (data.status_raw | 0x000020)) data.status.push_back(FURY);
        if (data.status_raw == (data.status_raw | 0x000040)) data.status.push_back(CONFU);
        if (data.status_raw == (data.status_raw | 0x000080)) data.status.push_back(SILENCE);
        if (data.status_raw == (data.status_raw | 0x000100)) data.status.push_back(HASTE);
        if (data.status_raw == (data.status_raw | 0x000200)) data.status.push_back(SLOW);
        if (data.status_raw == (data.status_raw | 0x000400)) data.status.push_back(STOP);
        if (data.status_raw == (data.status_raw | 0x000800)) data.status.push_back(FROG);
        if (data.status_raw == (data.status_raw | 0x001000)) data.status.push_back(SMALL);
        if (data.status_raw == (data.status_raw | 0x002000)) data.status.push_back(SLOW_NUMB);
        if (data.status_raw == (data.status_raw | 0x004000)) data.status.push_back(PETRIFY);
        if (data.status_raw == (data.status_raw | 0x008000)) data.status.push_back(REGEN);
        if (data.status_raw == (data.status_raw | 0x010000)) data.status.push_back(BARRIER);
        if (data.status_raw == (data.status_raw | 0x020000)) data.status.push_back(M_BARRIER);
        if (data.status_raw == (data.status_raw | 0x040000)) data.status.push_back(REFLECT);
        if (data.status_raw == (data.status_raw | 0x080000)) data.status.push_back(DUAL);
        if (data.status_raw == (data.status_raw | 0x100000)) data.status.push_back(SHIELD);
        if (data.status_raw == (data.status_raw | 0x200000)) data.status.push_back(D_SENTENCE);
        if (data.status_raw == (data.status_raw | 0x400000)) data.status.push_back(MANIPULATE);
        if (data.status_raw == (data.status_raw | 0x800000)) data.status.push_back(BERSERK);
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
        // Stat bonuses. Manually done.
        data.stats.str = 0;
        data.stats.vit = 0;
        data.stats.mag = 0;
        data.stats.spr = 0;
        data.stats.dex = 0;
        data.stats.lck = 0;
        data.stats.hp = 0;
        data.stats.mp = 0;
        data.stats.change = false;
        switch (data.stat_raw){
            case 0x01:
                data.stats.change = true;
                data.stats.str = -2;
                data.stats.vit = -1;
                data.stats.mag = 2;
                data.stats.spr = 1;
                data.stats.dex = 0;
                data.stats.lck = 0;
                data.stats.hp = -5;
                data.stats.mp = 5;
                break;
            case 0x02:
                data.stats.change = true;
                data.stats.str = -4;
                data.stats.vit = -2;
                data.stats.mag = 4;
                data.stats.spr = 2;
                data.stats.hp = -10;
                data.stats.mp = 10;
                break;
            case 0x03:
                data.stats.change = true;
                data.stats.dex = 2;
                data.stats.lck = -2;
                break;
            case 0x04:
                data.stats.change = true;
                data.stats.str = -1;
                data.stats.vit = -1;
                data.stats.mag = 1;
                data.stats.spr = 1;
                break;
            case 0x05:
                data.stats.change = true;
                data.stats.str = 1;
                data.stats.vit = 1;
                data.stats.mag = -1;
                data.stats.spr = -1;
                break;
            case 0x06:
                data.stats.change = true;
                data.stats.vit = 1;
                break;
            case 0x07:
                data.stats.change = true;
                data.stats.lck = 1;
                break;
            case 0x08:
                data.stats.change = true;
                data.stats.lck = -1;
                break;
            case 0x09:
                data.stats.change = true;
                data.stats.dex = -2;
                break;
            case 0x0A:
                data.stats.change = true;
                data.stats.dex = 2;
                break;
            case 0x0B:
                data.stats.change = true;
                data.stats.str = -1;
                data.stats.mag = 1;
                data.stats.hp = -2;
                data.stats.mp = 2;
                break;
            case 0x0C:
                data.stats.change = true;
                data.stats.mag = 1;
                data.stats.hp = -2;
                data.stats.mp = 2;
                break;
            case 0x0D:
                data.stats.change = true;
                data.stats.mag = 1;
                data.stats.spr = 1;
                data.stats.hp = -5;
                data.stats.mp = 5;
                break;
            case 0x0E:
                data.stats.change = true;
                data.stats.mag = 2;
                data.stats.spr = 2;
                data.stats.hp = -10;
                data.stats.mp = 10;
                break;
            case 0x0F:
                data.stats.change = true;
                data.stats.mag = 4;
                data.stats.spr = 4;
                data.stats.hp = -10;
                data.stats.mp = 15;
                break;
            case 0x10:
                data.stats.change = true;
                data.stats.mag = 8;
                data.stats.spr = 8;
                data.stats.hp = -10;
                data.stats.mp = 20;
                break;
        }

        // Attributtes. Very hardcoded!
        /*u8 sub_type = data.type_raw >> 4;
        u8 type = data.type_raw & 15;
        switch (type){
            case 0x09: // Magic, normal.
            case 0x0A: // Magic, high tier.
                // Always enable magic command.
                // Each attribute is an attack unlocked each level.
                data.command.push_back(2);
                for (int a = 0; a < 6; a ++){
                    if (data.attribute[i] == 0xFF) data.attack[i] = -1;
                    else data.attack[i] = data.attribute[i];
                    data.attack_times[i] = -1;
                }
                break;
            case 0x05: //Support materia.
                    switch (data.attribute[i]){
                        case 0x54: data.ability = "Materia.ABILITY.COMMAND_COUNTER"; break;
                        case 0x55: data.ability = "Materia.ABILITY.MAGIC_COUNTER"; break;
                        case 0x56: data.ability = "Materia.ABILITY.SNEAK_ATTACK"; break;
                        case 0x58: data.ability = "Materia.ABILITY.MP_TURBO"; break;
                        case 0x59: data.ability = "Materia.ABILITY.HP_ABSORB"; break;
                        case 0x5A: data.ability = "Materia.ABILITY.MP_ABSORB"; break;
                        case 0x5C: data.ability = "Materia.ABILITY.ADDED_CUT"; break;
                        case 0x5D: data.ability = "Materia.ABILITY.STEAL_AS_WELL"; break;
                        case 0x5E: data.ability = "Materia.ABILITY.ELEMENTAL"; break;
                        case 0x5F: data.ability = "Materia.ABILITY.ADDED_EFFECT"; break;
                    }
                    for (int a = 1; a < 6; a ++){
                        if (data.attribute[i] != 0xFF)
                            data.ability_parameters[a - 1] = data.attribute[i];
                    }
                break;
            case 0x02: // Command, non-existent.
                for (int a = 0; a < 6; a ++)
                    if (data.attribute[i] != 0xFF) data.command.push_back(data.attribute[i]);
                break;
            case 0x03: // Command, W commands.
                for (int a = 0; a < 6; a ++)
                    if (data.attribute[i] != 0xFF) data.command.push_back(data.attribute[i]);
                break;
            case 0x06: // Command, replace attack, different by level.
                for (int a = 0; a < 6; a ++)
                    if (data.attribute[i] != 0xFF) data.command.push_back(data.attribute[i]);
                break;
            case 0x07: // E. Skill
                data.command.push_back(13); break; // E.Skill.
                break;
            case 0x08: // Adds commands. Can be different by level.
                for (int a = 0; a < 6; a ++)
                    if (data.attribute[i] != 0xFF) data.command.push_back(data.attribute[i]);
                break;
            case 0x0B: // Summon, normal.
            case 0x0C: // Summon, high tier.
                // Always enable Summon command (3), always enable attack in attribute 0.
                // Attributes 1-5 are the times the summon can be used per battle. They are always
                // 1-5.
                data.command.push_back(3);
                //data.attack.push_back(data.attribute[i]);
                for (int a = 1; a <= 6; a ++){
                    data.attack_times[a - 1] = a;
                }
                break;
            //default:
                //data.type = static_cast<int>(MATERIA_TYPE::INDEPENDENT);
        }*/

        // Add to the list of materia.
        materia_.push_back(data);
        materia_count ++;
    }

    return materia_count;
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
        if (materia.type == static_cast<int>(MATERIA_TYPE::MAGIC)){
            file << "    magic = {\n";
            for (int a = 0; a < 6; a ++){
                if (materia.attribute[a] != 0xFF){
                    file << "        [" << a + 1 << "] = " << static_cast<int>(materia.attribute[a])
                      << ", -- " << attack_names_[static_cast<int>(materia.attribute[a])] << "\n";
                }
            }
            file << "    }\n";
        }
        else if (materia.type == static_cast<int>(MATERIA_TYPE::SUMMON)){
            file << "    summon = " << static_cast<int>(materia.attribute[0]) << ", -- "
              << attack_names_[static_cast<int>(materia.attribute[0])] << "\n";
            file << "    times = {";
            for (int a = 1; a < 6; a ++)
                file << "[" << a << "] = " << static_cast<int>(materia.attribute[a]) << ", ";
            file << "}\n";
        }
        else if (materia.type == static_cast<int>(MATERIA_TYPE::COMMAND)){
            switch (materia.sub_type){
                case 0x02: // Command, replace attack.
                    file << "    command = {\n";
                    for (int a = 0; a < 6; a ++){
                        if (materia.attribute[a] != 0xFF){
                            file << "        [" << a + 1 << "] = "
                              << static_cast<int>(materia.attribute[a]) << ", -- "
                              << command_names_[static_cast<int>(materia.attribute[a])] << "\n";
                        }
                    }
                    file << "    },\n    command_replace = 1 -- Attack\n";
                    break;
                case 0x03: // Command, W commands.
                    file << "    command = {\n";
                    for (int a = 0; a < 6; a ++){
                        if (materia.attribute[a] != 0xFF){
                            file << "        [" << a + 1 << "] = "
                              << static_cast<int>(materia.attribute[a]) << ", -- "
                              << command_names_[static_cast<int>(materia.attribute[a])] << "\n";
                        }
                    }
                    file << "    },\n    command_replace = "
                      << static_cast<int>(materia.attribute[0] - 19) << " -- "
                      << command_names_[static_cast<int>(materia.attribute[0] - 19)] << "\n";
                    break;
                case 0x06: // Normal command.
                case 0x07: // E.Skill.
                    file << "    command = {\n";
                        for (int a = 0; a < 6; a ++){
                            if (materia.attribute[a] != 0xFF){
                                file << "        [" << a + 1 << "] = "
                                  << static_cast<int>(materia.attribute[a]) << ", -- "
                                  << command_names_[static_cast<int>(materia.attribute[a])] << "\n";
                            }
                        }
                        file << "    }\n";
                    break;
                case 0x08: // Master command.
                    // TODO
                    break;
            }
        }
        else if (materia.type == static_cast<int>(MATERIA_TYPE::SUPPORT)){
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
        else if (materia.type == static_cast<int>(MATERIA_TYPE::INDEPENDENT)){
            bool invert_attrs = false;
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
                            if (materia.id == 7)
                                file << "Materia.ABILITY.ENCOUNTER_DOWN,\n";
                            else if (materia.id == 8)
                                file << "Materia.ABILITY.ENCOUNTER_UP,\n";
                            else if (materia.id == 9)
                                file << "Materia.ABILITY.ENCOUNTER_CHOCOBO,\n";
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
                        file << "[" << a << "] = "
                          << static_cast<int>(materia.attribute[a]) << ", ";
                    }
                }
                file << "}\n    }\n";
        }
        file << "}\n";
    }
    file.close();
}

int KernelDataInstaller::ReadKeyItems(){

    // Empty the item list
    key_items_.clear();

    File key_items_name_file = kernel_.ExtractGZip(KERNEL_KEY_ITEM_NAMES);
    File key_items_desc_file = kernel_.ExtractGZip(KERNEL_KEY_ITEM_DESCRIPTIONS);
    int key_item_count = 0;

    for (int i = 0; i < 128; i ++){

        KeyItemData data;
        int name_offset;
        int desc_offset;

        // From the current name file offset, read 16 bytes.
        // This is the pointer to the offset for the item name.
        // Start reading there until 0xFF is found.
        data.name = "";
        u8 ch = 1;
        name_offset = key_items_name_file.readU16LE();
        while (ch != 0xFF){
            ch = key_items_name_file.GetU8(name_offset);
            if (ch != 0xFF) data.name += ENGLISH_CHARS[ch];
            name_offset ++;
        }
        boost::replace_all(data.name, "\"", "'");

        // For descriptions, do the same as for names.
        data.description = "";
        ch = 1;
        desc_offset = key_items_desc_file.readU16LE();
        while (ch != 0xFF){
            ch = key_items_desc_file.GetU8(desc_offset);
            if (ch != 0xFF){
                // Warning: If the char is 0xF9, special function!
                // the next byte (in bits, aaoooooo) means to read aa data at offset oooooo.
                // This is to compress information in the kernel.
                if (ch == 0xF9){
                    // Get how much to read, two upper bits of the next byte.
                    // Multiply by 2 and add 4. That's just how it is.
                    u8 comp_size = (key_items_desc_file.GetU8(desc_offset + 1) >> 6) * 2 + 4;
                    // Get new offset, lower six bits of the next byte.
                    // The offset goes backwards from the position of the 0xF9 byte.
                    u8 comp_offset = key_items_desc_file.GetU8(desc_offset + 1) & 63;

                    for (int i = 0; i < comp_size; i ++){
                        data.description += ENGLISH_CHARS[
                          key_items_desc_file.GetU8(desc_offset - 1 - comp_offset + i)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }
        boost::replace_all(data.description, "\"", "'");

        // If the key item doesn't have a name, it means all key items have been read.
        // break the loop.
        if ("" == data.name) break;

        // Add to the list of items.
        data.id = i;
        key_items_.push_back(data);
        key_item_count ++;
    }

    return key_item_count;
}

void KernelDataInstaller::WriteKeyItems(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (KeyItemData key_item : key_items_){
        file << "Game.Key_Items[" << key_item.id << "] = {\n"
          << "    name = \"" << key_item.name << "\",\n"
          << "    description = \"" << key_item.description << "\"\n}\n";
    }
    file.close();
}

int KernelDataInstaller::ReadSummonNames(){

    // Empty the summon list
    key_items_.clear();

    File summon_name_file = kernel_.ExtractGZip(KERNEL_SUMMON_ATTACK_NAMES);
    int summon_count = 0;

    for (int i = 0; i < 16; i ++){ // There are exactly 16.

        SummonNameData data;
        int name_offset;

        // From the current name file offset, read 16 bytes.
        // This is the pointer to the offset for the item name.
        // Start reading there until 0xFF is found.
        data.name = "";
        u8 ch = 1;
        name_offset = summon_name_file.readU16LE();
        while (ch != 0xFF){
            ch = summon_name_file.GetU8(name_offset);
            if (ch != 0xFF) data.name += ENGLISH_CHARS[ch];
            name_offset ++;
        }
        boost::replace_all(data.name, "\"", "'");

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

void KernelDataInstaller::ReadInitialSaveMap(){

    // Empty the item list
    attacks_.clear();

    File savemap_file = kernel_.ExtractGZip(KERNEL_INITIALIZATION_DATA);
    savemap_file.WriteFile("/home/ivalentin/initialsavemap");
    // The initial savemap in KERNEL.BIN startas at 0x0054, directly into Cloud's level.
    // It skips the header (PC and PSX, checksum, preview data and window colour).
    for (int i = 0; i < 9; i ++){
        savemap_.characters[i].identifier = savemap_file.readU8();
        savemap_.characters[i].level = savemap_file.readU8();
        savemap_.characters[i].str = savemap_file.readU8();
        savemap_.characters[i].vit = savemap_file.readU8();
        savemap_.characters[i].mag = savemap_file.readU8();
        savemap_.characters[i].spr = savemap_file.readU8();
        savemap_.characters[i].dex = savemap_file.readU8();
        savemap_.characters[i].lck = savemap_file.readU8();
        savemap_.characters[i].str_bonus = savemap_file.readU8();
        savemap_.characters[i].vit_bonus = savemap_file.readU8();
        savemap_.characters[i].mag_bonus = savemap_file.readU8();
        savemap_.characters[i].spr_bonus = savemap_file.readU8();
        savemap_.characters[i].dex_bonus = savemap_file.readU8();
        savemap_.characters[i].lck_bonus = savemap_file.readU8();
        savemap_.characters[i].limit_level = savemap_file.readU8();
        savemap_.characters[i].limit_bar = savemap_file.readU8();
        bool name_end = false;
        savemap_.characters[i].name = "";
        for (int j = 0; j < 12; j ++){
            savemap_.characters[i].name_raw[i] = savemap_file.readU8();
            if (name_end == false){
                if (savemap_.characters[i].name_raw[i] != 0XFF){
                    savemap_.characters[i].name
                      += ENGLISH_CHARS[savemap_.characters[i].name_raw[i]];
                }
                else name_end = true;
            }
        }
        savemap_.characters[i].weapon = savemap_file.readU8();
        savemap_.characters[i].armor = savemap_file.readU8();
        savemap_.characters[i].accessory = savemap_file.readU8();
        savemap_.characters[i].status_raw = savemap_file.readU8();
        if (savemap_.characters[i].status_raw == (savemap_.characters[i].status_raw | 0x10)){
            savemap_.characters[i].sadness = true;
            savemap_.characters[i].fury = false;
        }
        else if (savemap_.characters[i].status_raw == (savemap_.characters[i].status_raw | 0x20)){
            savemap_.characters[i].sadness = false;
            savemap_.characters[i].fury = true;
        }
        else{
            savemap_.characters[i].sadness = false;
            savemap_.characters[i].fury = false;
        }
        savemap_.characters[i].row_raw = savemap_file.readU8();
        if (savemap_.characters[i].row_raw == 0xFE) savemap_.characters[i].back_row = true;
        else savemap_.characters[i].back_row = false;
        savemap_.characters[i].level_progress = savemap_file.readU8();
        savemap_.characters[i].learned_limits_raw = savemap_file.readU16LE();
        for (int j = 0; j < 4; j ++){
            savemap_.characters[i].limits_learned[j][0] = false;
            savemap_.characters[i].limits_learned[j][1] = false;
        }
        if (savemap_.characters[i].learned_limits_raw == (savemap_.characters[i].learned_limits_raw | 0x0001))
            savemap_.characters[i].limits_learned[0][0] = true;
        if (savemap_.characters[i].learned_limits_raw == (savemap_.characters[i].learned_limits_raw | 0x0002))
            savemap_.characters[i].limits_learned[0][1] = true;
        if (savemap_.characters[i].learned_limits_raw == (savemap_.characters[i].learned_limits_raw | 0x0008))
            savemap_.characters[i].limits_learned[1][0] = true;
        if (savemap_.characters[i].learned_limits_raw == (savemap_.characters[i].learned_limits_raw | 0x0010))
            savemap_.characters[i].limits_learned[1][1] = true;
        if (savemap_.characters[i].learned_limits_raw == (savemap_.characters[i].learned_limits_raw | 0x0040))
            savemap_.characters[i].limits_learned[2][0] = true;
        if (savemap_.characters[i].learned_limits_raw == (savemap_.characters[i].learned_limits_raw | 0x0080))
            savemap_.characters[i].limits_learned[2][1] = true;
        if (savemap_.characters[i].learned_limits_raw == (savemap_.characters[i].learned_limits_raw | 0x0200))
            savemap_.characters[i].limits_learned[3][0] = true;
        savemap_.characters[i].kills = savemap_file.readU16LE();
        savemap_.characters[i].limit_uses[0] = savemap_file.readU16LE();
        savemap_.characters[i].limit_uses[1] = savemap_file.readU16LE();
        savemap_.characters[i].limit_uses[2] = savemap_file.readU16LE();
        savemap_.characters[i].hp = savemap_file.readU16LE();
        savemap_.characters[i].base_hp = savemap_file.readU16LE();
        savemap_.characters[i].mp = savemap_file.readU16LE();
        savemap_.characters[i].base_mp = savemap_file.readU16LE();
        savemap_.characters[i].unknown = savemap_file.readU32LE();
        savemap_.characters[i].max_hp = savemap_file.readU16LE();
        savemap_.characters[i].max_mp = savemap_file.readU16LE();
        savemap_.characters[i].exp = savemap_file.readU32LE();
        for (int m = 0; m < 8; m ++){
            savemap_.characters[i].weapon_materia[m].id = savemap_file.readU8();
            // TODO: Identify Enemy Skill, do special treatment
            savemap_.characters[i].weapon_materia[m].ap // WARNING: AP has only three bytes.
              = savemap_file.readU8()
              + (savemap_file.readU8() << 8)
              + (savemap_file.readU8() << 16);
        }
        for (int m = 0; m < 8; m ++){
            savemap_.characters[i].armor_materia[m].id = savemap_file.readU8();
            // TODO: Identify Enemy Skill, do special treatment
            savemap_.characters[i].armor_materia[m].ap // WARNING: AP has only three bytes.
              = savemap_file.readU8()
              + (savemap_file.readU8() << 8)
              + (savemap_file.readU8() << 16);
        }
        savemap_.characters[i].exp_to_next = savemap_file.readU32LE();
    }
    // The initial savemap ends up after character data, end now.
}

void KernelDataInstaller::WriteInitialSaveMap(std::string file_name){
    // The initial save map only contains character data.
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (int c = 0; c < 9; c ++){
        int total_count; // Utility to count in loops.
        SaveMap::Character character = savemap_.characters[c];
        file
          << "Characters[" << c << "] = {\n"
          << "    char_id = " << static_cast<int>(character.identifier) << ",\n"
          << "    name = \"" << character.name << "\",\n"
          << "    enabled = false,\n"
          << "    locked = false,\n"
          << "    level = " << static_cast<int>(character.level) << ",\n"
          << "    kills = " << character.kills << ",\n"
          << "    back_row = " << (character.back_row ? "true" : "false") << ",\n"
          << "    exp = " << character.exp << ",\n"
          << "    exp_to_next = " << character.exp_to_next << ",\n"
          << "    stats = {\n"
          << "        str = {base = " << static_cast<int>(character.str)
          << ", bonus = " << static_cast<int>(character.str_bonus)
          << ", total = " << static_cast<int>(character.str + character.str_bonus) << "},\n"
          << "        vit = {base = " << static_cast<int>(character.vit)
          << ", bonus = " << static_cast<int>(character.vit_bonus)
          << ", total = " << static_cast<int>(character.vit + character.vit_bonus) << "},\n"
          << "        mag = {base = " << static_cast<int>(character.mag)
          << ", bonus = " << static_cast<int>(character.mag_bonus)
          << ", total = " << static_cast<int>(character.mag + character.mag_bonus) << "},\n"
          << "        spr = {base = " << static_cast<int>(character.spr)
          << ", bonus = " << static_cast<int>(character.spr_bonus)
          << ", total = " << static_cast<int>(character.spr + character.spr_bonus) << "},\n"
          << "        dex = {base = " << static_cast<int>(character.dex)
          << ", bonus = " << static_cast<int>(character.dex_bonus)
          << ", total = " << static_cast<int>(character.dex + character.dex_bonus) << "},\n"
          << "        lck = {base = " << static_cast<int>(character.lck)
          << ", bonus = " << static_cast<int>(character.lck_bonus)
          << ", total = " << static_cast<int>(character.lck + character.lck_bonus) << "},\n"
          << "        hp = {current = " << character.hp  << ", base = " << character.base_hp
          << ", max = " << character.max_hp << "},\n"
          << "        mp = {current = " << character.mp  << ", base = " << character.base_mp
          << ", max = " << character.max_mp << "},\n"
          << "        atk = 0,\n"
          << "        acc = 0,\n"
          << "        def = 0,\n"
          << "        eva = 0,\n"
          << "        matk = 0,\n"
          << "        mdef = 0,\n"
          << "        meva = 0\n"
          << "    },\n"
          << "    limit = {\n"
          << "        current = " << static_cast<int>(character.limit_level) << ",\n"
          << "        bar = " << static_cast<int>(character.limit_bar) << ",\n"
          << "        learned = {\n"
          << "            [1] = {[1] = " << (character.limits_learned[0][0] == 1 ? "true" : "false")
          << ", [2] = " << (character.limits_learned[0][1] == 1 ? "true" : "false") << "},\n"
          << "            [2] = {[1] = " << (character.limits_learned[1][0] == 1 ? "true" : "false")
          << ", [2] = " << (character.limits_learned[1][1] == 1 ? "true" : "false") << "},\n"
          << "            [3] = {[1] = " << (character.limits_learned[2][0] == 1 ? "true" : "false")
          << ", [2] = " << (character.limits_learned[2][1] == 1 ? "true" : "false") << "},\n"
          << "            [4] = {[1] = " << (character.limits_learned[3][0] == 1 ? "true" : "false")
          << ", [2] = " << (character.limits_learned[3][1] == 1 ? "true" : "false") << "}\n"
          << "        },\n"
          << "        uses = {[1] = " << character.limit_uses[0] << ", [2] = "
          << character.limit_uses[1] << ", [3] = " << character.limit_uses[2] << ", [4] = 0}\n"
          << "    },\n"
          << "    weapon = {\n"
          << "        id = " << static_cast<int>(character.weapon) + 128 << ",\n"
          << "        materia = {";
        total_count = 0;
        for (int m = 0; m < 8; m ++){
            if (character.weapon_materia[m].id < 255){
                total_count ++;
                file << "\n            [" << m << "] = {id = "
                  << static_cast<int>(character.weapon_materia[m].id) << ", ap = "
                  << character.weapon_materia[m].ap << "},";
            }
        }
        if (total_count > 0) file << "\n        }\n    },\n";
        else file << "}\n    },\n";
        file
          << "    armor = {\n"
          << "        id = " << static_cast<int>(character.armor) + 256 << ",\n"
          << "        materia = {";
        total_count = 0;
        for (int m = 0; m < 8; m ++){
            if (character.armor_materia[m].id < 255){
                total_count ++;
                file << "\n            [" << m << "] = {id = "
                  << static_cast<int>(character.armor_materia[m].id) << ", ap = "
                  << character.armor_materia[m].ap << "},";
            }
        }
        if (total_count > 0) file << "\n        }\n    },\n";
        else file << "}\n    },\n";
        file << "    accessory = ";
        if (character.accessory < 0xFF) file << static_cast<int>(character.accessory) + 287;
        else file << "nil";
        file << ",\n    status = {";
        if (character.fury) file << FURY;
        if (character.sadness) file << SADNESS;
        file << "}\n}\nCharacters.recalculate_stats(" << c << ")\n\n";
    }
    file.close();
}

std::string KernelDataInstaller::GetBasicStatName(int id){
    std::string name = "stat";
    switch (id){
        case 0: name = "str"; break;
        case 1: name = "vit"; break;
        case 2: name = "mag"; break;
        case 3: name = "spr"; break;
        case 4: name = "dex"; break;
        case 5: name = "lck"; break;
    }
    return name;
}
