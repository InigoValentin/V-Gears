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

#include "core/ConfigVar.h"
#include "core/DebugDraw.h"
#include "core/Logger.h"
#include "core/Walkmesh.h"
#include "map/VGearsWalkmeshFile.h"

ConfigVar cv_debug_walkmesh(
  "debug_walkmesh", "Draw walkmesh and walkmesh debug info", "false"
);

Walkmesh::Walkmesh(){}

Walkmesh::~Walkmesh(){}


void Walkmesh::UpdateDebug(){
    if (cv_debug_walkmesh.GetB() == true){
        DEBUG_DRAW.SetScreenSpace(true);
        DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.CENTER);

        for (const auto &triangle : triangles_){
            if (triangle.access_side[0] == -1) DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 0, 0, 1));
            else DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 1, 1, 1));
            DEBUG_DRAW.Line3d(triangle.a, triangle.b);
            if (triangle.access_side[1] == -1) DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 0, 0, 1));
            else DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 1, 1, 1));
            DEBUG_DRAW.Line3d(triangle.b, triangle.c);
            if (triangle.access_side[2] == -1) DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 0, 0, 1));
            else DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 1, 1, 1));
            DEBUG_DRAW.Line3d(triangle.c, triangle.a);
            DEBUG_DRAW.SetColour(Ogre::ColourValue(1, 1, 1, 1));
            DEBUG_DRAW.SetFadeDistance(40, 50);
            Ogre::Vector3 triangle_pos = (triangle.a + triangle.b + triangle.c) / 3;
        }
    }
}

void Walkmesh::Clear(){triangles_.clear();}

void Walkmesh::AddTriangle(const WalkmeshTriangle& triangle){triangles_.push_back(triangle);}

int Walkmesh::GetAccessSide(unsigned int triangle_id, unsigned char side) const{
    if (triangle_id >= triangles_.size()){
        LOG_ERROR("Triangle_id greater than number of triangles in walkmesh or less than zero.");
        return -1;
    }
    if (side >= 3){
        LOG_ERROR("Side greater than 2. Side indexed from 0 to 2.");
        return -1;
    }
    return triangles_[triangle_id].access_side[side];
}

const Ogre::Vector3& Walkmesh::GetA(unsigned int triangle_id) const{
    if (triangle_id >= triangles_.size()){
        LOG_ERROR("Triangle_id greater than number of triangles in walkmesh or less than zero.");
        return Ogre::Vector3::ZERO;
    }
    return triangles_[triangle_id].a;
}

const Ogre::Vector3& Walkmesh::GetB(unsigned int triangle_id) const{
    if (triangle_id >= triangles_.size()){
        LOG_ERROR("Triangle_id greater than number of triangles in walkmesh or less than zero.");
        return Ogre::Vector3::ZERO;
    }
    return triangles_[triangle_id].b;
}

const Ogre::Vector3& Walkmesh::GetC(unsigned int triangle_id) const{
    if (triangle_id >= triangles_.size()){
        LOG_ERROR("Triangle_id greater than number of triangles in walkmesh or less than zero.");
        return Ogre::Vector3::ZERO;
    }
    return triangles_[triangle_id].c;
}

int Walkmesh::GetNumberOfTriangles() const{return triangles_.size();}

void Walkmesh::LockWalkmesh(unsigned int triangle_id, bool lock){
    if (triangle_id >= triangles_.size()){
        LOG_ERROR("Triangle_id greater than number of triangles in walkmesh or less than zero.");
        return;
    }
    triangles_[triangle_id].locked = lock;
}

bool Walkmesh::IsLocked(unsigned int triangle_id) const{
    if (triangle_id >= triangles_.size()){
        LOG_ERROR("Triangle_id greater than number of triangles in walkmesh or less than zero.");
        return false;
    }
    return triangles_[triangle_id].locked;
}

void Walkmesh::load(const VGears::WalkmeshFilePtr &walkmesh){
    for (const auto &triangle : walkmesh->GetTriangles()) AddTriangle(triangle);
}
