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

#include "core/Background2D.h"
#include "core/Background2DAnimation.h"
#include "core/CameraManager.h"
#include "core/EntityManager.h"
#include "core/Logger.h"
#include "core/XmlBackground2DFile.h"

XmlBackground2DFile::XmlBackground2DFile(const Ogre::String& file): XmlFile(file){}

XmlBackground2DFile::~XmlBackground2DFile(){}

void XmlBackground2DFile::Load(){
    TiXmlNode* node = file_.RootElement();
    Background2D* background = EntityManager::getSingleton().GetBackground2D();
    if (node == nullptr || node->ValueStr() != "background2d"){
        LOG_ERROR(
          "XML 2D Background loader: " + file_.ValueStr()
          + " is not a valid background2d file! No <background2d> in root."
        );
        return;
    }
    Ogre::String image = GetString(node, "image", "");
    if (image != ""){
        background->SetImage(image);
        Ogre::Quaternion range = GetQuaternion(
          node, "range", Ogre::Quaternion(-100000, -100000, 100000, 100000)
        );
        background->SetRange((int)range.w, (int)range.x, (int)range.y, (int)range.z);
        Ogre::Vector3 position = GetVector3(node, "position", Ogre::Vector3::ZERO);
        Ogre::Quaternion orientation = GetQuaternion(
          node, "orientation", Ogre::Quaternion::IDENTITY
        );
        float fov = GetFloat(node, "fov", 45);
        CameraManager::getSingleton().Set2DCamera(
          position, orientation, Ogre::Radian(Ogre::Degree(fov))
        );
        int tile_id = 0;
        node = node->FirstChild();
        while (node != nullptr){
            if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "tile"){
                int width = GetInt(node, "width", 0);
                int height = GetInt(node, "height", 0);
                Ogre::Vector2 destination = GetVector2(node, "destination", Ogre::Vector2::ZERO);
                Ogre::Vector4 uv = GetVector4(node, "uv", Ogre::Vector4::ZERO);
                float depth = GetFloat(node, "depth", 0);
                Ogre::String blending_str = GetString(node, "blending", "alpha");
                VGears::Blending blending;
                if (blending_str == "add") blending = VGears::B_ADD;
                else if (blending_str == "subtract") blending = VGears::B_SUBTRACT;
                else blending = VGears::B_ALPHA;
                Ogre::Vector4 distance = Ogre::Vector4(0, 0, -depth, 1);
                Ogre::Vector4 res
                  = CameraManager::getSingleton().GetCurrentCamera()
                    ->getProjectionMatrixWithRSDepth() * distance;
                res = res / res.w;
                background->AddTile(
                  static_cast<int>(destination.x), static_cast<int>(destination.y),
                  width, height, res.z, uv.x, uv.y, uv.z, uv.w, blending
                );
                TiXmlNode* node2 = node->FirstChild();
                while (node2 != nullptr){
                    if (
                      node2->Type() == TiXmlNode::TINYXML_ELEMENT
                      && node2->ValueStr() == "animation"
                    ){
                        Ogre::String name = GetString(node2, "name", "");
                        if (name != ""){
                            Background2DAnimation* animation = new Background2DAnimation(
                              name, background, tile_id
                            );
                            animation->SetLength(GetFloat(node2, "length", 0));
                            Ogre::String uv_node2 = GetString(node2, "uv", "");
                            if (uv_node2 != ""){
                                Ogre::StringVector key_frame = Ogre::StringUtil::split(
                                  uv_node2, ","
                                );
                                for (unsigned int i = 0; i < key_frame.size(); ++ i){
                                    Ogre::StringUtil::trim(key_frame[i]);
                                    Ogre::StringVector data = Ogre::StringUtil::split(
                                      key_frame[i], ":"
                                    );
                                    if (data.size() > 1){
                                        float time = Ogre::StringConverter::parseReal(data[0]);
                                        Ogre::Vector4 value
                                          = Ogre::StringConverter::parseVector4(data[1]);
                                        animation->AddUVKeyFrame(
                                          time, value.x, value.y, value.z, value.w
                                        );
                                    }
                                }
                            }
                            background->AddAnimation(animation);
                        }
                    }
                    node2 = node2->NextSibling();
                }
                ++ tile_id;
            }
            node = node->NextSibling();
        }
    }
}
