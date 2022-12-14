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
#include "core/SavemapManager.h"
#include "core/Logger.h"

/**
 * Savemap manager singleton.
 */
template<>SavemapManager *Ogre::Singleton<SavemapManager>::msSingleton = nullptr;

int SavemapManager::MAX_SAVE_SLOTS(15);

std::string SavemapManager::SAVE_PATH("save/");

SavemapManager::SavemapManager(): current_savemap_(nullptr), savemaps_read_(false){}

SavemapManager::~SavemapManager(){
    //TODO
    LOG_TRIVIAL("SavemapManager destroyed.");
}

Savemap SavemapManager::GetCurrentSavemap(){return *current_savemap_;}

Savemap* SavemapManager::GetSavemap(unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return nullptr;
    if (!savemaps_read_) ReadSavemaps();
    return saved_savemaps_[slot];
}

std::vector<Savemap*> SavemapManager::GetSavemaps(){
    if (!savemaps_read_) ReadSavemaps();
    return saved_savemaps_;
}

bool SavemapManager::Save(unsigned int slot, const bool force){
    if (current_savemap_ == nullptr) return false;
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (!savemaps_read_) ReadSavemaps();
    if (
        force == false && saved_savemaps_[slot]->IsEmpty() == false
      && current_savemap_->GetControlKey() != saved_savemaps_[slot]->GetControlKey()
    ){
        return false;
    }
    else{
        saved_savemaps_[slot] = current_savemap_;
        saved_savemaps_[slot]->Write(slot, SAVE_PATH + std::to_string(slot) + ".xml");
        return true;
    }
}

bool SavemapManager::Save(Savemap savemap, unsigned int slot, bool force){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (!savemaps_read_) ReadSavemaps();
    if (
      force == false && saved_savemaps_[slot]->IsEmpty() == false
      && savemap.GetControlKey() != saved_savemaps_[slot]->GetControlKey()
    ){
        return false;
    }
    else{
        *current_savemap_ = savemap;
        saved_savemaps_[slot] = current_savemap_;
        saved_savemaps_[slot]->Write(slot, SAVE_PATH + std::to_string(slot) + ".xml");
        return true;
    }
}

void SavemapManager::Release(){
    current_savemap_ = nullptr;
    saved_savemaps_.clear();
    savemaps_read_ = false;
}

void SavemapManager::ReadSavemaps(){
    saved_savemaps_.clear();
    for (int i = 0; i < MAX_SAVE_SLOTS; i ++){
        Savemap* savemap = new Savemap();
        savemap->Read(SAVE_PATH + std::to_string(i) + ".xml");
        saved_savemaps_.push_back(savemap);
    }
    savemaps_read_ = true;
}

void SavemapManager::SetData(const unsigned int bank, const unsigned int address, const int value){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetData(bank, address, value);
}

void SavemapManager::SetControlKey(const char* control){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetControlKey(std::string(control));
}

