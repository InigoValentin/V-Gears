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

#pragma once

#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreMesh.h>
#include <OgreSubMesh.h>
#include <string>

/**
 * Gives a mesh its own copy of each of its materials.
 *
 * The original game names the materials of many unrelated models the same ("gzbb/0" is both a field model and a
 * battle enemy), but Ogre keeps all materials in a single namespace. Converting the models changes their materials
 * (texture names...), so a material shared by name was changed by one model under the feet of the next, and the
 * exported material scripts of different models defined the same material twice.
 *
 * Every sub mesh material is replaced by a copy named "<owner>/<original name>", which is unique to the model. It
 * must be called before the mesh is exported, so the exported mesh refers to the new names.
 *
 * @param[in] mesh The mesh.
 * @param[in] owner Name that identifies the model, used as prefix.
 */
inline void MakeMaterialsUnique(const Ogre::MeshPtr& mesh, const std::string& owner){
    if (mesh.isNull()) return;
    const std::string prefix = owner + "/";
    Ogre::MaterialManager& manager = Ogre::MaterialManager::getSingleton();
    for (Ogre::SubMesh* sub_mesh : mesh->getSubMeshes()){
        const std::string name = sub_mesh->getMaterialName();
        // Already done (the mesh is exported more than once or reused).
        if (name.compare(0, prefix.size(), prefix) == 0) continue;
        Ogre::MaterialPtr material = manager.getByName(name);
        if (material.isNull()) continue;
        const std::string unique_name = prefix + name;
        if (manager.getByName(unique_name).isNull()) material->clone(unique_name);
        sub_mesh->setMaterialName(unique_name);
    }
}
