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

#include "../ParticleAdditionalData.h"

/**
 * Additional data for a particle entity.
 */
struct ParticleEntityAdditionalData : public ParticleAdditionalData{

    /**
     * Constructor.
     *
     * @param[in] scene_node The scene node.
     */
    ParticleEntityAdditionalData(Ogre::SceneNode* scene_node)
    : node(scene_node)
    {};

    /**
     * Toggles the particle entity visibility.
     *
     * @param[in] visible True to make the particle visible, false to make it
     * invisible.
     */
    void SetVisible(bool visible){
        if (node != NULL) node->setVisible(visible);
    }

    /**
     * The node that is the particle entity.
     */
    Ogre::SceneNode* node;
};
