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


#include <OgreBone.h>
#include <OgreLogManager.h>
#include <OgreKeyFrame.h>
#include "data/QGearsAFile.h"
#include "data/QGearsAFileSerializer.h"
#include "core/Logger.h"

namespace QGears{

   const Ogre::Real AFile::FRAME_DURATION(1.0f / 30.0f);

   const String AFile::RESOURCE_TYPE("QGearsAFile");

   AFile::AFile(
     Ogre::ResourceManager *creator, const String &name,
     Ogre::ResourceHandle handle, const String &group,
     bool is_manual, Ogre::ManualResourceLoader *loader
   ) :
     Resource(creator, name, handle, group, is_manual, loader), bone_count_(0)
   {}

    AFile::~AFile(){unload();}

    void AFile::SetBoneCount(const uint32 bone_count){bone_count_ = bone_count;}

    void AFile::loadImpl(){
        AFileSerializer serializer;
        Ogre::DataStreamPtr stream(openResource());
        serializer.ImportAFile(stream, this);
    }

    void AFile::unloadImpl(){
        bone_count_ = 0;
        frames_.clear();
    }

    size_t AFile::CalculateSize() const{
        size_t size(0);
        FrameList::const_iterator it(frames_.begin()), it_end(frames_.end());
        while( it != it_end){
            size += sizeof(it->root_rotation);
            size += sizeof(it->root_translation);
            size
              += sizeof(BoneRotationList::value_type)
                * it->bone_rotations.size();
            ++ it;
        }
        return size;
    }

    void AFile::SetFrameRotation(
      Ogre::TransformKeyFrame* key_frame, const Ogre::Vector3& v
    ) const{
        Ogre::Quaternion rot;
        Ogre::Matrix3 mat;
        mat.FromEulerAnglesZXY(
          Ogre::Radian(Ogre::Degree(-v.y)), Ogre::Radian(Ogre::Degree(-v.x)),
          Ogre::Radian(Ogre::Degree(-v.z))
        );
        rot.FromRotationMatrix(mat);
        key_frame->setRotation(rot);
    }

    void AFile::AddTo(Ogre::SkeletonPtr skeleton, const String& name) const{
        if(skeleton->hasAnimation(name)) return;
        Ogre::Real length((frames_.size() - 1) * FRAME_DURATION);
        Ogre::Animation *anim(skeleton->createAnimation(name, length));
        uint16 track_handle(0);
        Ogre::Bone* bone(skeleton->getBone("root"));
        Ogre::NodeAnimationTrack* track;
        track = anim->createNodeTrack(track_handle++, bone);
        Ogre::Real time;
        size_t index(0);
        for(
          FrameList::const_iterator frame(frames_.begin());
          frame != frames_.end();
          ++ frame
        ){
            time = (index++) * FRAME_DURATION;
            Ogre::TransformKeyFrame* key_frame(track->createNodeKeyFrame(time));
            key_frame->setTranslate(frame->root_translation);
            SetFrameRotation(key_frame, frame->root_rotation);
        }
        for(uint32 i(0); i < bone_count_; ++ i){
            if (i + 1 >= skeleton->getNumBones()){
                // TODO: Figure out why this happens/fix it.
                LOG_ERROR(
                  "Bone " + std::to_string(i + 1) + "  is out of bounds "
                  + std::to_string(skeleton->getNumBones()) + " for: " + name
                  + " in: " + skeleton->getName()
                );
            }
            else{
                bone = skeleton->getBone(i + 1);
                track = anim->createNodeTrack(track_handle ++, bone);
                time = 0;
                for (
                  FrameList::const_iterator frame(frames_.begin());
                  frame != frames_.end();
                  ++ frame
                ){
                    const Ogre::Vector3& rot(frame->bone_rotations[i]);
                    Ogre::TransformKeyFrame* key_frame(
                      track->createNodeKeyFrame(time)
                    );
                    SetFrameRotation(key_frame, rot);
                    time += FRAME_DURATION;
                }
            }
        }
    }
}
