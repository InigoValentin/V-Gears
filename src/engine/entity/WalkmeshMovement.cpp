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

#include <algorithm>
#include <cmath>
#include <OgreMath.h>
#include "common/Logger.h"
#include "engine/Manager.h"
#include "engine/Timer.h"
#include "engine/entity/Entity.h"
#include "engine/entity/Walkmesh.h"
#include "engine/entity/WalkmeshMovement.h"

WalkmeshMovement::WalkmeshMovement(
  const Manager& manager, const Walkmesh& walkmesh, const std::vector<Entity*>& entities,
  Entity* const& player, const bool& player_locked, const bool& player_slip,
  const Ogre::Vector3& player_move, const LineCheck& check_lines
):
  manager_(manager),
  walkmesh_(walkmesh),
  entities_(entities),
  player_(player),
  player_locked_(player_locked),
  player_slip_(player_slip),
  player_move_(player_move),
  check_lines_(check_lines)
{}

float WalkmeshMovement::GetTriangleHeight(int triangle, float x, float y, float fallback) const{
    if (manager_.IsBattleModule() || triangle < 0 || triangle >= walkmesh_.GetNumberOfTriangles())
        return fallback;
    const float height = PointElevation(
      Ogre::Vector2(x, y), walkmesh_.GetA(triangle), walkmesh_.GetB(triangle),
      walkmesh_.GetC(triangle)
    );
    return std::isfinite(height) ? height : fallback;
}

float WalkmeshMovement::PointElevation(
  const Ogre::Vector2& point, const Ogre::Vector3& a, const Ogre::Vector3& b, const Ogre::Vector3& c
){
    float _a = a.z * (b.y - c.y) + b.z * (c.y - a.y) + c.z * (a.y - b.y);
    float _b = a.y * (b.x - c.x) + b.y * (c.x - a.x) + c.y * (a.x - b.x);
    float _c = a.x * (b.z - c.z) + b.x * (c.z - a.z) + c.x * (a.z - b.z);
    float _d
      = a.x * (b.z * c.y - c.z * b.y) + b.x * (c.z * a.y - a.z * c.y)
        + c.x * (a.z * b.y - b.z * a.y);
    return (_d - _c * point.y - _a * point.x) / _b;
}

float WalkmeshMovement::SideOfVector(
  const Ogre::Vector2& point, const Ogre::Vector2& p1, const Ogre::Vector2& p2
){
    Ogre::Vector2 AB = p2    - p1;
    Ogre::Vector2 AP = point - p1;
    return AB.x * AP.y - AB.y * AP.x;
}

bool WalkmeshMovement::IsInCurrentTriangle(const Entity& entity) const{
    const int triangle = entity.GetMoveTriangleId();
    if (triangle < 0 || triangle >= walkmesh_.GetNumberOfTriangles()) return false;
    const Ogre::Vector3 position = entity.GetPosition();
    const Ogre::Vector3 a = walkmesh_.GetA(triangle);
    const Ogre::Vector3 b = walkmesh_.GetB(triangle);
    const Ogre::Vector3 c = walkmesh_.GetC(triangle);
    return Ogre::Math::pointInTri2D(
      Ogre::Vector2(position.x, position.y), Ogre::Vector2(a.x, a.y),
      Ogre::Vector2(b.x, b.y), Ogre::Vector2(c.x, c.y)
    );
}

