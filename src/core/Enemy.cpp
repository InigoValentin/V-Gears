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

#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>
#include "core/Enemy.h"
#include "core/ConfigVar.h"
#include "core/DebugDraw.h"
#include "core/Logger.h"
#include "core/XmlEnemyFile.h"

Enemy::Enemy(const unsigned int id):
  id_(id),
  name_("UNKNOWN"),
  level_(1),
  exp_(0),
  ap_(0),
  money_(0),
  morph_(-1),
  back_damage_(1.5f),
  str_(0),
  mag_(0),
  def_(0),
  spr_(0),
  dex_(0),
  lck_(0),
  eva_(0),
  meva_(0),
  hp_(1),
  mp_(0),
  hp_max_(1),
  mp_max_(0),
  pos_(Ogre::Vector3(0, 0, 0)),
  front_(false),
  visible_(false),
  active_(false),
  cover_("00000")
{
    ReadFromXml();
    LOG_TRIVIAL("Enemy " + std::to_string(id_) + " '" + name_ + "' created.");
}

Enemy::Enemy(
  const unsigned int id, const Ogre::Vector3 pos, const bool front, const bool visible,
  const bool targeteable, const bool active, const std::string cover
):
  id_(id),
  name_("UNKNOWN"),
  level_(1),
  exp_(0),
  ap_(0),
  money_(0),
  morph_(-1),
  back_damage_(1.5f),
  str_(0),
  mag_(0),
  def_(0),
  spr_(0),
  dex_(0),
  lck_(0),
  eva_(0),
  meva_(0),
  hp_(1),
  mp_(0),
  hp_max_(1),
  mp_max_(0),
  pos_(pos),
  front_(front),
  visible_(visible),
  active_(active),
  cover_(cover)
{
    ReadFromXml();
    LOG_TRIVIAL("Enemy " + std::to_string(id_) + " '" + name_ + "' created.");
}

Enemy::~Enemy(){
    LOG_TRIVIAL("Enemy " + std::to_string(id_) + " '" + name_ + "' destroyed.");
}


const int Enemy::GetId() const{return id_;}

void Enemy::SetId(const int id){id_ = id;}

const std::string Enemy::GetModel() const{return model_;}

void Enemy::SetModel(const std::string model){model_ = model;}

const Ogre::String& Enemy::GetName() const{return name_;}

void Enemy::SetName(const Ogre::String& name){name_ = name;}

unsigned int Enemy::GetLevel() const{return level_;}

void Enemy::SetLevel(const unsigned int level){level_ = level;}

unsigned int Enemy::GetAp() const{return ap_;}

void Enemy::SetAp(unsigned int ap){ap_ = ap;}

unsigned int Enemy::GetExp() const{return exp_;}

void Enemy::SetExp(const unsigned int exp){exp_ = exp;}

unsigned int Enemy::GetMoney() const{return money_;}

void Enemy::SetMoney(const unsigned int money){money_ = money;}

std::vector<unsigned int> Enemy::GetAnimations() const{return animations_;}

void Enemy::AddAnimation(const unsigned int animation){animations_.push_back(animation);}

std::vector<Enemy::Attack> Enemy::GetAttacks() const{return attacks_;}

void Enemy::AddAttack(const Attack attack){attacks_.push_back(attack);}

std::vector<Enemy::Item> Enemy::GetDrop() const{return drop_;}

void Enemy::AddDrop(const Item item){drop_.push_back(item);}

std::vector<Enemy::Element> Enemy::GetElements() const{return elements_;}

void Enemy::AddElement(const Element element){elements_.push_back(element);}

std::vector<Enemy::Immunity> Enemy::GetImmunities() const{return immunities_;}

void Enemy::AddImmunity(const Immunity immunity){immunities_.push_back(immunity);}

std::vector<unsigned int> Enemy::GetManipulateAttacks() const{return manipulate_attacks_;}

void Enemy::AddManipulateAttack(const unsigned int attack){manipulate_attacks_.push_back(attack);}

std::vector<Enemy::Item> Enemy::GetSteal() const{return steal_;}

void Enemy::AddSteal(const Item item){steal_.push_back(item);}

unsigned int Enemy::GetMorph() const{return morph_;}

void Enemy::SetMorph(const int morph){morph_ = std::max(-1, morph);}

float Enemy::GetBackDamage() const{return back_damage_;}

void Enemy::SetBackDamage(const float back_damage){
    if (back_damage < 0.0f) back_damage_ = 0.0f;
    else back_damage_ = back_damage;
}

unsigned int Enemy::GetStr() const{return str_;}

void Enemy::SetStr(const unsigned int str){str_ = str;}

unsigned int Enemy::GetDef() const{return def_;}

void Enemy::SetDef(const unsigned int def){def_ = def;}

unsigned int Enemy::GetMag() const{return mag_;}

void Enemy::SetMag(const unsigned int mag){mag_ = mag;}

unsigned int Enemy::GetSpr() const{return spr_;}

void Enemy::SetSpr(const unsigned int spr){spr_ = spr;}

unsigned int Enemy::GetDex() const{return dex_;}

void Enemy::SetDex(const unsigned int dex){dex_ = dex;}

unsigned int Enemy::GetLck() const{return lck_;}

void Enemy::SetLck(const unsigned int lck){lck_ = lck;}

unsigned int Enemy::GetEva() const{return eva_;}

void Enemy::SetEva(const unsigned int eva){eva_ = eva;}

unsigned int Enemy::GetMeva() const{return meva_;}

void Enemy::SetMeva(const unsigned int meva){meva_ = meva;}

unsigned int Enemy::GetHp() const{return hp_;}

void Enemy::SetHp(const unsigned int hp){hp_ = std::min(hp, hp_max_);}

unsigned int Enemy::GetHpMax() const{return hp_max_;}

void Enemy::SetHpMax(const unsigned int hp_max){
    hp_max_ = hp_max;
    hp_ = std::min(hp_, hp_max_);
}

unsigned int Enemy::GetMp() const{return mp_;}

void Enemy::SetMp(const unsigned int mp){mp_ = std::min(mp, mp_max_);}

unsigned int Enemy::GetMpMax() const{return mp_max_;}

void Enemy::SetMpMax(const unsigned int mp_max){
    mp_max_ = mp_max;
    mp_ = std::min(mp_, mp_max_);
}

Ogre::Vector3& Enemy::GetPos(){return pos_;}

void Enemy::SetPos(const Ogre::Vector3 &pos){pos_ = pos;}

bool Enemy::IsFront() const{return front_;}

void Enemy::SetFront(bool front){front_ = front;}

bool Enemy::IsVisible() const{return visible_;}

void Enemy::SetVisible(bool visible){visible_ = visible;}

bool Enemy::IsTargeteable() const{return targeteable_;}

void Enemy::SetTargeteable(bool targeteable){targeteable_ = targeteable;}

bool Enemy::IsActive() const{return active_;}

void Enemy::SetActive(bool active){active_ = active;}

std::string Enemy::GetCover() const{return cover_;}

void Enemy::SetCover(std::string cover){cover_ = cover;}

void Enemy::ReadFromXml(){
    std::string filename = std::to_string(id_);
    while (filename.length() < 4) filename = "0" + filename;
    filename = "./data/gamedata/enemy/" + filename + ".xml";
    XmlEnemyFile(filename).LoadEnemy(*this);
}




