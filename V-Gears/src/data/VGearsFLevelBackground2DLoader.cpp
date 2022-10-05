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

#include "data/VGearsFLevelBackground2DLoader.h"
#include "data/VGearsFLevelFile.h"
#include "map/VGearsBackground2DFile.h"
#include "FF7Common.h"

namespace VGears{

    FLevelBackground2DLoader::FLevelBackground2DLoader(FLevelFile &flevel_file)
      : flevel_file_(flevel_file){}

    FLevelBackground2DLoader::~FLevelBackground2DLoader(){}

    void FLevelBackground2DLoader::loadResource(Ogre::Resource *resource){
        Background2DFile *background_2d(
          static_cast<Background2DFile *>(resource)
        );
        assert(background_2d);
        flevel_file_.load();
        background_2d->_notifyOrigin(flevel_file_.getName());
        background_2d->SetTextureName(flevel_file_.GetBackgroundTextureName());
        background_2d->SetClip(
          Ogre::Vector2(SCREEN_WIDTH, SCREEN_HEIGHT)
        );
        CameraMatrixFilePtr camera_matrix(flevel_file_.GetCameraMatrix());
        Ogre::Real scale(Ogre::Real(camera_matrix->GetCount()));
        background_2d->SetPosition(
          camera_matrix->GetPosition() / -(scale * FIELD_POSITION_SCALE)
        );
        background_2d->SetOrientation(camera_matrix->GetOrientation());
        background_2d->SetFov(camera_matrix->GetFov(SCREEN_WIDTH));
        Ogre::Vector4 range(0, 0, 0, 0);
        TileList& tiles(background_2d->GetTiles());
        BackgroundFilePtr background(flevel_file_.GetBackground());
        BackgroundFile::SpritePtrList sprites;
        background->AddAllSprites(sprites);
        auto it(sprites.begin());
        auto it_end(sprites.end());
        Ogre::Real step(BackgroundFile::SPRITE_WIDTH / 1024.0);
        Ogre::Vector4 uv(0, 0, step, step);
        Ogre::Vector4 col(step, 0, step, 0);
        Ogre::Vector4 row(0, step, 0, step);
        while (it != it_end){
            const SpriteData& sprite(**it);
            Tile tile;
            tile.width  = BackgroundFile::SPRITE_WIDTH;
            tile.height = BackgroundFile::SPRITE_HEIGHT;
            tile.blending = B_ALPHA;
            tile.depth = sprite.depth / (scale * FIELD_DEPTH_SCALE);
            /*
            if depth >= 1
                if(??? / 4.0 < depth)
                    if(depth < 4095)
                    else
                        depth = 0.9999
                else
                    depth = depth / 1000.0
            else
                depth = 0.0001
            */

            tile.destination.x = sprite.dst.x;
            tile.destination.y = sprite.dst.y;
            tile.uv = uv;
            range.x = std::min(tile.destination.x, range.x);
            range.y = std::min(tile.destination.y, range.y);
            range.z = std::max(tile.destination.x + tile.width , range.z);
            range.w = std::max(tile.destination.y + tile.height, range.w);
            tiles.push_back(tile);
            uv += col;
            if (uv.x >= 1){
                uv.x = 0;
                uv.z = step;
                uv += row;
            }
            ++ it;
        }
        background_2d->SetRange(range);
    }

}
