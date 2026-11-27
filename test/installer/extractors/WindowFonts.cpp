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

#include <boost/test/unit_test.hpp>
#include "installer/extractors/WindowFonts.h"

namespace{

    void PutU16(std::vector<u8>& data, u32 value){
        data.push_back(value & 0xFF);
        data.push_back((value >> 8) & 0xFF);
    }

    void PutU32(std::vector<u8>& data, u32 value){
        PutU16(data, value & 0xFFFF);
        PutU16(data, value >> 16);
    }

    /**
     * Builds a 4 bpp TIM with one palette. Pixel (x, y) has index (x + y) % 16.
     */
    std::vector<u8> MakeTim(int width, int height){
        std::vector<u8> data;
        PutU32(data, 0x10);
        PutU32(data, 0x08);
        // Palette block: 16 colours.
        PutU32(data, 12 + 32);
        PutU16(data, 0);
        PutU16(data, 0);
        PutU16(data, 16);
        PutU16(data, 1);
        for (int i = 0; i < 16; i ++) PutU16(data, 0x7FFF);
        // Image block.
        PutU32(data, 12 + width * height / 2);
        PutU16(data, 0);
        PutU16(data, 0);
        PutU16(data, width / 4);
        PutU16(data, height);
        for (int y = 0; y < height; y ++){
            for (int x = 0; x < width; x += 2)
                data.push_back(((x + y) % 16) | (((x + 1 + y) % 16) << 4));
        }
        return data;
    }

    /**
     * Makes an index image of the size of the WINDOW.BIN textures, filled with one index.
     */
    WindowFonts::IndexImage MakeImage(u8 index){
        WindowFonts::IndexImage image;
        image.width = 256;
        image.height = 256;
        image.pixels.assign(256 * 256, index);
        return image;
    }

    /**
     * Gets a pixel of an RGBA image as a 32 bit value, 0xRRGGBBAA.
     */
    u32 Pixel(const WindowFonts::RgbaImage& image, int x, int y){
        const u8* p = &image.pixels[(y * image.width + x) * 4];
        return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    }
}

BOOST_AUTO_TEST_CASE(WindowFontsReadTim4){
    const std::vector<u8> tim = MakeTim(16, 4);
    WindowFonts::IndexImage image;
    BOOST_REQUIRE(WindowFonts::ReadTim4(tim.data(), tim.size(), image));
    BOOST_CHECK_EQUAL(image.width, 16);
    BOOST_CHECK_EQUAL(image.height, 4);
    for (int y = 0; y < 4; y ++)
        for (int x = 0; x < 16; x ++) BOOST_CHECK_EQUAL(image.At(x, y), (x + y) % 16);
    // Outside of the image.
    BOOST_CHECK_EQUAL(image.At(16, 0), 0);
    BOOST_CHECK_EQUAL(image.At(0, -1), 0);
}

BOOST_AUTO_TEST_CASE(WindowFontsReadTim4Invalid){
    WindowFonts::IndexImage image;
    std::vector<u8> tim = MakeTim(16, 4);
    // Truncated.
    BOOST_CHECK(!WindowFonts::ReadTim4(tim.data(), tim.size() - 1, image));
    // 8 bpp.
    tim[4] = 0x09;
    BOOST_CHECK(!WindowFonts::ReadTim4(tim.data(), tim.size(), image));
    // Not a TIM.
    tim[0] = 0x11;
    BOOST_CHECK(!WindowFonts::ReadTim4(tim.data(), tim.size(), image));
}

BOOST_AUTO_TEST_CASE(WindowFontsTextFont){
    WindowFonts::IndexImage font = MakeImage(0);
    font.pixels[0] = 3;
    font.pixels[1] = 1;
    font.pixels[2] = 2;
    const WindowFonts::RgbaImage image = WindowFonts::BuildTextFont(font);
    BOOST_CHECK_EQUAL(image.width, 256);
    BOOST_CHECK_EQUAL(image.height, 256);
    BOOST_CHECK_EQUAL(Pixel(image, 0, 0), 0xE6E6E6FF);
    BOOST_CHECK_EQUAL(Pixel(image, 1, 0), 0x202020FF);
    BOOST_CHECK_EQUAL(Pixel(image, 2, 0), 0x00000000);
    BOOST_CHECK_EQUAL(Pixel(image, 3, 0), 0x00000000);
}

