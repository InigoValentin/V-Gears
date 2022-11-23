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

#include "common/VGearsStringUtil.h"

namespace VGears{

    void StringUtil::splitPath(const String &name, String &path){
        String base, ext;
        splitFullFilename(name, base, ext, path);
    }

    void StringUtil::splitBase(const String &name, String &basename){
        String path, ext;
        splitFullFilename(name, basename, ext, path);
    }

    void StringUtil::splitFull(const String &name, String &basename){
        String ext;
        splitBaseFilename(name, basename, ext);
    }

}