bool WalkmeshMovement::PlaceOnWalkmesh(Entity* entity){
    if (manager_.IsBattleModule()){
        LOG_WARNING("Tried to set entity on the walkmesh, but the EntityManager is in battle mode");
        return false;
    }
    Ogre::Vector3 position3 = entity->GetPosition();
    Ogre::Vector2 position2;
    position2.x = position3.x;
    position2.y = position3.y;
    std::vector<std::pair<int, float>> triangles;
    // Search for possible triangles.
    for (int i = 0; i < walkmesh_.GetNumberOfTriangles(); ++ i){
        Ogre::Vector3 A3 = walkmesh_.GetA(i);
        Ogre::Vector3 B3 = walkmesh_.GetB(i);
        Ogre::Vector3 C3 = walkmesh_.GetC(i);
        Ogre::Vector2 A2(A3.x, A3.y);
        Ogre::Vector2 B2(B3.x, B3.y);
        Ogre::Vector2 C2(C3.x, C3.y);
        if (Ogre::Math::pointInTri2D(position2, A2, B2, C2) == true){
            triangles.push_back(std::make_pair(i, PointElevation(
              position2, walkmesh_.GetA(i), walkmesh_.GetB(i), walkmesh_.GetC(i)
            )));
        }
    }
    // If the coordinates doesn't match any triangle, exit.
    if (triangles.size() == 0 && entity == player_){
        LOG_ERROR(
          "Can't find any triangle to place entity '" + entity->GetName() + "' on walkmesh."
        );
        return false;
    }
    // From the possible triangles, select the closest one (on the Z edge).
    int closest_z_triangle = -1;
    float closest_z_distance = 9999.0f;
    float pos_z = position3.z;
    for (size_t i = 0; i < triangles.size(); ++ i){
        if (std::abs(pos_z - triangles[i].second) < closest_z_distance){
            closest_z_triangle = triangles[i].first;
            closest_z_distance = std::abs(pos_z - triangles[i].second);
        }
    }
    if (closest_z_triangle == -1 && entity == player_){
        LOG_ERROR(
          "Can't find nearby triangle to place entity '" + entity->GetName() + "' on walkmesh."
        );
        return false;
    }
    entity->SetPosition(Ogre::Vector3(position2.x, position2.y, pos_z));
    entity->SetMoveTriangleId(closest_z_triangle);
    return true;
}

bool WalkmeshMovement::EdgeBlocks(bool is_player, bool scripted_move){
    return is_player || !scripted_move;
}

float WalkmeshMovement::ScriptedStep(float distance, float stop_distance, float max_step){
    return std::min(max_step, std::max(0.0f, distance - stop_distance));
}

bool WalkmeshMovement::Obstacles::Any() const{
    return left_wall || right_wall || ahead_wall || left_entity || right_entity || ahead_entity;
}

bool WalkmeshMovement::Walk(Entity* entity, const float speed, const float delta_time){
    if (manager_.IsBattleModule()){
        LOG_WARNING("Tried to perform a walkmesh move, but the EntityManager is in battle mode");
        return false;
    }
    const Ogre::Vector3 start_point = entity->GetPosition();
    const Ogre::Vector3 move_vector = entity->GetMovePosition() - start_point;
    Ogre::Vector2 direction(move_vector.x, move_vector.y);
    const float distance_to_target = direction.length();
    const bool scripted_move = entity->IsScriptedMove();
    const float stop_distance = entity->GetMoveStopDistance();
    if (scripted_move && distance_to_target <= stop_distance + 0.0001f){
        entity->UnsetMove();
        return false;
    }
    // The step of this frame.
    direction.normalise();
    direction *= delta_time;
    direction *= speed;
    if (scripted_move && !direction.isZeroLength()){
        direction.normalise();
        direction *= ScriptedStep(distance_to_target, stop_distance, speed * delta_time);
    }
    // If movement is still needed but speed or time don't allow it, just return for now.
    if (direction.isZeroLength() == true){
        if (player_ == entity) check_lines_(entity, entity->GetPosition());
        return false;
    }
    const int triangle = entity->GetMoveTriangleId();
    if (triangle == -1){
        // A character that is not on the walkmesh (Jenova of shpin_3, that is placed outside it
        // and moves with FMOVE) is walked in a straight line, or the scene that waits for it would
        // never go on.
        if (scripted_move && entity != player_) return WalkOffWalkmesh(entity, direction);
        // The entities that a scene places later (the soldiers of the parade of junonr4) are asked
        // to move before it, and wait. Only the player has to be on the walkmesh.
        if (entity == player_)
            LOG_ERROR("Entity '" + entity->GetName() + "' not placed on walkmesh and can't move.");
        return false;
    }
    FollowSlope(triangle, start_point, direction);
    // Set the direction for the entity if it has to move.
    if (entity->GetMoveAutoRotation() == true) FaceDirection(entity, direction);

    Obstacles obstacles;
    Ogre::Vector3 end_point = FindWay(entity, triangle, direction, obstacles);
    // This also stores in the entity the triangle of the end point, even if it can't move there.
    const bool off_walkmesh = CrossBorder(entity, end_point);
    if (obstacles.Any() || off_walkmesh){
        // Event if the entity can't move, check for triggers, but use current position.
        if (player_ == entity) check_lines_(entity, entity->GetPosition());
        return false;
    }

    // Check triggers before set entity position because move line is checked.
    if (player_ == entity) check_lines_(entity, end_point);
    entity->SetPosition(end_point);

    // If the entity has come to destination point, finish movement.
    const Ogre::Vector3 remaining = entity->GetMovePosition() - end_point;
    const float tolerance = scripted_move ? 0.0001f : speed / 10;
    if (Ogre::Vector2(remaining.x, remaining.y).length() <= stop_distance + tolerance)
        entity->UnsetMove();
    return true;
}

