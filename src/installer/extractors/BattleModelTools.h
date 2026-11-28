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

#pragma once

#include <string>
#include <vector>
#include <OgrePrerequisites.h>
#include <OgreResourceGroupManager.h>
#include <OgreSkeleton.h>
#include "common/File.h"

/**
 * A file of a LGP archive.
 */
struct LgpEntry{

    /**
     * The contents of the file.
     */
    File file;

    /**
     * Name of the file.
     */
    std::string name;

    /**
     * Folder of the file, empty for the files in the root of the archive.
     */
    std::string folder;
};

/**
 * Reads the files of a LGP archive.
 *
 * @param[in] path Path to the archive.
 * @return The files.
 */
std::vector<LgpEntry> ReadLgp(const std::string& path);

/**
 * Decompiles a compiled HRC file of battle.lgp or magic.lgp, which have the same format.
 * The scene models of battle.lgp are the exception.
 *
 * @param[in] compiled The compiled HRC file.
 * @param[in] meshes Names of the polygon files of the bones that have one, in order.
 * @param[in] path Path to save the decompiled file to.
 * @param[in] skeleton_name Name for the skeleton.
 */
void DecompileHrc(
  File compiled, const std::vector<std::string>& meshes, const std::string& path,
  const std::string& skeleton_name
);

/**
 * Adds animations to a skeleton.
 *
 * @param[in] skeleton The skeleton.
 * @param[in] animations Names of the .a files.
 */
void AddAnimations(const Ogre::SkeletonPtr& skeleton, const std::vector<std::string>& animations);

/**
 * Reads again the resources of a folder, after files have been added to it.
 *
 * @param[in] res_mgr The resource manager.
 * @param[in] directory The folder.
 */
void ReloadModelResources(Ogre::ResourceGroupManager* res_mgr, const std::string& directory);
