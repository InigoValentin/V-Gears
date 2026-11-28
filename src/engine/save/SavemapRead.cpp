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
#include "common/Logger.h"
#include "engine/save/Savemap.h"

namespace{

/**
 * Calls a function for each child element of a node that has a name.
 */
template<typename Function>
void ForEachElement(TiXmlNode& parent, const char* name, Function function){
    for (
      TiXmlElement* element = parent.FirstChildElement(name); element != nullptr;
      element = element->NextSiblingElement(name)
    ) function(*element);
}

/**
 * Reads an integer attribute.
 *
 * @param[in] missing Value returned if there is no such attribute, or it's not a number.
 */
int ReadInt(const TiXmlElement& element, const char* name, const int missing = -1){
    int value = missing;
    element.QueryIntAttribute(name, &value);
    return value;
}

/**
 * Sets a value from an attribute, if the attribute has a number that is not negative.
 */
void ReadUnsigned(const TiXmlElement& element, const char* name, unsigned int& value){
    const int read = ReadInt(element, name);
    if (read >= 0) value = read;
}

/**
 * Reads the skills that an Enemy Skill materia has learned.
 */
void ReadEnemySkills(TiXmlElement& materia_element, SavemapData::Materia& materia){
    for (bool& learned : materia.enemy_skill_learned) learned = false;
    ForEachElement(materia_element, "EnemySkill", [&](TiXmlElement& skills){
        ForEachElement(skills, "Skill", [&](TiXmlElement& skill){
            const int id = ReadInt(skill, "id");
            if (id < 0 || id >= static_cast<int>(SavemapData::MAX_ENEMY_SKILLS)) return;
            materia.enemy_skill_learned[id] = (ReadInt(skill, "learned") == 1);
        });
    });
}

/**
 * Reads a list of materia: the ones of the inventory, the stash or a piece of equipment.
 */
void ReadMaterias(TiXmlElement& list, SavemapData::Materia* materias, const unsigned int count){
    ForEachElement(list, "Materia", [&](TiXmlElement& element){
        const int slot = ReadInt(element, "slot");
        const int id = ReadInt(element, "id", -2);
        const int ap = ReadInt(element, "ap");
        const int enemy_skill = ReadInt(element, "enemy_skill", 0);
        if (
          slot < 0 || slot >= static_cast<int>(count) || id < -1 || ap < 0
          || (enemy_skill != 0 && enemy_skill != 1)
        ) return;
        materias[slot].id = id;
        materias[slot].ap = ap;
        materias[slot].enemy_skill = (enemy_skill == 1);
        ReadEnemySkills(element, materias[slot]);
    });
}

/**
 * Reads a stat of a character.
 */
void ReadStat(const TiXmlElement& element, SavemapData::Character& character){
    struct Entry{
        const char* id;
        SavemapData::Character::Stat SavemapData::Character::* stat;
        const char* second;
        int minimum_base;
    };
    static const Entry STATS[] = {
        {"str", &SavemapData::Character::str, "bonus", 0},
        {"vit", &SavemapData::Character::vit, "bonus", 0},
        {"mag", &SavemapData::Character::mag, "bonus", 0},
        {"spr", &SavemapData::Character::spr, "bonus", 0},
        {"dex", &SavemapData::Character::dex, "bonus", 0},
        {"lck", &SavemapData::Character::lck, "bonus", 0},
        {"hp", &SavemapData::Character::hp, "current", 1},
        {"mp", &SavemapData::Character::mp, "current", 1}
    };
    std::string id;
    element.QueryStringAttribute("id", &id);
    for (const Entry& entry : STATS){
        if (id != entry.id) continue;
        const int base = ReadInt(element, "base");
        const int second = ReadInt(element, entry.second);
        if (base < entry.minimum_base || second < 0) return;
        (character.*entry.stat).base = base;
        (character.*entry.stat).extra = second;
    }
}

/**
 * Reads the limit breaks of a character.
 */
void ReadLimits(TiXmlElement& limits, SavemapData::Character& character){
    const int current = ReadInt(limits, "current");
    if (current >= 0 && current < static_cast<int>(SavemapData::MAX_LIMIT_LEVELS))
        character.limit_level = current;
    const int bar = ReadInt(limits, "bar");
    if (bar >= 0 && bar <= static_cast<int>(SavemapData::MAX_LIMIT_BAR))
        character.limit_bar = bar;
    ForEachElement(limits, "Level", [&](TiXmlElement& element){
        const int level = ReadInt(element, "level");
        if (level < 0 || level >= static_cast<int>(SavemapData::MAX_LIMIT_LEVELS)) return;
        ReadUnsigned(element, "uses", character.limit_uses[level]);
        ForEachElement(element, "Technique", [&](TiXmlElement& technique){
            const int id = ReadInt(technique, "id");
            if (id < 0 || id >= static_cast<int>(SavemapData::MAX_LIMIT_TECHNIQUES)) return;
            character.limits_learned[level][id] = (ReadInt(technique, "learned") == 1);
        });
    });
}

/**
 * Reads a weapon or an armor of a character, with its materia.
 */
void ReadEquipment(TiXmlElement& element, SavemapData::Character::Equipment& equipment){
    const int id = ReadInt(element, "id");
    if (id >= 0) equipment.id = id;
    // The list of materia is the first thing inside.
    TiXmlNode* first = element.FirstChild();
    if (first == nullptr || first->ToElement() == nullptr || first->ValueStr() != "Materia")
        return;
    ReadMaterias(*first->ToElement(), equipment.materia, SavemapData::MAX_EQUIP_SLOTS);
}

/**
 * Reads a character.
 */
void ReadCharacter(TiXmlElement& element, SavemapData::Character& character){
    const int char_id = ReadInt(element, "char_id", -2);
    if (char_id >= -1) character.char_id = char_id;
    element.QueryStringAttribute("name", &character.name);
    character.enabled = (ReadInt(element, "enabled") == 1);
    character.locked = (ReadInt(element, "locked") == 1);
    const int level = ReadInt(element, "level");
    if (level > 0) character.level = level;
    ReadUnsigned(element, "kills", character.kills);
    character.back_row = (ReadInt(element, "back_row") == 1);
    ReadUnsigned(element, "exp", character.exp);
    ReadUnsigned(element, "exp_to_next", character.exp_to_next);
    for (
      TiXmlElement* child = element.FirstChildElement(); child != nullptr;
      child = child->NextSiblingElement()
    ){
        const std::string& name = child->ValueStr();
        if (name == "Stats"){
            ForEachElement(*child, "Stat", [&](TiXmlElement& stat){ReadStat(stat, character);});
        }
        else if (name == "Limits") ReadLimits(*child, character);
        else if (name == "Weapon") ReadEquipment(*child, character.weapon);
        else if (name == "Armor") ReadEquipment(*child, character.armor);
        else if (name == "Accessory"){
            const int accessory = ReadInt(*child, "id", -2);
            if (accessory >= -1) character.accessory = accessory;
        }
        // TODO: Status
    }
}

}

