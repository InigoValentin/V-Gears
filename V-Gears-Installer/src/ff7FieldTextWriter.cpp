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

#include <iostream>
#include "../include/ff7FieldTextWriter.h"
#include "common/VGearsStringUtil.h"

// TODO: Refactor: this is based on/copied from DatFile::DumpText.

/**
 * Table of English characters.
 */
static const unsigned char ENGLISH_CHARS[256] = {
    // 0  1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // 0x00 - 0x0F
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // 0x10 - 0x1F
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, // 0x20 - 0x2F
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // 0x30 - 0x3F
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // 0x40 - 0x4F
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x50 - 0x5F
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x60 - 0x6F
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x70 - 0x7F
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 - 0x8F
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90 - 0x9F
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xA0 - 0xAF
    0x00, 0x00, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB0 - 0xBF
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xC0 - 0xCF
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xD0 - 0xDF
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xE0 - 0xEF
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xF0 - 0xFF
};

/**
 *  First table of Japanese characters.
 */
static const unsigned short japanese_chars[256] = {
    // 0    1       2       3       4       5       6       7       8       9       A       B       C       D       E       F
    0xD030, 0x7030, 0xD330, 0x7330, 0xD630, 0x7630, 0xD930, 0x0000, 0xDC30, 0x7C30, 0xAC30, 0x4C30, 0xAE30, 0x4E30, 0xB030, 0x5030, // 0x00 - 0x0F
    0xB230, 0x5230, 0xB430, 0x5430, 0xB630, 0x5630, 0xB830, 0x5830, 0xBA30, 0x5A30, 0xBC30, 0x5C30, 0xBE30, 0x5E30, 0xC030, 0x6030, // 0x10 - 0x1F
    0x0000, 0x0000, 0xC530, 0x0000, 0xC730, 0x6730, 0xC930, 0x6930, 0x0000, 0xD130, 0x7130, 0xD430, 0x0000, 0xD730, 0x7730, 0xDA30, // 0x20 - 0x2F
    0x0000, 0xDD30, 0x0000, 0x3000, 0x3100, 0x3200, 0x3300, 0x3400, 0x3500, 0x3600, 0x3700, 0x3800, 0x3900, 0x0130, 0x0230, 0x2000, // 0x30 - 0x3F
    0xCF30, 0x6F30, 0xD230, 0x7230, 0xD530, 0x7530, 0xD830, 0x7830, 0xDB30, 0x7B30, 0xAB30, 0x4B30, 0xAD30, 0x4D30, 0xAF30, 0x4F30, // 0x40 - 0x4F
    0xB130, 0x5130, 0xB330, 0x5330, 0xB530, 0x5530, 0xB730, 0x5730, 0xB930, 0x5930, 0xBB30, 0x5B30, 0xBD30, 0x5D30, 0xBF30, 0x5F30, // 0x50 - 0x5F
    0xC130, 0x6130, 0xC430, 0x6430, 0xC630, 0x6630, 0xC830, 0x6830, 0xA630, 0x4630, 0xA230, 0x4230, 0xA430, 0x4430, 0xA830, 0x4830, // 0x60 - 0x6F
    0xAA30, 0x4A30, 0xCA30, 0x6A30, 0xCB30, 0x6B30, 0x0000, 0x6C30, 0xCD30, 0x6D30, 0xCE30, 0x6E30, 0xDE30, 0x7E30, 0xDF30, 0x7F30, // 0x70 - 0x7F
    0xE030, 0x8030, 0xE130, 0x8130, 0xE230, 0x8230, 0xE930, 0x8930, 0xEA30, 0x8A30, 0xEB30, 0x8B30, 0xEC30, 0x8C30, 0xED30, 0x8D30, // 0x80 - 0x8F
    0xE430, 0x8430, 0xE630, 0x0000, 0x0000, 0x8830, 0xEF30, 0x8F30, 0xF330, 0x9330, 0x0000, 0x9230, 0xC330, 0x6330, 0xE330, 0x8330, // 0x90 - 0x9F
    0xE530, 0x8530, 0xE730, 0x8730, 0xA130, 0x4130, 0xA330, 0x0000, 0xA530, 0x0000, 0xA730, 0x4730, 0x0000, 0x4930, 0x01FF, 0x1FFF, // 0xA0 - 0xAF
    0x0E30, 0x0F30, 0x0000, 0x0000, 0x4100, 0x4200, 0x4300, 0x4400, 0x4500, 0x4600, 0x4700, 0x4800, 0x4900, 0x4A00, 0x4B00, 0x4C00, // 0xB0 - 0xBF
    0x4D00, 0x4E00, 0x4F00, 0x5000, 0x5100, 0x5200, 0x5300, 0x5400, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFB30, 0x0000, // 0xC0 - 0xCF
    0xFC30, 0x7E00, 0x2620, 0x0000, 0x0000, 0x0000, 0x0000, 0x1030, 0x1130, 0x0000, 0x0000, 0x0000, 0x0000, 0x0C30, 0x0D30, 0x08FF, // 0xD0 - 0xDF
    0x09FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0D00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xE0 - 0xEF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xF0 - 0xFF
};

