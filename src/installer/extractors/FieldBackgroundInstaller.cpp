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

#include <cmath>
#include <fstream>
#include <OgreImage.h>
#include <OgreStringConverter.h>
#include "FieldBackgroundInstaller.h"

const int FieldBackgroundInstaller::SCALE_UP_FACTOR = 3;

const int FieldBackgroundInstaller::SCREEN_WIDTH = 320;

const int FieldBackgroundInstaller::SCREEN_HEIGHT = 240;

FieldBackgroundInstaller::FieldBackgroundInstaller(const std::string& data_dir): data_dir_(data_dir){}

void FieldBackgroundInstaller::Install(
  const VGears::FLevelFilePtr& field, const FieldDecompiler::DecompiledScript& decompiled,
  const float scale
){
    const std::string dir = data_dir_ + "/" + FieldData::MAPS_DIR + "/";
    const VGears::PaletteFilePtr& palette = field->GetPalette();
    const VGears::BackgroundFilePtr& background = field->GetBackground();
    std::unique_ptr<Ogre::Image> image(background->CreateImage(palette));
    image->encode("png");
    image->save(dir + field->getName() + "/tiles.png");
    TiXmlDocument doc;
    std::unique_ptr<TiXmlElement> element(new TiXmlElement("background2d"));
    element->SetAttribute("image", field->getName() + "/tiles.png");
    WriteCamera(field, scale, *element);
    // The palette animations: the tiles of the palettes a script changes are painted again by
    // the engine.
    if (!decompiled.palette_pages.empty() || decompiled.palette_pages_dynamic){
        const std::string palette_name = field->getName() + "/palette.bin";
        if (WritePaletteFile(
          dir + palette_name, palette, background->GetPaletteTiles(), decompiled.palette_pages,
          decompiled.palette_pages_dynamic
        )){
            element->SetAttribute("palette", palette_name);
        }
    }
    // TODO: Should only the tiles of the enabled layers be written?
    const int width = image->getWidth();
    const int height = image->getHeight();
    int layer_index = -1;
    for (const auto& layer : background->GetLayers()){
        ++ layer_index;
        for (const VGears::BackgroundFile::SpriteData& sprite : layer.sprites)
            element->LinkEndChild(CreateTile(sprite, layer_index, width, height, scale).release());
    }
    doc.LinkEndChild(element.release());
    doc.SaveFile(dir + field->getName() + "/bg.xml");
}

std::string FieldBackgroundInstaller::BlendingName(const int blending){
    if (blending == 0) return "alpha";
    // 3 is source + 0.25 * destination:
    if (blending == 1 || blending == 3) return "add";
    if (blending == 2) return "subtract";
    // TODO: Should probably throw to fail conversion:
    return "unknown";
}

void FieldBackgroundInstaller::WriteCamera(
  const VGears::FLevelFilePtr& field, const float scale, TiXmlElement& background
){
    const VGears::CameraMatrixFilePtr& camera_matrix = field->GetCameraMatrix();
    const Ogre::Vector3 position = camera_matrix->GetPosition() / scale;
    const Ogre::Quaternion orientation = camera_matrix->GetOrientation();
    const Ogre::Degree fov = camera_matrix->GetFov(static_cast<float>(SCREEN_HEIGHT));
    const auto& range = field->GetTriggers()->GetCameraRange();
    background.SetAttribute("position", Ogre::StringConverter::toString(position));
    background.SetAttribute("orientation", Ogre::StringConverter::toString(orientation));
    background.SetAttribute("fov", Ogre::StringConverter::toString(fov));
    background.SetAttribute(
      "range",
      std::to_string(range.left * SCALE_UP_FACTOR) + " " + std::to_string(range.top * SCALE_UP_FACTOR)
      + " " + std::to_string(range.right * SCALE_UP_FACTOR) + " "
      + std::to_string(range.bottom * SCALE_UP_FACTOR)
    );
    background.SetAttribute(
      "clip",
      std::to_string(SCREEN_WIDTH * SCALE_UP_FACTOR) + " " + std::to_string(SCREEN_HEIGHT * SCALE_UP_FACTOR)
    );
}

