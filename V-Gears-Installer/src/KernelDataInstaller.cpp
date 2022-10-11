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
#include "common/FinalFantasy7/FF7NameLookup.h"

KernelDataInstaller::KernelDataInstaller(std::string path): kernel_(path){
    for (int i = 0; i < 416; i ++) prices_[i] = 50;
}

KernelDataInstaller::~KernelDataInstaller(){}

void KernelDataInstaller::ReadPrices(std::string exe_path){
    File exe_file(exe_path);
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
    int name_offset = 0;
    int desc_offset = 0;
    int command_count = 0;

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

int KernelDataInstaller::ReadAttacks(){

    // Empty the item list
    attacks_.clear();

    File attacks_file = kernel_.ExtractGZip(KERNEL_ATTACK_DATA);
    int attack_count = 0;

    for (int i = 0; i < 128; i ++){

        AttackData data;

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
        attack_count ++;
    }

    return attack_count;
}

void KernelDataInstaller::WriteAttacks(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("attacks"));
    for (AttackData attack : attacks_){
        std::unique_ptr<TiXmlElement> xml_attack(new TiXmlElement("attack"));
        xml_attack->SetAttribute("id", attack.id);
        xml_attack->SetAttribute("accuracy", attack.accuracy);
        xml_attack->SetAttribute("impact_effect", attack.impact_effect);
        xml_attack->SetAttribute("hurt_anim", attack.hurt_anim);
        xml_attack->SetAttribute("mp", attack.mp);
        xml_attack->SetAttribute("sounds", attack.sound);
        xml_attack->SetAttribute("camera_1", attack.camera_single);
        xml_attack->SetAttribute("camera_x", attack.camera_multiple);
        xml_attack->SetAttribute("effect", attack.effect);
        xml_attack->SetAttribute("dmg_formula", attack.damage_formula);
        xml_attack->SetAttribute("dmg_modifier", attack.damage_modifier);
        xml_attack->SetAttribute("power", attack.power);
        xml_attack->SetAttribute("restore", attack.restore_type);
        xml_attack->SetAttribute("target_select", attack.target.selection_enabled);
        xml_attack->SetAttribute("target_default_enemy", attack.target.default_enemy);
        xml_attack->SetAttribute("target_default_multiple", attack.target.default_multiple);
        xml_attack->SetAttribute("target_toggle_multiple", attack.target.toggle_multiple);
        xml_attack->SetAttribute("target_fixed", attack.target.fixed_row);
        xml_attack->SetAttribute("target_short_range", attack.target.short_range);
        xml_attack->SetAttribute("target_all", attack.target.all_rows);
        xml_attack->SetAttribute("target_random", attack.target.random);
        // Statuses.
        if (attack.status.status.size() > 0){
            std::unique_ptr<TiXmlElement> xml_status(new TiXmlElement("statuses"));
            xml_status->SetAttribute("mode", attack.status.mode);
            xml_status->SetAttribute("chance", attack.status.chance);
            for (int s : attack.status.status){
                std::unique_ptr<TiXmlElement> xml_status_status(new TiXmlElement("status"));
                xml_status_status->SetAttribute("id", s);
                xml_status->LinkEndChild(xml_status_status.release());
            }
            xml_attack->LinkEndChild(xml_status.release());
        }
        // Add elements.
        if (attack.elements.size() > 0){
            std::unique_ptr<TiXmlElement> xml_elements(new TiXmlElement("elements"));
            for (int s : attack.elements){
                std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("element"));
                xml_element->SetAttribute("id", s);
                xml_elements->LinkEndChild(xml_element.release());
            }
            xml_attack->LinkEndChild(xml_elements.release());
        }

        container->LinkEndChild(xml_attack.release());

    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
}

