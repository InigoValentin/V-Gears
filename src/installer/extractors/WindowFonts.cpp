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

#include <array>
#include <OgreImage.h>
#include <OgreColourValue.h>
#include "WindowFonts.h"

namespace{

    /**
     * An RGBA colour.
     */
    struct Colour{
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };

    /**
     * Fully transparent colour.
     */
    const Colour TRANSPARENT = {0, 0, 0, 0};

    /**
     * Colour of every palette index of a 4 bpp image.
     */
    typedef std::array<Colour, 16> Palette;

    /**
     * Palette of the dialog font: index 3 is the glyph, index 1 its shadow.
     */
    Palette TextPalette(){
        Palette palette;
        palette.fill(TRANSPARENT);
        palette[1] = {32, 32, 32, 255};
        palette[3] = {230, 230, 230, 255};
        return palette;
    }

    /**
     * Palette of the menu font and the small digits: index 1 is the glyph, index 15 its shadow.
     */
    Palette MenuPalette(){
        Palette palette;
        palette.fill(TRANSPARENT);
        palette[1] = {230, 230, 230, 255};
        palette[15] = {24, 24, 24, 255};
        return palette;
    }

    /**
     * Palette of the timer font. It's opaque: indices 2 to 4 are the glyph, the others the background.
     */
    Palette TimerPalette(){
        Palette palette;
        palette.fill({0, 0, 0, 255});
        palette[2] = {255, 0, 255, 255};
        palette[3] = {197, 0, 197, 255};
        palette[4] = {139, 0, 139, 255};
        palette[13] = {41, 41, 41, 255};
        palette[14] = {24, 24, 24, 255};
        palette[15] = {24, 24, 24, 255};
        return palette;
    }

    /**
     * Creates an image filled with a colour.
     *
     * @param[in] width Image width.
     * @param[in] height Image height.
     * @param[in] colour Colour of every pixel.
     * @return The image.
     */
    WindowFonts::RgbaImage CreateImage(int width, int height, const Colour& colour){
        WindowFonts::RgbaImage image;
        image.width = width;
        image.height = height;
        image.pixels.resize(width * height * 4);
        for (int i = 0; i < width * height; i ++){
            image.pixels[i * 4] = colour.r;
            image.pixels[i * 4 + 1] = colour.g;
            image.pixels[i * 4 + 2] = colour.b;
            image.pixels[i * 4 + 3] = colour.a;
        }
        return image;
    }

    /**
     * Copies a rectangle of a TIM image into an RGBA image, converting it with a palette.
     *
     * Pixels outside of the destination are ignored.
     *
     * @param[in] src The TIM image.
     * @param[in] src_x X coordinate of the rectangle in the TIM image.
     * @param[in] src_y Y coordinate of the rectangle in the TIM image.
     * @param[in] width Width of the rectangle.
     * @param[in] height Height of the rectangle.
     * @param[in] palette Colour of every palette index.
     * @param[out] dst The destination image.
     * @param[in] dst_x X coordinate of the rectangle in the destination image.
     * @param[in] dst_y Y coordinate of the rectangle in the destination image.
     */
    void Blit(
      const WindowFonts::IndexImage& src, int src_x, int src_y, int width, int height, const Palette& palette,
      WindowFonts::RgbaImage& dst, int dst_x, int dst_y
    ){
        for (int y = 0; y < height; y ++){
            for (int x = 0; x < width; x ++){
                const int tx = dst_x + x;
                const int ty = dst_y + y;
                if (tx < 0 || ty < 0 || tx >= dst.width || ty >= dst.height) continue;
                const Colour& c = palette[src.At(src_x + x, src_y + y) & 0x0F];
                const int i = (ty * dst.width + tx) * 4;
                dst.pixels[i] = c.r;
                dst.pixels[i + 1] = c.g;
                dst.pixels[i + 2] = c.b;
                dst.pixels[i + 3] = c.a;
            }
        }
    }