std::unique_ptr<TiXmlElement> FieldBackgroundInstaller::CreateTile(
  const VGears::BackgroundFile::SpriteData& sprite, const int layer, const int width,
  const int height, const float scale
){
    std::unique_ptr<TiXmlElement> element(new TiXmlElement("tile"));
    element->SetAttribute(
      "destination",
      Ogre::StringConverter::toString(sprite.dst.x * SCALE_UP_FACTOR) + " "
      + Ogre::StringConverter::toString(sprite.dst.y * SCALE_UP_FACTOR)
    );
    element->SetAttribute("width", Ogre::StringConverter::toString(sprite.width * SCALE_UP_FACTOR));
    element->SetAttribute("height", Ogre::StringConverter::toString(sprite.height * SCALE_UP_FACTOR));
    // The tiles are in one image: the UV coordinates are in the 0 to 1 range.
    const float u0 = static_cast<float>(sprite.src.x) / width;
    const float v0 = static_cast<float>(sprite.src.y) / height;
    const float u1 = static_cast<float>(sprite.src.x + sprite.width) / width;
    const float v1 = static_cast<float>(sprite.src.y + sprite.height) / height;
    element->SetAttribute(
      "uv",
      Ogre::StringConverter::toString(u0) + " " + Ogre::StringConverter::toString(v0) + " "
      + Ogre::StringConverter::toString(u1) + " " + Ogre::StringConverter::toString(v1)
    );
    // TODO: It works (on FFVII PC), but why this number?
    // The depth is a distance from the camera, so it has the scale of the field like the position
    // of the camera does. The far depths (the backgrounds, from 127 on) are only an order and are
    // kept.
    float depth = static_cast<float>(sprite.depth) * (0.03125f);
    if (depth < 127.0f) depth /= scale;
    element->SetAttribute("depth", Ogre::StringConverter::toString(depth));
    element->SetAttribute("blending", BlendingName(sprite.blending));
    // Type 3 adds only a quarter of the colour of the tile: with all of it the glows turn white
    // (ujunon4).
    if (sprite.blending == 3) element->SetAttribute("intensity", "0.25");
    // The layer is needed by the scripts that scroll them at their own speed.
    element->SetAttribute("layer", layer);
    // The sprites that belong to a group (animation_id) are drawn only while a script (BGON) or a
    // background trigger turns on one of their states. The original game clears the state of
    // every group when it loads a field, so a group that nothing turns on is never drawn (the
    // debris behind the doors of mds6_2, for example).
    if (sprite.animation_id != 0){
        element->SetAttribute("area", static_cast<int>(sprite.animation_id));
        element->SetAttribute("state", static_cast<int>(sprite.animation_frame));
    }
    return element;
}

bool FieldBackgroundInstaller::WritePaletteFile(
  const std::string& path, const VGears::PaletteFilePtr& palette,
  const std::vector<VGears::BackgroundFile::PaletteTile>& tiles, const std::set<int>& pages,
  bool all_pages
){
    std::vector<const VGears::BackgroundFile::PaletteTile*> used;
    for (const auto& tile : tiles){
        if (all_pages || pages.count(tile.palette_page) > 0) used.push_back(&tile);
    }
    if (used.empty() || palette.isNull() || palette->GetPages().empty()) return false;
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    auto write16 = [&file](uint16_t v){
        const char bytes[2] = {static_cast<char>(v & 0xFF), static_cast<char>(v >> 8)};
        file.write(bytes, 2);
    };
    const auto& palette_pages = palette->GetPages();
    file.write("VGPL", 4);
    write16(static_cast<uint16_t>(palette_pages.size()));
    write16(static_cast<uint16_t>(palette_pages[0].size()));
    for (const auto& page : palette_pages){
        for (const auto& color : page){
            const int r = static_cast<int>(std::lround(color.r * 31.0f));
            const int g = static_cast<int>(std::lround(color.g * 31.0f));
            const int b = static_cast<int>(std::lround(color.b * 31.0f));
            write16(static_cast<uint16_t>(r | (g << 5) | (b << 10) | (color.a > 0.0f ? 0x8000 : 0)));
        }
    }
    const uint32_t count = static_cast<uint32_t>(used.size());
    write16(static_cast<uint16_t>(count & 0xFFFF));
    write16(static_cast<uint16_t>(count >> 16));
    for (const auto* tile : used){
        write16(tile->x);
        write16(tile->y);
        write16(tile->width);
        write16(tile->height);
        write16(tile->palette_page);
        file.put(tile->first_color_hidden ? 1 : 0);
        file.write(
          reinterpret_cast<const char*>(tile->indexes.data()),
          static_cast<std::streamsize>(tile->indexes.size())
        );
    }
    return true;
}
