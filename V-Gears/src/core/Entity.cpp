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
#include "core/ConfigVar.h"
#include "core/DebugDraw.h"
#include "core/Entity.h"
#include "core/Logger.h"

ConfigVar cv_debug_entity("debug_entity", "Draw entity debug info", "0");

Entity::Entity(const Ogre::String& name, Ogre::SceneNode* node):
  name_(name),
  scene_node_(node),
  height_(1.0f),
  //solid_radius_(0.24f),
  solid_radius_(0.21f),
  solid_(false),
  talk_radius_(0.45f),
  talkable_(false),
  state_(Entity::NONE),
  move_auto_speed_(0.7f),
  move_walk_speed_(0.7f),
  move_run_speed_(0.8f),
  move_position_(Ogre::Vector3(0, 0, 0)),
  move_entity_(nullptr),
  move_stop_distance_(0.0f),
  move_triangle_id_(-1),
  move_auto_rotation_(true),
  move_auto_animation_(true),
  move_animation_walk_("Walk"),
  move_animation_run_("Run"),
  linear_movement_(LM_UP_TO_DOWN),
  linear_start_(0.0f, 0.0f, 0.0f),
  linear_end_(0.0f, 0.0f, 0.0f),
  linear_dest_triangle_(-1),
  jump_start_(0.0f, 0.0f, 0.0f),
  jump_end_(0.0f, 0.0f, 0.0f),
  jump_seconds_(0.0f),
  jump_current_seconds_(0.0f),
  jump_dest_triangle_(-1),
  offset_position_start_(0.0f, 0.0f, 0.0f),
  offset_position_end_(0.0f, 0.0f, 0.0f),
  offset_type_(AT_NONE),
  offset_seconds_(0.0f),
  offset_current_seconds_(0.0f),
  turn_direction_(TD_CLOSEST),
  turn_direction_start_(0),
  turn_direction_end_(0),
  turn_entity_(nullptr),
  turn_type_(AT_NONE),
  turn_seconds_(0.0f),
  turn_current_seconds_(0.0f),
  animation_speed_(1.0f),
  animation_default_("Idle"),
  animation_current_name_(""),
  animation_auto_play_(true),
  is_character_(false),
  character_id_(-1),
  is_line_(false)
{
    model_root_node_ = scene_node_->createChildSceneNode();
    model_node_ = model_root_node_->createChildSceneNode();
    direction_ = new EntityDirection();
    //direction_->setMaterial("entity/direction");
    Ogre::MaterialPtr mat_direction
      = Ogre::MaterialManager::getSingleton().getByName("entity/direction");
    direction_->setMaterial(mat_direction);
    direction_node_ = scene_node_->createChildSceneNode();
    direction_node_->attachObject(direction_);
    solid_collision_ = new EntityCollision();
    //solid_collision_->setMaterial("entity/solid_collision");
    Ogre::MaterialPtr mat_solid_collision
      = Ogre::MaterialManager::getSingleton().getByName("entity/solid_collision");
    solid_collision_->setMaterial(mat_solid_collision);
    solid_collision_node_ = scene_node_->createChildSceneNode();
    solid_collision_node_->setScale(solid_radius_, solid_radius_, height_);
    solid_collision_node_->attachObject(solid_collision_);
    talk_collision_ = new EntityCollision();
    //talk_collision_->setMaterial("entity/talk_collision");
    Ogre::MaterialPtr mat_talk_collision
      = Ogre::MaterialManager::getSingleton().getByName("entity/talk_collision");
    solid_collision_->setMaterial(mat_talk_collision);
    talk_collision_node_ = scene_node_->createChildSceneNode();
    talk_collision_node_->setScale(talk_radius_, talk_radius_, height_);
    talk_collision_node_->attachObject(talk_collision_);
    model_root_node_->setPosition(Ogre::Vector3::ZERO);
    model_node_->setPosition(Ogre::Vector3::ZERO);
    scene_node_->setPosition(Ogre::Vector3::ZERO);
    LOG_TRIVIAL("Entity '" + name_ + "' created.");
}

Entity::~Entity(){
    delete solid_collision_;
    delete talk_collision_;
    scene_node_->removeAndDestroyAllChildren();
    LOG_TRIVIAL("Entity '" + name_ + "' destroyed.");
}