/**
 * Second table of Japanese characters.
 */
static const unsigned short JAPANESE_CHARS_FA[256] = {
    // 0    1       2       3       4       5       6       7       8       9       A       B       C       D       E       F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2759, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x00 - 0x0F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x10 - 0x1F
    0x0672, 0x7F4F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3478, 0x4466, 0x8364, 0x0000, 0x0000, 0x0000, // 0x20 - 0x2F
    0x0000, 0x0000, 0x0000, 0x0000, 0x666B, 0x5E79, 0x0000, 0x0000, 0xA898, 0x0000, 0x176C, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x30 - 0x3F
    0x885B, 0x0000, 0x7D54, 0x0000, 0xD552, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6297, 0x0000, 0x0000, 0x0000, 0x0854, 0x0000, // 0x40 - 0x4F
    0x0000, 0x0000, 0x0E66, 0x0000, 0x0000, 0x9C62, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x004E, 0x0000, 0x0580, // 0x50 - 0x5F
    0x0000, 0x0000, 0x857F, 0x0000, 0x0000, 0x0000, 0x9950, 0x6856, 0x0000, 0x549B, 0xD56C, 0x0000, 0x0000, 0x0000, 0x0000, 0xFA51, // 0x60 - 0x6F
    0x0163, 0xF876, 0x4B62, 0x0000, 0xBA78, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xB965, 0x4C88, 0x0000, // 0x70 - 0x7F
    0x9A5B, 0x0000, 0x0000, 0x4D52, 0x1F77, 0x0000, 0x9A89, 0x1752, 0x8C5F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xDE56, 0x0000, // 0x80 - 0x8F
    0x9A7D, 0x7565, 0x0000, 0x0000, 0x0000, 0x0000, 0x3E5C, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0882, 0x0000, 0x0000, // 0x90 - 0x9F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3E5F, 0x0000, 0x0000, 0x0000, 0x4266, 0x0000, 0x0000, 0x2662, 0x0000, // 0xA0 - 0xAF
    0x0000, 0x0000, 0x0000, 0x0000, 0xB751, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xEA8C, 0x0000, 0x0000, 0x0000, // 0xB0 - 0xBF
    0xCD53, 0x0000, 0xEE76, 0x0000, 0x0000, 0x9B52, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7551, 0x0000, 0x0000, 0x0000, // 0xC0 - 0xCF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xD0 - 0xDF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xE0 - 0xEF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xF0 - 0xFF
};

/**
 * Third table of Japanese characters.
 */
