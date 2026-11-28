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

int KernelDataInstaller::ReadWeapons(){

    // Empty the item list
    weapons_.clear();

    File weapons_file = kernel_.ExtractGZip(KERNEL_WEAPON_DATA);
    KernelTextList names(kernel_.ExtractGZip(KERNEL_WEAPON_NAMES));
    KernelTextList descriptions(kernel_.ExtractGZip(KERNEL_WEAPON_DESCRIPTIONS));
    int weapon_count = 0;

    for (int i = 0; i < 128; i ++){

        WeaponData data;

        data.name = names.NextName();
        data.description = descriptions.NextDescription();

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
        ReadRestrictions(data);
        ReadTarget(data.target_raw, data.target);
        data.damage_formula = data.damage_raw >> 4; // Upper nybble.
        data.damage_modifier = data.damage_raw & 15; // Lower nybble.
        data.animation_index = data.model_raw >> 4; // Upper nybble.
        data.model = data.model_raw & 15; // Lower nybble.
        ReadUsers(data.equip_raw, data.equip);
        ReadStatBonuses(data.stat_raw, data.stat_bonus_raw, 4, data.stat_bonus);
        ReadMateriaSlots(data.slots_raw, data.materia_slots);
        ReadElements(data.element_raw, data.elements);

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
          << "    animation = " << static_cast<int>(weapon.animation_index) << ",\n";
        WriteTarget(file, weapon.target);
        file << "    elements = {";
        for (int s : weapon.elements) file << s << ", ";
        file << "},\n";
        file << "    slots = {";
        for (u8 slot : weapon.materia_slots) file << static_cast<int>(slot) << ", ";
        file << "},\n";
        WriteStatBonuses(file, weapon.stat_bonus);
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
    KernelTextList names(kernel_.ExtractGZip(KERNEL_ARMOR_NAMES));
    KernelTextList descriptions(kernel_.ExtractGZip(KERNEL_ARMOR_DESCRIPTIONS));
    int armor_count = 0;

    for (int i = 0; i < 32; i ++){ // There are exactly 32.

        ArmorData data;

        data.name = names.NextName();
        data.description = descriptions.NextDescription();

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
        ReadRestrictions(data);
        ReadUsers(data.equip_raw, data.equip);
        ReadStatBonuses(data.stat_raw, data.stat_bonus_raw, 4, data.stat_bonus);
        ReadMateriaSlots(data.slots_raw, data.materia_slots);
        // In KERNEL.BIN, records with no elemental defense actually have all elements marked,
        // with mode 255.
        if (data.element_defense_mode != 0xFF) ReadElements(data.element_raw, data.elements);

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
          << "    throw = " << armor.throwable << ",\n"
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
        file << "},\n";
        WriteStatBonuses(file, armor.stat_bonus);
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
    KernelTextList names(kernel_.ExtractGZip(KERNEL_ACCESSORY_NAMES));
    KernelTextList descriptions(kernel_.ExtractGZip(KERNEL_ACCESSORY_DESCRIPTIONS));
    int accessory_count = 0;

    for (int i = 0; i < 32; i ++){ // There are exactly 32.

        AccessoryData data;

        data.name = names.NextName();
        data.description = descriptions.NextDescription();

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
        ReadRestrictions(data);
        ReadStatuses(data.status_raw, data.status);
        ReadUsers(data.equip_raw, data.equip);
        ReadStatBonuses(data.stat_raw, data.stat_bonus_raw, 2, data.stat_bonus);
        // In KERNEL.BIN, records with no elemental defense actually have all elements marked,
        // with mode 255.
        if (data.element_defense_mode != 0xFF) ReadElements(data.element_raw, data.elements);

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
        file << "Game.Items[" << accessory.id + 288 << "] = {\n"
          << "    type = Inventory.ITEM_TYPE.ACCESSORY,\n"
          << "    id = " << accessory.id + 288 << ",\n"
          << "    price = " << prices_[accessory.id + 288] << ",\n"
          << "    name = \"" << accessory.name << "\",\n"
          << "    description = \"" << accessory.description << "\",\n"
          << "    sell = " << accessory.sellable << ",\n"
          << "    battle = " << accessory.useable_battle << ",\n"
          << "    menu = " << accessory.useable_menu << ",\n"
          << "    throw = " << accessory.throwable << ",\n"
          << "    special_effect = {";
        if (accessory.effect != 0XFF) file << static_cast<int>(accessory.effect);
        file
          << "},\n"
          << "    status = {";
        for (int s : accessory.status) file << s << ", ";
        file
          << "},\n"
          << "    elements = {";
        // Elements and their resistance mode (0 absorbs, 1 nullifies, 2 halves), as in armors.
        for (int s : accessory.elements){
            file << "\n        {id = " << s << ", mode = "
              << static_cast<int>(accessory.element_defense_mode) << "},";
        }
        if (accessory.elements.size() > 0) file << "\n    ";
        file << "},\n";
        WriteStatBonuses(file, accessory.stat_bonus);
        file << "    users = {";
        for (int s : accessory.equip) file << s << ", ";
        file << "}\n}\n";
    }
    file.close();
}