void Entity::Update() {
    if (move_entity_ != nullptr){
        move_position_ = move_entity_->GetPosition();
        move_stop_distance_ = GetSolidRadius() + move_entity_->GetSolidRadius();
    }
    if (turn_entity_ != nullptr){
        Ogre::Degree angle = GetDirectionToEntity(turn_entity_);
        angle = CalculateTurnAngle(turn_direction_start_, angle);
        turn_direction_end_ = angle;
    }
}

void Entity::UpdateDebug(){
    int debug = cv_debug_entity.GetI();
    direction_node_->setVisible(debug > 0);
    solid_collision_node_->setVisible(debug > 0 && solid_);
    talk_collision_node_->setVisible(debug > 0 && talkable_);
    // debug output
    if (debug > 0){
        DEBUG_DRAW.SetColour(Ogre::ColourValue::White);
        DEBUG_DRAW.SetScreenSpace(true);
        DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.CENTER);
        DEBUG_DRAW.SetFadeDistance(40, 50);
        Ogre::Vector3 entity_pos = GetPosition();
        DEBUG_DRAW.Text(entity_pos, 0, 0, name_);
        DEBUG_DRAW.Text(entity_pos, 0, 12, animation_current_name_);

        if (debug > 1){
            Ogre::String move_state_string[] = {"NONE", "WALKMESH", "LINEAR", "JUMP"};
            DEBUG_DRAW.Text(entity_pos, 0, 24, Ogre::StringConverter::toString(entity_pos));
            DEBUG_DRAW.Text(
              entity_pos, 0, 36, "Triangle: " + Ogre::StringConverter::toString(move_triangle_id_)
            );
            DEBUG_DRAW.Text(entity_pos, 0, 48, "Move state: " + move_state_string[state_]);
            switch(state_){
                case Entity::WALKMESH:
                    DEBUG_DRAW.Line3d(entity_pos, move_position_);
                    break;
                case Entity::LINEAR:
                    DEBUG_DRAW.Text(
                      linear_start_, 0, 0,
                      "Start: " + Ogre::StringConverter::toString(linear_start_)
                    );
                    DEBUG_DRAW.Text(
                      linear_end_, 0, 0, "End: " + Ogre::StringConverter::toString(linear_end_)
                    );
                    DEBUG_DRAW.Line3d(linear_start_, linear_end_);
                    break;
                case Entity::JUMP:
                    DEBUG_DRAW.Text(
                      jump_start_, 0, 0, "Start: " + Ogre::StringConverter::toString(jump_start_)
                    );
                    DEBUG_DRAW.Text(
                      jump_end_, 0, 0, "End: " + Ogre::StringConverter::toString(jump_end_)
                    );
                    Ogre::Vector3 distance = jump_end_ - jump_start_;
                    Ogre::Vector3 pos1, pos2, pos3;
                    pos1.x = jump_start_.x + distance.x * 0.25f;
                    pos1.y = jump_start_.y + distance.y * 0.25f;
                    float current1 = jump_seconds_ / 4;
                    pos1.z
                      = current1 * current1 * -13.08f + current1 * ((distance.z)
                      / jump_seconds_ + jump_seconds_ * 13.08f) + jump_start_.z;
                    pos2.x = jump_start_.x + distance.x * 0.5f;
                    pos2.y = jump_start_.y + distance.y * 0.5f;
                    float current2 = current1 * 2;
                    pos2.z
                      = current2 * current2 * -13.08f + current2 * ((distance.z)
                      / jump_seconds_ + jump_seconds_ * 13.08f) + jump_start_.z;
                    pos3.x = jump_start_.x + distance.x * 0.75f;
                    pos3.y = jump_start_.y + distance.y * 0.75f;
                    float current3 = current1 * 3;
                    pos3.z
                      = current3 * current3 * -13.08f + current3 * ((distance.z)
                      / jump_seconds_ + jump_seconds_ * 13.08f) + jump_start_.z;
                    DEBUG_DRAW.Line3d(jump_start_, pos1);
                    DEBUG_DRAW.Line3d(pos1, pos2);
                    DEBUG_DRAW.Line3d(pos2, pos3);
                    DEBUG_DRAW.Line3d(pos3, jump_end_);
                    break;
            }
        }
    }
}

const Ogre::String& Entity::GetName() const{return name_;}