void SavemapManager::SetWindowColours(
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

void SavemapManager::SetMoney(const unsigned int money){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetMoney(money);
}

void SavemapManager::SetGameTime(const unsigned int seconds){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetGameTime(seconds);
}
void SavemapManager::SetCountdownTime(const unsigned int seconds){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCountdownTime(seconds);
}

void SavemapManager::SetKeyItem(const unsigned int item, const bool owned){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetKeyItem(item, owned);
}

void SavemapManager::SetParty(const int member_1, const int member_2, const int member_3){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetParty(member_1, member_2, member_3);
}

void SavemapManager::SetItem(
  const unsigned int slot, const unsigned int id, const unsigned int quantity
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetItem(slot, id, quantity);
}

void SavemapManager::SetMateria(const unsigned int slot, const int id, const unsigned int ap){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetMateria(slot, id, ap);
}

void SavemapManager::SetESkillMateria(
  const unsigned slot, const unsigned int skill, const bool learned
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetESkillMateria(slot, skill, learned);
}

void SavemapManager::SetMateriaStash(const unsigned int slot, const int id, const unsigned int ap){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetMateriaStash(slot, id, ap);
}

void SavemapManager::SetESkillMateriaStash(
  const unsigned slot, const unsigned int skill, const bool learned
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetESkillMateriaStash(slot, skill, learned);
}

void SavemapManager::SetLocation(
  const float x, const float y, const float z,
  const unsigned int triangle, const int angle, const char* field, const char* name
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetLocation(x, y, z, triangle, angle, std::string(field), std::string(name));
}

void SavemapManager::SetSetting(const unsigned int key, const unsigned int value){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetSetting(key, value);
}

void SavemapManager::SetCharacterInfo(
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

void SavemapManager::SetCharacterStat(
  const unsigned int id, const unsigned int stat, const unsigned int base, const unsigned int extra
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterStat(id, stat, base, extra);
}

void SavemapManager::SetCharacterLimitLearned(
  const unsigned int id, const unsigned int level,
  const unsigned int technique, const bool learned, const unsigned int uses
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterLimitLearned(id, level, technique, learned, uses);
}

void SavemapManager::SetCharacterMateria(
  const unsigned int id, const bool weapon, const unsigned int slot,
  const int materia, const unsigned int ap
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterMateria(id, weapon, slot, materia, ap);
}

void SavemapManager::SetCharacterESkillMateria(
  const unsigned int id, const bool weapon, const unsigned int slot,
  const unsigned int skill, const bool learned
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterESkillMateria(id, weapon, slot, skill, learned);
}

void SavemapManager::SetCharacterStatus(
  const unsigned int id, const unsigned int status, const bool inflicted
){
    if (current_savemap_ == nullptr) current_savemap_ = new Savemap();
    current_savemap_->SetCharacterStatus(id, status, inflicted);
}

bool SavemapManager::IsSlotEmpty(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return true;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsEmpty();
}

std::string SavemapManager::GetSlotControlKey(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return std::string("");
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetControlKey();
}

unsigned int SavemapManager::GetSlotWindowCornerColourComponent(
  const unsigned int slot, const unsigned int corner, const unsigned int comp
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetWindowCornerColourComponent(corner, comp);
}

unsigned int SavemapManager::GetSlotMoney(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetMoney();
}

unsigned int SavemapManager::GetSlotGameTime(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetGameTime();
}

unsigned int SavemapManager::GetSlotCountdownTime(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCountdownTime();
}

int SavemapManager::GetSlotPartyMember(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return -1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetPartyMember(pos);
}

unsigned int SavemapManager::GetSlotItemAtPosId(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetItemAtPosId(pos);
}

unsigned int SavemapManager::GetSlotItemAtPosQty(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetItemAtPosQty(pos);
}

bool SavemapManager::GetSlotKeyItem(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetKeyItem(id);
}

int SavemapManager::GetSlotMateriaAtPosId(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return -1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetMateriaAtPosId(pos);
}

unsigned int SavemapManager::GetSlotMateriaAtPosAp(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetMateriaAtPosAp(pos);
}

bool SavemapManager::IsSlotMateriaAtPosESkill(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsMateriaAtPosESkill(pos);
}

bool SavemapManager::IsSlotMateriaAtPosESkillLearned(const unsigned int slot, const unsigned int pos, const unsigned int skill){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsMateriaAtPosESkillLearned(pos, skill);
}

int SavemapManager::GetSlotStashAtPosId(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return -1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetStashAtPosId(pos);
}

unsigned int SavemapManager::GetSlotStashAtPosAp(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetStashAtPosAp(pos);
}

bool SavemapManager::IsSlotStashAtPosESkill(const unsigned int slot, const unsigned int pos){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsStashAtPosESkill(pos);
}

bool SavemapManager::IsSlotStashAtPosESkillLearned(const unsigned int slot, const unsigned int pos, const unsigned int skill){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsStashAtPosESkillLearned(pos, skill);
}

float SavemapManager::GetSlotLocationX(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0.0f;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationX();
}

float SavemapManager::GetSlotLocationY(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0.0f;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationY();
}

float SavemapManager::GetSlotLocationZ(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return -1.0f;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationZ();
}

unsigned int SavemapManager::GetSlotLocationTriangle(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationTriangle();
}

int SavemapManager::GetSlotLocationAngle(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationAngle();
}

std::string SavemapManager::GetSlotLocationField(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return "";
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationField();
}

std::string SavemapManager::GetSlotLocationName(const unsigned int slot){
    if (slot >= MAX_SAVE_SLOTS) return "";
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetLocationName();
}

int SavemapManager::GetSlotSetting(const unsigned int slot, const unsigned int key){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetSetting(key);
}

int SavemapManager::GetSlotCharacterCharId(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterCharId(id);
}

std::string SavemapManager::GetSlotCharacterName(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return "";
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterName(id);
}

unsigned int SavemapManager::GetSlotCharacterLevel(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return 1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterLevel(id);
}

unsigned int SavemapManager::GetSlotCharacterKills(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterKills(id);
}

bool SavemapManager::IsSlotCharacterEnabled(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterEnabled(id);
}

bool SavemapManager::IsSlotCharacterLocked(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterLocked(id);
}

bool SavemapManager::IsSlotCharacterBackRow(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterBackRow(id);
}

unsigned int SavemapManager::GetSlotCharacterExp(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterExp(id);
}

unsigned int SavemapManager::GetSlotCharacterExpToNext(
  const unsigned int slot, const unsigned int id
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterExpToNext(id);
}

unsigned int SavemapManager::GetSlotCharacterLimitLevel(
  const unsigned int slot, const unsigned int id
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterLimitLevel(id);
}

unsigned int SavemapManager::GetSlotCharacterLimitBar(
  const unsigned int slot, const unsigned int id
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterLimitBar(id);
}

unsigned int SavemapManager::GetSlotCharacterWeaponId(
  const unsigned int slot, const unsigned int id
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterWeaponId(id);
}

unsigned int SavemapManager::GetSlotCharacterArmorId(
  const unsigned int slot, const unsigned int id
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterArmorId(id);
}

int SavemapManager::GetSlotCharacterAccessoryId(const unsigned int slot, const unsigned int id){
    if (slot >= MAX_SAVE_SLOTS) return -1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterAccessoryId(id);
}

unsigned int SavemapManager::GetSlotCharacterStatBase(
  const unsigned int slot, const unsigned int id, const unsigned int stat
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterStatBase(id, stat);
}

unsigned int SavemapManager::GetSlotCharacterStatExtra(
  const unsigned int slot, const unsigned int id, const unsigned int stat
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterStatExtra(id, stat);
}

unsigned int SavemapManager::GetSlotCharacterLimitUses(
  const unsigned int slot, const unsigned int id, const unsigned int level
){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterLimitUses(id, level);
}

bool SavemapManager::IsSlotCharacterLimitLearned(
  const unsigned int slot, const unsigned int id, const unsigned int level, const unsigned int tech
){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterLimitLearned(id, level, tech);
}

int SavemapManager::GetSlotCharacterMateriaId(
  const unsigned int slot, const unsigned int id, const bool weapon, const unsigned int pos
){
    if (slot >= MAX_SAVE_SLOTS) return -1;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterMateriaId(id, weapon, pos);
}

unsigned int SavemapManager::GetSlotCharacterMateriaAp(
  const unsigned int slot, const unsigned int id, const bool weapon, const unsigned int pos
 ){
    if (slot >= MAX_SAVE_SLOTS) return 0;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetCharacterMateriaAp(id, weapon, pos);
}

bool SavemapManager::IsSlotCharacterMateriaESkill(
  const unsigned int slot, const unsigned int id, const bool weapon, const unsigned int pos
){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterMateriaESkill(id, weapon, pos);
}

bool SavemapManager::IsSlotCharacterMateriaESkillLearned(
  const unsigned int slot, const unsigned int id, const bool weapon,
  const unsigned int pos, const unsigned int skill
){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->IsCharacterMateriaESkillLearned(id, weapon, pos, skill);
}

int SavemapManager::GetSlotData(
  const unsigned int slot, const unsigned int bank, const unsigned int address
){
    if (slot >= MAX_SAVE_SLOTS) return false;
    if (savemaps_read_ == false) ReadSavemaps();
    return saved_savemaps_[slot]->GetData(bank, address);
}