    /**
     * Reads a little endian 16 bit value.
     */
    u32 ReadU16(const u8* data){return data[0] | (data[1] << 8);}

    /**
     * Reads a little endian 32 bit value.
     */
    u32 ReadU32(const u8* data){return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);}
}

u8 WindowFonts::IndexImage::At(int x, int y) const{
    if (x < 0 || y < 0 || x >= width || y >= height) return 0;
    return pixels[y * width + x];
}

bool WindowFonts::ReadTim4(const u8* data, size_t size, IndexImage& image){
    // Header: magic (0x10) and flags. 4 bpp with a palette is 0x08.
    if (size < 8 || ReadU32(data) != 0x10 || (ReadU32(data + 4) & 0x0F) != 0x08) return false;
    size_t offset = 8;
    // Palette block: its length comes first, the colours are not needed.
    if (offset + 12 > size) return false;
    offset += ReadU32(data + offset);
    // Image block: length, x, y, width in 16 bit units, height.
    if (offset + 12 > size) return false;
    const int width = ReadU16(data + offset + 8) * 4;
    const int height = ReadU16(data + offset + 10);
    offset += 12;
    if (offset + static_cast<size_t>(width / 2) * height > size) return false;
    image.width = width;
    image.height = height;
    image.pixels.resize(width * height);
    for (int i = 0; i < width * height / 2; i ++){
        image.pixels[i * 2] = data[offset + i] & 0x0F;
        image.pixels[i * 2 + 1] = data[offset + i] >> 4;
    }
    return true;
}

WindowFonts::RgbaImage WindowFonts::BuildTextFont(const IndexImage& font){
    RgbaImage image = CreateImage(256, 256, TRANSPARENT);
    Blit(font, 0, 0, 256, 256, TextPalette(), image, 0, 0);
    return image;
}

WindowFonts::RgbaImage WindowFonts::BuildMenuFont(const IndexImage& window){
    RgbaImage image = CreateImage(128, 256, TRANSPARENT);
    // Twelve rows of 8 pixels at (128, 152).
    for (int row = 0; row < 12; row ++) Blit(window, 128, 152 + row * 8, 128, 8, MenuPalette(), image, 0, row * 16 + 8);
    return image;
}

WindowFonts::RgbaImage WindowFonts::BuildDigitsFont(const IndexImage& window){
    RgbaImage image = CreateImage(128, 8, TRANSPARENT);
    const Palette palette = MenuPalette();
    // Digits 0 to 9, in cells of 8 pixels at (136, 0).
    Blit(window, 136, 0, 80, 8, palette, image, 0, 0);
    // Symbols, taken from the menu font: '-', '+', '%' and ':'. Cell 10 is the blank space.
    Blit(window, 136, 240, 8, 8, palette, image, 88, 0);
    Blit(window, 152, 216, 8, 8, palette, image, 96, 0);
    Blit(window, 152, 232, 8, 8, palette, image, 104, 0);
    Blit(window, 168, 232, 8, 8, palette, image, 112, 0);
    return image;
}

WindowFonts::RgbaImage WindowFonts::BuildTimerFont(const IndexImage& window){
    // Two rows of 24 pixels at (144, 80). The image is wider than what's left of the texture.
    RgbaImage image = CreateImage(128, 48, {0, 0, 0, 255});
    Blit(window, 144, 80, 112, 48, TimerPalette(), image, 0, 0);
    return image;
}

void WindowFonts::SavePng(const RgbaImage& image, const std::string& file_name){
    Ogre::Image png(Ogre::PF_R8G8B8A8_UINT, image.width, image.height);
    for (int y = 0; y < image.height; y ++){
        for (int x = 0; x < image.width; x ++){
            const u8* p = &image.pixels[(y * image.width + x) * 4];
            png.setColourAt(Ogre::ColourValue(p[0] / 255.0f, p[1] / 255.0f, p[2] / 255.0f, p[3] / 255.0f), x, y, 0);
        }
    }
    png.save(file_name);
}
