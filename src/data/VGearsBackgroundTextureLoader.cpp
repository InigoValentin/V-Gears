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

#include <OgreTexture.h>
#include "data/VGearsBackgroundTextureLoader.h"
#include "data/VGearsBackgroundFile.h"

namespace VGears{

    BackgroundTextureLoader::BackgroundTextureLoader(
      BackgroundFile &background_file
    ) : background_file_( background_file ){}

    BackgroundTextureLoader::~BackgroundTextureLoader(){}

    void BackgroundTextureLoader::loadResource(Ogre::Resource *resource){
        Ogre::Texture *texture(static_cast<Ogre::Texture *>(resource));
        assert(texture);
        background_file_.load();
        texture->_notifyOrigin(background_file_.getName());
    }

}
