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

#include <boost/algorithm/string.hpp>
#include "installer/support/Utility.h"
#include "installer/tables/Characters.h"

namespace VGears{

    Utility::Utility(int argc, char *argv[]) :
      Application(argc, argv), frame_listener_(NULL), scene_manager_(NULL), camera_(NULL), viewport_(NULL)
    {}

    Utility::~Utility(){}

    std::string Utility::DecodeString(const u8* bytes, const unsigned int size){
        std::string decoded("");
        for (int i = 0; i < size; i ++) if (bytes[i] > 0x00 && bytes[i] < 0XFF) decoded += ENGLISH_CHARS[bytes[i]];
        boost::algorithm::trim(decoded);
        return decoded;
    }

}
