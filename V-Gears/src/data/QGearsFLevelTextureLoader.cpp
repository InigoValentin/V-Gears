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
#include "data/QGearsFLevelTextureLoader.h"
#include "data/QGearsFLevelFile.h"

namespace QGears{

    FLevelTextureLoader::FLevelTextureLoader(FLevelFile &flevel_file) :
      flevel_file_(flevel_file)
    {}

    FLevelTextureLoader::~FLevelTextureLoader(){}

    void FLevelTextureLoader::loadResource(Ogre::Resource *resource){
        Ogre::Texture *texture(static_cast<Ogre::Texture *>(resource));
        assert(texture);
        flevel_file_.load();
        texture->_notifyOrigin(flevel_file_.getName());
        texture->setTextureType(Ogre::TEX_TYPE_2D);
        texture->setNumMipmaps(0);
        texture->setGamma(1.0);
        texture->setTreatLuminanceAsAlpha(false);
        texture->setFormat(Ogre::PF_UNKNOWN);
        texture->setHardwareGammaEnabled(false);
        BackgroundFilePtr background(flevel_file_.GetBackground());
        PaletteFilePtr palette(flevel_file_.GetPalette());
        Ogre::Image *img(background->CreateImage(palette));
        Ogre::ConstImagePtrList images;
        images.push_back(img);
        texture->_loadImages(images);
        delete img;
    }

}
