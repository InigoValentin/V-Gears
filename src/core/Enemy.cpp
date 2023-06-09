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

#include <cmath>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>
#include "core/Enemy.h"
#include "core/ConfigVar.h"
#include "core/DebugDraw.h"
#include "core/Logger.h"

ConfigVar cv_debug_enemy("debug_enemy", "Draw enemy debug info", "0");

Enemy::Enemy(const int enemy_id, Ogre::SceneNode* node):
  enemy_id_(enemy_id),
  name_("UNKNOWN"),
  scene_node_(node),
  height_(1.0f),
  move_position_(Ogre::Vector3(0, 0, 0)),
  move_auto_rotation_(true),
  offset_position_start_(0.0f, 0.0f, 0.0f),
  offset_position_end_(0.0f, 0.0f, 0.0f),
  offset_type_(AT_NONE),
  offset_seconds_(0.0f),
  offset_current_seconds_(0.0f),
  turn_direction_(TD_CLOSEST),
  turn_direction_start_(0),
  turn_direction_end_(0),
  turn_type_(AT_NONE),
  turn_seconds_(0.0f),
  turn_current_seconds_(0.0f),
  animation_speed_(1.0f),
  animation_default_("Idle"),
  animation_current_name_(""),
  animation_auto_play_(true)
{
    model_root_node_ = scene_node_->createChildSceneNode();
    model_node_ = model_root_node_->createChildSceneNode();
    model_root_node_->setPosition(Ogre::Vector3::ZERO);
    model_node_->setPosition(Ogre::Vector3::ZERO);
    scene_node_->setPosition(Ogre::Vector3::ZERO);
    LOG_TRIVIAL("Enemy " + std::to_string(enemy_id_) + " '" + name_ + "' created.");
}

Enemy::~Enemy(){
    scene_node_->removeAndDestroyAllChildren();
    LOG_TRIVIAL("Enemy " + std::to_string(enemy_id_) + " '" + name_ + "' destroyed.");
}

void Enemy::Update(){}

void Enemy::UpdateDebug(){
    int debug = cv_debug_enemy.GetI();
    if (debug > 0){
        DEBUG_DRAW.SetColour(Ogre::ColourValue::White);
        DEBUG_DRAW.SetScreenSpace(true);
        DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.CENTER);
        DEBUG_DRAW.SetFadeDistance(40, 50);
        Ogre::Vector3 enemy_pos = GetPosition();
        DEBUG_DRAW.Text(enemy_pos, 0, 0, name_);
        DEBUG_DRAW.Text(enemy_pos, 0, 12, animation_current_name_);
    }
}

const int Enemy::GetEnemyId() const{return enemy_id_;}

void Enemy::SetEnemyId(const int id){enemy_id_ = id;}

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

void Enemy::SetPosition(const Ogre::Vector3& position){scene_node_->setPosition(position);}

void Enemy::ScriptSetPosition(const float x, const float y, const float z){
    SetPosition(Ogre::Vector3(x, y, z));
    // Make the entity visible. TODO: Check if necessary.
    SetVisible(true);
}

const Ogre::Vector3 Enemy::GetPosition() const{return scene_node_->getPosition();}

void Enemy::ScriptGetPosition() const{
    Ogre::Vector3 position = scene_node_->getPosition();
    ScriptManager::getSingleton().AddValueToStack(position.x);
    ScriptManager::getSingleton().AddValueToStack(position.y);
    ScriptManager::getSingleton().AddValueToStack(position.z);
}

void Enemy::SetOffset(const Ogre::Vector3& position){
    assert(model_root_node_);
    model_root_node_->setPosition(position);
}

const Ogre::Vector3 Enemy::GetOffset() const{
    assert(model_root_node_);
    return model_root_node_->getPosition();
}

void Enemy::SetRotation(const Ogre::Degree& rotation){
    assert(model_root_node_);
    float angle
      = rotation.valueDegrees() - Ogre::Math::Floor(rotation.valueDegrees() / 360.0f) * 360.0f;
    if (angle < 0) angle = 360 + angle;
    Ogre::Quaternion q;
    Ogre::Vector3 vec = Ogre::Vector3::UNIT_Z;
    q.FromAngleAxis(Ogre::Radian(Ogre::Degree(angle)), vec);
    model_root_node_->setOrientation(q);
}

