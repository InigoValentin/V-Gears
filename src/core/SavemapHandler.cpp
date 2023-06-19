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

#include <iostream>
#include <string>
#include <vector>
#include "core/SavemapHandler.h"
#include "core/Logger.h"

/**
 * Savemap handler singleton.
 */
template<>SavemapHandler *Ogre::Singleton<SavemapHandler>::msSingleton = nullptr;

int SavemapHandler::MAX_SAVE_SLOTS(15);

std::string SavemapHandler::SAVE_PATH("save/");

SavemapHandler::SavemapHandler(): current_savemap_(nullptr), savemaps_read_(false){}

SavemapHandler::~SavemapHandler(){
    //TODO
    LOG_TRIVIAL("SavemapHandler destroyed.");
}

Savemap SavemapHandler::GetCurrentSavemap(){return *current_savemap_;}

Savemap* SavemapHandler::GetSavemap(unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return nullptr;
    if (!savemaps_read_) ReadSavemaps();
    return saved_savemaps_[slot];
}

std::vector<Savemap*> SavemapHandler::GetSavemaps(){
    if (!savemaps_read_) ReadSavemaps();
    return saved_savemaps_;
}

bool SavemapHandler::Save(unsigned int slot, const bool force){
    if (current_savemap_ == nullptr) return false;
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (!savemaps_read_) ReadSavemaps();
    if (
      force == false && saved_savemaps_[slot]->IsEmpty() == false
      && current_savemap_->GetControlKey() != saved_savemaps_[slot]->GetControlKey()
    ) return false;
    else{
        saved_savemaps_[slot] = current_savemap_;
        saved_savemaps_[slot]->Write(slot, SAVE_PATH + std::to_string(slot) + ".xml");
        return true;
    }
}

bool SavemapHandler::Save(Savemap savemap, unsigned int slot, bool force){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (!savemaps_read_) ReadSavemaps();
    if (
      force == false && saved_savemaps_[slot]->IsEmpty() == false
      && savemap.GetControlKey() != saved_savemaps_[slot]->GetControlKey()
    ) return false;
    else{
        *current_savemap_ = savemap;
        saved_savemaps_[slot] = current_savemap_;
        saved_savemaps_[slot]->Write(slot, SAVE_PATH + std::to_string(slot) + ".xml");
        return true;
    }
}

void SavemapHandler::Release(){
    current_savemap_ = nullptr;
    saved_savemaps_.clear();
    savemaps_read_ = false;
}

void SavemapHandler::ReadSavemaps(){
    saved_savemaps_.clear();
    for (int i = 0; i < MAX_SAVE_SLOTS; i ++){
        Savemap* savemap = new Savemap();
        savemap->Read(SAVE_PATH + std::to_string(i) + ".xml");
        saved_savemaps_.push_back(savemap);
    }
    savemaps_read_ = true;
}

void SavemapHandler::SetData(const unsigned int bank, const unsigned int address, const int value){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetData(bank, address, value);
}

void SavemapHandler::SetControlKey(const char* control){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetControlKey(std::string(control));
}

void SavemapHandler::SetWindowColours(
  const unsigned int t_l_r, const unsigned int t_l_g, const unsigned int t_l_b,
  const unsigned int t_r_r, const unsigned int t_r_g, const unsigned int t_r_b,
  const unsigned int b_r_r, const unsigned int b_r_g, const unsigned int b_r_b,
  const unsigned int b_l_r, const unsigned int b_l_g, const unsigned int b_l_b
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetWindowColours(
      t_l_r, t_l_g, t_l_b, t_r_r, t_r_g, t_r_b, b_r_r, b_r_g, b_r_b, b_l_r, b_l_g, b_l_b
    );
}

void SavemapHandler::SetMoney(const unsigned int money){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetMoney(money);
}

void SavemapHandler::SetGameTime(const unsigned int seconds){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetGameTime(seconds);
}
void SavemapHandler::SetCountdownTime(const unsigned int seconds){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCountdownTime(seconds);
}

void SavemapHandler::SetKeyItem(const unsigned int item, const bool owned){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetKeyItem(item, owned);
}

void SavemapHandler::SetParty(const int member_1, const int member_2, const int member_3){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetParty(member_1, member_2, member_3);
}

void SavemapHandler::SetItem(
  const unsigned int slot, const unsigned int id, const unsigned int quantity
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetItem(slot, id, quantity);
}

