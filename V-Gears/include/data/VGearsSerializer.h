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

#include <OgreAxisAlignedBox.h>
#include <OgreColourValue.h>
#include <OgreDataStream.h>
#include <OgreSerializer.h>
#include <Ogre.h>
#include "common/TypeDefine.h"

namespace VGears{

    /**
     * Handles file serialization.
     */
    class Serializer : public Ogre::Serializer{

        public:

            /**
             * Constructor.
             *
             * Determines the endian mode of the file.
             */
            Serializer();

            /**
             * Destructor
             */
            virtual ~Serializer();

            // TODO implement some more serialization!?
            // ISerializeable  readObject(serializer)
            // virtual void readObject(ISerializable o) { o.readObject(this); }
            // template<T> readObject(T)

        protected:

            /**
             * Reads an object as a 2 dimensional vector.
             *
             * @param[in] stream Input data.
             * @param[out] dest The formed vector data.
             */
            void readObject(Ogre::DataStreamPtr &stream, Ogre::Vector2 &dest);

            /**
             * Reads an object as a 3 dimensional vector.
             *
             * @param[in] stream Input data.
             * @param[out] dest The formed vector data.
             */
            void readObject(Ogre::DataStreamPtr &stream, Ogre::Vector3 &dest);

            /**
             * Reads an object as an axis aligned box.
             *
             * Can be used for bounding boxes.
             *
             * @param[in] stream Input data.
             * @param[out] dest The formed box data.
             */
            void readObject(
              Ogre::DataStreamPtr &stream, Ogre::AxisAlignedBox &dest
            );

            /**
             * Reads an object as a pixel.
             *
             * @param[in] stream Input data.
             * @param[out] dest The formed pixel data.
             */
            void readObject(Ogre::DataStreamPtr &stream, Pixel &dest);

            /**
             * Reads a stream as text.
             *
             * @param[in] stream Input data.
             * @param[out] dest The read characters will be stored here.
             * @param[in] count Read this many characters.
             */
            void ReadChars(
              Ogre::DataStreamPtr& stream, char* dest, size_t count
            );

            /**
             * Reads one byte from a stream and evaluates it as a boolean.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated boolean.
             * @todo 1 is true?
             */
            void Read1ByteBool(Ogre::DataStreamPtr &stream, bool &dest);

            /**
             * Reads two bytes from a stream and evaluates them as a boolean.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated boolean.
             * @todo 1 is true?
             */
            void Read2ByteBool(Ogre::DataStreamPtr &stream, bool &dest);

            /**
             * Reads 16 bytes from a stream and evaluates as an unsigned short.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadShort(Ogre::DataStreamPtr &stream, uint16 &dest);

            /**
             * Reads 16 bytes from a stream and evaluates as a signed integer.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadInt16(Ogre::DataStream &stream, sint16 &dest);

            /**
             * Reads 16 bytes from a stream and evaluates as an unsigned int.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadUInt16(Ogre::DataStream &stream, uint16 &dest);

            /**
             * Reads 16 bytes from a stream and evaluates as a signed integer.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadInt16(Ogre::DataStreamPtr &stream, sint16 &dest);

            /**
             * Reads 16 bytes from a stream and evaluates as an unsigned int.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadUInt16(Ogre::DataStreamPtr &stream, uint16 &dest);

            /**
             * Reads 32 bytes from a stream and evaluates as an unsigned int.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadUInt32(Ogre::DataStreamPtr &stream, uint32 &dest);

            /**
             * Reads 32 bytes from a stream and evaluates as a signed int.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadSInt32(Ogre::DataStreamPtr &stream, sint32 &dest);

            /**
             * Reads 32 bytes from a stream and evaluates as an unsigned int.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadUInt32(Ogre::DataStream &stream, uint32 &dest);

            /**
             * Reads 8 bytes from a stream and evaluates as an unsigned int.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadUInt8(Ogre::DataStream& stream, uint8 &dest);

            /**
             * Reads 8 bytes from a stream and evaluates as an unsigned int.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadUInt8(Ogre::DataStreamPtr &stream, uint8 &dest);

            /**
             * Reads bytes from a stream and evaluates them as a float.
             *
             * @param[in] stream Input data.
             * @param[out] dest The evaluated number.
             */
            void ReadFloat(Ogre::DataStreamPtr &stream, float &dest);

            /**
             * @todo Understand and document.
             */
            void ReadEndString(
              Ogre::DataStreamPtr &stream, const String &end_text
            );

            /**
             * @todo Understand and document.
             */
            String GetLine(Ogre::DataStreamPtr &stream) const;

            /**
             * A comment tag.
             */
            static const String TAG_COMMENT;

    };
}

