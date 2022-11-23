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
#include <OgreStringConverter.h>
#include "data/VGearsPFile.h"
#include "common/VGearsStringUtil.h"
#include "data/VGearsPFileSerializer.h"
#include "data/VGearsHRCFile.h"

namespace VGears{

    const String PFile::RESOURCE_TYPE("VGearsPFile");

    const Ogre::Quaternion PFile::STATIC_ROTATION(PFile::CreateStaticRotation());

    PFile::PFile(
      Ogre::ResourceManager *creator, const String &name,
      Ogre::ResourceHandle handle, const String &group, bool is_manual,
      Ogre::ManualResourceLoader *loader
    ) : Ogre::Resource(creator, name, handle, group, is_manual, loader)
    {createParamDictionary(RESOURCE_TYPE);}

    PFile::~PFile(){unload();}

    void PFile::loadImpl(){
        PFileSerializer serializer;
        Ogre::DataStreamPtr stream(
          Ogre::ResourceGroupManager::getSingleton().openResource(
            mName, mGroup, true, this
          )
        );
        serializer.ImportPFile(stream, this);
    }

    void PFile::unloadImpl(){
        vertices_.clear();
        normals_.clear();
        unknown_1_.clear();
        texture_coordinates_.clear();
        vertex_colours_.clear();
        polygon_colours_.clear();
        edges_.clear();
        polygon_definitions_.clear();
        groups_.clear();
        bounding_boxes_.clear();
    }

    size_t PFile::calculateSize() const{
        return
          vertices_.size() * sizeof(vertices_.front())
          + normals_.size() * sizeof(normals_.front())
          + unknown_1_.size() * sizeof(unknown_1_.front())
          + texture_coordinates_.size() * sizeof(texture_coordinates_.front())
          + vertex_colours_.size() * sizeof(vertex_colours_.front())
          + polygon_colours_.size() * sizeof(polygon_colours_.front())
          + edges_.size() * sizeof(edges_.front())
          + polygon_definitions_.size() * sizeof(polygon_definitions_.front())
          + groups_.size() * sizeof(groups_.front())
          + bounding_boxes_.size() * sizeof(bounding_boxes_.front());
    }

    bool PFile::IsValid(){return true && IsPolygonDefinitionListValid();}

    bool PFile::IsPolygonDefinitionListValid(){
        Ogre::Log::Stream log(Ogre::LogManager::getSingleton().stream());
        size_t vertex_count(vertices_.size());
        size_t normal_count(normals_.size());
        size_t edge_count  (edges_.size());
        for (size_t p(polygon_definitions_.size()); p --;){
            const PolygonDefinition& def(polygon_definitions_[p]);
            for (int i(3); i --;){
                if (def.vertex[i] >= vertex_count){
                    log << "Error: index to vertex is out of Bounds "
                      << " polygon_definitions_[" << p << "]"
                      << ".vertex[" << i << "]: " << def.vertex[i];
                    return false;
                }
                if (def.normal[i] >= normal_count){
                    log << "Error: index to normal is out of Bounds "
                      << " polygon_definitions_[" << p << "]"
                      << ".normal[" << i << "]: " << def.normal[i];
                    return false;
                }
                if (def.edge[i] >= edge_count){
                    log << "Error: index to edge is out of Bounds "
                      << " polygon_definitions_[" << p << "]"
                      << ".edge[" << i << "]: " << def.edge[i];
                    return false;
                }
            }
        }
        return true;
    }

    void PFile::AddGroups(
      Ogre::Mesh *mesh, const String &bone_name, const RSDFilePtr &rsd
    ) const{
        const Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
        const String material_base_name(rsd->GetMaterialBaseName());
        String rsd_base;
        StringUtil::splitBase(rsd->getName(), rsd_base);
        ManualObject mo(mesh);
        for (size_t g(0); g < groups_.size(); ++ g){
            const String sub_name(
              bone_name + "/" + rsd_base + "/"
              + Ogre::StringConverter::toString(g)
            );
            AddGroup(
              groups_[g], mo, sub_name, material_base_name,
              skeleton->getBone(bone_name)
            );
        }
    }

    Ogre::Vector3 PFile::GetPosition(const Ogre::Bone *bone) const{
        Ogre::Vector3 pos(0, 0, 0);
        const Ogre::Node* node(bone);
        while (node){
            pos.z += node->getPosition().y;
            node = node->getParent();
        }
        return pos;
    }

    void PFile::AddGroup(
      const Group &group, ManualObject &mo, const String &sub_name,
      const String &material_base_name, const Ogre::Bone *bone
    ) const{
        size_t material_index(0);
        if (group.has_texture) material_index = group.texture_index + 1;
        String material_name(material_base_name + "/" + Ogre::StringConverter::toString(material_index));
        const uint16 bone_handle(bone->getHandle());
        const Ogre::Vector3 bone_position(GetPosition(bone));
        size_t index(0);
        size_t vertex_count(group.num_polygons * 3);
        size_t index_count(vertex_count);
        size_t polygon_end_index(group.polygon_start_index + group.num_polygons);
        mo.begin(sub_name, material_name, vertex_count, index_count);
        for (size_t p(group.polygon_start_index); p < polygon_end_index; ++ p){
            const PolygonDefinition& polygon(polygon_definitions_[p]);
            for (int i(3); i --;){
                uint32 v(group.vertex_start_index + polygon.vertex[i]);
                uint32 n(0 + polygon.normal[i]);
                uint32 t(
                  group.texture_coordinate_start_index + polygon.vertex[i]
                );
                Ogre::Vector3 pos(vertices_[v]);
                mo.position(
                  (STATIC_ROTATION * (pos / HRCFile::DOWN_SCALER))
                  + bone_position
                );
                mo.colour(vertex_colours_[v]);
                mo.normal(STATIC_ROTATION * normals_[n]);
                if (group.has_texture) mo.textureCoord(texture_coordinates_[t]);
                mo.bone(index, bone_handle);
                mo.index(index++);
            }
        }
        mo.end();
    }

    Ogre::Quaternion PFile::CreateStaticRotation(){
        Ogre::Radian angle(Ogre::Degree(180));
        return Ogre::Quaternion(angle, Ogre::Vector3::UNIT_X);
    }

}
