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
#include <tinyxml.h>
#include "data/WorldMapWalkmesh.h"

WorldMapWalkmesh::Triangle::Triangle(
  const unsigned int id, const Ogre::Vector3 a, const Ogre::Vector3 b,
  const Ogre::Vector3 c, const unsigned int walkability, const int event
): id_(id), a_(a), b_(b), c_(c), walkability_(walkability), event_(event){
    ab_.id = -1;
    ab_.warp = false;
    bc_.id = -1;
    bc_.warp = false;
    ca_.id = -1;
    ca_.warp = false;
}

unsigned int WorldMapWalkmesh::Triangle::getId() const{return id_;}

Ogre::Vector3 WorldMapWalkmesh::Triangle::getA() const {return a_;}

Ogre::Vector3 WorldMapWalkmesh::Triangle::getB() const {return b_;}

Ogre::Vector3 WorldMapWalkmesh::Triangle::getC() const {return c_;}

WorldMapWalkmesh::Triangle::Target WorldMapWalkmesh::Triangle::getAB() const {return ab_;}

WorldMapWalkmesh::Triangle::Target WorldMapWalkmesh::Triangle::getBC() const {return bc_;}

WorldMapWalkmesh::Triangle::Target WorldMapWalkmesh::Triangle::getCA() const {return ca_;}

unsigned int WorldMapWalkmesh::Triangle::getWalkability() const {return walkability_;}

int WorldMapWalkmesh::Triangle::getEvent() const {return event_;}

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
    if (id_ == 0 && triangles_.size() < 50){
        std::cout << "Adding triangle " << tri.getId() << " W: "<< walkability << " - " << tri.getWalkability() << "     E: " << event << " - " << tri.getEvent() << std::endl;
    }
    triangles_.push_back(tri);
}

void WorldMapWalkmesh::generate(std::string path){
    if (!calculated_) calculate();
    TiXmlDocument doc;
    std::unique_ptr<TiXmlElement> xml(new TiXmlElement("walkmesh"));
    for (Triangle tri: triangles_){
        std::unique_ptr<TiXmlElement> xml_tri(new TiXmlElement("triangle"));
        xml_tri->SetAttribute("id", std::to_string(tri.getId()));
        xml_tri->SetAttribute(
          "a",
          std::to_string(static_cast<int>(tri.getA()[0])) + " "
          + std::to_string(static_cast<int>(tri.getA()[1])) + " "
          + std::to_string(static_cast<int>(tri.getA()[2]))
        );
        xml_tri->SetAttribute(
          "b",
          std::to_string(static_cast<int>(tri.getB()[0])) + " "
          + std::to_string(static_cast<int>(tri.getB()[1])) + " "
          + std::to_string(static_cast<int>(tri.getB()[2]))
        );
        xml_tri->SetAttribute(
          "c",
          std::to_string(static_cast<int>(tri.getA()[0])) + " "
          + std::to_string(static_cast<int>(tri.getC()[1])) + " "
          + std::to_string(static_cast<int>(tri.getC()[2]))
        );
        // WARN: This is never actually calculated. See the comments on calculate()
        if (tri.getAB().id > -1)
            xml_tri->SetAttribute(
              "a_b", std::to_string(tri.getAB().id) + (tri.getAB().warp ? " warp" : "")
            );
        if (tri.getBC().id > -1)
            xml_tri->SetAttribute(
              "b_c", std::to_string(tri.getBC().id) + (tri.getBC().warp ? " warp" : "")
            );
        if (tri.getCA().id > -1)
            xml_tri->SetAttribute(
              "c_a", std::to_string(tri.getCA().id) + (tri.getCA().warp ? " warp" : "")
            );
        xml_tri->SetAttribute("walkability", std::to_string(tri.getWalkability()));
        if (tri.getEvent() > -1) xml_tri->SetAttribute("event", std::to_string(tri.getEvent()));
        xml->LinkEndChild(xml_tri.release());
    }
    doc.LinkEndChild(xml.release());
    doc.SaveFile(path);
}

void WorldMapWalkmesh::calculate(){
    // FIXME: This is super time-consumming. Either optimize it or skip it and calculate it on
    // runtime.
    calculated_ = true;
    return;
    // FIXME: The actual function starts here.
    if (calculated_) return;
    for (Triangle tri: triangles_){
        tri.setAB(-1, false);
        tri.setBC(-1, false);
        tri.setCA(-1, false);
    }
    for (Triangle tri: triangles_){
        if (tri.getAB().id != -1) continue;
        for (Triangle compare: triangles_){
            // For each triangle combination, check if any two vertices match,
            // and set AB/BC/CA for both of them.
            if (tri.getId() != compare.getId()){
                if (
                  (tri.getA() == compare.getA() && tri.getB() == compare.getB())
                  || (tri.getB() == compare.getA() && tri.getA() == compare.getB())
                ){
                    // AB <-> AB match
                    tri.setAB(compare.getId(), false);
                    compare.setAB(tri.getId(), false);
                }
                if (
                  (tri.getA() == compare.getB() && tri.getB() == compare.getC())
                  || (tri.getB() == compare.getB() && tri.getA() == compare.getC())
                ){
                    // AB <-> BC match
                    tri.setAB(compare.getId(), false);
                    compare.setBC(tri.getId(), false);
                }
                if (
                  (tri.getA() == compare.getC() && tri.getB() == compare.getA())
                  || (tri.getB() == compare.getC() && tri.getA() == compare.getA())
                ){
                    // AB <-> CA match
                    tri.setAB(compare.getId(), false);
                    compare.setCA(tri.getId(), false);
                }
                if (
                  (tri.getB() == compare.getA() && tri.getC() == compare.getB())
                  || (tri.getC() == compare.getA() && tri.getB() == compare.getB())
                ){
                    // BC <-> AB match
                    tri.setBC(compare.getId(), false);
                    compare.setAB(tri.getId(), false);
                }
                if (
                  (tri.getB() == compare.getB() && tri.getC() == compare.getC())
                  || (tri.getC() == compare.getB() && tri.getB() == compare.getC())
                ){
                    // BC <-> BC match
                    tri.setBC(compare.getId(), false);
                    compare.setBC(tri.getId(), false);
                }
                if (
                  (tri.getB() == compare.getC() && tri.getC() == compare.getA())
                  || (tri.getC() == compare.getC() && tri.getB() == compare.getA())
                ){
                    // BC <-> CA match
                    tri.setBC(compare.getId(), false);
                    compare.setCA(tri.getId(), false);
                }
                if (
                  (tri.getC() == compare.getA() && tri.getA() == compare.getB())
                  || (tri.getA() == compare.getA() && tri.getC() == compare.getB())
                ){
                    // CA <-> AB match
                    tri.setBC(compare.getId(), false);
                    compare.setAB(tri.getId(), false);
                }
                if (
                  (tri.getC() == compare.getB() && tri.getA() == compare.getC())
                  || (tri.getA() == compare.getB() && tri.getC() == compare.getC())
                ){
                    // CA <-> BC match
                    tri.setCA(compare.getId(), false);
                    compare.setBC(tri.getId(), false);
                }
                if (
                  (tri.getC() == compare.getC() && tri.getA() == compare.getA())
                  || (tri.getA() == compare.getC() && tri.getC() == compare.getA())
                ){
                    // CA <-> CA match
                    tri.setCA(compare.getId(), false);
                    compare.setCA(tri.getId(), false);
                }
                // TODO: For warping worlds, check warping
            }
        }
    }
    calculated_ = true;
}
