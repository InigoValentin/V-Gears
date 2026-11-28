/*
 * V-Gears
 * Copyright (C) 2022 V-Gears Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <set>
#include <algorithm>
#include <Ogre.h>
#include <OgreMesh.h>
#include <OgreMeshSerializer.h>
#include <OgreSkeletonSerializer.h>
#include "installer/tables/ModelMaterials.h"
#include "common/StringUtil.h"
#include "installer/extractors/BattleMeshExporter.h"

BattleMeshExporter::BattleMeshExporter(const std::string& data_dir): data_dir_(data_dir){}

void BattleMeshExporter::Export(const std::string outdir, const Ogre::MeshPtr &mesh){
    VGears::String base_mesh_name;
    VGears::StringUtil::splitFull(mesh->getName(), base_mesh_name);
    MakeMaterialsUnique(mesh, outdir + base_mesh_name);
    // A model without geometry (enemy 15, Eye, has only a skeleton and animations) has no bounds, and Ogre
    // doesn't export a mesh without them. It is exported as an empty mesh at the origin.
    // Ogre also wants a radius that is not 0, so the bounds are a tiny box.
    if (mesh->getBounds().isNull() || mesh->getBoundingSphereRadius() <= 0){
        mesh->_setBounds(Ogre::AxisAlignedBox(Ogre::Vector3(-0.01f), Ogre::Vector3(0.01f)), false);
        mesh->_setBoundingSphereRadius(0.02f);
    }
    Ogre::MeshSerializer mesh_serializer;
    Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
    Ogre::SkeletonSerializer skeleton_serializer;
    skeleton_serializer.exportSkeleton(
      skeleton.get(), data_dir_ + "battle/models/" + outdir + base_mesh_name + ".skeleton"
    );
    mesh->setSkeletonName(outdir + base_mesh_name + ".skeleton");
    mesh_serializer.exportMesh(mesh.get(), data_dir_ + "battle/models/" + outdir + mesh->getName());
    Ogre::MaterialSerializer mat_ser;
    bool queued = false;
    std::set<std::string> textures;
    Ogre::Mesh::SubMeshList sub_meshes = mesh->getSubMeshes();
    for (Ogre::SubMesh* sub_mesh : sub_meshes){
        Ogre::MaterialPtr mat(Ogre::MaterialManager::getSingleton().getByName(sub_mesh->getMaterialName()));
        if (mat == nullptr) continue;
        for (size_t techs = 0; techs < mat->getNumTechniques(); techs ++){
            Ogre::Technique* tech = mat->getTechnique(techs);
            if (tech == nullptr) continue;
            for (size_t pass_num = 0; pass_num < tech->getNumPasses(); pass_num ++){
                Ogre::Pass* pass = tech->getPass(pass_num);
                if (pass == nullptr) continue;
                for (
                  size_t texture_unit_num = 0; texture_unit_num < pass->getNumTextureUnitStates(); texture_unit_num ++
                ){
                    Ogre::TextureUnitState* unit = pass->getTextureUnitState(texture_unit_num);
                    if (unit && unit->getTextureName().empty() == false){
                        // Ensure the output material script references png files, no tex files.
                        Ogre::String base_name;
                        VGears::StringUtil::splitBase(unit->getTextureName(), base_name);
                        unit->setTextureName(outdir + base_name + ".png");
                        textures.insert(unit->getTextureName());
                    }
                }
            }
        }
        if (std::count(materials_.begin(), materials_.end(), sub_mesh->getMaterialName()) == 0){
            mat_ser.queueForExport(mat);
            queued = true;
            materials_.push_back(sub_mesh->getMaterialName());
        }
    }
    // Ogre throws if there is nothing to export (a mesh without geometry, or whose materials are already exported).
    if (queued)
        mat_ser.exportQueued(data_dir_ + "battle/models/" + outdir + base_mesh_name + VGears::EXT_MATERIAL);
}
