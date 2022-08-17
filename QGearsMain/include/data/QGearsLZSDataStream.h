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

#include <OgreDataStream.h>
#include "common/TypeDefine.h"

namespace QGears{

    /**
     * A ring buffer used by LZS streams.
     */
    template<size_t buffer_size> class RingBuffer{

        public:

            /**
             * Constructor.
             */
            RingBuffer(){clear();}

            /**
             * Writes to the buffer from other buffer.
             *
             * @param in_buffer[in] The buffer to write from.
             */
            void Write(const void *in_buffer_){
                buffer_[position_] = *(static_cast<const uint8*>(in_buffer_));
                ++ position_ %= buffer_size;
                ++ available_ = std::min(available_, buffer_size);
            }

            /**
             * Reads data from the buffer and extracts it into other buffer.
             *
             * @param out_buffer[out] Buffer to copy the data from this buffer.
             * @param count[in] Number of bytes to read.
             */
            size_t Read(void *out_buffer, size_t count){
                assert(available_ && "Can't read if no data is available");
                size_t read(0);
                uint8 *buffer(static_cast<uint8*>(out_buffer));
                while (read < count && available_){
                    *(buffer ++)
                      = buffer_[(position_ - available_) % buffer_size];
                    -- available_;
                    ++ read;
                }
                return read;
            }

            /**
             * Gets data from the buffer at an offset.
             *
             * @param offset[in] Offset to the data to read.
             * @return The data at the offset.
             * @todo Explain the size of the returned data and offset.
             */
            uint8 Get(const size_t offset){
                return buffer_[offset % buffer_size];
            }

            /**
             * Checks the available data size in the buffer.
             *
             * @return How much data is left to read in the buffer.
             */
            size_t Available() const{return available_;}

            void clear(){
                position_ = 0;
                available_ = 0;
                memset(buffer_, 0, buffer_size);
            }

        protected:

            /**
             * The buffer.
             */
            uint8 buffer_[buffer_size];

            /**
             * Current buffer position.
             */
            size_t position_;

            /**
             * Available data to read.
             */
            size_t available_;

    };

    /**
     * Handles LZS compressed data streams.
     */
    class LZSDataStream : public Ogre::DataStream{

        public:

            /**
             * Constructor.
             *
             * @param compressed_stream[in] The compressed stream.
             */
            LZSDataStream(const Ogre::DataStreamPtr &compressed_stream);

            /**
             * Constructor.
             *
             * @param name[in] NAme for th stream.
             * @param compressed_stream[in] The compressed stream.
             */
            LZSDataStream(
              const String &name, const Ogre::DataStreamPtr &compressed_stream
            );

            /**
             * Destructor.
             */
            virtual ~LZSDataStream();

            /**
             * Reads from the current position of stream into a buffer.
             *
             * It also advances the stream cursor.
             *
             * @param buf[out] Buffer to read to.
             * @param count[in] Number of data to read. If the end of the
             * stream is reached before reading this much, it will stop
             * reading.
             */
            virtual size_t read(void *buf, size_t count) override;

            /**
             * Indicates if the stream has reached the end.
             *
             * @return True if the stream is ended, false otherwise.
             */
            virtual bool eof() const override;

            /**
             * Unimplemented.
             *
             * Not required, can use read instead.
             *
             * @param count[in] Unused.
             */
            virtual void skip(long count) override;

            /**
             * Unimplemented.
             *
             * Not required.
             *
             * @param count[in] Unused.
             */
            virtual void seek(size_t pos) override;

            /**
             * Retrieves the current stream position.
             *
             * @return The current stream position.
             */
            virtual size_t tell() const override;

            /**
             * Closes the stream.
             */
            virtual void close() override;

            /**
             * @todo Understand and document.
             *
             * @return @todo.
             */
            size_t AvailableCompressed() const {
                return available_compressed_;
            }

            /**
             * @todo Understand and document.
             *
             * @return @todo.
             */
            size_t AvailableUncompressed() const{
                return buffer_.Available();
            }

            /**
             * Flips the endian mode of some data.
             *
             * Little endian data will be transformed into big endian, and big
             * endian data will be transformed into little endian.
             *
             * @param inout_data[in|out] The data to flip.
             */
            void FlipEndian(uint32 &inout_data);

        protected:

            /**
             * Initializes the stream and sets instance data.
             */
            virtual void init();

            /**
             * Decompresses the stream.
             *
             * @todo The whole stream?
             */
            virtual void DecompressChunk();

            Ogre::DataStreamPtr compressed_stream_;
            uint32              available_compressed_;
            size_t              position_;
            RingBuffer<4096>    buffer_;
    };

    typedef Ogre::SharedPtr<LZSDataStream>  LZSDataStreamPtr;
}