int KernelDataInstaller::ReadCharacters(){

    // TODO: Do something about young Cloud and Sephiroth.

    // Empty the item list
    characters_.clear();

    File characters_file = kernel_.ExtractGZip(KERNEL_BATTLE_AND_GROWTH_DATA);
    int character_count = 0;

    for (int i = 0; i < 10; i ++){ // 9 characters

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

void KernelDataInstaller::WriteCharacters(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("attacks"));
    for (CharacterData character : characters_){
        std::unique_ptr<TiXmlElement> xml_character(new TiXmlElement("attack"));
        xml_character->SetAttribute("id", character.id);
        xml_character->SetAttribute("name", character.name);
        xml_character->SetAttribute("initial_level", character.initial_level);
        // Curves.
        std::unique_ptr<TiXmlElement> xml_curves(new TiXmlElement("curves"));
        std::unique_ptr<TiXmlElement> xml_curve_str(new TiXmlElement("curve"));
        xml_curve_str->SetAttribute("stat", "strength");
        xml_curve_str->SetAttribute("id", character.curve_str);
        xml_curves->LinkEndChild(xml_curve_str.release());
        std::unique_ptr<TiXmlElement> xml_curve_vit(new TiXmlElement("curve"));
        xml_curve_vit->SetAttribute("stat", "vitality");
        xml_curve_vit->SetAttribute("id", character.curve_vit);
        xml_curves->LinkEndChild(xml_curve_vit.release());
        std::unique_ptr<TiXmlElement> xml_curve_mag(new TiXmlElement("curve"));
        xml_curve_mag->SetAttribute("stat", "magic");
        xml_curve_mag->SetAttribute("id", character.curve_mag);
        xml_curves->LinkEndChild(xml_curve_mag.release());
        std::unique_ptr<TiXmlElement> xml_curve_spr(new TiXmlElement("curve"));
        xml_curve_spr->SetAttribute("stat", "spirit");
        xml_curve_spr->SetAttribute("id", character.curve_spr);
        xml_curves->LinkEndChild(xml_curve_spr.release());
        std::unique_ptr<TiXmlElement> xml_curve_dex(new TiXmlElement("curve"));
        xml_curve_dex->SetAttribute("stat", "dexterity");
        xml_curve_dex->SetAttribute("id", character.curve_dex);
        xml_curves->LinkEndChild(xml_curve_dex.release());
        std::unique_ptr<TiXmlElement> xml_curve_lck(new TiXmlElement("curve"));
        xml_curve_lck->SetAttribute("stat", "luck");
        xml_curve_lck->SetAttribute("id", character.curve_lck);
        xml_curves->LinkEndChild(xml_curve_lck.release());
        std::unique_ptr<TiXmlElement> xml_curve_hp(new TiXmlElement("curve"));
        xml_curve_hp->SetAttribute("stat", "hp");
        xml_curve_hp->SetAttribute("id", character.curve_hp);
        xml_curves->LinkEndChild(xml_curve_hp.release());
        std::unique_ptr<TiXmlElement> xml_curve_mp(new TiXmlElement("curve"));
        xml_curve_mp->SetAttribute("stat", "mp");
        xml_curve_mp->SetAttribute("id", character.curve_mp);
        xml_curves->LinkEndChild(xml_curve_mp.release());
        std::unique_ptr<TiXmlElement> xml_curve_exp(new TiXmlElement("curve"));
        xml_curve_exp->SetAttribute("stat", "experience");
        xml_curve_exp->SetAttribute("id", character.curve_exp);
        xml_curves->LinkEndChild(xml_curve_exp.release());
        xml_character->LinkEndChild(xml_curves.release());
        // Limits.
        std::unique_ptr<TiXmlElement> xml_limits(new TiXmlElement("limits"));
        // Limit level 1.
        std::unique_ptr<TiXmlElement> xml_limit_1(new TiXmlElement("level"));
        xml_limit_1->SetAttribute("level", 1);
        xml_limit_1->SetAttribute("kills", 0);
        xml_limit_1->SetAttribute("divisor", character.limit_1_div);
        std::unique_ptr<TiXmlElement> xml_limit_1_1(new TiXmlElement("limit"));
        xml_limit_1_1->SetAttribute("level", 1);
        xml_limit_1_1->SetAttribute("uses", 0);
        xml_limit_1_1->SetAttribute("command", character.limit_1_1);
        xml_limit_1->LinkEndChild(xml_limit_1_1.release());
        if (character.limit_1_2 < 255){ // Some characters only have 1 limit per level.
            std::unique_ptr<TiXmlElement> xml_limit_1_2(new TiXmlElement("limit"));
            xml_limit_1_2->SetAttribute("level", 2);
            xml_limit_1_2->SetAttribute("uses", character.limit_1_2_uses);
            xml_limit_1_2->SetAttribute("command", character.limit_1_2);
            xml_limit_1->LinkEndChild(xml_limit_1_2.release());
        }
        xml_limits->LinkEndChild(xml_limit_1.release());
        // Limit level 2.
        if (character.limit_2_kills < 65535){
            std::unique_ptr<TiXmlElement> xml_limit_2(new TiXmlElement("level"));
            xml_limit_2->SetAttribute("level", 2);
            xml_limit_2->SetAttribute("kills", character.limit_2_kills);
            xml_limit_2->SetAttribute("divisor", character.limit_2_div);
            std::unique_ptr<TiXmlElement> xml_limit_2_1(new TiXmlElement("limit"));
            xml_limit_2_1->SetAttribute("level", 1);
            xml_limit_2_1->SetAttribute("uses", 0);
            xml_limit_2_1->SetAttribute("command", character.limit_2_1);
            xml_limit_2->LinkEndChild(xml_limit_2_1.release());
            if (character.limit_2_2 < 255){ // Some characters only have 1 limit per level.
                std::unique_ptr<TiXmlElement> xml_limit_2_2(new TiXmlElement("limit"));
                xml_limit_2_2->SetAttribute("level", 2);
                xml_limit_2_2->SetAttribute("uses", character.limit_2_2_uses);
                xml_limit_2_2->SetAttribute("command", character.limit_2_2);
                xml_limit_2->LinkEndChild(xml_limit_2_2.release());
            }
            xml_limits->LinkEndChild(xml_limit_2.release());
        }
        // Limit level 3.
        if (character.limit_3_kills < 65535 && character.limit_3_kills > 0){
            std::unique_ptr<TiXmlElement> xml_limit_3(new TiXmlElement("level"));
            xml_limit_3->SetAttribute("level", 3);
            xml_limit_3->SetAttribute("kills", character.limit_3_kills);
            xml_limit_3->SetAttribute("divisor", character.limit_3_div);
            std::unique_ptr<TiXmlElement> xml_limit_3_1(new TiXmlElement("limit"));
            xml_limit_3_1->SetAttribute("level", 1);
            xml_limit_3_1->SetAttribute("uses", 0);
            xml_limit_3_1->SetAttribute("command", character.limit_3_1);
            xml_limit_3->LinkEndChild(xml_limit_3_1.release());
            if (character.limit_3_2 < 255){ // Some characters only have 1 limit per level.
                std::unique_ptr<TiXmlElement> xml_limit_3_2(new TiXmlElement("limit"));
                xml_limit_3_2->SetAttribute("level", 2);
                xml_limit_3_2->SetAttribute("uses", character.limit_3_2_uses);
                xml_limit_3_2->SetAttribute("command", character.limit_3_2);
                xml_limit_3->LinkEndChild(xml_limit_3_2.release());
            }
            xml_limits->LinkEndChild(xml_limit_3.release());
        }
        // Limit level 4.
        if (character.limit_4_1 < 255){
            std::unique_ptr<TiXmlElement> xml_limit_4(new TiXmlElement("level"));
            xml_limit_4->SetAttribute("level", 4);
            xml_limit_4->SetAttribute("kills", 0);
            xml_limit_4->SetAttribute("divisor", character.limit_4_div);
            std::unique_ptr<TiXmlElement> xml_limit_4_1(new TiXmlElement("limit"));
            xml_limit_4_1->SetAttribute("level", 1);
            xml_limit_4_1->SetAttribute("uses", 0);
            xml_limit_4_1->SetAttribute("command", character.limit_4_1);
            xml_limit_4->LinkEndChild(xml_limit_4_1.release());
            xml_limits->LinkEndChild(xml_limit_4.release());
        }

        xml_character->LinkEndChild(xml_limits.release());

        container->LinkEndChild(xml_character.release());

    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
}

void KernelDataInstaller::ReadGrowth(){

    File growth_file = kernel_.ExtractGZip(KERNEL_BATTLE_AND_GROWTH_DATA);
    growth_file.SetOffset(0x01F8); //Skip character data.

    for (int i = 0; i < 12; i ++) growth_.bonus_stat[i] = growth_file.readU8();
    for (int i = 0; i < 12; i ++) growth_.bonus_hp[i] = growth_file.readU8();
    for (int i = 0; i < 12; i ++) growth_.bonus_mp[i] = growth_file.readU8();
    for (int i = 0; i < 64; i ++){
        growth_.curves[i].id = i;
        if (i >=0 && i < 37) growth_.curves[i].type = CURVE_PRIMARY;
        else if (i >=37 && i < 46) growth_.curves[i].type = CURVE_HP;
        else if (i >=46 && i < 55) growth_.curves[i].type = CURVE_MP;
        else growth_.curves[i].type = CURVE_EXP;
        for (int j = 0; j < 8; j ++){
            growth_.curves[i].gradient[j] = growth_file.readU8();
            growth_.curves[i].base[j] = growth_file.readU8();
        }
    }

}

void KernelDataInstaller::WriteGrowth(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("growth"));
    // Bonus.
    std::unique_ptr<TiXmlElement> xml_bonuses(new TiXmlElement("stat_bonus"));
    for (int i = 0; i < 12; i ++){
        std::unique_ptr<TiXmlElement> xml_bonus(new TiXmlElement("bonus"));
        xml_bonus->SetAttribute("stat", "primary");
        xml_bonus->SetAttribute("value", growth_.bonus_stat[i]);
        xml_bonuses->LinkEndChild(xml_bonus.release());
    }
    for (int i = 0; i < 12; i ++){
        std::unique_ptr<TiXmlElement> xml_bonus(new TiXmlElement("bonus"));
        xml_bonus->SetAttribute("stat", "hp");
        xml_bonus->SetAttribute("value", growth_.bonus_hp[i]);
        xml_bonuses->LinkEndChild(xml_bonus.release());
    }
    for (int i = 0; i < 12; i ++){
        std::unique_ptr<TiXmlElement> xml_bonus(new TiXmlElement("bonus"));
        xml_bonus->SetAttribute("stat", "mp");
        xml_bonus->SetAttribute("value", growth_.bonus_mp[i]);
        xml_bonuses->LinkEndChild(xml_bonus.release());
    }
    container->LinkEndChild(xml_bonuses.release());
    // Curves
    std::unique_ptr<TiXmlElement> xml_curves(new TiXmlElement("curves"));
    for (int i = 0; i < 64; i ++){
        std::unique_ptr<TiXmlElement> xml_curve(new TiXmlElement("curve"));
        xml_curve->SetAttribute("id", growth_.curves[i].id);
        xml_curve->SetAttribute("type", growth_.curves[i].type);
        for (int j = 0; j < 8; j ++){
            std::unique_ptr<TiXmlElement> xml_curve_pair(new TiXmlElement("pair"));
            xml_curve_pair->SetAttribute("id", j);
            xml_curve_pair->SetAttribute("gradient", growth_.curves[i].gradient[j]);
            xml_curve_pair->SetAttribute("base", growth_.curves[i].base[j]);
            xml_curve->LinkEndChild(xml_curve_pair.release());
        }
        xml_curves->LinkEndChild(xml_curve.release());
    }
    container->LinkEndChild(xml_curves.release());
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

void KernelDataInstaller::WriteItems(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("items"));
    for (ItemData item : items_){
        std::unique_ptr<TiXmlElement> xml_item(new TiXmlElement("item"));
        xml_item->SetAttribute("id", item.id);
        xml_item->SetAttribute("inventory_id", item.id);
        xml_item->SetAttribute("price", prices_[item.id]);
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

int KernelDataInstaller::ReadWeapons(){

    // Empty the item list
    weapons_.clear();

    File weapons_file = kernel_.ExtractGZip(KERNEL_WEAPON_DATA);
    File weapons_name_file = kernel_.ExtractGZip(KERNEL_WEAPON_NAMES);
    File weapons_desc_file = kernel_.ExtractGZip(KERNEL_WEAPON_DESCRIPTIONS);
    int name_offset = 0;
    int desc_offset = 0;
    int weapon_count = 0;

    for (int i = 0; i < 128; i ++){

        WeaponData data;

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

                    for (int i = 0; i < comp_size; i ++){
                        data.description += ENGLISH_CHARS[
                          weapons_desc_file.GetU8(desc_offset - 1 - comp_offset + i)
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
        for (int i = 0; i < 4; i ++) data.stat_raw[i] = weapons_file.readU8();
        for (int i = 0; i < 4; i ++) data.stat_bonus_raw[i] = weapons_file.readU8();
        for (int i = 0; i < 8; i ++) data.slots_raw[i] = weapons_file.readU8();
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
        if (data.equip_raw == (data.equip_raw | 0x0001)) data.equip.push_back(1);
        if (data.equip_raw == (data.equip_raw | 0x0002)) data.equip.push_back(2);
        if (data.equip_raw == (data.equip_raw | 0x0004)) data.equip.push_back(3);
        if (data.equip_raw == (data.equip_raw | 0x0008)) data.equip.push_back(4);
        if (data.equip_raw == (data.equip_raw | 0x0010)) data.equip.push_back(5);
        if (data.equip_raw == (data.equip_raw | 0x0020)) data.equip.push_back(6);
        if (data.equip_raw == (data.equip_raw | 0x0040)) data.equip.push_back(7);
        if (data.equip_raw == (data.equip_raw | 0x0080)) data.equip.push_back(8);
        if (data.equip_raw == (data.equip_raw | 0x0100)) data.equip.push_back(9);
        if (data.equip_raw == (data.equip_raw | 0x0200)) data.equip.push_back(10);
        if (data.equip_raw == (data.equip_raw | 0x0400)) data.equip.push_back(11);
        for (int i = 0; i < 4; i ++){
            if (data.stat_raw[i] < 0XFF && data.stat_bonus_raw[i] > 0){
                StatBonus stat_bonus;
                stat_bonus.stat = data.stat_raw[i];
                stat_bonus.bonus = data.stat_bonus_raw[i];
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

void KernelDataInstaller::WriteWeapons(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("weapons"));
    for (WeaponData weapon : weapons_){
        std::unique_ptr<TiXmlElement> xml_weapon(new TiXmlElement("weapon"));
        xml_weapon->SetAttribute("id", weapon.id);
        xml_weapon->SetAttribute("inventory_id", weapon.id + 128);
        xml_weapon->SetAttribute("price", prices_[weapon.id + 128]);
        xml_weapon->SetAttribute("name", weapon.name);
        xml_weapon->SetAttribute("description", weapon.description);
        xml_weapon->SetAttribute("camera", weapon.camera);
        xml_weapon->SetAttribute("sell", weapon.sellable);
        xml_weapon->SetAttribute("battle", weapon.useable_battle);
        xml_weapon->SetAttribute("menu", weapon.useable_menu);
        xml_weapon->SetAttribute("throw", weapon.throwable);
        xml_weapon->SetAttribute("effect", weapon.effect);
        xml_weapon->SetAttribute("dmg_formula", weapon.damage_formula);
        xml_weapon->SetAttribute("dmg_modifier", weapon.damage_modifier);
        xml_weapon->SetAttribute("power", weapon.power);
        xml_weapon->SetAttribute("growth", weapon.growth);
        xml_weapon->SetAttribute("accuracy", weapon.accuracy);
        xml_weapon->SetAttribute("critical", weapon.critical);
        xml_weapon->SetAttribute("model", weapon.model);
        xml_weapon->SetAttribute("animation", weapon.animation_index);
        xml_weapon->SetAttribute("target_select", weapon.target.selection_enabled);
        xml_weapon->SetAttribute("target_default_enemy", weapon.target.default_enemy);
        xml_weapon->SetAttribute("target_default_multiple", weapon.target.default_multiple);
        xml_weapon->SetAttribute("target_toggle_multiple", weapon.target.toggle_multiple);
        xml_weapon->SetAttribute("target_fixed", weapon.target.fixed_row);
        xml_weapon->SetAttribute("target_short_range", weapon.target.short_range);
        xml_weapon->SetAttribute("target_all", weapon.target.all_rows);
        xml_weapon->SetAttribute("target_random", weapon.target.random);
        // Statuses.
        if (weapon.status != 0XFF){
            std::unique_ptr<TiXmlElement> xml_status(new TiXmlElement("statuses"));
            std::unique_ptr<TiXmlElement> xml_status_status(new TiXmlElement("status"));
            xml_status_status->SetAttribute("id", weapon.status);
            xml_status->LinkEndChild(xml_status_status.release());
            xml_weapon->LinkEndChild(xml_status.release());
        }
        // Add elements.
        if (weapon.elements.size() > 0){
            std::unique_ptr<TiXmlElement> xml_elements(new TiXmlElement("elements"));
            for (int s : weapon.elements){
                std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("element"));
                xml_element->SetAttribute("id", s);
                xml_elements->LinkEndChild(xml_element.release());
            }
            xml_weapon->LinkEndChild(xml_elements.release());
        }
        if (weapon.materia_slots.size() > 0){
            std::unique_ptr<TiXmlElement> xml_slots(new TiXmlElement("slots"));
            for (u8 slot : weapon.materia_slots){
                std::unique_ptr<TiXmlElement> xml_slot(new TiXmlElement("slot"));
                xml_slot->SetAttribute("type", slot);
                xml_slots->LinkEndChild(xml_slot.release());
            }
            xml_weapon->LinkEndChild(xml_slots.release());
        }
        if (weapon.stat_bonus.size() > 0){
            std::unique_ptr<TiXmlElement> xml_stats(new TiXmlElement("stat_bonuses"));
            for (StatBonus s : weapon.stat_bonus){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", s.stat);
                xml_stat->SetAttribute("bonus", s.bonus);
                xml_stats->LinkEndChild(xml_stat.release());
            }
            xml_weapon->LinkEndChild(xml_stats.release());
        }
        if (weapon.equip.size() > 0){
            std::unique_ptr<TiXmlElement> xml_characters(new TiXmlElement("characters"));
            for (int s : weapon.equip){
                std::unique_ptr<TiXmlElement> xml_character(new TiXmlElement("character"));
                xml_character->SetAttribute("id", s);
                xml_characters->LinkEndChild(xml_character.release());
            }
            xml_weapon->LinkEndChild(xml_characters.release());
        }
        container->LinkEndChild(xml_weapon.release());

    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
}

int KernelDataInstaller::ReadArmors(){

    // Empty the item list
    armors_.clear();

    File armor_file = kernel_.ExtractGZip(KERNEL_ARMOR_DATA);
    File armor_name_file = kernel_.ExtractGZip(KERNEL_ARMOR_NAMES);
    File armor_desc_file = kernel_.ExtractGZip(KERNEL_ARMOR_DESCRIPTIONS);
    int name_offset = 0;
    int desc_offset = 0;
    int armor_count = 0;

    for (int i = 0; i < 32; i ++){ // There are exactly 32.

        ArmorData data;

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

                    for (int i = 0; i < comp_size; i ++){
                        data.description += ENGLISH_CHARS[
                          armor_desc_file.GetU8(desc_offset - 1 - comp_offset + i)
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

        // Read data from the weapon data section.
        data.unknown_0 = armor_file.readU8();
        data.element_defense_mode = armor_file.readU8();
        data.defense = armor_file.readU8();
        data.m_defense = armor_file.readU8();
        data.evasion = armor_file.readU8();
        data.m_evasion = armor_file.readU8();
        data.status = armor_file.readU8();
        data.unknown_1 = armor_file.readU16LE();
        for (int i = 0; i < 8; i ++) data.slots_raw[i] = armor_file.readU8();
        data.growth = armor_file.readU8();
        data.equip_raw = armor_file.readU16LE();
        data.element_raw = armor_file.readU16LE();
        data.unknown_2 = armor_file.readU16LE();
        for (int i = 0; i < 4; i ++) data.stat_raw[i] = armor_file.readU8();
        for (int i = 0; i < 4; i ++) data.stat_bonus_raw[i] = armor_file.readU8();
        data.restrict_raw = armor_file.readU16LE();
        data.unknown_3 = armor_file.readU16LE();

        // Derive data from the read data.
        data.id = i;
        if (data.growth != 0 && data.growth != 2 && data.growth != 3) data.growth = 1;
        data.sellable = !(data.restrict_raw & (1 << 0));
        data.useable_battle = !(data.restrict_raw & (2 << 0));
        data.useable_menu = !(data.restrict_raw & (4 << 0));
        if (data.equip_raw == (data.equip_raw | 0x0001)) data.equip.push_back(1);
        if (data.equip_raw == (data.equip_raw | 0x0002)) data.equip.push_back(2);
        if (data.equip_raw == (data.equip_raw | 0x0004)) data.equip.push_back(3);
        if (data.equip_raw == (data.equip_raw | 0x0008)) data.equip.push_back(4);
        if (data.equip_raw == (data.equip_raw | 0x0010)) data.equip.push_back(5);
        if (data.equip_raw == (data.equip_raw | 0x0020)) data.equip.push_back(6);
        if (data.equip_raw == (data.equip_raw | 0x0040)) data.equip.push_back(7);
        if (data.equip_raw == (data.equip_raw | 0x0080)) data.equip.push_back(8);
        if (data.equip_raw == (data.equip_raw | 0x0100)) data.equip.push_back(9);
        if (data.equip_raw == (data.equip_raw | 0x0200)) data.equip.push_back(10);
        if (data.equip_raw == (data.equip_raw | 0x0400)) data.equip.push_back(11);
        for (int i = 0; i < 4; i ++){
            if (data.stat_raw[i] < 0XFF && data.stat_bonus_raw[i] > 0){
                StatBonus stat_bonus;
                stat_bonus.stat = data.stat_raw[i];
                stat_bonus.bonus = data.stat_bonus_raw[i];
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

void KernelDataInstaller::WriteArmors(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("armors"));
    for (ArmorData armor : armors_){
        std::unique_ptr<TiXmlElement> xml_armor(new TiXmlElement("armor"));
        xml_armor->SetAttribute("id", armor.id);
        xml_armor->SetAttribute("inventory_id", armor.id + 256);
        xml_armor->SetAttribute("price", prices_[armor.id + 256]);
        xml_armor->SetAttribute("name", armor.name);
        xml_armor->SetAttribute("description", armor.description);
        xml_armor->SetAttribute("sell", armor.sellable);
        xml_armor->SetAttribute("battle", armor.useable_battle);
        xml_armor->SetAttribute("menu", armor.useable_menu);
        xml_armor->SetAttribute("phi_defense", armor.defense);
        xml_armor->SetAttribute("mag_defense", armor.m_defense);
        xml_armor->SetAttribute("phi_evasion", armor.evasion);
        xml_armor->SetAttribute("mag_evasion", armor.m_evasion);
        xml_armor->SetAttribute("growth", armor.growth);
        // Statuses.
        if (armor.status != 0XFF){
            std::unique_ptr<TiXmlElement> xml_status(new TiXmlElement("statuses"));
            std::unique_ptr<TiXmlElement> xml_status_status(new TiXmlElement("status"));
            xml_status_status->SetAttribute("id", armor.status);
            xml_status->LinkEndChild(xml_status_status.release());
            xml_armor->LinkEndChild(xml_status.release());
        }
        // Add elements.
        if (armor.elements.size() > 0){
            std::unique_ptr<TiXmlElement> xml_elements(new TiXmlElement("elements"));
            for (int s : armor.elements){
                std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("element"));
                xml_element->SetAttribute("id", s);
                xml_element->SetAttribute("mode", armor.element_defense_mode);
                xml_elements->LinkEndChild(xml_element.release());
            }
            xml_armor->LinkEndChild(xml_elements.release());
        }
        if (armor.materia_slots.size() > 0){
            std::unique_ptr<TiXmlElement> xml_slots(new TiXmlElement("slots"));
            for (u8 slot : armor.materia_slots){
                std::unique_ptr<TiXmlElement> xml_slot(new TiXmlElement("slot"));
                xml_slot->SetAttribute("type", slot);
                xml_slots->LinkEndChild(xml_slot.release());
            }
            xml_armor->LinkEndChild(xml_slots.release());
        }
        if (armor.stat_bonus.size() > 0){
            std::unique_ptr<TiXmlElement> xml_stats(new TiXmlElement("stat_bonuses"));
            for (StatBonus s : armor.stat_bonus){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", s.stat);
                xml_stat->SetAttribute("bonus", s.bonus);
                xml_stats->LinkEndChild(xml_stat.release());
            }
            xml_armor->LinkEndChild(xml_stats.release());
        }
        if (armor.equip.size() > 0){
            std::unique_ptr<TiXmlElement> xml_characters(new TiXmlElement("characters"));
            for (int s : armor.equip){
                std::unique_ptr<TiXmlElement> xml_character(new TiXmlElement("character"));
                xml_character->SetAttribute("id", s);
                xml_characters->LinkEndChild(xml_character.release());
            }
            xml_armor->LinkEndChild(xml_characters.release());
        }
        container->LinkEndChild(xml_armor.release());

    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
}

int KernelDataInstaller::ReadAccessories(){

    // Empty the item list
    accessories_.clear();

    File accessory_file = kernel_.ExtractGZip(KERNEL_ACCESSORY_DATA);
    File accessory_name_file = kernel_.ExtractGZip(KERNEL_ACCESSORY_NAMES);
    File accessory_desc_file = kernel_.ExtractGZip(KERNEL_ACCESSORY_DESCRIPTIONS);
    int name_offset = 0;
    int desc_offset = 0;
    int accessory_count = 0;

    for (int i = 0; i < 32; i ++){ // There are exactly 32.

        AccessoryData data;

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

                    for (int i = 0; i < comp_size; i ++){
                        data.description += ENGLISH_CHARS[
                          accessory_desc_file.GetU8(desc_offset - 1 - comp_offset + i)
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

        // Read data from the accessory data section.
        for (int i = 0; i < 2; i ++) data.stat_raw[i] = accessory_file.readU8();
        for (int i = 0; i < 2; i ++) data.stat_bonus_raw[i] = accessory_file.readU8();
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
        if (data.equip_raw == (data.equip_raw | 0x0001)) data.equip.push_back(1);
        if (data.equip_raw == (data.equip_raw | 0x0002)) data.equip.push_back(2);
        if (data.equip_raw == (data.equip_raw | 0x0004)) data.equip.push_back(3);
        if (data.equip_raw == (data.equip_raw | 0x0008)) data.equip.push_back(4);
        if (data.equip_raw == (data.equip_raw | 0x0010)) data.equip.push_back(5);
        if (data.equip_raw == (data.equip_raw | 0x0020)) data.equip.push_back(6);
        if (data.equip_raw == (data.equip_raw | 0x0040)) data.equip.push_back(7);
        if (data.equip_raw == (data.equip_raw | 0x0080)) data.equip.push_back(8);
        if (data.equip_raw == (data.equip_raw | 0x0100)) data.equip.push_back(9);
        if (data.equip_raw == (data.equip_raw | 0x0200)) data.equip.push_back(10);
        if (data.equip_raw == (data.equip_raw | 0x0400)) data.equip.push_back(11);
        for (int i = 0; i < 2; i ++){
            if (data.stat_raw[i] < 0XFF && data.stat_bonus_raw[i] > 0){
                StatBonus stat_bonus;
                stat_bonus.stat = data.stat_raw[i];
                stat_bonus.bonus = data.stat_bonus_raw[i];
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

void KernelDataInstaller::WriteAccessories(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("accessories"));
    for (AccessoryData accessory : accessories_){
        std::unique_ptr<TiXmlElement> xml_accessory(new TiXmlElement("accessory"));
        xml_accessory->SetAttribute("id", accessory.id);
        xml_accessory->SetAttribute("inventory_id", accessory.id + 287);
        xml_accessory->SetAttribute("price", prices_[accessory.id + 287]);
        xml_accessory->SetAttribute("name", accessory.name);
        xml_accessory->SetAttribute("description", accessory.description);
        xml_accessory->SetAttribute("sell", accessory.sellable);
        xml_accessory->SetAttribute("battle", accessory.useable_battle);
        xml_accessory->SetAttribute("menu", accessory.useable_menu);
        // Special effect
        if (accessory.effect != 0XFF){
            std::unique_ptr<TiXmlElement> xml_status(new TiXmlElement("special_effects"));
            std::unique_ptr<TiXmlElement> xml_status_status(new TiXmlElement("effect"));
            xml_status_status->SetAttribute("id", accessory.effect);
            xml_status->LinkEndChild(xml_status_status.release());
            xml_accessory->LinkEndChild(xml_status.release());
        }
        // Statuses.
        if (accessory.status.size() > 0){
            std::unique_ptr<TiXmlElement> xml_status(new TiXmlElement("statuses"));
            for (int s : accessory.status){
                std::unique_ptr<TiXmlElement> xml_status_status(new TiXmlElement("status"));
                xml_status_status->SetAttribute("id", s);
                xml_status->LinkEndChild(xml_status_status.release());
            }
            xml_accessory->LinkEndChild(xml_status.release());
        }
        // Add elements.
        if (accessory.elements.size() > 0){
            std::unique_ptr<TiXmlElement> xml_elements(new TiXmlElement("elements"));
            for (int s : accessory.elements){
                std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("element"));
                xml_element->SetAttribute("id", s);
                xml_element->SetAttribute("mode", accessory.element_defense_mode);
                xml_elements->LinkEndChild(xml_element.release());
            }
            xml_accessory->LinkEndChild(xml_elements.release());
        }
        if (accessory.stat_bonus.size() > 0){
            std::unique_ptr<TiXmlElement> xml_stats(new TiXmlElement("stat_bonuses"));
            for (StatBonus s : accessory.stat_bonus){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", s.stat);
                xml_stat->SetAttribute("bonus", s.bonus);
                xml_stats->LinkEndChild(xml_stat.release());
            }
            xml_accessory->LinkEndChild(xml_stats.release());
        }
        if (accessory.equip.size() > 0){
            std::unique_ptr<TiXmlElement> xml_characters(new TiXmlElement("characters"));
            for (int s : accessory.equip){
                std::unique_ptr<TiXmlElement> xml_character(new TiXmlElement("character"));
                xml_character->SetAttribute("id", s);
                xml_characters->LinkEndChild(xml_character.release());
            }
            xml_accessory->LinkEndChild(xml_characters.release());
        }
        container->LinkEndChild(xml_accessory.release());

    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
}

int KernelDataInstaller::ReadMateria(){

    // Empty the item list
    materia_.clear();

    File materia_file = kernel_.ExtractGZip(KERNEL_MATERIA_DATA);
    File materia_name_file = kernel_.ExtractGZip(KERNEL_MATERIA_NAMES);
    File materia_desc_file = kernel_.ExtractGZip(KERNEL_MATERIA_DESCRIPTIONS);
    int name_offset = 0;
    int desc_offset = 0;
    int materia_count = 0;

    for (int i = 0; i < 96; i ++){ // There are exactly 96.

        MateriaData data;

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

                    for (int i = 0; i < comp_size; i ++){
                        data.description += ENGLISH_CHARS[
                          materia_desc_file.GetU8(desc_offset - 1 - comp_offset + i)
                        ];
                    }
                    desc_offset ++;
                }
                else data.description += ENGLISH_CHARS[ch];
            }
            desc_offset ++;
        }

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

        // Add to the list of materia.
        materia_.push_back(data);
        materia_count ++;
    }

    return materia_count;
}

void KernelDataInstaller::WriteMateria(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("materias"));
    for (MateriaData materia : materia_){
        std::unique_ptr<TiXmlElement> xml_materia(new TiXmlElement("materia"));
        xml_materia->SetAttribute("id", materia.id);
        xml_materia->SetAttribute("price", prices_[materia.id + 320]);
        xml_materia->SetAttribute("name", materia.name);
        xml_materia->SetAttribute("description", materia.description);
        xml_materia->SetAttribute("type", materia.type);
        // Levels.
        std::unique_ptr<TiXmlElement> xml_levels(new TiXmlElement("levels"));
        std::unique_ptr<TiXmlElement> xml_level(new TiXmlElement("level"));
        xml_level->SetAttribute("level", 1);
        xml_level->SetAttribute("ap", 0);
        xml_levels->LinkEndChild(xml_level.release());
        for (int i = 0; i < 4; i ++){
            if (materia.level_up_ap[i] < 65535){
                std::unique_ptr<TiXmlElement> xml_level(new TiXmlElement("level"));
                xml_level->SetAttribute("level", i + 2);
                xml_level->SetAttribute("ap", materia.level_up_ap[i] * 100);
                xml_levels->LinkEndChild(xml_level.release());
            }
        }

        xml_materia->LinkEndChild(xml_levels.release());

        // Statuses.
        if (materia.status.size() > 0){
            std::unique_ptr<TiXmlElement> xml_status(new TiXmlElement("statuses"));
            for (int s : materia.status){
                std::unique_ptr<TiXmlElement> xml_status_status(new TiXmlElement("status"));
                xml_status_status->SetAttribute("id", s);
                xml_status->LinkEndChild(xml_status_status.release());
            }
            xml_materia->LinkEndChild(xml_status.release());
        }
        // Add elements.
        if (materia.element != 0xFF){
            std::unique_ptr<TiXmlElement> xml_elements(new TiXmlElement("elements"));
            std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("element"));
            xml_element->SetAttribute("id", materia.element);
            xml_elements->LinkEndChild(xml_element.release());
            xml_materia->LinkEndChild(xml_elements.release());
        }
        if (materia.stats.change == true){
            std::unique_ptr<TiXmlElement> xml_stats(new TiXmlElement("stat_bonuses"));
            // TODO: Lookup stat IDs.
            if (materia.stats.str != 0){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", 0);
                xml_stat->SetAttribute("bonus", materia.stats.str);
                xml_stat->SetAttribute("mode", "abs");
                xml_stats->LinkEndChild(xml_stat.release());
            }
            if (materia.stats.vit != 0){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", 1);
                xml_stat->SetAttribute("bonus", materia.stats.vit);
                xml_stat->SetAttribute("mode", "abs");
                xml_stats->LinkEndChild(xml_stat.release());
            }
            if (materia.stats.mag != 0){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", 2);
                xml_stat->SetAttribute("bonus", materia.stats.mag);
                xml_stat->SetAttribute("mode", "abs");
                xml_stats->LinkEndChild(xml_stat.release());
            }
            if (materia.stats.spr != 0){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", 3);
                xml_stat->SetAttribute("bonus", materia.stats.spr);
                xml_stat->SetAttribute("mode", "abs");
                xml_stats->LinkEndChild(xml_stat.release());
            }
            if (materia.stats.dex != 0){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", 4);
                xml_stat->SetAttribute("bonus", materia.stats.dex);
                xml_stat->SetAttribute("mode", "abs");
                xml_stats->LinkEndChild(xml_stat.release());
            }
            if (materia.stats.lck != 0){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", 5);
                xml_stat->SetAttribute("bonus", materia.stats.lck);
                xml_stat->SetAttribute("mode", "abs");
                xml_stats->LinkEndChild(xml_stat.release());
            }
            if (materia.stats.hp != 0){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", 6);
                xml_stat->SetAttribute("bonus", materia.stats.hp);
                xml_stat->SetAttribute("mode", "rel");
                xml_stats->LinkEndChild(xml_stat.release());
            }
            if (materia.stats.mp != 0){
                std::unique_ptr<TiXmlElement> xml_stat(new TiXmlElement("stat_bonus"));
                xml_stat->SetAttribute("stat", 7);
                xml_stat->SetAttribute("bonus", materia.stats.mp);
                xml_stat->SetAttribute("mode", "rel");
                xml_stats->LinkEndChild(xml_stat.release());
            }
            xml_materia->LinkEndChild(xml_stats.release());
        }
        container->LinkEndChild(xml_materia.release());

    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
}

int KernelDataInstaller::ReadKeyItems(){

    // Empty the item list
    key_items_.clear();

    File key_items_name_file = kernel_.ExtractGZip(KERNEL_KEY_ITEM_NAMES);
    File key_items_desc_file = kernel_.ExtractGZip(KERNEL_KEY_ITEM_DESCRIPTIONS);
    int name_offset = 0;
    int desc_offset = 0;
    int key_item_count = 0;

    for (int i = 0; i < 128; i ++){

        KeyItemData data;

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

void KernelDataInstaller::WriteKeyItems(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("key_items"));
    for (KeyItemData key_item : key_items_){
        std::unique_ptr<TiXmlElement> xml_key_item(new TiXmlElement("key_item"));
        xml_key_item->SetAttribute("id", key_item.id);
        xml_key_item->SetAttribute("name", key_item.name);
        xml_key_item->SetAttribute("description", key_item.description);
        container->LinkEndChild(xml_key_item.release());

    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
}

int KernelDataInstaller::ReadSummonNames(){

    // Empty the summon list
    key_items_.clear();

    File summon_name_file = kernel_.ExtractGZip(KERNEL_SUMMON_ATTACK_NAMES);
    int name_offset = 0;
    int summon_count = 0;

    for (int i = 0; i < 16; i ++){ // There are exactly 16.

        SummonNameData data;

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

void KernelDataInstaller::WriteSummonNames(std::string file){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("summons"));
    for (SummonNameData summon : summon_names_){
        std::unique_ptr<TiXmlElement> xml_summon(new TiXmlElement("summon"));
        xml_summon->SetAttribute("id", summon.id);
        xml_summon->SetAttribute("name", summon.name);
        container->LinkEndChild(xml_summon.release());

    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
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

void KernelDataInstaller::WriteInitialSaveMap(std::string file){
    // The initial save map only contains character data.
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("savemap"));
    std::unique_ptr<TiXmlElement> xml_characters(new TiXmlElement("characters"));
    int total_count = 0; // Utility to count in loops.
    // Characters.
    for (int c = 0; c < 9; c ++){
        std::unique_ptr<TiXmlElement> xml_character(new TiXmlElement("character"));
        xml_character->SetAttribute("id", c);
        xml_character->SetAttribute("char_id", savemap_.characters[c].identifier);
        xml_character->SetAttribute("name", savemap_.characters[c].name);
        xml_character->SetAttribute("level", savemap_.characters[c].level);
        xml_character->SetAttribute("kills", savemap_.characters[c].kills);
        xml_character->SetAttribute("exp", savemap_.characters[c].exp);
        xml_character->SetAttribute("exp_to_next_lv", savemap_.characters[c].exp_to_next);
        // Character stats.
        std::unique_ptr<TiXmlElement> xml_stats(new TiXmlElement("stats"));
        // TODO: Lookup stat names or IDs (one or the other, don't hardcode both).
        std::unique_ptr<TiXmlElement> xml_stat_str(new TiXmlElement("stat"));
        xml_stat_str->SetAttribute("id", 0);
        xml_stat_str->SetAttribute("name", "str");
        xml_stat_str->SetAttribute("value", savemap_.characters[c].str);
        xml_stat_str->SetAttribute("bonus", savemap_.characters[c].str_bonus);
        xml_stats->LinkEndChild(xml_stat_str.release());
        std::unique_ptr<TiXmlElement> xml_stat_vit(new TiXmlElement("stat"));
        xml_stat_vit->SetAttribute("id", 1);
        xml_stat_vit->SetAttribute("name", "vit");
        xml_stat_vit->SetAttribute("value", savemap_.characters[c].vit);
        xml_stat_vit->SetAttribute("bonus", savemap_.characters[c].vit_bonus);
        xml_stats->LinkEndChild(xml_stat_vit.release());
        std::unique_ptr<TiXmlElement> xml_stat_mag(new TiXmlElement("stat"));
        xml_stat_mag->SetAttribute("id", 2);
        xml_stat_mag->SetAttribute("name", "mag");
        xml_stat_mag->SetAttribute("value", savemap_.characters[c].mag);
        xml_stat_mag->SetAttribute("bonus", savemap_.characters[c].mag_bonus);
        xml_stats->LinkEndChild(xml_stat_mag.release());
        std::unique_ptr<TiXmlElement> xml_stat_spr(new TiXmlElement("stat"));
        xml_stat_spr->SetAttribute("id", 3);
        xml_stat_spr->SetAttribute("name", "spr");
        xml_stat_spr->SetAttribute("value", savemap_.characters[c].spr);
        xml_stat_spr->SetAttribute("bonus", savemap_.characters[c].spr_bonus);
        xml_stats->LinkEndChild(xml_stat_spr.release());
        std::unique_ptr<TiXmlElement> xml_stat_dex(new TiXmlElement("stat"));
        xml_stat_dex->SetAttribute("id", 4);
        xml_stat_dex->SetAttribute("name", "dex");
        xml_stat_dex->SetAttribute("value", savemap_.characters[c].dex);
        xml_stat_dex->SetAttribute("bonus", savemap_.characters[c].dex_bonus);
        xml_stats->LinkEndChild(xml_stat_dex.release());
        std::unique_ptr<TiXmlElement> xml_stat_lck(new TiXmlElement("stat"));
        xml_stat_lck->SetAttribute("id", 5);
        xml_stat_lck->SetAttribute("name", "lck");
        xml_stat_lck->SetAttribute("value", savemap_.characters[c].lck);
        xml_stat_lck->SetAttribute("bonus", savemap_.characters[c].lck_bonus);
        xml_stats->LinkEndChild(xml_stat_lck.release());
        std::unique_ptr<TiXmlElement> xml_stat_hp(new TiXmlElement("stat"));
        xml_stat_hp->SetAttribute("id", 6);
        xml_stat_hp->SetAttribute("name", "hp");
        xml_stat_hp->SetAttribute("value", savemap_.characters[c].hp);
        xml_stat_hp->SetAttribute("base", savemap_.characters[c].base_hp);
        // TODO: MAX HP always as 0xFF, must be calculated manually.
        xml_stat_hp->SetAttribute("max", savemap_.characters[c].max_hp);
        xml_stats->LinkEndChild(xml_stat_hp.release());
        std::unique_ptr<TiXmlElement> xml_stat_mp(new TiXmlElement("stat"));
        xml_stat_mp->SetAttribute("id", 6);
        xml_stat_mp->SetAttribute("name", "mp");
        xml_stat_mp->SetAttribute("value", savemap_.characters[c].mp);
        xml_stat_mp->SetAttribute("base", savemap_.characters[c].base_mp);
        // TODO: MAX MP always as 0xFF, must be calculated manually.
        xml_stat_mp->SetAttribute("max", savemap_.characters[c].max_mp);
        xml_stats->LinkEndChild(xml_stat_mp.release());
        xml_character->LinkEndChild(xml_stats.release());
        // Character limits
        std::unique_ptr<TiXmlElement> xml_limits(new TiXmlElement("limits"));
        xml_limits->SetAttribute("selected", savemap_.characters[c].limit_level);
        xml_limits->SetAttribute("bar", savemap_.characters[c].limit_bar);
        for (int l = 0; l < 4; l ++){
            if (savemap_.characters[c].limits_learned[l][0]){
                std::unique_ptr<TiXmlElement> xml_limit_level(new TiXmlElement("level"));
                xml_limit_level->SetAttribute("id", l + 1);
                std::unique_ptr<TiXmlElement> xml_limit_technique(new TiXmlElement("technique"));
                xml_limit_technique->SetAttribute("id", 1);
                if (l < 3)
                    xml_limit_technique->SetAttribute("uses", savemap_.characters[c].limit_uses[l]);
                else xml_limit_technique->SetAttribute("uses", 0);
                xml_limit_level->LinkEndChild(xml_limit_technique.release());
                if (savemap_.characters[c].limits_learned[l][1]){
                    std::unique_ptr<TiXmlElement> xml_limit_technique(
                      new TiXmlElement("technique")
                    );
                    xml_limit_technique->SetAttribute("id", 2);
                    xml_limit_technique->SetAttribute("uses", 0);
                    xml_limit_level->LinkEndChild(xml_limit_technique.release());
                }
                xml_limits->LinkEndChild(xml_limit_level.release());
            }
        }
        xml_character->LinkEndChild(xml_limits.release());
        // Character equipments.
        std::unique_ptr<TiXmlElement> xml_equipment(new TiXmlElement("equipment"));
        std::unique_ptr<TiXmlElement> xml_weapon(new TiXmlElement("weapon"));
        xml_weapon->SetAttribute("id", savemap_.characters[c].weapon);
        std::unique_ptr<TiXmlElement> xml_weapon_materias(new TiXmlElement("materias"));
        total_count = 0;
        for (int m = 0; m < 8; m ++){
            if (savemap_.characters[c].weapon_materia[m].id < 255){
                total_count ++;
                std::unique_ptr<TiXmlElement> xml_weapon_materia(new TiXmlElement("materia"));
                xml_weapon_materia->SetAttribute("id", savemap_.characters[c].weapon_materia[m].id);
                xml_weapon_materia->SetAttribute("ap", savemap_.characters[c].weapon_materia[m].ap);
                xml_weapon_materias->LinkEndChild(xml_weapon_materia.release());
            }
        }
        // Only add materia section to equipment if at least one is equipped, else ignore section.
        if (total_count > 0) xml_weapon->LinkEndChild(xml_weapon_materias.release());
        xml_equipment->LinkEndChild(xml_weapon.release());
        std::unique_ptr<TiXmlElement> xml_armor(new TiXmlElement("armor"));
        xml_armor->SetAttribute("id", savemap_.characters[c].armor);
        std::unique_ptr<TiXmlElement> xml_armor_materias(new TiXmlElement("materias"));
        total_count = 0;
        for (int m = 0; m < 8; m ++){
            if (savemap_.characters[c].armor_materia[m].id < 255){
                total_count ++;
                std::unique_ptr<TiXmlElement> xml_armor_materia(new TiXmlElement("materia"));
                xml_armor_materia->SetAttribute("id", savemap_.characters[c].weapon_materia[m].id);
                xml_armor_materia->SetAttribute("ap", savemap_.characters[c].weapon_materia[m].ap);
                xml_armor_materias->LinkEndChild(xml_armor_materia.release());
            }
        }
        // Only add materia section to equipment if at least one is equipped, else ignore section.
        if (total_count > 0) xml_armor->LinkEndChild(xml_armor_materias.release());
        xml_equipment->LinkEndChild(xml_armor.release());
        if (savemap_.characters[c].accessory < 255){
            std::unique_ptr<TiXmlElement> xml_accessory(new TiXmlElement("accessory"));
            xml_accessory->SetAttribute("id", savemap_.characters[c].accessory);
            xml_equipment->LinkEndChild(xml_accessory.release());
        }
        xml_character->LinkEndChild(xml_equipment.release());
        // Statuses (Only saddness/fury)
        if (savemap_.characters[c].fury || savemap_.characters[c].sadness){
            std::unique_ptr<TiXmlElement> xml_statuses(new TiXmlElement("statuses"));
            if (savemap_.characters[c].fury){
                std::unique_ptr<TiXmlElement> xml_status(new TiXmlElement("status"));
                xml_status->SetAttribute("id", FURY);
                xml_statuses->LinkEndChild(xml_status.release());
            }
            if (savemap_.characters[c].sadness){
                std::unique_ptr<TiXmlElement> xml_status(new TiXmlElement("status"));
                xml_status->SetAttribute("id", SADNESS);
                xml_statuses->LinkEndChild(xml_status.release());
            }
            xml_character->LinkEndChild(xml_statuses.release());
        }
        xml_characters->LinkEndChild(xml_character.release());
    }
    container->LinkEndChild(xml_characters.release());
    xml.LinkEndChild(container.release());
    xml.SaveFile(file);
}