bool WalkmeshMovement::WalkOffWalkmesh(Entity* entity, const Ogre::Vector2& direction){
    const Ogre::Vector3 start_point = entity->GetPosition();
    const Ogre::Vector3 move_vector = entity->GetMovePosition() - start_point;
    const float distance_to_target = Ogre::Vector2(move_vector.x, move_vector.y).length();
    const float step = std::min(direction.length(), distance_to_target);
    Ogre::Vector2 move = direction;
    if (!move.isZeroLength()) move.normalise();
    move *= step;
    entity->SetPosition(
      Ogre::Vector3(start_point.x + move.x, start_point.y + move.y, start_point.z)
    );
    if (distance_to_target - step <= entity->GetMoveStopDistance() + 0.0001f) entity->UnsetMove();
    return step > 0.0f;
}

void WalkmeshMovement::FollowSlope(
  const int triangle, const Ogre::Vector3& start_point, Ogre::Vector2& direction
) const{
    Ogre::Vector3 end_point(start_point.x + direction.x, start_point.y + direction.y, 0.0f);
    end_point.z = PointElevation(
      Ogre::Vector2(end_point.x, end_point.y), walkmesh_.GetA(triangle),
      walkmesh_.GetB(triangle), walkmesh_.GetC(triangle)
    );
    Ogre::Vector3 slope_step = end_point - start_point;
    slope_step.normalise();
    slope_step = slope_step * direction.length();
    direction.x = slope_step.x;
    direction.y = slope_step.y;
}

void WalkmeshMovement::FaceDirection(Entity* entity, const Ogre::Vector2& direction){
    const Ogre::Vector2 up(0.0f, -1.0f);
    // Angle between vectors
    Ogre::Degree angle(Ogre::Radian(SafeAcos(
      direction.dotProduct(up), direction.length() * up.length()
    )));
    angle = (direction.x < 0) ? Ogre::Degree(360) - angle : angle;
    entity->SetRotation(Ogre::Degree(angle));
}

Ogre::Vector3 WalkmeshMovement::FindWay(
  Entity* entity, const int triangle, Ogre::Vector2 direction, Obstacles& obstacles
){
    const Ogre::Vector3 start_point = entity->GetPosition();
    const bool is_player = entity == player_;
    // A scripted walk (also of the player entity, while a scene has it locked) must end, or the
    // scene waiting for it would never go on. Several scripts place solid entities next to where
    // another one has to walk to, so they don't block it: only the walkmesh does. A manual move
    // of the player is never scripted.
    // The same goes for the points around the end of the step: they are there for the player and
    // NPCs to slide along walls, and a destination next to an edge of the walkmesh would never be
    // reached. The end point itself is still checked by the caller.
    const bool check_probes = !entity->IsScriptedMove();
    float radius = (entity->IsSolid() == true) ? entity->GetSolidRadius() : 0.01f;
    // For not playable entities, be a bit more lenient with solid collisions.
    // Don't use the solid radius.
    if (!is_player) radius /= 10;

    const int attempts = is_player ? 16 : 17;
    Ogre::Vector3 end_point;
    for (int i = 0; i < attempts; ++ i){
        end_point = Ogre::Vector3(
          start_point.x + direction.x, start_point.y + direction.y, start_point.z
        );
        if (!check_probes) break;
        obstacles = ProbeAround(entity, triangle, end_point, direction, radius);
        if (!obstacles.Any() || !Steer(is_player, player_slip_, obstacles, direction)) break;
    }
    return end_point;
}