void SavemapHandler::SetMateria(const unsigned int slot, const int id, const unsigned int ap){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetMateria(slot, id, ap);
}

void SavemapHandler::SetESkillMateria(
  const unsigned slot, const unsigned int skill, const bool learned
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetESkillMateria(slot, skill, learned);
}

void SavemapHandler::SetMateriaStash(const unsigned int slot, const int id, const unsigned int ap){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetMateriaStash(slot, id, ap);
}

void SavemapHandler::SetESkillMateriaStash(
  const unsigned slot, const unsigned int skill, const bool learned
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetESkillMateriaStash(slot, skill, learned);
}

void SavemapHandler::SetLocation(
  const float x, const float y, const float z,
  const unsigned int triangle, const int angle, const char* field, const char* name
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetLocation(x, y, z, triangle, angle, std::string(field), std::string(name));
}

void SavemapHandler::SetSetting(const unsigned int key, const unsigned int value){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetSetting(key, value);
}

void SavemapHandler::SetCharacterInfo(
  const unsigned int id, const int char_id, const char* name,
  const bool enabled, const bool locked,
  const unsigned int level, const unsigned int kills,
  const bool back_row, const unsigned int exp, const unsigned int exp_to_next,
  const unsigned int limit_level, const unsigned int limit_bar,
  const unsigned int weapon, const unsigned int armor, const int accessory
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterInfo(
      id, char_id, std::string(name), enabled, locked, level, kills, back_row, exp, exp_to_next,
      limit_level, limit_bar, weapon, armor, accessory
    );
}

void SavemapHandler::SetCharacterStat(
  const unsigned int id, const unsigned int stat, const unsigned int base, const unsigned int extra
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterStat(id, stat, base, extra);
}

void SavemapHandler::SetCharacterLimitLearned(
  const unsigned int id, const unsigned int level,
  const unsigned int technique, const bool learned, const unsigned int uses
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterLimitLearned(id, level, technique, learned, uses);
}

void SavemapHandler::SetCharacterMateria(
  const unsigned int id, const bool weapon, const unsigned int slot,
  const int materia, const unsigned int ap
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterMateria(id, weapon, slot, materia, ap);
}

void SavemapHandler::SetCharacterESkillMateria(
  const unsigned int id, const bool weapon, const unsigned int slot,
  const unsigned int skill, const bool learned
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterESkillMateria(id, weapon, slot, skill, learned);
}

void SavemapHandler::SetCharacterStatus(
  const unsigned int id, const unsigned int status, const bool inflicted
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterStatus(id, status, inflicted);
}

bool SavemapHandler::IsSlotEmpty(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return true;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsEmpty();
}

std::string SavemapHandler::GetSlotControlKey(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return std::string("");
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetControlKey();
}

unsigned int SavemapHandler::GetSlotWindowCornerColourComponent(
  const unsigned int slot, const unsigned int corner, const unsigned int comp
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetWindowCornerColourComponent(corner, comp);
}

unsigned int SavemapHandler::GetSlotMoney(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetMoney();
}

unsigned int SavemapHandler::GetSlotGameTime(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetGameTime();
}

unsigned int SavemapHandler::GetSlotCountdownTime(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCountdownTime();
}

int SavemapHandler::GetSlotPartyMember(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return -1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetPartyMember(pos);
}

unsigned int SavemapHandler::GetSlotItemAtPosId(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetItemAtPosId(pos);
}

unsigned int SavemapHandler::GetSlotItemAtPosQty(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetItemAtPosQty(pos);
}

bool SavemapHandler::GetSlotKeyItem(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetKeyItem(id);
}

int SavemapHandler::GetSlotMateriaAtPosId(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return -1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetMateriaAtPosId(pos);
}

unsigned int SavemapHandler::GetSlotMateriaAtPosAp(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetMateriaAtPosAp(pos);
}

bool SavemapHandler::IsSlotMateriaAtPosESkill(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsMateriaAtPosESkill(pos);
}

bool SavemapHandler::IsSlotMateriaAtPosESkillLearned(
  const unsigned int slot, const unsigned int pos, const unsigned int skill
){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsMateriaAtPosESkillLearned(pos, skill);
}

int SavemapHandler::GetSlotStashAtPosId(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return -1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetStashAtPosId(pos);
}

unsigned int SavemapHandler::GetSlotStashAtPosAp(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetStashAtPosAp(pos);
}