void Savemap::Read(std::string file_name){
    // TODO: This need a try/catch, it's impossible to validate every value and type in the file.
    TiXmlDocument xml_file(file_name);
    if (xml_file.LoadFile() == false){
        empty_ = true;
        return;
    }

    TiXmlElement* root = xml_file.RootElement();
    if (root == nullptr || root->ValueStr() != "Savemap"){
        LOG_ERROR(
          "Savemap: " + file_name + " is not a valid savemap file! No <Savemap> in root."
        );
        empty_ = true;
        return;
    }

    root->QueryIntAttribute("slot", &slot_);
    root->QueryStringAttribute("key", &control_);
    // Games saved without a control key, or with an empty one, get a placeholder.
    if (control_.size() < 2) control_ = "X";
    empty_ = false;

    for (
      TiXmlElement* element = root->FirstChildElement(); element != nullptr;
      element = element->NextSiblingElement()
    ){
        const std::string& name = element->ValueStr();
        if (name == "WindowColours") ReadWindowColours(*element);
        else if (name == "Settings") ReadSettings(*element);
        else if (name == "Money") ReadUnsigned(*element, "quantity", money_);
        else if (name == "GameTime") ReadUnsigned(*element, "seconds", seconds_);
        else if (name == "Countdown") ReadUnsigned(*element, "seconds", countdown_);
        else if (name == "Location") ReadLocation(*element);
        else if (name == "Party") ReadParty(*element);
        else if (name == "Items") ReadItems(*element);
        else if (name == "KeyItems") ReadKeyItems(*element);
        else if (name == "Materia") ReadMaterias(*element, materia_, MAX_MATERIA_SLOTS);
        else if (name == "MateriaStash")
            ReadMaterias(*element, materia_stash_, MAX_STASH_SLOTS);
        else if (name == "Characters") ReadCharacters(*element);
        else if (name == "DataBanks") ReadDataBanks(*element);
    }
}