WalkmeshMovement::Obstacles WalkmeshMovement::ProbeAround(
  Entity* entity, const int triangle, const Ogre::Vector3& end_point,
  const Ogre::Vector2& direction, const float radius
){
    Ogre::Vector3 ahead(direction.x, direction.y, 0.0f);
    ahead.normalise();
    Ogre::Quaternion turn;
    turn.FromAngleAxis(Ogre::Radian(Ogre::Degree(45)), Ogre::Vector3::UNIT_Z);
    const Ogre::Vector3 left = turn * ahead;
    turn.FromAngleAxis(Ogre::Radian(Ogre::Degree(-45)), Ogre::Vector3::UNIT_Z);
    const Ogre::Vector3 right = turn * ahead;

    Obstacles found;
    // Checks the point at the solid radius from the end of the step, in a direction.
    const auto probe = [&](const Ogre::Vector3& towards, bool& wall, bool& other_entity){
        const Ogre::Vector3 offset = towards * radius;
        Ogre::Vector3 point(end_point.x + offset.x, end_point.y + offset.y, 0.0f);
        // It sets the height of the point, needed to check the entities.
        wall = CrossBorder(entity, point);
        entity->SetMoveTriangleId(triangle);
        other_entity = CheckSolidCollisions(entity, point);
    };
    probe(left, found.left_wall, found.left_entity);
    probe(right, found.right_wall, found.right_entity);
    probe(ahead, found.ahead_wall, found.ahead_entity);
    return found;
}

bool WalkmeshMovement::Steer(
  const bool is_player, const bool slip, const Obstacles& obstacles, Ogre::Vector2& direction
){
    const bool sides_blocked = obstacles.left_wall && obstacles.right_wall;
    if (is_player){
        // The player only slides along walls, never around other entities.
        if (obstacles.left_entity || obstacles.right_entity || obstacles.ahead_entity)
            return false;
        // With SLIP off the move is not turned along the wall: it stays blocked.
        if (sides_blocked || !slip) return false;
        if (obstacles.left_wall) Turn(direction, -11.25f);
        else if (obstacles.right_wall) Turn(direction, 11.25f);
        return true;
    }
    // If the entity collides only directly into a triangle border.
    if (!obstacles.left_wall && !obstacles.right_wall && obstacles.ahead_wall)
        Turn(direction, -7.0f);
    // Or if the entity only collides into other entity directly.
    else if (!obstacles.left_entity && !obstacles.right_entity && obstacles.ahead_entity)
        Turn(direction, -11.25f);
    if (sides_blocked) return false;
    if (
      (!obstacles.left_wall && obstacles.left_entity)
      || (obstacles.left_wall && !obstacles.right_wall)
    ) Turn(direction, -11.25f);
    if (
      !obstacles.left_wall && !obstacles.left_entity
      && (obstacles.right_wall || obstacles.right_entity)
    ) Turn(direction, 11.25f);
    return true;
}

void WalkmeshMovement::Turn(Ogre::Vector2& direction, const float degrees){
    Ogre::Quaternion turn;
    turn.FromAngleAxis(Ogre::Radian(Ogre::Degree(degrees)), Ogre::Vector3::UNIT_Z);
    const Ogre::Vector3 turned = turn * Ogre::Vector3(direction.x, direction.y, 0.0f);
    direction.x = turned.x;
    direction.y = turned.y;
}