void Entity::SetPosition(const Ogre::Vector3& position){scene_node_->setPosition(position);}

void Entity::ScriptSetPosition(const float x, const float y, const float z){
    SetPosition(Ogre::Vector3(x, y, z));

    // Make the entity solid, visible and talkable. TODO: Check if neccessary.
    SetVisible(true);
    SetSolid(true);
    SetTalkable(true);

    // If set from a script:
    // Reset walkmesh triangle to reattach entity to walkmesh again if needed.
    move_triangle_id_ = -1;
}

const Ogre::Vector3 Entity::GetPosition() const{return scene_node_->getPosition();}

void Entity::ScriptGetPosition() const{
    Ogre::Vector3 position = scene_node_->getPosition();
    ScriptManager::getSingleton().AddValueToStack(position.x);
    ScriptManager::getSingleton().AddValueToStack(position.y);
    ScriptManager::getSingleton().AddValueToStack(position.z);
}

void Entity::SetOffset(const Ogre::Vector3& position){
    assert(model_root_node_);
    model_root_node_->setPosition(position);
}

const Ogre::Vector3 Entity::GetOffset() const{
    assert(model_root_node_);
    return model_root_node_->getPosition();
}

void Entity::SetRotation(const Ogre::Degree& rotation){
    assert(model_root_node_);
    float angle
      = rotation.valueDegrees() - Ogre::Math::Floor(rotation.valueDegrees() / 360.0f) * 360.0f;
    if (angle < 0) angle = 360 + angle;
    Ogre::Quaternion q;
    Ogre::Vector3 vec = Ogre::Vector3::UNIT_Z;
    q.FromAngleAxis(Ogre::Radian(Ogre::Degree(angle)), vec);
    model_root_node_->setOrientation(q);
    direction_node_->setOrientation(q);
}

void Entity::ScriptSetRotation(const float rotation){SetRotation(Ogre::Degree(rotation));}

Ogre::Degree Entity::GetRotation() const{
    assert(model_root_node_);
    Ogre::Quaternion q = model_root_node_->getOrientation();
    Ogre::Degree temp;
    Ogre::Vector3 vec = Ogre::Vector3::UNIT_Z;
    q.ToAngleAxis(temp, vec);
    return temp;
}

float Entity::ScriptGetRotation() const{return GetRotation().valueDegrees();}

void Entity::setScale(const Ogre::Vector3 &scale) {
    assert(model_root_node_);
    model_root_node_->setScale(scale);
}

void Entity::SetIndex(const int index){
    assert(model_root_node_);
    index_ = index;
}

int Entity::GetIndex(){return index_;}

void Entity::setRootOrientation(const Ogre::Quaternion &root_orientation){
    assert(model_node_);
    model_node_->setOrientation(root_orientation);
}

float Entity::GetHeight() const{return height_;}

void Entity::SetSolidRadius(const float radius){
    solid_radius_ = radius;
    solid_collision_node_->setScale(solid_radius_, solid_radius_, height_);
}

float Entity::GetSolidRadius() const{return solid_radius_;}

void Entity::SetSolid(const bool solid){solid_ = solid;}

bool Entity::IsSolid() const{return solid_;}

void Entity::SetTalkRadius(const float radius){
    talk_radius_ = radius;
    talk_collision_node_->setScale(talk_radius_, talk_radius_, height_);
}

float Entity::GetTalkRadius() const{return talk_radius_;}

void Entity::SetTalkable(const bool talkable){talkable_ = talkable;}

bool Entity::IsTalkable() const{return talkable_;}

void Entity::SetState(const Entity::State state){state_ = state;}

Entity::State Entity::GetState() const{return state_;}

void Entity::SetMoveAutoSpeed(const float speed){move_auto_speed_ = speed;}

float Entity::GetMoveAutoSpeed() const{return move_auto_speed_;}

void Entity::SetMoveWalkSpeed(const float speed){move_walk_speed_ = speed;}

float Entity::GetMoveWalkSpeed() const{return move_walk_speed_;}

void Entity::SetMoveRunSpeed(const float speed){move_run_speed_ = speed;}

float Entity::GetMoveRunSpeed() const{return move_run_speed_;}

void Entity::SetMovePosition(const Ogre::Vector3& target){move_position_ = target;}

const Ogre::Vector3& Entity::GetMovePosition() const{return move_position_;}