void Savemap::ReadWindowColours(TiXmlElement& element){
    static const char* const COMPONENTS[3] = {"red", "green", "blue"};
    ForEachElement(element, "Corner", [&](TiXmlElement& corner){
        std::string position;
        corner.QueryStringAttribute("position", &position);
        for (int c = 0; c < 4; c ++){
            if (position != CORNER_NAMES[c]) continue;
            for (int i = 0; i < 3; i ++){
                const int colour = ReadInt(corner, COMPONENTS[i]);
                if (colour >= 0 && colour <= static_cast<int>(MAX_COLOUR))
                    window_colours_[c][i] = colour;
            }
        }
    });
}

void Savemap::ReadSettings(TiXmlElement& element){
    ForEachElement(element, "Setting", [&](TiXmlElement& setting){
        std::string name;
        int value = 0;
        if (
          setting.QueryStringAttribute("name", &name) == TIXML_SUCCESS
          && setting.QueryIntAttribute("value", &value) == TIXML_SUCCESS
        ) settings_[name] = value;
    });
}

void Savemap::ReadLocation(TiXmlElement& element){
    element.QueryFloatAttribute("x", &location_.x);
    element.QueryFloatAttribute("y", &location_.y);
    element.QueryFloatAttribute("z", &location_.z);
    ReadUnsigned(element, "triangle", location_.triangle);
    const int angle = ReadInt(element, "orientation");
    if (angle >= -360 && angle <= 360) location_.angle = angle;
    element.QueryStringAttribute("field", &location_.field);
    element.QueryStringAttribute("name", &location_.name);
}

void Savemap::ReadParty(TiXmlElement& element){
    ForEachElement(element, "Member", [&](TiXmlElement& member){
        const int position = ReadInt(member, "position");
        const int character = ReadInt(member, "character_id");
        if (
          position >= 0 && position < static_cast<int>(MAX_PARTY_MEMBERS)
          && character >= 0 && character < static_cast<int>(MAX_CHARACTERS)
        ) party_[position] = character;
    });
}

void Savemap::ReadItems(TiXmlElement& element){
    ForEachElement(element, "Item", [&](TiXmlElement& item){
        const int slot = ReadInt(item, "slot");
        const int id = ReadInt(item, "id");
        const int quantity = ReadInt(item, "quantity");
        if (slot >= 0 && slot < static_cast<int>(MAX_ITEM_SLOTS) && id >= 0 && quantity > 0){
            items_[slot].id = id;
            items_[slot].quantity = quantity;
        }
    });
}

void Savemap::ReadKeyItems(TiXmlElement& element){
    ForEachElement(element, "Item", [&](TiXmlElement& item){
        const int id = ReadInt(item, "id");
        if (
          id >= 0 && id < static_cast<int>(MAX_KEY_ITEM_SLOTS) && ReadInt(item, "owned") == 1
        ) key_items_[id] = true;
    });
}

void Savemap::ReadCharacters(TiXmlElement& element){
    ForEachElement(element, "Character", [&](TiXmlElement& character){
        const int id = ReadInt(character, "id");
        if (id < 0 || id >= static_cast<int>(MAX_CHARACTERS)) return;
        characters_[id].id = id;
        ReadCharacter(character, characters_[id]);
    });
}

void Savemap::ReadDataBanks(TiXmlElement& element){
    ForEachElement(element, "Bank", [&](TiXmlElement& bank){
        const int id = ReadInt(bank, "id");
        if (id < 0 || id >= static_cast<int>(BANK_COUNT)) return;
        ForEachElement(bank, "Address", [&](TiXmlElement& address){
            const int number = ReadInt(address, "address");
            const int value = ReadInt(address, "value", 0);
            if (number >= 0 && number < static_cast<int>(BANK_ADDRESS_COUNT))
                data_[id][number] = value;
        });
    });
}