bool WalkmeshMovement::CrossBorder(Entity* entity, Ogre::Vector3& position){
    if (manager_.IsBattleModule()){
        LOG_WARNING("Tried to check for walkmesh cross, but the EntityManager is in battle mode");
        return false;
    }
    int current_triangle = entity->GetMoveTriangleId();
    if (current_triangle == -1) return true;
    // While a scene has the player locked, the scene walks the player like any other character,
    // and it can lead over the edge of the walkmesh.
    const bool free_walk = !EdgeBlocks(
      entity == player_ && !player_locked_, entity->IsScriptedMove()
    );
    Ogre::Vector2 pos = Ogre::Vector2(position.x, position.y);
    bool searched_other_triangle = false;
    for (;;){
        Ogre::Vector3 A3 = walkmesh_.GetA(current_triangle);
        Ogre::Vector3 B3 = walkmesh_.GetB(current_triangle);
        Ogre::Vector3 C3 = walkmesh_.GetC(current_triangle);
        Ogre::Vector2 A(A3.x, A3.y);
        Ogre::Vector2 B(B3.x, B3.y);
        Ogre::Vector2 C(C3.x, C3.y);
        float sign1 = SideOfVector(pos, B, A);
        float sign2 = SideOfVector(pos, C, B);
        float sign3 = SideOfVector(pos, A, C);
        int next_triangle = -1;
        if (sign1 < 0) next_triangle = walkmesh_.GetAccessSide(current_triangle, 0);
        else if (sign2 < 0) next_triangle = walkmesh_.GetAccessSide(current_triangle, 1);
        else if (sign3 < 0) next_triangle = walkmesh_.GetAccessSide(current_triangle, 2);
        else{
            position.z = PointElevation(pos, A3, B3, C3);
            entity->SetMoveTriangleId(current_triangle);
            return false;
        }
        if (next_triangle >= 0){
            bool lock = walkmesh_.IsLocked(next_triangle);
            if (lock == false || free_walk){
                current_triangle = next_triangle;
                continue;
            }
        }
        // A walk of a script goes on over the edge: it keeps the last triangle, unless the point is
        // inside another one (the triangles apart from the rest, that scenes use to put a character
        // higher than the floor, are not joined to it: the character would keep that height after
        // leaving it).
        if (free_walk && !searched_other_triangle){
            searched_other_triangle = true;
            int other_triangle = -1;
            float other_distance = 9999.0f;
            for (int i = 0; i < walkmesh_.GetNumberOfTriangles(); ++ i){
                if (i == current_triangle) continue;
                const Ogre::Vector3 a = walkmesh_.GetA(i);
                const Ogre::Vector3 b = walkmesh_.GetB(i);
                const Ogre::Vector3 c = walkmesh_.GetC(i);
                if (!Ogre::Math::pointInTri2D(
                  pos, Ogre::Vector2(a.x, a.y), Ogre::Vector2(b.x, b.y), Ogre::Vector2(c.x, c.y)
                )) continue;
                const float distance = std::abs(position.z - PointElevation(pos, a, b, c));
                if (distance < other_distance){
                    other_distance = distance;
                    other_triangle = i;
                }
            }
            if (other_triangle >= 0){
                current_triangle = other_triangle;
                continue;
            }
        }
        position.z = PointElevation(pos, A3, B3, C3);
        entity->SetMoveTriangleId(current_triangle);
        // A walk of a script goes on over the edge, keeping the last triangle.
        return !free_walk;
    }
}

bool WalkmeshMovement::CheckSolidCollisions(Entity* entity, Ogre::Vector3& position){
    if (manager_.IsBattleModule()){
        LOG_WARNING("Tried to check for solid collisions, but the EntityManager is in battle mode");
        return false;
    }
    if (entity->IsSolid() == false) return false;
    for (size_t i = 0; i < entities_.size(); ++ i){
        if (entities_[i]->IsSolid() == false) continue;
        if (entities_[i] == entity) continue;
        Ogre::Vector3 pos1 = entities_[i]->GetPosition();
        float solid_range = entities_[i]->GetSolidRadius();
        solid_range *= solid_range;
        float height = (pos1.z < position.z) ? entities_[i]->GetHeight() : entity->GetHeight();
        if (
          ((pos1.z - position.z + height) < (height * 2)) && ((pos1.z - position.z + height) >= 0)
        ){
            float distance
              = (pos1.x - position.x) * (pos1.x - position.x)
              + (pos1.y - position.y) * (pos1.y - position.y);
            if (distance < solid_range) return true;
        }
    }
    return false;
}

