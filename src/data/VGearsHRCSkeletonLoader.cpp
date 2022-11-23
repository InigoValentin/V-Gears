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
#include "data/VGearsHRCSkeletonLoader.h"
#include "data/VGearsHRCFile.h"

namespace VGears{

    const String HRCSkeletonLoader::ROOT_BONE_NAME("root");

    const Ogre::Quaternion HRCSkeletonLoader::ROOT_ORIENTATION(
      HRCSkeletonLoader::CreateRootOrientation()
    );

    typedef HRCFile::BoneList BoneList;

    HRCSkeletonLoader::HRCSkeletonLoader(HRCFile &hrc_file)
      : hrc_file_(hrc_file){}

    HRCSkeletonLoader::~HRCSkeletonLoader(){}

    Ogre::Quaternion HRCSkeletonLoader::CreateRootOrientation(){
        Ogre::Radian angle(Ogre::Degree(180));
        Ogre::Vector3 axis(0, 1, 1);
        return Ogre::Quaternion(angle, axis);
    }

    void HRCSkeletonLoader::loadResource(Ogre::Resource *resource){
        Ogre::Skeleton *skeleton(static_cast<Ogre::Skeleton *>(resource));
        assert(skeleton);
        hrc_file_.load();
        skeleton->_notifyOrigin(hrc_file_.getName());
        Ogre::Bone *root(skeleton->createBone(ROOT_BONE_NAME));
        root->setOrientation(ROOT_ORIENTATION);
        typedef std::map< String, Ogre::Real> LengthMap;
        LengthMap length_map;
        LengthMap::const_iterator length;
        BoneList &bones(hrc_file_.GetBones());
        for(
          BoneList::const_iterator bone(bones.begin());
          bone != bones.end();
          ++ bone
        ){
            Ogre::Bone* child(skeleton->createBone(bone->name));
            Ogre::Bone* parent(skeleton->getBone(bone->parent));
            length_map[ bone->name ] = bone->length;
            length = length_map.find(bone->parent);
            if(length != length_map.end())
                child->setPosition(0, length->second, 0);
            parent->addChild(child);
        }
    }

}