void Enemy::ScriptSetRotation(const float rotation){SetRotation(Ogre::Degree(rotation));}

Ogre::Degree Enemy::GetRotation() const{
    assert(model_root_node_);
    Ogre::Quaternion q = model_root_node_->getOrientation();
    Ogre::Degree temp;
    Ogre::Vector3 vec = Ogre::Vector3::UNIT_Z;
    q.ToAngleAxis(temp, vec);
    return temp;
}

float Enemy::ScriptGetRotation() const{return GetRotation().valueDegrees();}

void Enemy::setScale(const Ogre::Vector3 &scale) {
    assert(model_root_node_);
    model_root_node_->setScale(scale);
}

void Enemy::SetIndex(const int index){
    assert(model_root_node_);
    index_ = index;
}

int Enemy::GetIndex(){return index_;}

void Enemy::setRootOrientation(const Ogre::Quaternion &root_orientation){
    assert(model_node_);
    model_node_->setOrientation(root_orientation);
}

float Enemy::GetHeight() const{return height_;}

void Enemy::SetMoveSpeed(const float speed){move_speed_ = speed;}

float Enemy::GetMoveSpeed() const{return move_speed_;}

void Enemy::SetMovePosition(const Ogre::Vector3& target){move_position_ = target;}

const Ogre::Vector3& Enemy::GetMovePosition() const{return move_position_;}

float Enemy::GetMoveStopDistance() const{return move_stop_distance_;}

void Enemy::ScriptMoveToPosition(const float x, const float y){
    move_position_ = Ogre::Vector3(x, y, 0);
    move_stop_distance_ = 0;
    LOG_TRIVIAL(
      "Enemy " + std::to_string(enemy_id_) + " '" + name_ + "' set move to position '"
      + Ogre::StringConverter::toString(move_position_) + "'."
    );
}

void Enemy::UnsetMove(){
    move_stop_distance_ = 0;
    sync_.clear();
}

void Enemy::ScriptOffsetToPosition(
  const float x, const float y, const float z, const ActionType type, const float seconds
){
    LOG_TRIVIAL(
      "Enemy " + std::to_string(enemy_id_) +" '" + name_ + "' set offset to position '"
      + Ogre::StringConverter::toString(Ogre::Vector3(x, y, z)) + "'."
    );
    Ogre::Vector3 position = Ogre::Vector3(x, y, z);
    if (type == AT_NONE){
        this->SetOffset(position);
        return;
    }
    offset_position_start_ = GetOffset();
    offset_position_end_ = position;
    offset_type_ = type;
    offset_seconds_ = seconds;
    offset_current_seconds_ = 0;
}

int Enemy::ScriptOffsetSync(){
    // TODO
    /*ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "Wait entity '" + name_ + "' offset for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    offset_sync_.push_back(script);
    return -1;*/
}

void Enemy::UnsetOffset(){
    // TODO
    /*offset_type_ = AT_NONE;
    for (unsigned int i = 0; i < offset_sync_.size(); ++ i)
        ScriptManager::getSingleton().ContinueScriptExecution(offset_sync_[i]);
    offset_sync_.clear();*/
}

const Ogre::Vector3& Enemy::GetOffsetPositionStart() const{return offset_position_start_;}

const Ogre::Vector3& Enemy::GetOffsetPositionEnd() const{return offset_position_end_;}

Enemy::ActionType Enemy::GetOffsetType() const{return offset_type_;}

float Enemy::GetOffsetSeconds() const{return offset_seconds_;}

void Enemy::SetOffsetCurrentSeconds(const float seconds){offset_current_seconds_ = seconds;}

float Enemy::GetOffsetCurrentSeconds() const{return offset_current_seconds_;}

void Enemy::ScriptTurnToDirection(
  const float direction, const TurnDirection turn_direction,
  const ActionType turn_type, const float seconds
){
    SetTurn(Ogre::Degree(direction), turn_direction, turn_type, seconds);
    LOG_TRIVIAL(
      "Enemy " + std::to_string(enemy_id_) + " '" + name_ + "' turn to angle '"
      + Ogre::StringConverter::toString(direction) + "'."
    );
}

int Enemy::ScriptTurnSync(){
    // TODO
    /*ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "Wait enemy " + std::to_string(enemy_id_) + " '" + name_ + "' turn for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    turn_sync_.push_back(script);
    return -1;*/
}

