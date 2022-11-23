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

#include <OgreLogManager.h>
#include <OgreException.h>
//#include <OgreDataStream.h>
//#include <OgrePrerequisites.h>
#include <OgreGpuProgramManager.h>
#include "data/VGearsTexCodec.h"
#include "data/VGearsTexFile.h"

using Ogre::Codec;
using Ogre::DataStreamPtr;
using Ogre::Exception;
using Ogre::LogManager;
using Ogre::MemoryDataStreamPtr;
using Ogre::String;
using Ogre::StringUtil;

namespace VGears{

    String TexCodec::TYPE_NAME("tex");

    TexCodec::TexCodec(){}

    TexCodec::~TexCodec(){}

    String TexCodec::getType() const{return TYPE_NAME;}

    Ogre::DataStreamPtr TexCodec::encode(
      Ogre::MemoryDataStreamPtr& input, CodecDataPtr& output
    ) const{
        OGRE_EXCEPT(
          Exception::ERR_NOT_IMPLEMENTED,
          "Encoding not yet supported",
          "TexImageCodec::encode"
        );
    }

    void TexCodec::encodeToFile(
      Ogre::MemoryDataStreamPtr& input, const Ogre::String& output,
      CodecDataPtr& extra
    ) const{
        OGRE_EXCEPT(
          Exception::ERR_NOT_IMPLEMENTED,
          "Encoding not yet supported",
          "TexImageCodec::encodeToFile"
        );
    }

    Codec::DecodeResult TexCodec::decode(DataStreamPtr& input) const{
        TexFile tex_file;
        tex_file.Read(input);
        DecodeResult ret;
        ret.first = MemoryDataStreamPtr(tex_file.GetPixelData());
        ret.second = Codec::CodecDataPtr(tex_file.GetImageData());
        return ret;
    }

    String TexCodec::magicNumberToFileExt(
      const char *magic_number, size_t max_bytes
    ) const{
        LogManager::getSingleton().logMessage(
          "TexImageCodec::magicNumberToFileExt: No Magic Number"
         );
        return Ogre::BLANKSTRING;
    }

    TexCodec *TexCodec::tex_codec_(nullptr);

    void TexCodec::install(){
        if (tex_codec_ == nullptr) tex_codec_ = OGRE_NEW TexCodec();
    }

    void TexCodec::initialise(){
        LogManager::getSingleton().logMessage("Registering TEX codec");
        registerCodec(tex_codec_);
    }

    void TexCodec::shutdown(){if (tex_codec_) unregisterCodec(tex_codec_);}

    void TexCodec::uninstall(){
        if (tex_codec_ != nullptr){
            delete tex_codec_;
            tex_codec_ = nullptr;
        }
    }
}
