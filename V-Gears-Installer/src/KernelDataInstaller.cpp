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
#include "KernelDataInstaller.h"

KernelDataInstaller::KernelDataInstaller(std::string path): kernel_(path){}

KernelDataInstaller::~KernelDataInstaller(){}

int KernelDataInstaller::ReadCommands(){

    // Empty the item list
    commands_.clear();

    File command_file = kernel_.ExtractGZip(KERNEL_COMMAND_DATA);
    File command_name_file = kernel_.ExtractGZip(KERNEL_COMMAND_NAMES);
    File command_desc_file = kernel_.ExtractGZip(KERNEL_COMMAND_DESCRIPTIONS);
    int name_offset = 0;
    int desc_offset = 0;
    int command_count = 0;

    std::cout << "|  ID | cur | c_s | c_m| tar |\n";

    // There are exactly 32 commands. Some of them are blank, so there is not really a
    // reliable exit condition.
    for (int i = 0; i < 32; i ++){

        CommandData data;

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

                    for (int i = 0; i < comp_size; i ++){
                        data.description += ENGLISH_CHARS[
                          command_desc_file.GetU8(desc_offset - 1 - comp_offset + i)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }

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
            commands_.push_back(data);
            command_count ++;
        }

        printf(
          "| %3d | %3d | %3d | %3d | ",
          i, data.camera_single, data.camera_multiple, data.target_raw
        );
        std::cout << data.name << ": " << data.description << std::endl;


    }

    return command_count;
}

void KernelDataInstaller::WriteCommands(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("commands"));
    for (CommandData command : commands_){
        std::unique_ptr<TiXmlElement> xml_command(new TiXmlElement("item"));
        xml_command->SetAttribute("id", command.id);
        xml_command->SetAttribute("name", command.name);
        xml_command->SetAttribute("description", command.description);
        xml_command->SetAttribute("camera_1", command.camera_single);
        xml_command->SetAttribute("camera_x", command.camera_single);
        container->LinkEndChild(xml_command.release());
    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
}

int KernelDataInstaller::ReadItems(){

    // Empty the item list
    items_.clear();

    File items_file = kernel_.ExtractGZip(KERNEL_ITEM_DATA);
    File items_name_file = kernel_.ExtractGZip(KERNEL_ITEM_NAMES);
    File items_desc_file = kernel_.ExtractGZip(KERNEL_ITEM_DESCRIPTIONS);
    int name_offset = 0;
    int desc_offset = 0;
    int item_count = 0;

    std::cout <<
      "|  ID | tar | eff | dmg | pow | con | sch | eff | efm | sta | elm |  spe  |\n";

    for (int i = 0; i < 128; i ++){

        ItemData data;

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

                    for (int i = 0; i < comp_size; i ++){
                        data.description += ENGLISH_CHARS[
                          items_desc_file.GetU8(desc_offset - 1 - comp_offset + i)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }

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
        // TODO: XOR, not individual bits
        /*
        data.target.selection_enabled = data.target_raw & (0 << 0); // First bit...
        data.target.default_enemy = data.target_raw & (1 << 0);
        data.target.default_multiple = data.target_raw & (2 << 0);
        data.target.toggle_multiple = data.target_raw & (3 << 0);
        data.target.fixed_row = data.target_raw & (4 << 0);
        data.target.short_range = data.target_raw & (5 << 0);
        data.target.all_rows = data.target_raw & (6 << 0);
        data.target.random = data.target_raw & (7 << 0); // ... last bit.
        */
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
        /*data.condition.party_hp = data.condition_raw & (0 << 0); //First bit...
        data.condition.mp = data.condition_raw & (1 << 0);
        data.condition.party_status = data.condition_raw & (2 << 0); // ... last bit.*/

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

        printf(
          "| %3d | %3d | %3d | %3d | %3d | %3d | %3d | %3d | %3d | %3d | %5d | ",
          i, data.target_raw, data.effect, data.damage_raw, data.power, data.condition_raw,
          data.status_change_raw, data.additional_effects_raw, data.additional_effects_mod_raw,
          data.status_raw, data.element_raw, data.special_raw
        );
        std::cout << data.name << ": " << data.description << std::endl;


    }

    return item_count;
}

void KernelDataInstaller::WriteItems(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("items"));
    for (ItemData item : items_){
        std::unique_ptr<TiXmlElement> xml_item(new TiXmlElement("item"));
        xml_item->SetAttribute("id", item.id);
        xml_item->SetAttribute("name", item.name);
        xml_item->SetAttribute("description", item.description);
        xml_item->SetAttribute("camera", item.camera);
        xml_item->SetAttribute("sell", item.sellable);
        xml_item->SetAttribute("battle", item.useable_battle);
        xml_item->SetAttribute("menu", item.useable_menu);
        xml_item->SetAttribute("effect", item.effect);
        xml_item->SetAttribute("dmg_formula", item.damage_formula);
        xml_item->SetAttribute("dmg_modifier", item.damage_modifier);
        xml_item->SetAttribute("power", item.power);
        xml_item->SetAttribute("restore", item.restore_type);
        xml_item->SetAttribute("target_select", item.target.selection_enabled);
        xml_item->SetAttribute("target_default_enemy", item.target.default_enemy);
        xml_item->SetAttribute("target_default_multiple", item.target.default_multiple);
        xml_item->SetAttribute("target_toggle_multiple", item.target.toggle_multiple);
        xml_item->SetAttribute("target_fixed", item.target.fixed_row);
        xml_item->SetAttribute("target_short_range", item.target.short_range);
        xml_item->SetAttribute("target_all", item.target.all_rows);
        xml_item->SetAttribute("target_random", item.target.random);
        // Statuses.
        if (item.status.status.size() > 0){
            std::unique_ptr<TiXmlElement> xml_status(new TiXmlElement("statuses"));
            xml_status->SetAttribute("mode", item.status.mode);
            xml_status->SetAttribute("chance", item.status.chance);
            for (int s : item.status.status){
                std::unique_ptr<TiXmlElement> xml_status_status(new TiXmlElement("status"));
                xml_status_status->SetAttribute("id", s);
                xml_status->LinkEndChild(xml_status_status.release());
            }
            xml_item->LinkEndChild(xml_status.release());
        }
        // Add elements.
        if (item.elements.size() > 0){
            std::unique_ptr<TiXmlElement> xml_elements(new TiXmlElement("elements"));
            for (int s : item.elements){
                std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("element"));
                xml_element->SetAttribute("id", s);
                xml_elements->LinkEndChild(xml_element.release());
            }
            xml_item->LinkEndChild(xml_elements.release());
        }

        container->LinkEndChild(xml_item.release());

    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
}