void Enemy::SetTurn(
  const Ogre::Degree& direction_to, const TurnDirection turn_direction,
  const ActionType turn_type, const float seconds
){
    if (turn_type == AT_NONE){
        SetRotation(direction_to);
        return;
    }
    turn_direction_ = turn_direction;
    Ogre::Degree angle_start = GetRotation();
    Ogre::Degree angle_end = CalculateTurnAngle(angle_start, direction_to);
    turn_direction_start_ = angle_start;
    turn_direction_end_ = angle_end;
    turn_type_ = turn_type;
    turn_seconds_ = seconds;
    turn_current_seconds_ = 0;
}

void Enemy::UnsetTurn(){
    turn_type_ = AT_NONE;
    //for (unsigned int i = 0; i < turn_sync_.size(); ++ i)
    //    ScriptManager::getSingleton().ContinueScriptExecution(turn_sync_[i]);
    turn_sync_.clear();
}

Ogre::Degree Enemy::CalculateTurnAngle(const Ogre::Degree& start, const Ogre::Degree& end) const{
    Ogre::Degree ret = end;
    switch(turn_direction_){
        case TD_CLOCKWISE:
            if (end <= start) ret = end + Ogre::Degree(360);
            break;
        case TD_ANTICLOCKWISE:
            if (end >= start) ret = end - Ogre::Degree(360);
            break;
        case TD_CLOSEST:
            Ogre::Degree delta = end - start;
            delta = (delta < Ogre::Degree(0)) ? -delta : delta;
            if (delta > Ogre::Degree(180)){
                if (start < end) ret = end - Ogre::Degree(360);
                else ret = end + Ogre::Degree(360);
            }
        break;
    }
    return ret;
}

Ogre::Degree Enemy::GetTurnDirectionStart() const{return turn_direction_start_;}

Ogre::Degree Enemy::GetTurnDirectionEnd() const{return turn_direction_end_;}

Enemy::ActionType Enemy::GetTurnType() const{return turn_type_;}

float Enemy::GetTurnSeconds() const{return turn_seconds_;}

void Enemy::SetTurnCurrentSeconds(const float seconds){turn_current_seconds_ = seconds;}

float Enemy::GetTurnCurrentSeconds() const{return turn_current_seconds_;}

void Enemy::ScriptSetAnimationSpeed(const float speed){animation_speed_ = speed;}

const Ogre::String& Enemy::GetDefaultAnimationName() const{return animation_default_;}

const Ogre::String& Enemy::GetCurrentAnimationName() const{return animation_current_name_;}

Enemy::AnimationState Enemy::GetAnimationState() const{return animation_state_;}

void Enemy::ScriptPlayAnimation(const char* name){
    PlayAnimation(Ogre::String(name), Enemy::REQUESTED_ANIMATION, Enemy::PLAY_DEFAULT, 0, -1);
}

void Enemy::ScriptPlayAnimationStop(const char* name){
    PlayAnimation(Ogre::String(name), Enemy::REQUESTED_ANIMATION, Enemy::PLAY_ONCE, 0, -1);
}

void Enemy::ScriptPlayAnimation(const char* name, const float start, const float end){
    PlayAnimation(
      Ogre::String(name), Enemy::REQUESTED_ANIMATION, Enemy::PLAY_DEFAULT, start, end
    );
}

void Enemy::ScriptPlayAnimationStop(const char* name, const float start, const float end){
    PlayAnimation(Ogre::String(name), Enemy::REQUESTED_ANIMATION, Enemy::PLAY_ONCE, start, end);
}

void Enemy::ScriptSetDefaultAnimation(const char* animation){
    animation_default_ = Ogre::String(animation);
}

int Enemy::ScriptAnimationSync(){
    // TODO
    /*ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "Wait entity '" + name_ + "' animation for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    animation_sync_.push_back(script);*/
    return -1;
}

Ogre::Degree Enemy::GetDirectionToPoint(Ogre::Vector2 point) const{
    Ogre::Vector3 current_point = GetPosition();
    Ogre::Vector2 up(0.0f, -1.0f);
    Ogre::Vector2 dir(point.x - current_point.x, point.y - current_point.y);
    // Angle between vectors
    Ogre::Degree angle(Ogre::Radian(acosf(dir.dotProduct(up) / (dir.length() * up.length()))));
    return (dir.x < 0) ? Ogre::Degree(360) - angle : angle;
}