void WalkmeshMovement::LinearStep(Entity* entity){
    if (manager_.IsBattleModule()){
        LOG_WARNING("Tried to set next linear step, but the EntityManager is in battle mode");
        return;
    }
    bool to_end = true;
    bool is_move = false;
    Ogre::Vector3 start = entity->GetLinearStart();
    Ogre::Vector3 end = entity->GetLinearEnd();
    Ogre::Vector3 current = entity->GetPosition();
    float delta = Timer::getSingleton().GetGameTimeDelta();
    // The player climbs a ladder with the directional keys. While a scene has the player locked
    // the keys have no effect, and the scene makes the player move on its own, as with any other
    // entity.
    if (entity == player_ && !player_locked_){
        LinearMovement move = entity->GetLinearMovement();
        if (move == LM_DOWN_TO_UP || move == LM_UP_TO_DOWN){
            if (player_move_.y > 0){ // move up
                to_end = (move == LM_DOWN_TO_UP) ? true : false;
                is_move = true;
            }
            else if (player_move_.y < 0){ // move down
                to_end = (move == LM_DOWN_TO_UP) ? false : true;
                is_move = true;
            }
        }
        else if (move == LM_LEFT_TO_RIGHT || move == LM_RIGHT_TO_LEFT){
            if (player_move_.x > 0){ // move right
                to_end = (move == LM_LEFT_TO_RIGHT) ? true : false;
                is_move = true;
            }
            else if (player_move_.x < 0){ // move left
                to_end = (move == LM_LEFT_TO_RIGHT) ? false : true;
                is_move = true;
            }
        }
    }
    else is_move = true;
    if (is_move == true && (end - start).length() < 0.0001f){
        // Start and end are the same place, so there is nothing to walk. Dividing by the time to
        // cross the line (zero) would give a NaN position. Just finish the move.
        entity->UnsetLinear();
        if (to_end == true) entity->SetMoveTriangleId(entity->GetLinearDestTriangle());
        PlaceOnWalkmesh(entity);
        entity->SetPosition(to_end ? end : start);
    }
    else if (is_move == true){
        Ogre::Vector3 position;
        if (to_end == true){
            Ogre::Vector3 end_start = end - start;
            // Move across the whole line by this number of seconds.
            float time = end_start.length() / 1.2f;
            position.x = current.x + (end_start.x / time) * delta;
            position.y = current.y + (end_start.y / time) * delta;
            position.z = current.z + (end_start.z / time) * delta;
            position.x = (end_start.x < 0)
              ? std::max(position.x, end.x) : std::min(position.x, end.x);
            position.y = (end_start.y < 0)
              ? std::max(position.y, end.y) : std::min(position.y, end.y);
            position.z = (end_start.z < 0)
              ? std::max(position.z, end.z) : std::min(position.z, end.z);
            // Compared with a tolerance: if the entity starts a bit off the end in one of the
            // axes (the entry points are written with a few digits only), that axis could move
            // less than the precision of a float per frame and never be exactly equal.
            if ((position - end).length() < 0.0001f){
                position = end;
                entity->UnsetLinear();
                entity->SetMoveTriangleId(entity->GetLinearDestTriangle());
                PlaceOnWalkmesh(entity);
            }
        }
        else{
            Ogre::Vector3 end_start = start - end;
            // Move across the whole line by this number of seconds.
            float time = end_start.length() / 1.2f;
            position.x = current.x + (end_start.x / time) * delta;
            position.y = current.y + (end_start.y / time) * delta;
            position.z = current.z + (end_start.z / time) * delta;
            position.x = (end_start.x < 0)
              ? std::max(position.x, start.x) : std::min(position.x, start.x);
            position.y = (end_start.y < 0)
              ? std::max(position.y, start.y) : std::min(position.y, start.y);
            position.z = (end_start.z < 0)
              ? std::max(position.z, start.z) : std::min(position.z, start.z);
            if ((position - start).length() < 0.0001f){
                position = start;
                entity->UnsetLinear();
                PlaceOnWalkmesh(entity);
            }
        }
        entity->SetPosition(position);
        entity->UpdateAnimation((to_end == true) ? delta : -delta);
    }
}

void WalkmeshMovement::JumpStep(Entity* entity){
    if (manager_.IsBattleModule()){
        LOG_WARNING("Tried to set next jump step, but the EntityManager is in battle mode");
        return;
    }
    float total = entity->GetJumpSeconds();
    float current = entity->GetJumpCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    // A jump of zero seconds lands at once, without dividing by zero.
    if (total <= 0.0f) total = 0.0001f;
    current = (current > total) ? total : current;
    Ogre::Vector3 start_position = entity->GetJumpStart();
    Ogre::Vector3 end_position   = entity->GetJumpEnd();
    Ogre::Vector3 position;
    position.x = start_position.x + ((end_position.x - start_position.x) / total) * current;
    position.y = start_position.y + ((end_position.y - start_position.y) / total) * current;
    position.z = current * current * -1.0f + current
      * ((end_position.z - start_position.z) / total + total * 1.0f) + start_position.z;
    entity->SetPosition(position);
    if (total == current){
        entity->UnsetJump();
        entity->SetMoveTriangleId(entity->GetJumpDestTriangle());
        PlaceOnWalkmesh(entity);
        // The scripts set the animation of the jump as the default one (DFANM) and don't set it
        // back, but the player stands normally after landing in the original game (the scene of
        // mds6_2 where Cloud jumps down, for example). The scripts that do set it, do it after
        // the jump, so they aren't affected.
        if (entity == player_){
            entity->ScriptSetDefaultAnimation("Idle");
            entity->PlayAnimationContinue("Idle");
        }
    }
    else entity->SetJumpCurrentSeconds(current);
}
