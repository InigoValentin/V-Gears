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

int KernelDataInstaller::ReadItems(){

    // Empty the item list
    items_.clear();

    File items_file = kernel_.ExtractGZip(KERNEL_ITEM_DATA);
    KernelTextList names(kernel_.ExtractGZip(KERNEL_ITEM_NAMES));
    KernelTextList descriptions(kernel_.ExtractGZip(KERNEL_ITEM_DESCRIPTIONS));
    int item_count = 0;

    for (int i = 0; i < 128; i ++){

        ItemData data;

        data.name = names.NextName();
        data.description = descriptions.NextDescription();

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
        ReadRestrictions(data);
        ReadTarget(data.target_raw, data.target);
        data.damage_formula = data.damage_raw >> 4; // Upper nybble.
        data.damage_modifier = data.damage_raw & 15; // Lower nybble.
        data.restore_type = ReadRestoreType(data.condition_raw);
        ReadStatusEffect(data.status_change_raw, data.status_raw, data.status);

        ReadElements(data.element_raw, data.elements);

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
          << "    throw = " << item.throwable << ",\n"
          << "    effect = " << static_cast<int>(item.effect) << ",\n"
          << "    dmg_formula = " << static_cast<int>(item.damage_formula) << ",\n"
          << "    dmg_modifier = " << static_cast<int>(item.damage_modifier) << ",\n"
          << "    power = " << static_cast<int>(item.power) << ",\n"
          << "    restore = " << item.restore_type << ",\n";
        WriteTarget(file, item.target);
        file << "    status = {";
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

int KernelDataInstaller::ReadKeyItems(){

    // Empty the item list
    key_items_.clear();

    KernelTextList names(kernel_.ExtractGZip(KERNEL_KEY_ITEM_NAMES));
    KernelTextList descriptions(kernel_.ExtractGZip(KERNEL_KEY_ITEM_DESCRIPTIONS));
    int key_item_count = 0;

    for (int i = 0; i < 128; i ++){

        KeyItemData data;

        data.name = names.NextName();
        data.description = descriptions.NextDescription();

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
          << "    name = \"" << key_item.name << "\",\n description = \"" << key_item.description << "\"\n}\n";
    }
    file.close();
}
