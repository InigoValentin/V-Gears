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

#include "common/VGearsStringUtil.h"

typedef std::map<std::string, std::set<std::string>> ModelAnimationMap;

/**
 * Database of model animations.
 */
class ModelsAndAnimationsDb{

    public:

        /**
         * Normalizes an animation name.
         *
         * Generates a normalized file name, without path, lowercase, and an
         * '.a' extension.
         *
         * @param name[in] Current name.
         * @return The normalized name.
         */
        std::string NormalizeAnimationName(const std::string& name);

        /**
         * @todo Understand and document.
         *
         * @param model[in] @todo Understand and document.
         * @return @todo Understand and document.
         */
        std::set<std::string>& ModelAnimations(const std::string model);

        /**
         * Retrieves the metadata file name associated to a model.
         *
         * @param model_name[in] The model name.
         * @return Name of the metadata file.
         */
        std::string ModelMetaDataName(const std::string& model_name);

        /**
         * Map of models and animations.
         */
        ModelAnimationMap map;
};