float Entity::GetMoveStopDistance() const{return move_stop_distance_;}

void Entity::SetMoveTriangleId(const int triangle){move_triangle_id_ = triangle;}

int Entity::GetMoveTriangleId() const{return move_triangle_id_;}

void Entity::SetMoveAutoRotation(const bool rotate){move_auto_rotation_ = rotate;}

bool Entity::GetMoveAutoRotation() const{return move_auto_rotation_;}

void Entity::SetMoveAutoAnimation(const bool animate){move_auto_animation_ = animate;}

bool Entity::GetMoveAutoAnimation() const{return move_auto_animation_;}

const Ogre::String& Entity::GetMoveAnimationWalkName() const{return move_animation_walk_;}

const Ogre::String& Entity::GetMoveAnimationRunName() const{return move_animation_run_;}

void Entity::ScriptMoveToPosition(const float x, const float y){
    state_ = Entity::WALKMESH;
    move_position_ = Ogre::Vector3(x, y, 0);
    move_entity_ = nullptr;
    move_stop_distance_ = 0;
    LOG_TRIVIAL(
      "Entity '" + name_ + "' set move to walkmesh position '"
      + Ogre::StringConverter::toString(move_position_) + "'."
    );
}

void Entity::ScriptMoveToEntity(Entity* entity){
    state_ = Entity::WALKMESH;
    move_position_ = entity->GetPosition();
    move_entity_ = entity;
    move_stop_distance_ = GetSolidRadius() + entity->GetSolidRadius();
    LOG_TRIVIAL("Entity '" + name_ + "' set move to entity '" + entity->GetName() + "'.");
}

