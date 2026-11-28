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

#include <boost/filesystem.hpp>
#include <tinyxml.h>
#include "common/Logger.h"
#include "engine/save/Savemap.h"

namespace{

/**
 * Adds a new element at the end of another one.
 */
TiXmlElement& AddElement(TiXmlElement& parent, const char* name){
    TiXmlElement* element = new TiXmlElement(name);
    // The parent takes care of deleting it.
    parent.LinkEndChild(element);
    return *element;
}

/**
 * Writes a list of materia: the ones of the inventory, the stash or a piece of equipment. The
 * empty slots are not written.
 */
void WriteMaterias(
  TiXmlElement& parent, const char* name, const SavemapData::Materia* materias,
  const unsigned int count
){
    TiXmlElement& list = AddElement(parent, name);
    for (unsigned int i = 0; i < count; i ++){
        const SavemapData::Materia& materia = materias[i];
        if (materia.id < 0) continue;
        TiXmlElement& element = AddElement(list, "Materia");
        element.SetAttribute("slot", i);
        element.SetAttribute("id", materia.id);
        element.SetAttribute("ap", materia.ap);
        if (!materia.enemy_skill) continue;
        element.SetAttribute("enemy_skill", true);
        TiXmlElement& skills = AddElement(element, "EnemySkill");
        for (unsigned int s = 0; s < SavemapData::MAX_ENEMY_SKILLS; s ++){
            if (!materia.enemy_skill_learned[s]) continue;
            TiXmlElement& skill = AddElement(skills, "Skill");
            skill.SetAttribute("id", s);
            skill.SetAttribute("learned", true);
        }
    }
}

/**
 * Writes a stat of a character.
 *
 * @param[in] second Name of the second value of the stat.
 */
void WriteStat(
  TiXmlElement& parent, const char* id, const SavemapData::Character::Stat& stat,
  const char* second
){
    TiXmlElement& element = AddElement(parent, "Stat");
    element.SetAttribute("id", id);
    element.SetAttribute("base", stat.base);
    element.SetAttribute(second, stat.extra);
}

/**
 * Writes a weapon or an armor of a character, with its materia.
 */
void WriteEquipment(
  TiXmlElement& parent, const char* name, const SavemapData::Character::Equipment& equipment
){
    TiXmlElement& element = AddElement(parent, name);
    element.SetAttribute("id", equipment.id);
    WriteMaterias(element, "Materia", equipment.materia, SavemapData::MAX_EQUIP_SLOTS);
}

/**
 * Writes the limit breaks of a character.
 */
void WriteLimits(TiXmlElement& parent, const SavemapData::Character& character){
    TiXmlElement& limits = AddElement(parent, "Limits");
    limits.SetAttribute("current", character.limit_level);
    limits.SetAttribute("bar", character.limit_bar);
    for (unsigned int l = 0; l < SavemapData::MAX_LIMIT_LEVELS; l ++){
        TiXmlElement& level = AddElement(limits, "Level");
        level.SetAttribute("level", l);
        level.SetAttribute("uses", character.limit_uses[l]);
        for (unsigned int t = 0; t < SavemapData::MAX_LIMIT_TECHNIQUES; t ++){
            if (!character.limits_learned[l][t]) continue;
            TiXmlElement& technique = AddElement(level, "Technique");
            technique.SetAttribute("id", t);
            technique.SetAttribute("learned", true);
        }
    }
}

/**
 * Writes a character.
 */
void WriteCharacter(TiXmlElement& parent, const SavemapData::Character& character){
    TiXmlElement& element = AddElement(parent, "Character");
    element.SetAttribute("id", character.id);
    element.SetAttribute("char_id", character.char_id);
    element.SetAttribute("name", character.name);
    element.SetAttribute("level", character.level);
    element.SetAttribute("enabled", character.enabled);
    element.SetAttribute("locked", character.locked);
    element.SetAttribute("kills", character.kills);
    element.SetAttribute("back_row", character.back_row);
    element.SetAttribute("exp", character.exp);
    element.SetAttribute("exp_to_next", character.exp_to_next);

    TiXmlElement& stats = AddElement(element, "Stats");
    WriteStat(stats, "str", character.str, "bonus");
    WriteStat(stats, "vit", character.vit, "bonus");
    WriteStat(stats, "mag", character.mag, "bonus");
    WriteStat(stats, "spr", character.spr, "bonus");
    WriteStat(stats, "dex", character.dex, "bonus");
    WriteStat(stats, "lck", character.lck, "bonus");
    WriteStat(stats, "hp", character.hp, "current");
    WriteStat(stats, "mp", character.mp, "current");

    WriteLimits(element, character);
    WriteEquipment(element, "Weapon", character.weapon);
    WriteEquipment(element, "Armor", character.armor);
    if (character.accessory >= 0)
        AddElement(element, "Accessory").SetAttribute("id", character.accessory);
    // TODO: Statuses
}

}

