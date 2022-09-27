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
#include "map/VGearsWalkmeshFile.h"

namespace VGears{

    namespace FF7{

        /**
         * Handles the serialization of walkmesh files.
         */
        class WalkmeshFileSerializer : public Serializer{

            public:

                /**
                 * Constructor.
                 */
                WalkmeshFileSerializer();

                /**
                 * Destructor.
                 */
                virtual ~WalkmeshFileSerializer();

                /**
                 * Imports a walkmesh file.
                 *
                 * @param stream[in] The contents of the walkmesh file.
                 * @param dest[out] The formed walkmesh file.
                 */
                virtual void ImportWalkmeshFile(
                  Ogre::DataStreamPtr &stream, WalkmeshFile *dest
                );

                typedef WalkmeshFile::Triangle WalkmeshTriangle;
                typedef WalkmeshFile::TriangleList WalkmeshTriangleList;

                /**
                 * @todo Understand and document.
                 */
                enum{

                    /**
                     * @todo Understand and document.
                     */
                    VERTEX_PADDING_COUNT = 1,

                    /**
                     * @todo Understand and document.
                     */
                    VERTEX_COMPONENT_COUNT = 3 + VERTEX_PADDING_COUNT,

                    /**
                     * @todo Understand and document.
                     */
                    ACCESS_COMPONENT_COUNT = 3
                };

                /**
                 * A walkmesh triangle.
                 */
                struct Triangle{

                    /**
                     * A side of the triangle.
                     */
                    Ogre::Vector3 a;

                    /**
                     * A side of the triangle.
                     */
                    Ogre::Vector3 b;

                    /**
                     * A side of the triangle.
                     */
                    Ogre::Vector3 c;
                };

                /**
                 * Defines the access to a triangle.
                 */
                struct Access{
                    /**
                     * @todo Understand and document.
                     */
                    sint16 a;
                    /**
                     * @todo Understand and document.
                     */
                    sint16 b;
                    /**
                     * @todo Understand and document.
                     */
                    sint16 c;
                };

                typedef std::vector<Triangle> TriangleList;

                typedef std::vector<Access> AccessList;

            protected:

                /**
                 * Reads an object as a walkmesh triangle.
                 *
                 * @param stream[in] Input data.
                 * @param dest[out] The formed triangle data.
                 */
                virtual void readObject(
                  Ogre::DataStreamPtr &stream, Triangle &dest
                );

                /**
                 * Reads an object as a 3-dimensional vector.
                 *
                 * @param stream[in] Input data.
                 * @param dest[out] The formed vector data.
                 */
                virtual void readObject(
                  Ogre::DataStreamPtr &stream, Ogre::Vector3 &dest
                );

                /**
                 * Reads an object as a triangle access point.
                 *
                 * @param stream[in] Input data.
                 * @param dest[out] The formed access point data.
                 */
                virtual void readObject(
                  Ogre::DataStreamPtr &stream, Access &dest
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