static const unsigned short JAPANESE_CHARS_FB[256] = {
    // 0    1       2       3       4       5       6       7       8       9       A       B       C       D       E       F
    0x0000, 0x0000, 0xB182, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0B4E, 0x0000, 0x6551, 0x4851, 0x0D4E, 0x505B, 0x9B4F, 0x4B5C, // 0x00 - 0x0F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8E96, 0x0000, 0x0000, 0xE890, 0x0000, 0x3458, 0x0000, 0xF24E, 0x9395, 0x0000, 0x0000, // 0x10 - 0x1F
    0x5096, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x554F, 0x0000, 0x9965, 0x0000, 0x0A4E, 0x0000, 0x8B4E, 0x0000, 0x0000, // 0x20 - 0x2F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6559, 0x0000, 0x0000, 0x0000, 0x1D52, 0x5167, 0x0000, 0x0000, 0x0000, 0x0000, // 0x30 - 0x3F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x40 - 0x4F
    0x0000, 0x0000, 0xCA8E, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xBA87, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x50 - 0x5F
    0x0000, 0x7751, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xB672, 0x4B61, 0x0000, // 0x60 - 0x6F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8B95, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x70 - 0x7F
    0x0000, 0x0C54, 0x1F90, 0x4590, 0x0000, 0x0000, 0x0000, 0x0000, 0x7354, 0x0000, 0x0000, 0x0000, 0x0D50, 0x0000, 0x0000, 0x0000, // 0x80 - 0x8F
    0x0000, 0xBA4E, 0xCA4E, 0x0000, 0x0000, 0x535F, 0x0000, 0x0000, 0xDB98, 0xE54E, 0x1659, 0x0000, 0x0000, 0x0000, 0xAB8E, 0x0000, // 0x90 - 0x9F
    0xCB65, 0x0000, 0x0000, 0x0000, 0x5F6A, 0xB068, 0x0000, 0x8970, 0xB065, 0x214E, 0x2C67, 0x1B54, 0x0000, 0x8551, 0x5C4F, 0x668B, // 0xA0 - 0xAF
    0x7972, 0x0000, 0x0000, 0x0000, 0x747A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xB38D, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xB0 - 0xBF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7795, 0x0000, 0x0000, 0x5458, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8B89, // 0xC0 - 0xCF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4351, 0x0000, 0x0000, 0x0000, 0x427D, 0x0000, 0x0000, 0x0D54, 0x0000, 0x0000, // 0xD0 - 0xDF
    0x0000, 0x1062, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xE0 - 0xEF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xF0 - 0xFF
};

/**
 * Fourth table of Japanese characters.
 */
static const unsigned short JAPANESE_CHARS_FC[256] = {
    // 0    1       2       3       4       5       6       7       8       9       A       B       C       D       E       F
    0x0000, 0x0000, 0x188A, 0x0000, 0x7890, 0x6A75, 0x5788, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x00 - 0x0F
    0x9358, 0xC35F, 0x0000, 0x0000, 0x0000, 0x0000, 0x5390, 0x0000, 0x0000, 0x0000, 0x0000, 0xFB5D, 0x0000, 0x0000, 0x0000, 0x0000, // 0x10 - 0x1F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x884E, 0x0000, 0x3D84, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x20 - 0x2F
    0xE565, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xE353, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x30 - 0x3F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x296E, 0x0000, 0x0000, 0x0000, 0x0000, 0x708D, 0x0000, 0x0000, 0x0000, 0x0000, // 0x40 - 0x4F
    0x0000, 0x0000, 0x7153, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x008A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x50 - 0x5F
    0x3159, 0x5765, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x60 - 0x6F
    0x0000, 0x0000, 0xBC62, 0x0000, 0x0000, 0x0000, 0x5965, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x70 - 0x7F
    0x0000, 0x0000, 0x0000, 0x0000, 0xC253, 0xE577, 0x5E80, 0x0000, 0xD54E, 0x2552, 0xC696, 0xE14F, 0x2875, 0x0000, 0x0000, 0x0000, // 0x80 - 0x8F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xE965, 0x0000, 0x0000, 0x0000, 0x0000, 0x718A, 0x305E, 0x0000, 0x0000, 0x0000, // 0x90 - 0x9F
    0x0000, 0xF258, 0x6E6D, 0x0000, 0xA052, 0x0000, 0x047D, 0x0000, 0x3C79, 0x5F67, 0x0000, 0x3775, 0x0000, 0x0000, 0x0000, 0xCB7A, // 0xA0 - 0xAF
    0x0000, 0xFA4F, 0x0000, 0x0000, 0x0000, 0xC179, 0x0000, 0x0000, 0x1250, 0x114F, 0x0000, 0x0000, 0x0000, 0xA952, 0x0000, 0x0000, // 0xB0 - 0xBF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xAC8A, 0x0000, 0x0000, 0xF167, 0x0000, 0x0000, // 0xC0 - 0xCF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xD0 - 0xDF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xE0 - 0xEF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xF0 - 0xFF
};

/**
 * Fifth table of Japanese characters.
 */
