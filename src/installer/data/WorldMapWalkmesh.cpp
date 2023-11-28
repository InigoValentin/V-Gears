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
#include "data/WorldMapWalkmesh.h"

WorldMapWalkmesh::Triangle::Triangle(
  const unsigned int id, const Ogre::Vector3 a, const Ogre::Vector3 b,
  const Ogre::Vector3 c, const unsigned int walkability, const int event
): id_(id), a_(a), b_(b), c_(c){
    ab_.id = -1;
    ab_.warp = false;
    bc_.id = -1;
    bc_.warp = false;
    ca_.id = -1;
    ca_.warp = false;
}

Ogre::Vector3 WorldMapWalkmesh::Triangle::getA(){return a_;}

Ogre::Vector3 WorldMapWalkmesh::Triangle::getB(){return b_;}

Ogre::Vector3 WorldMapWalkmesh::Triangle::getC(){return c_;}

WorldMapWalkmesh::Triangle::Target WorldMapWalkmesh::Triangle::getAB(){return ab_;}

WorldMapWalkmesh::Triangle::Target WorldMapWalkmesh::Triangle::getBC(){return bc_;}

WorldMapWalkmesh::Triangle::Target WorldMapWalkmesh::Triangle::getCA(){return ca_;}

void WorldMapWalkmesh::Triangle::setAB(const int id, const bool warp){
    ab_.id = std::max(id, -1);
    ab_.warp = warp;
}

void WorldMapWalkmesh::Triangle::setBC(const int id, const bool warp){
    bc_.id = std::max(id, -1);
    bc_.warp = warp;
}

void WorldMapWalkmesh::Triangle::setCA(const int id, const bool warp){
    ca_.id = std::max(id, -1);
    ca_.warp = warp;
}

WorldMapWalkmesh::WorldMapWalkmesh(const unsigned int id, const bool warp):
  id_(id), warp_(warp), calculated_(false){
    triangles_.clear();
}

void WorldMapWalkmesh::addTriangle(
  const Ogre::Vector3 a, const Ogre::Vector3 b, const Ogre::Vector3 c,
  const unsigned int walkability, const int event
){
    calculated_ = false;
    Triangle tri(triangles_.size(), a, b, c, walkability, event);
    triangles_.push_back(tri);
}

void WorldMapWalkmesh::generate(std::string path){\
    if (!calculated_) calculate();
}

void WorldMapWalkmesh::calculate(){
    // TODO
    calculated_ = true;
}
