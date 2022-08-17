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


#include <OgreException.h>
#include "data/QGearsLZSDataStream.h"

namespace QGears{

    LZSDataStream::LZSDataStream(const Ogre::DataStreamPtr &compressed_stream) :
      Ogre::DataStream(compressed_stream->getAccessMode()),
      compressed_stream_(compressed_stream),
      available_compressed_(0),
      position_(0)
    {init();}

    LZSDataStream::LZSDataStream(
      const String &name, const Ogre::DataStreamPtr &compressed_stream
    ) :
      Ogre::DataStream(name, compressed_stream->getAccessMode()),
      compressed_stream_(compressed_stream),
      available_compressed_(0),
      position_(0)
    {init();}

    LZSDataStream::~LZSDataStream(){close();}

    void LZSDataStream::init(){
        if (compressed_stream_->read(&available_compressed_, 4) < 4){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "Can't read LZS Header from stream",
              "LZSDataStream::init"
            );
        }
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        FlipEndian(available_compressed_);
#endif
        mSize = available_compressed_; // so the base size() works
    }

    void LZSDataStream::FlipEndian(uint32 &inout_data){
        inout_data
          = ((inout_data & 0x000000FF) << 24)
            | ((inout_data & 0x0000FF00) <<  8)
            | ((inout_data & 0x00FF0000) >>  8)
            | ((inout_data & 0xFF000000) >> 24);
    }

    size_t LZSDataStream::read(void *buf, size_t count){
        assert(buf && "NullPointer");
        size_t  read_total(0);
        while (count && !eof()){
            if (!buffer_.Available()) DecompressChunk();
            size_t read(buffer_.Read(
              static_cast<uint8*>(buf) + read_total, count
            ));
            count -= read;
            read_total += read;
        }
        position_ += read_total;
        return read_total;
    }

    void LZSDataStream::DecompressChunk(){
        if (compressed_stream_->eof()){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "not enough data in Stream to read control byte",
              "LZSDataStream::DecompressChunk"
            );
        }
        uint8 data;
        uint8 control_byte;
        uint8 ref[2];
        uint16 refoffset_;
        available_compressed_ -= compressed_stream_->read(
          &control_byte, sizeof(control_byte)
        );

        for (
          uint8 control_bit(8);
            control_bit --
              && !compressed_stream_->eof()
              && available_compressed_;
            control_byte >>= 1
        ){
            if (control_byte & 1){
                available_compressed_ -= compressed_stream_->read(
                  &data, sizeof(data)
                );
                buffer_.Write(&data);
            }
            else if (available_compressed_ < 2){
                OGRE_EXCEPT(
                  Ogre::Exception::ERR_INVALIDPARAMS,
                  "not enough data in Stream to read reference bytes",
                  "LZSDataStream::DecompressChunk"
                );
            }
            else{
                available_compressed_ -= compressed_stream_->read(
                  &ref, sizeof(ref)
                );
                refoffset_ = 18 + (((ref[1] & 0xF0) << 4) | ref[0]);
                for (uint8 ref_length((ref[1] & 0xF) + 3); ref_length --;){
                    data = buffer_.Get(refoffset_++);
                    buffer_.Write(&data);
                }
            }
        }
    }

    void LZSDataStream::skip(long count){
        OGRE_EXCEPT(
          Ogre::Exception::ERR_NOT_IMPLEMENTED,
          "not yet implemented",
          "LZSDataStream::skip"
        );
    }

    void LZSDataStream::seek(size_t pos){
        OGRE_EXCEPT(
          Ogre::Exception::ERR_NOT_IMPLEMENTED,
          "not yet implemented",
          "LZSDataStream::seek"
        );
    }

    bool LZSDataStream::eof() const{
        return !available_compressed_ && !buffer_.Available();
    }

    size_t LZSDataStream::tell() const{return position_;}

    void LZSDataStream::close(){compressed_stream_.reset();}

}