static const unsigned short JAPANESE_CHARS_FD[256] = {
    // 0    1       2       3       4       5       6       7       8       9       A       B       C       D       E       F
    0x0000, 0x1D4F, 0x0000, 0xA263, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x00 - 0x0F
    0xE682, 0x0000, 0xF056, 0x0000, 0x0000, 0x0000, 0x0000, 0xA25B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x10 - 0x1F
    0x0000, 0x2B59, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xB88C, 0x1F67, 0x0000, 0x0000, 0x0000, 0x0000, 0xDD4F, 0x2F5E, // 0x20 - 0x2F
    0x0000, 0x0000, 0x0000, 0x1154, 0x0000, 0x0000, 0x7289, 0x0000, 0x216B, 0x0000, 0x207D, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x30 - 0x3F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xB78C, 0x0000, 0x0000, 0x0000, 0x7C5E, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x40 - 0x4F
    0x0000, 0x0000, 0x0000, 0x0000, 0x8179, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x50 - 0x5F
    0x0000, 0x975E, 0x0000, 0x0000, 0x0000, 0xCA7D, 0x1478, 0x0000, 0x0000, 0x8766, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x60 - 0x6F
    0xC599, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x70 - 0x7F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFD88, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x80 - 0x8F
    0x0000, 0x0000, 0x3293, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x5453, 0x0000, 0x0000, // 0x90 - 0x9F
    0x0000, 0x0000, 0x0000, 0x0000, 0x3181, 0x0000, 0x8D8E, 0x0000, 0x0000, 0x0000, 0x0000, 0x3052, 0x0000, 0x0000, 0x0000, 0x0000, // 0xA0 - 0xAF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xB0 - 0xBF
    0x9F52, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x5291, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xC0 - 0xCF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xD0 - 0xDF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xE0 - 0xEF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xF0 - 0xFF
};

/**
 * Sixth table of Japanese characters.
 */
static const unsigned short JAPANESE_CHARS_FE[256] = {
    // 0    1       2       3       4       5       6       7       8       9       A       B       C       D       E       F
    0x0000, 0x0000, 0x0000, 0x667D, 0x0000, 0x0000, 0x0000, 0xAD65, 0x0000, 0x0000, 0xB96C, 0x0000, 0xD550, 0x585B, 0x0000, 0x0000, // 0x00 - 0x0F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x10 - 0x1F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8B4F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x20 - 0x2F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x9758, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x30 - 0x3F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x40 - 0x4F
    0x0000, 0x0000, 0x0000, 0xB48C, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4256, 0xF158, 0x0000, 0x0000, 0x0000, 0x0000, // 0x50 - 0x5F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xB54F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x60 - 0x6F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1D8B, 0x0000, 0x0000, 0x0000, 0xA14F, 0x206B, 0xC45B, 0xD95F, // 0x70 - 0x7F
    0x0000, 0x944E, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xA65E, 0x0000, 0x0000, 0x0000, 0x0000, 0x988A, // 0x80 - 0x8F
    0x0000, 0x0000, 0x298F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x668A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x90 - 0x9F
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7272, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xA0 - 0xAF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xC079, 0x0000, 0xFD8E, 0x0000, 0x0000, 0x0000, // 0xB0 - 0xBF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3B52, // 0xC0 - 0xCF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xD0 - 0xDF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xE0 - 0xEF
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xF0 - 0xFF
};

void FF7FieldTextWriter::Begin(std::string file_name){
    data_.clear();
    logger_.reset(new Logger(file_name));

    // Write BOM
    /* FIX ME - It needs to write data as UTF8.
    std::vector< unsigned char > bomBytes;
    bomBytes.push_back(0xFF);
    bomBytes.push_back(0xFE);
    logger_->Log(bomBytes);
    */
    tag_open_ = false;
    logger_->Log("<texts>\n");
}

u16 FF7FieldTextWriter::GetU16LE(u32 offset){
    if (offset+1 >= data_.size()) throw std::out_of_range("");
    return *reinterpret_cast<u16*>(&data_[offset]);
}

u8 FF7FieldTextWriter::GetU8(u32 offset){
    if (offset >= data_.size()) throw std::out_of_range("");
    return data_[offset];
}

