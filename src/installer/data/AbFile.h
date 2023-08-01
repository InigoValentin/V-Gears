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

#pragma once

#include <vector>
#include "common/File.h"
#include "common/TypeDefine.h"

/**
 * Represents an animation scripts file.
 *
 * *ab files are bundled into battle.lgp. They contain animation scripts. More info about them in
 * {@see https://forums.qhimm.com/index.php?topic=14204.0}
 */
class AbFile{

    public:

        /**
         * Constructor.
         *
         * Reads the file.
         *
         * @param[in, out] file The file to read from. The file contents will not be altered, but
         * it's offset will be changed while reading it.
         * @param[in] enemy Indicates if the animation is for an enemy. Enemies have a different
         * number of animations than characters or backgrounds.
         */
        AbFile(File file, bool enemy);

        /**
         * Generates script files from the ba file.
         *
         * A file will be generated with every script in the *ab file. The file will be created in
         * the specified path, and will be named like the model id, followed by the ".lua"
         * extension. For example, if path is "/home/user/.v-gears", the model id is "rt", the file
         * name will be: "/home/user/.v-gears/rt.lua". Each script will be named "script" + index
         * (script1(), script2() ... script44() ...)
         *
         * @param[in] model_id ID of the model the animation belongs to, usually a two letter
         * code. Used to generate the file name.
         * @param[in] path Path to the directory where the files will be saved.
         * @return The number of scripts written to the file.
         */
        unsigned int GenerateScripts(std::string model_id, std::string path) const;

    private:

        /**
         * Reads the Ab file and extracts all the data.
         */
        void Read();

        /**
         * Decompiles a script in the *ab file.
         *
         * It won't nodify the file current offset.
         *
         * @param[in] offset Offset at which the script starts.
         * @return The script text.
         */
        std::string DecompileScript(u32 offset);

        /**
         * The ab file.
         */
        File ab_file_;

        /**
         * Indicates if the model is for an enemy.
         */
        bool is_enemy_;

        /**
         * List of animations.
         */
        std::vector<std::string> scripts_;


};
