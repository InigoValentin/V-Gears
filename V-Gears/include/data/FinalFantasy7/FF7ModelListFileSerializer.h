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

#include "common/TypeDefine.h"
#include "data/VGearsSerializer.h"

#include "FF7ModelListFile.h"

namespace VGears{

    namespace FF7{

        /**
         * Handles the serialization of model list files.
         */
        class ModelListFileSerializer : public Serializer{

            public:

                /**
                 * Constructor.
                 */
                ModelListFileSerializer();

                /**
                 * Destructor.
                 */
                virtual ~ModelListFileSerializer();

                /**
                 * Imports a model list file.
                 *
                 * @param stream[in] The contents of the model list file.
                 * @param dest[out] The formed model list file.
                 */
                virtual void ImportModelListFile(
                  Ogre::DataStreamPtr &stream, ModelListFile *dest
                );

                typedef ModelListFile::AnimationDescription
                  AnimationDescription;

                typedef ModelListFile::ModelDescription ModelDescription;

                /**
                 * Data sizes in the file.
                 */
                enum {

                    /**
                     * Length of the name of the HRC file assigned to a model.
                     */
                    HRC_NAME_LENGTH = 8,

                    /**
                     * Scale data length.
                     */
                    SCALE_LENGTH = 4
                };

            protected:

                /**
                 * Reads an object as an animation description.
                 *
                 * @param stream[in] Input data.
                 * @param dest[out] The formed animation description data.
                 */
                virtual void readObject(
                  Ogre::DataStreamPtr &stream, AnimationDescription &dest
                );

                /**
                 * Reads an object as a model description.
                 *
                 * @param stream[in] Input data.
                 * @param dest[out] The formed model description data.
                 */
                virtual void readObject(
                  Ogre::DataStreamPtr &stream, ModelDescription &dest
                );

                using Serializer::readObject;

                /**
                 * Reads an object as a vector.
                 *
                 * @param stream[in] Input data.
                 * @param dest[out] The formed vector data.
                 * @param count[in] The size of the data to read.
                 */
                template<typename ValueType> void ReadVector(
                  Ogre::DataStreamPtr &stream, std::vector<ValueType> &dest,
                  size_t count
                );

        };
    }
}