BOOST_AUTO_TEST_CASE(WindowFontsMenuFont){
    WindowFonts::IndexImage window = MakeImage(1);
    // Shadow on the first row of the second glyph row.
    for (int x = 128; x < 256; x ++) window.pixels[160 * 256 + x] = 15;
    const WindowFonts::RgbaImage image = WindowFonts::BuildMenuFont(window);
    BOOST_CHECK_EQUAL(image.width, 128);
    BOOST_CHECK_EQUAL(image.height, 256);
    // Each 8 pixel row goes to the lower half of a 16 pixel slot.
    BOOST_CHECK_EQUAL(Pixel(image, 0, 7), 0x00000000);
    BOOST_CHECK_EQUAL(Pixel(image, 0, 8), 0xE6E6E6FF);
    BOOST_CHECK_EQUAL(Pixel(image, 127, 15), 0xE6E6E6FF);
    BOOST_CHECK_EQUAL(Pixel(image, 5, 24), 0x181818FF);
    BOOST_CHECK_EQUAL(Pixel(image, 5, 25), 0xE6E6E6FF);
    BOOST_CHECK_EQUAL(Pixel(image, 0, 191), 0xE6E6E6FF);
    // The last four slots are empty.
    BOOST_CHECK_EQUAL(Pixel(image, 0, 200), 0x00000000);
    BOOST_CHECK_EQUAL(Pixel(image, 0, 255), 0x00000000);
}

BOOST_AUTO_TEST_CASE(WindowFontsDigitsFont){
    WindowFonts::IndexImage window = MakeImage(0);
    window.pixels[0 * 256 + 136] = 1; // '0'
    window.pixels[3 * 256 + 215] = 15; // '9', last column.
    window.pixels[240 * 256 + 137] = 1; // '-'
    window.pixels[216 * 256 + 152] = 1; // '+'
    window.pixels[232 * 256 + 152] = 1; // '%'
    window.pixels[232 * 256 + 168] = 1; // ':'
    window.pixels[0 * 256 + 216] = 1; // After the digits, not copied.
    const WindowFonts::RgbaImage image = WindowFonts::BuildDigitsFont(window);
    BOOST_CHECK_EQUAL(image.width, 128);
    BOOST_CHECK_EQUAL(image.height, 8);
    BOOST_CHECK_EQUAL(Pixel(image, 0, 0), 0xE6E6E6FF);
    BOOST_CHECK_EQUAL(Pixel(image, 79, 3), 0x181818FF);
    BOOST_CHECK_EQUAL(Pixel(image, 80, 0), 0x00000000);
    BOOST_CHECK_EQUAL(Pixel(image, 89, 0), 0xE6E6E6FF);
    BOOST_CHECK_EQUAL(Pixel(image, 96, 0), 0xE6E6E6FF);
    BOOST_CHECK_EQUAL(Pixel(image, 104, 0), 0xE6E6E6FF);
    BOOST_CHECK_EQUAL(Pixel(image, 112, 0), 0xE6E6E6FF);
}

BOOST_AUTO_TEST_CASE(WindowFontsTimerFont){
    WindowFonts::IndexImage window = MakeImage(2);
    window.pixels[80 * 256 + 144] = 13;
    const WindowFonts::RgbaImage image = WindowFonts::BuildTimerFont(window);
    BOOST_CHECK_EQUAL(image.width, 128);
    BOOST_CHECK_EQUAL(image.height, 48);
    BOOST_CHECK_EQUAL(Pixel(image, 0, 0), 0x292929FF);
    BOOST_CHECK_EQUAL(Pixel(image, 1, 0), 0xFF00FFFF);
    BOOST_CHECK_EQUAL(Pixel(image, 111, 47), 0xFF00FFFF);
    // Past the right edge of the texture.
    BOOST_CHECK_EQUAL(Pixel(image, 112, 0), 0x000000FF);
}