void Savemap::Write(int slot, std::string file_name){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("Savemap"));
    container->SetAttribute("slot", slot);
    container->SetAttribute("key", control_);

    WriteWindowColours(*container);
    WriteTimes(*container);
    WriteLocation(*container);
    WriteParty(*container);
    WriteItems(*container);
    WriteMaterias(*container, "Materia", materia_, MAX_MATERIA_SLOTS);
    WriteMaterias(*container, "MateriaStash", materia_stash_, MAX_STASH_SLOTS);
    WriteSettings(*container);
    TiXmlElement& characters = AddElement(*container, "Characters");
    for (const Character& character : characters_)
        if (character.char_id != -1) WriteCharacter(characters, character);
    WriteDataBanks(*container);

    xml.LinkEndChild(container.release());
    // The folder of the saved games may not exist yet (a fresh installation): create it, or the
    // game would not be saved.
    const boost::filesystem::path path(file_name);
    if (path.has_parent_path()){
        boost::system::error_code error;
        boost::filesystem::create_directories(path.parent_path(), error);
    }
    if (!xml.SaveFile(file_name)) LOG_ERROR("Can't write the saved game \"" + file_name + "\".");
    slot_ = slot;
}

void Savemap::WriteWindowColours(TiXmlElement& parent) const{
    TiXmlElement& colours = AddElement(parent, "WindowColours");
    for (int c = 0; c < 4; c ++){
        TiXmlElement& corner = AddElement(colours, "Corner");
        corner.SetAttribute("position", CORNER_NAMES[c]);
        corner.SetAttribute("red", window_colours_[c][0]);
        corner.SetAttribute("green", window_colours_[c][1]);
        corner.SetAttribute("blue", window_colours_[c][2]);
    }
}

void Savemap::WriteTimes(TiXmlElement& parent) const{
    if (money_ > 0) AddElement(parent, "Money").SetAttribute("quantity", money_);
    if (seconds_ > 0) AddElement(parent, "GameTime").SetAttribute("seconds", seconds_);
    if (countdown_ > 0) AddElement(parent, "Countdown").SetAttribute("seconds", countdown_);
}

void Savemap::WriteLocation(TiXmlElement& parent) const{
    TiXmlElement& location = AddElement(parent, "Location");
    location.SetDoubleAttribute("x", location_.x);
    location.SetDoubleAttribute("y", location_.y);
    if (location_.z < 0) location.SetAttribute("z", -1);
    else location.SetDoubleAttribute("z", location_.z);
    location.SetAttribute("triangle", location_.triangle);
    location.SetAttribute("orientation", location_.angle);
    location.SetAttribute("field", location_.field);
    location.SetAttribute("name", location_.name);
}

void Savemap::WriteParty(TiXmlElement& parent) const{
    TiXmlElement& party = AddElement(parent, "Party");
    for (unsigned int p = 0; p < MAX_PARTY_MEMBERS; p ++){
        if (party_[p] < 0 || party_[p] >= static_cast<int>(MAX_CHARACTERS)) continue;
        TiXmlElement& member = AddElement(party, "Member");
        member.SetAttribute("position", p);
        member.SetAttribute("character_id", party_[p]);
    }
}

void Savemap::WriteItems(TiXmlElement& parent) const{
    TiXmlElement& items = AddElement(parent, "Items");
    for (unsigned int i = 0; i < MAX_ITEM_SLOTS; i ++){
        if (items_[i].quantity == 0) continue;
        TiXmlElement& item = AddElement(items, "Item");
        item.SetAttribute("slot", i);
        item.SetAttribute("id", items_[i].id);
        item.SetAttribute("quantity", items_[i].quantity);
    }
    TiXmlElement& key_items = AddElement(parent, "KeyItems");
    for (unsigned int i = 0; i < MAX_KEY_ITEM_SLOTS; i ++){
        if (!key_items_[i]) continue;
        TiXmlElement& item = AddElement(key_items, "Item");
        item.SetAttribute("id", i);
        item.SetAttribute("owned", true);
    }
}

void Savemap::WriteSettings(TiXmlElement& parent) const{
    // Settings of the config menu. The window colours are saved apart.
    if (settings_.empty()) return;
    TiXmlElement& settings = AddElement(parent, "Settings");
    for (const auto& setting : settings_){
        TiXmlElement& element = AddElement(settings, "Setting");
        element.SetAttribute("name", setting.first);
        element.SetAttribute("value", setting.second);
    }
}

void Savemap::WriteDataBanks(TiXmlElement& parent) const{
    TiXmlElement& banks = AddElement(parent, "DataBanks");
    for (unsigned int b = 0; b < BANK_COUNT; b ++){
        TiXmlElement& bank = AddElement(banks, "Bank");
        bank.SetAttribute("id", b);
        for (unsigned int a = 0; a < BANK_ADDRESS_COUNT; a ++){
            if (data_[b][a] == 0) continue;
            TiXmlElement& address = AddElement(bank, "Address");
            address.SetAttribute("address", a);
            address.SetAttribute("value", data_[b][a]);
        }
    }
}