bool SavemapHandler::IsSlotStashAtPosESkill(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsStashAtPosESkill(pos);
}

bool SavemapHandler::IsSlotStashAtPosESkillLearned(
  const unsigned int slot, const unsigned int pos, const unsigned int skill
){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsStashAtPosESkillLearned(pos, skill);
}

float SavemapHandler::GetSlotLocationX(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0.0f;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationX();
}

float SavemapHandler::GetSlotLocationY(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0.0f;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationY();
}

float SavemapHandler::GetSlotLocationZ(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return -1.0f;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationZ();
}

unsigned int SavemapHandler::GetSlotLocationTriangle(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationTriangle();
}

int SavemapHandler::GetSlotLocationAngle(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationAngle();
}

std::string SavemapHandler::GetSlotLocationField(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return "";
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationField();
}

std::string SavemapHandler::GetSlotLocationName(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return "";
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationName();
}

int SavemapHandler::GetSlotSetting(const unsigned int slot, const unsigned int key){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetSetting(key);
}

int SavemapHandler::GetSlotCharacterCharId(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterCharId(id);
}

std::string SavemapHandler::GetSlotCharacterName(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return "";
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterName(id);
}

unsigned int SavemapHandler::GetSlotCharacterLevel(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return 1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterLevel(id);
}

unsigned int SavemapHandler::GetSlotCharacterKills(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterKills(id);
}

bool SavemapHandler::IsSlotCharacterEnabled(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterEnabled(id);
}

bool SavemapHandler::IsSlotCharacterLocked(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterLocked(id);
}

bool SavemapHandler::IsSlotCharacterBackRow(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterBackRow(id);
}

unsigned int SavemapHandler::GetSlotCharacterExp(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterExp(id);
}

unsigned int SavemapHandler::GetSlotCharacterExpToNext(
  const unsigned int slot, const unsigned int id
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterExpToNext(id);
}

unsigned int SavemapHandler::GetSlotCharacterLimitLevel(
  const unsigned int slot, const unsigned int id
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterLimitLevel(id);
}

unsigned int SavemapHandler::GetSlotCharacterLimitBar(
  const unsigned int slot, const unsigned int id
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterLimitBar(id);
}

unsigned int SavemapHandler::GetSlotCharacterWeaponId(
  const unsigned int slot, const unsigned int id
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterWeaponId(id);
}

unsigned int SavemapHandler::GetSlotCharacterArmorId(
  const unsigned int slot, const unsigned int id
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterArmorId(id);
}

int SavemapHandler::GetSlotCharacterAccessoryId(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return -1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterAccessoryId(id);
}

unsigned int SavemapHandler::GetSlotCharacterStatBase(
  const unsigned int slot, const unsigned int id, const unsigned int stat
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterStatBase(id, stat);
}

unsigned int SavemapHandler::GetSlotCharacterStatExtra(
  const unsigned int slot, const unsigned int id, const unsigned int stat
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterStatExtra(id, stat);
}

unsigned int SavemapHandler::GetSlotCharacterLimitUses(
  const unsigned int slot, const unsigned int id, const unsigned int level
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterLimitUses(id, level);
}

bool SavemapHandler::IsSlotCharacterLimitLearned(
  const unsigned int slot, const unsigned int id, const unsigned int level, const unsigned int tech
){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterLimitLearned(id, level, tech);
}

int SavemapHandler::GetSlotCharacterMateriaId(
  const unsigned int slot, const unsigned int id, const bool weapon, const unsigned int pos
){
    if (slot >= MAX_SAVE_SLOTS) return -1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterMateriaId(id, weapon, pos);
}

unsigned int SavemapHandler::GetSlotCharacterMateriaAp(
  const unsigned int slot, const unsigned int id, const bool weapon, const unsigned int pos
 ){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterMateriaAp(id, weapon, pos);
}

bool SavemapHandler::IsSlotCharacterMateriaESkill(
  const unsigned int slot, const unsigned int id, const bool weapon, const unsigned int pos
){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterMateriaESkill(id, weapon, pos);
}

bool SavemapHandler::IsSlotCharacterMateriaESkillLearned(
  const unsigned int slot, const unsigned int id, const bool weapon,
  const unsigned int pos, const unsigned int skill
){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterMateriaESkillLearned(id, weapon, pos, skill);
}

int SavemapHandler::GetSlotData(
  const unsigned int slot, const unsigned int bank, const unsigned int address
){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetData(bank, address);
}







