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

    std::cout <<
      "-- Commands ------------------\n" <<
      "|  ID | cur | c_s | c_m| tar |\n";

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

int KernelDataInstaller::ReadAttacks(){

    // Empty the item list
    attacks_.clear();

    File attacks_file = kernel_.ExtractGZip(KERNEL_ATTACK_DATA);
    int attack_count = 0;

    std::cout <<
      "-- Attacks ---------------------------------------------------------------------\n" <<
      "|  ID | acc |tar | eff | dmg | pow | con | sch | eff | efm | sta | elm |  spe  |\n";

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

        printf(
          "| %3d | %3d | %3d | %3d | %3d | %3d | %3d | %3d | %3d | %3d | %3d | %5d |\n",
          i, data.accuracy, data.target_raw, data.effect, data.damage_raw, data.power, data.condition_raw,
          data.status_change_raw, data.additional_effects_raw, data.additional_effects_mod_raw,
          data.status_raw, data.element_raw, data.special_raw
        );


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

    std::cout <<
      "-- Chars ----\n" <<
      "|  ID | lvl |\n";

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

        printf("| %3d | %3d |", i, data.initial_level);
        std::cout << data.name << std::endl;


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

    std::cout <<
      "-- Items ------------------------------------------------------------------\n" <<
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

int KernelDataInstaller::ReadWeapons(){

    // Empty the item list
    weapons_.clear();

    File weapons_file = kernel_.ExtractGZip(KERNEL_WEAPON_DATA);
    File weapons_name_file = kernel_.ExtractGZip(KERNEL_WEAPON_NAMES);
    File weapons_desc_file = kernel_.ExtractGZip(KERNEL_WEAPON_DESCRIPTIONS);
    int name_offset = 0;
    int desc_offset = 0;
    int weapon_count = 0;

    std::cout <<
      "-- Weapons ----------------------------------------------------------------\n" <<
      "|  ID | tar | eff | dmg | pow | con | sch | eff | efm | sta | elm |  spe  |\n";

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

        // Read data from the item data section.
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
        data.stat_raw = weapons_file.readU32LE();
        data.stat_bonus_raw = weapons_file.readU32LE();
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
        if (data.stat_raw < 0XFF){
            StatBonus stat_bonus;
            stat_bonus.stat = data.stat_raw;
            stat_bonus.bonus = data.stat_bonus_raw;
            data.stat_bonus.push_back(stat_bonus);
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
