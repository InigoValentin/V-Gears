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

#include <Ogre.h>
#include "EnemyFile.h"
#include "common/File.h"

/**
 * Extracts an animation from a file.
 *
 * @param file[in] The file from which to read the animation data.
 * @param skeleton[in|out] Skeleton to add the animation to.
 * @param info[in] @todo Understand and document.
 * @param skeleton_data[in|out] Information about the skeleton the animation
 * animates.
 * @param with_weapon[in] Includes the weapon in the animation.
 * @todo Add more documentation. It's a battle animation? Acharacter animation?
 * An enemy animation?
 */
void AnimationExtractor(
  File* file, const Ogre::SkeletonPtr& skeleton, const EnemyInfo& info,
  Skeleton& skeleton_data, bool with_weapon
);
