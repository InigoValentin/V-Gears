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

#include <boost/algorithm/string/replace.hpp>
#include "installer/extractors/KernelTextList.h"
#include "installer/tables/Characters.h"

KernelTextList::KernelTextList(const File& section): section_(section){}

std::string KernelTextList::NextName(){
    std::string text;
    int offset = section_.readU16LE();
    for (u8 ch = section_.GetU8(offset); ch != 0xFF; ch = section_.GetU8(++ offset))
        text += ENGLISH_CHARS[ch];
    boost::replace_all(text, "\"", "'");
    return text;
}

std::string KernelTextList::NextDescription(){
    std::string text;
    int offset = section_.readU16LE();
    for (u8 ch = section_.GetU8(offset); ch != 0xFF; ch = section_.GetU8(++ offset)){
        if (ch != 0xF9){
            text += ENGLISH_CHARS[ch];
            continue;
        }
        // The next byte (in bits, aaoooooo) tells to copy some bytes from before this one: their
        // number is aa * 2 + 4, and they start oooooo + 1 bytes before the 0xF9 byte.
        const u8 reference = section_.GetU8(offset + 1);
        const int size = (reference >> 6) * 2 + 4;
        const int start = offset - 1 - (reference & 63);
        for (int c = 0; c < size; c ++) text += ENGLISH_CHARS[section_.GetU8(start + c)];
        offset ++;
    }
    boost::replace_all(text, "\"", "'");
    return text;
}