int Entity::ScriptMoveSync(){
    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    sync_.push_back(script);
    LOG_TRIVIAL(
      "Wait entity '" + name_ + "' move for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    return -1;
}

void Entity::UnsetMove(){
    state_ = Entity::NONE;
    move_entity_ = nullptr;
    move_stop_distance_ = 0;
    for (const auto &script : sync_) ScriptManager::getSingleton().ContinueScriptExecution(script);
    sync_.clear();
}

void Entity::ScriptLinearToPosition(
  const float x, const float y, const float z, const LinearMovement movement,
  const char* animation, const float orientation, const int dest_triangle
){
    Ogre::Vector3 pos = Ogre::Vector3(x, y, z);
    SetLinear(pos, movement, animation, orientation, dest_triangle);
    LOG_TRIVIAL(
      "Entity '" + name_ + "' set linear move to position '"
      + Ogre::StringConverter::toString(pos) + "' with animation '" + animation + "'."
    );
}

int Entity::ScriptLinearSync(){
    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "Wait entity '" + name_ + "' linear move for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    sync_.push_back(script);
    return -1;
}

void Entity::SetLinear(
  const Ogre::Vector3& end, const LinearMovement movement,
  const Ogre::String& animation, const float orientation, const int dest_triangle
){
    SetRotation(Ogre::Angle(orientation));
    state_ = Entity::LINEAR;
    linear_movement_ = movement;
    linear_start_ = GetPosition();
    linear_end_ = end;
    linear_dest_triangle_ = dest_triangle;
    // Linear animation.
    animation_auto_play_ = false;
    PlayAnimation(animation, Entity::AUTO_ANIMATION, Entity::PLAY_LOOPED, 0, -1);
    // While moving, the entity is not in a triangle.
    move_triangle_id_ = -1;
}

void Entity::UnsetLinear(){
    state_ = Entity::NONE;
    move_triangle_id_ = linear_dest_triangle_; // Set for NPCs. Playable can end up in two places.
    animation_auto_play_ = true;
    PlayAnimation(animation_default_, Entity::AUTO_ANIMATION, Entity::PLAY_LOOPED, 0, -1);
    for (size_t i = 0; i < sync_.size(); ++ i)
        ScriptManager::getSingleton().ContinueScriptExecution(sync_[i]);
    sync_.clear();
}

LinearMovement Entity::GetLinearMovement() const{return linear_movement_;}

const Ogre::Vector3& Entity::GetLinearStart() const{return linear_start_;}

const Ogre::Vector3& Entity::GetLinearEnd() const{return linear_end_;}

const int Entity::GetLinearDestTriangle() const{return linear_dest_triangle_;}

void Entity::ScriptJumpToPosition(
  const float x, const float y, const float z, const float seconds, const int dest_triangle
){
    float new_z = z;
    // If z not specified (-1), asume same Z.
    if (z < 0) new_z = GetPosition().z;
    Ogre::Vector3 jump_to(x, y, new_z);
    SetJump(jump_to, seconds, dest_triangle);
    LOG_TRIVIAL(
      "Entity '" + name_ + "' set jump to position '" + Ogre::StringConverter::toString(jump_to)
      + "' in " + Ogre::StringConverter::toString(seconds) + " seconds."
    );
}

int Entity::ScriptJumpSync(){
    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "Wait entity '" + name_ + "' jump for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    sync_.push_back(script);
    return -1;
}

void Entity::SetJump(const Ogre::Vector3& jump_to, const float seconds, const int dest_triangle){
    state_ = Entity::JUMP;
    jump_start_ = GetPosition();
    jump_end_ = jump_to;
    jump_seconds_ = seconds;
    jump_current_seconds_ = 0;
    jump_was_solid_ = IsSolid();
    jump_dest_triangle_ = dest_triangle;
    SetSolid(false);
    // After moving the entity needs to be reattached to the walkmesh.
    move_triangle_id_ = -1;
}

void Entity::UnsetJump(){
    SetSolid(jump_was_solid_);
    move_triangle_id_ = linear_dest_triangle_;
    state_ = Entity::NONE;
    for (size_t i = 0; i < sync_.size(); ++ i)
        ScriptManager::getSingleton().ContinueScriptExecution(sync_[i]);
    sync_.clear();
}

const Ogre::Vector3& Entity::GetJumpStart() const{return jump_start_;}

const Ogre::Vector3& Entity::GetJumpEnd() const{return jump_end_;}

float Entity::GetJumpSeconds() const{return jump_seconds_;}

void Entity::SetJumpCurrentSeconds(const float seconds){jump_current_seconds_ = seconds;}

float Entity::GetJumpCurrentSeconds() const{ return jump_current_seconds_;}

const int Entity::GetJumpDestTriangle() const{return jump_dest_triangle_;}

void Entity::ScriptOffsetToPosition(
  const float x, const float y, const float z, const ActionType type, const float seconds
){
    LOG_TRIVIAL(
      "Entity '" + name_ + "' set offset to position '"
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

int Entity::ScriptOffsetSync(){
    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "Wait entity '" + name_ + "' offset for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    offset_sync_.push_back(script);
    return -1;
}

void Entity::UnsetOffset(){
    offset_type_ = AT_NONE;
    for (unsigned int i = 0; i < offset_sync_.size(); ++ i)
        ScriptManager::getSingleton().ContinueScriptExecution(offset_sync_[i]);
    offset_sync_.clear();
}

const Ogre::Vector3& Entity::GetOffsetPositionStart() const{return offset_position_start_;}

const Ogre::Vector3& Entity::GetOffsetPositionEnd() const{return offset_position_end_;}

ActionType Entity::GetOffsetType() const{return offset_type_;}

float Entity::GetOffsetSeconds() const{return offset_seconds_;}

void Entity::SetOffsetCurrentSeconds(const float seconds){offset_current_seconds_ = seconds;}

float Entity::GetOffsetCurrentSeconds() const{return offset_current_seconds_;}

void Entity::ScriptTurnToDirection(
  const float direction, const TurnDirection turn_direction,
  const ActionType turn_type, const float seconds
){
    SetTurn(Ogre::Degree(direction), nullptr, turn_direction, turn_type, seconds);
    LOG_TRIVIAL(
      "Entity '" + name_ + "' turn to angle '" + Ogre::StringConverter::toString(direction) + "'."
    );
}

void
Entity::ScriptTurnToEntity(Entity* entity, const TurnDirection turn_direction, const float seconds){
    if (entity == nullptr || entity == this){
        LOG_ERROR("Turn to entity: Invalid entity pointer (NULL or self).");
        return;
    }
    Ogre::Degree angle = GetDirectionToEntity(entity);
    SetTurn(angle, entity, turn_direction, AT_SMOOTH, seconds);
    LOG_TRIVIAL("Entity '" + name_ + "' turn to entity '" + entity->GetName() + ".");
}

int Entity::ScriptTurnSync(){
    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "Wait entity '" + name_ + "' turn for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    turn_sync_.push_back(script);
    return -1;
}

void Entity::SetTurn(
  const Ogre::Degree& direction_to, Entity* entity, const TurnDirection turn_direction,
  const ActionType turn_type, const float seconds
){
    if (turn_type == AT_NONE){
        SetRotation(direction_to);
        return;
    }
    turn_direction_ = turn_direction;
    Ogre::Degree angle_start = GetRotation();
    Ogre::Degree angle_end = CalculateTurnAngle(angle_start, direction_to);
    turn_entity_ = entity;
    turn_direction_start_ = angle_start;
    turn_direction_end_ = angle_end;
    turn_type_ = turn_type;
    turn_seconds_ = seconds;
    turn_current_seconds_ = 0;
}

void Entity::UnsetTurn(){
    turn_type_ = AT_NONE;
    for (unsigned int i = 0; i < turn_sync_.size(); ++ i)
        ScriptManager::getSingleton().ContinueScriptExecution(turn_sync_[i]);
    turn_sync_.clear();
}

Ogre::Degree Entity::CalculateTurnAngle(const Ogre::Degree& start, const Ogre::Degree& end) const{
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

Ogre::Degree Entity::GetTurnDirectionStart() const{return turn_direction_start_;}

Ogre::Degree Entity::GetTurnDirectionEnd() const{return turn_direction_end_;}

ActionType Entity::GetTurnType() const{return turn_type_;}

float Entity::GetTurnSeconds() const{return turn_seconds_;}

void Entity::SetTurnCurrentSeconds(const float seconds){turn_current_seconds_ = seconds;}

float Entity::GetTurnCurrentSeconds() const{return turn_current_seconds_;}

void Entity::ScriptSetAnimationSpeed(const float speed){animation_speed_ = speed;}

const Ogre::String& Entity::GetDefaultAnimationName() const{return animation_default_;}

const Ogre::String& Entity::GetCurrentAnimationName() const{return animation_current_name_;}

Entity::AnimationState Entity::GetAnimationState() const{return animation_state_;}

void Entity::ScriptPlayAnimation(const char* name){
    PlayAnimation(Ogre::String(name), Entity::REQUESTED_ANIMATION, Entity::PLAY_DEFAULT, 0, -1);
}

void Entity::ScriptPlayAnimationStop(const char* name){
    PlayAnimation(Ogre::String(name), Entity::REQUESTED_ANIMATION, Entity::PLAY_ONCE, 0, -1);
}

void Entity::ScriptPlayAnimation(const char* name, const float start, const float end){
    PlayAnimation(
      Ogre::String(name), Entity::REQUESTED_ANIMATION, Entity::PLAY_DEFAULT, start, end
    );
}

void Entity::ScriptPlayAnimationStop(const char* name, const float start, const float end){
    PlayAnimation(Ogre::String(name), Entity::REQUESTED_ANIMATION, Entity::PLAY_ONCE, start, end);
}

void Entity::ScriptSetDefaultAnimation(const char* animation){
    animation_default_ = Ogre::String(animation);
}

int Entity::ScriptAnimationSync(){
    ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
    LOG_TRIVIAL(
      "Wait entity '" + name_ + "' animation for function '"
      + script.function + "' in script entity '" + script.entity + "'."
    );
    animation_sync_.push_back(script);
    return -1;
}

void Entity::SetCharacter(int char_id){
    if (char_id >= 0) is_character_ = true;
    else is_character_ = false;
    character_id_ = char_id;
}

bool Entity::IsCharacter(){return is_character_;}

int Entity::GetCharacterId(){return character_id_;}

bool Entity::IsLine(){return is_line_;}

Ogre::Degree Entity::GetDirectionToEntity(Entity* entity) const{
    Ogre::Vector3 current_point = GetPosition();
    Ogre::Vector3 direction_point = entity->GetPosition();
    Ogre::Vector2 up(0.0f, -1.0f);
    Ogre::Vector2 dir(direction_point.x - current_point.x, direction_point.y - current_point.y);
    // Angle between vectors
    Ogre::Degree angle(Ogre::Radian(acosf(dir.dotProduct(up) / (dir.length() * up.length()))));
    return (dir.x < 0) ? Ogre::Degree(360) - angle : angle;
}
