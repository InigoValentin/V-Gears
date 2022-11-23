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
#include "data/VGearsCameraMatrixFile.h"
#include "data/VGearsCameraMatrixFileSerializer.h"

namespace VGears{

    const String CameraMatrixFile::RESOURCE_TYPE("VGearsCameraMatrixFile");

    CameraMatrixFile::CameraMatrixFile(
      Ogre::ResourceManager *creator, const String &name,
      Ogre::ResourceHandle handle, const String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader
    ) : Resource(creator, name, handle, group, is_manual, loader){}

    CameraMatrixFile::~CameraMatrixFile(){unload();}

    void CameraMatrixFile::loadImpl(){
        CameraMatrixFileSerializer serializer;
        Ogre::DataStreamPtr stream(openResource());
        serializer.ImportCameraMatrixFile(stream, this);
    }

    void CameraMatrixFile::unloadImpl(){
        matrix_ = Ogre::Matrix3::ZERO;
        position_ = Ogre::Vector3::ZERO;
        offset_.x = 0;
        offset_.y = 0;
        focal_length_ = 0;
        count_ = 0;
    }

    size_t CameraMatrixFile::CalculateSize() const{
        // TODO: Here something should probably get counted. But what?
        return 0;
    }

    Ogre::Quaternion CameraMatrixFile::GetOrientation() const{
        return Ogre::Quaternion(matrix_);
    }

    const Ogre::Matrix3& CameraMatrixFile::GetMatrix() const{return matrix_;}

    void CameraMatrixFile::SetMatrix(const Ogre::Matrix3& matrix){
        matrix_ = matrix;
    }

    const Ogre::Vector3& CameraMatrixFile::GetPosition() const{
        return position_;
    }

    void CameraMatrixFile::SetPosition(const Ogre::Vector3& position){
        position_ = position;
    }

    const Pixel& CameraMatrixFile::GetOffset() const{return offset_;}

    void CameraMatrixFile::SetOffset(const Pixel& offset){offset_ = offset;}

    const size_t& CameraMatrixFile::GetCount() const{return count_;}

    void CameraMatrixFile::SetCount(const size_t count){count_ = count;}

    const Ogre::Real& CameraMatrixFile::GetFocalLength() const{
        return focal_length_;
    }

    void CameraMatrixFile::SetFocalLength(const Ogre::Real focal_length){
        focal_length_ = focal_length;
    }

    Ogre::Radian CameraMatrixFile::GetFov(Ogre::Real width) const{
        return 2 * Ogre::Math::ATan(width / (2 * focal_length_));
    }
}