void FF7FieldTextWriter::Write(
  const std::vector<u8>& script_section_buffer, std::string field_name,
  bool english
){
    data_ = script_section_buffer;
    // Get sector 1 offset (scripts and dialog).
    u32 offset_to_sector = 0; // Our buffer IS the start of scripts and dialog.
    /*
    u16 unknown1; // Always 0x0502.
    char nEntities; // Number of entities.
    char nModels; // Number of models.
    u16 wStringOffset; // Offset to strings code below extracts this offset.
    */
    u16 offset_to_dialogs = GetU16LE(offset_to_sector + 0x04);
    std::vector< unsigned char > dialog; // text line to write out
    // HACK: If this function throws then this iteration generates invalid XML
    // so close the unclosed XML tag - this function should be made thread safe
    // and/or fix the fields that fail to convert correctly.
    if (tag_open_){
        logger_->Log("[ERROR exception]");
        logger_->Log("</dialog>\n");
    }
    const u16 dialogCount = GetU16LE(offset_to_sector + offset_to_dialogs);
    for (u16 i = 0; i <dialogCount; ++ i){
        // gGt offset of string data.
        u32 offset
          = offset_to_sector + offset_to_dialogs + GetU16LE(
            offset_to_sector + offset_to_dialogs
            + 0x02 +  // +2 to skip dialog count
            i * 0x02 // *2 because each char is 2 bytes
          );
        AddText(dialog);
        logger_->Log(
          "<dialog name=\"" + field_name + "_" + std::to_string(i) + "\">"
        );
        tag_open_ = true;
        for (unsigned char temp = 0x00;; ++ offset){
            temp = GetU8(offset);
            if (temp == 0xFF) break;
            if (temp == 0xE0 && english == true){
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
            }
            else if (temp == 0xE1 && english == true){
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
            }
            else if (temp == 0xE2 && english == true){
                dialog.push_back(ENGLISH_CHARS[0x0C]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
            }
            else if (temp == 0xE8){
                AddText(dialog);
                logger_->Log("<next_page />");
            }
            else if (temp == 0xEA){
                AddText(dialog);
                logger_->Log("<include name=\"char_cloud\" />");
            }
            else if (temp == 0xEB){
                AddText(dialog);
                logger_->Log("<include name=\"char_barret\" />");
            }
            else if (temp == 0xEC){
                AddText(dialog);
                logger_->Log("<character id=\"2\" />");
            }
            else if (temp == 0xED){
                AddText(dialog);
                logger_->Log("<character id=\"3\" />");
            }
            else if (temp == 0xEE){
                AddText(dialog);
                logger_->Log("<character id=\"4\" />");
            }
            else if (temp == 0xEF){
                AddText(dialog);
                logger_->Log("<character id=\"5\" />");
            }
            else if (temp == 0xF0){
                AddText(dialog);
                logger_->Log("<character id=\"6\" />");
            }
            else if (temp == 0xF1){
                AddText(dialog);
                logger_->Log("<character id=\"7\" />");
            }
            else if (temp == 0xF2){
                AddText(dialog);
                logger_->Log("<character id=\"8\" />");
            }
            else if (temp == 0xF3){
                AddText(dialog);
                logger_->Log("<character party_id=\"0\" />");
            }
            else if (temp == 0xF4){
                AddText(dialog);
                logger_->Log("<character party_id=\"1\" />");
            }
            else if (temp == 0xF5){
                AddText(dialog);
                logger_->Log("<character party_id=\"2\" />");
            }
            else if (temp == 0xF6){
                AddText(dialog);
                logger_->Log("<image sprite=\"ButtonCircle\" />");
            }
            else if (temp == 0xF7){
                AddText(dialog);
                logger_->Log("<image sprite=\"ButtonTriangle\" />");
            }
            else if (temp == 0xF8){
                AddText(dialog);
                logger_->Log("<image sprite=\"ButtonSquare\" />");
            }
            else if (temp == 0xF9){
                AddText(dialog);
                logger_->Log("<image sprite=\"ButtonCross\" />");
            }
            else if (
              (temp == 0xFA || temp == 0xFB || temp == 0xFC || temp == 0xFD)
              && english == false
            ){
                ++ offset;
                unsigned char temp2 = GetU8(offset);
                if (temp == 0xFA){
                    if (JAPANESE_CHARS_FA[temp2] != 0x0000){
                        dialog.push_back(JAPANESE_CHARS_FA[temp2] >> 8);
                        dialog.push_back(JAPANESE_CHARS_FA[temp2] & 0xFF);
                    }
                    else{
                        AddText(dialog);
                        logger_->Log(
                          "[MISSING 0xFA " + HexToString(temp2, 2, '0') + "]"
                        );
                    }
                }
                else if (temp == 0xFB){
                    if (JAPANESE_CHARS_FB[temp2] != 0x0000){
                        dialog.push_back(JAPANESE_CHARS_FB[temp2] >> 8);
                        dialog.push_back(JAPANESE_CHARS_FB[temp2] & 0xFF);
                    }
                    else{
                        AddText(dialog);
                        logger_->Log(
                          "[MISSING 0xFB " + HexToString(temp2, 2, '0') + "]"
                        );
                    }
                }
                else if (temp == 0xFC){
                    if (JAPANESE_CHARS_FC[temp2] != 0x0000){
                        dialog.push_back(JAPANESE_CHARS_FC[temp2] >> 8);
                        dialog.push_back(JAPANESE_CHARS_FC[temp2] & 0xFF);
                    }
                    else{
                        AddText(dialog);
                        logger_->Log(
                          "[MISSING 0xFC " + HexToString(temp2, 2, '0') + "]"
                        );
                    }
                }
                else if (temp == 0xFD){
                    if (JAPANESE_CHARS_FD[temp2] != 0x0000){
                        dialog.push_back(JAPANESE_CHARS_FD[temp2] >> 8);
                        dialog.push_back(JAPANESE_CHARS_FD[temp2] & 0xFF);
                    }
                    else{
                        AddText(dialog);
                        logger_->Log(
                          "[MISSING 0xFD " + HexToString(temp2, 2, '0') + "]"
                        );
                    }
                }
            }
            else if (temp == 0xFE){
                ++ offset;
                unsigned char temp2 = GetU8(offset);

                // TODO: Maybe value must be RGB?
                // TODO: anfrst map fails to terminate a colour breaking all of
                // the XML, so colours turned off for now.
                if (temp2 == 0xD4){
                    AddText(dialog);
                   // logger_->Log("<colour value=\"red\">");
                }
                else if (temp2 == 0xD5){
                    AddText(dialog);
                   // logger_->Log("<colour value=\"purple\" >");
                }
                else if (temp2 == 0xD6){
                    AddText(dialog);
                    //logger_->Log("<colour value=\"green\" >");
                }
                else if (temp2 == 0xD7){
                    AddText(dialog);
                   // logger_->Log("<colour value=\"cyan\" >");
                }
                else if (temp2 == 0xD8){
                    AddText(dialog);
                   // logger_->Log("<colour value=\"yellow\">");
                }
                else if (temp2 == 0xD9){
                    AddText(dialog);
                   // logger_->Log("</colour>");
                }
                else if (temp2 == 0xDC){
                    AddText(dialog);
                   // logger_->Log("<pause_ok >");
                }
                else if (temp2 == 0xDD){
                    u16 wait = GetU16LE(offset + 1);
                    ++ offset;
                    ++ offset;
                    AddText(dialog);
                    logger_->Log(
                      "<pause time=\"" + IntToString(wait) + "\" />"
                    );
                }
                else{
                    if (JAPANESE_CHARS_FE[temp2] != 0x0000 && english == false){
                        dialog.push_back(JAPANESE_CHARS_FE[temp2] >> 8);
                        dialog.push_back(JAPANESE_CHARS_FE[temp2] & 0xFF);
                    }
                    else{
                        AddText(dialog);
                        logger_->Log(
                          "[MISSING 0xFE " + HexToString(temp2, 2, '0') + "]"
                        );
                    }
                }
            }
            else{
                if (japanese_chars[temp] != 0x0000 && english == false){
                   // dialog.push_back(japanese_chars[temp]);
                }
                else if (ENGLISH_CHARS[temp] != 0x0000 && english == true){
                    dialog.push_back(ENGLISH_CHARS[temp]);
                }
                else{
                    AddText(dialog);
                    if (temp == 0xa9) logger_->Log("..."); // TODO Verify.
                    else{
                        logger_->Log(
                          "[MISSING CHAR " + HexToString(temp, 2, '0') + "]"
                        );
                    }
                }
            }
        }
        AddText(dialog);
        logger_->Log("</dialog>\n\n");
        tag_open_ = false;
    }

}

void FF7FieldTextWriter::End(){
    if (logger_) logger_->Log("</texts>\n");
    logger_.reset();
}

void FF7FieldTextWriter::AddText(std::vector< unsigned char >& text){
    if (text.empty()) return;
    std::string str(reinterpret_cast<char*>(text.data()), text.size());
    str = Ogre::StringUtil::replaceAll(str, "&", "&amp;");
    str = Ogre::StringUtil::replaceAll(str, "\"", "&quot;");
    str = Ogre::StringUtil::replaceAll(str, "'", "&apos;");
    str = Ogre::StringUtil::replaceAll(str, "<", "&lt;");
    str = Ogre::StringUtil::replaceAll(str, ">", "&gt;");
    logger_->Log(str);
    text.clear();
}
