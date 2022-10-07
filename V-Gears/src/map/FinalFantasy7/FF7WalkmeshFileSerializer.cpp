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

#include <OgreLogManager.h>
#include <OgreException.h>
#include "map/FinalFantasy7/FF7WalkmeshFileSerializer.h"

namespace VGears{

    WalkmeshFileSerializer::WalkmeshFileSerializer(): Serializer(){}

    WalkmeshFileSerializer::~WalkmeshFileSerializer(){}

    template<typename ValueType> void WalkmeshFileSerializer::ReadVector(
      Ogre::DataStreamPtr &stream, std::vector<ValueType> &dest, size_t count
    ){
        dest.clear();
        dest.reserve(count);
        for (size_t i(count); i --;){
            ValueType in_tmp;
            readObject(stream, in_tmp);
            dest.push_back(in_tmp);
        }
    }

    void WalkmeshFileSerializer::ImportWalkmeshFile(
      Ogre::DataStreamPtr &stream, WalkmeshFile *dest
    ){
        uint32 triangle_count(0);
        ReadUInt32(stream, triangle_count);
        TriangleList triangles;
        ReadVector(stream, triangles, triangle_count);
        AccessList access;
        ReadVector(stream, access, triangle_count);
        WalkmeshTriangleList &triangle_list(dest->GetTriangles());
        triangle_list.clear();
        triangle_list.reserve(triangle_count);
        for (uint32 i(0); i < triangle_count; ++ i){
            Triangle &in_triangle(triangles[i]);
            Access &in_access(access[i]);
            WalkmeshTriangle tmp_triangle;
            tmp_triangle.a = in_triangle.a;
            tmp_triangle.b = in_triangle.b;
            tmp_triangle.c = in_triangle.c;
            tmp_triangle.access_side[0] = in_access.a;
            tmp_triangle.access_side[1] = in_access.b;
            tmp_triangle.access_side[2] = in_access.c;
            triangle_list.push_back(tmp_triangle);
        }
    }

    void WalkmeshFileSerializer::readObject(
      Ogre::DataStreamPtr &stream, WalkmeshFileSerializer::Triangle &dest
    ){
        readObject(stream, dest.a);
        readObject(stream, dest.b);
        readObject(stream, dest.c);
    }

    void WalkmeshFileSerializer::readObject(Ogre::DataStreamPtr &stream, Ogre::Vector3 &dest){
        sint16 tmp[VERTEX_COMPONENT_COUNT];
        readShorts(stream, reinterpret_cast<uint16*>(tmp), VERTEX_COMPONENT_COUNT);
        dest.x = static_cast<Ogre::Real>(tmp[0]);
        dest.y = static_cast<Ogre::Real>(tmp[1]);
        dest.z = static_cast<Ogre::Real>(tmp[2]);
    }

    void WalkmeshFileSerializer::readObject(
      Ogre::DataStreamPtr &stream, WalkmeshFileSerializer::Access &dest
    ){
        uint16 tmp[ACCESS_COMPONENT_COUNT];
        readShorts(stream, tmp, ACCESS_COMPONENT_COUNT);
        dest.a = tmp[0];
        dest.b = tmp[1];
        dest.c = tmp[2];
    }

}
