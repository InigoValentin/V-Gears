/*
-----------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2013-08-24 Tobias Peters <tobias.peters@kreativeffekt.at>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include <boost/test/unit_test.hpp>

#include <Ogre.h>

#include "data/VGearsFLevelFileSerializer.h"

#include "data/VGearsBackgroundFileManager.h"
#include "data/VGearsCameraMatrixFileManager.h"
#include "data/VGearsHRCFileManager.h"
#include "data/VGearsPaletteFileManager.h"
#include "data/VGearsLZSFLevelFileManager.h"
#include "data/FF7ModelListFileManager.h"
#include "map/VGearsBackground2DFileManager.h"
#include "map/VGearsWalkmeshFileManager.h"

using namespace Ogre;

BOOST_AUTO_TEST_CASE( read_file )
{
    class TestFile : public VGears::FLevelFile
    {
    public:
        TestFile() : VGears::FLevelFile( NULL, "reference.flevel", 0, "General" ) {}
    };

    class TestTexture : public Texture
    {
    public:
        TestTexture( ResourceManager* creator, const String& name, ResourceHandle handle,
                  const String& group, bool isManual = false, ManualResourceLoader* loader = 0 )
            : Texture( creator, name, handle, group, isManual, loader )
        {}

    protected:
        virtual void loadImpl() {}
        virtual void unloadImpl() {}
        // IVV compiler error
        //virtual HardwarePixelBufferSharedPtr getBuffer( size_t, size_t ){ return HardwarePixelBufferSharedPtr( NULL ); }
        virtual void createInternalResourcesImpl(){}
        virtual void freeInternalResourcesImpl(){}
    };

    class TestTextureManager : public TextureManager
    {
    public:
        virtual PixelFormat getNativeFormat( TextureType ttype, PixelFormat format, int usage )
        {
            return format;
        }
        virtual bool    isHardwareFilteringSupported( TextureType ttype, PixelFormat format, int usage, bool preciseFormatOnly=false )
        {
            return false;
        }

    protected:
        virtual Resource* createImpl( const String &name
          , ResourceHandle handle, const String &group, bool isManual
          , ManualResourceLoader *loader
          , const NameValuePairList *createParams )
        {
            return new TestTexture( this, name, handle, group, isManual, loader );
        }
    };


    LogManager                          logMgr;
    Root                                root("","");
    ResourceGroupManager               &rgm( ResourceGroupManager::getSingleton() );
    TestTextureManager                  tmgr;
    VGears::CameraMatrixFileManager     cmgr;
    VGears::WalkmeshFileManager         wmgr;
    VGears::PaletteFileManager          pmgr;
    VGears::BackgroundFileManager       bmgr;
    VGears::Background2DFileManager     b2mgr;
    VGears::ModelListFileManager   mmgr;
    VGears::HRCFileManager              hmgr;
    VGears::LZSFLevelFileManager        fmgr;
    logMgr.createLog( "Default Log", true, true, true );

    rgm.addResourceLocation( "misc", "FileSystem" );
    rgm.initialiseAllResourceGroups();

    TestFile        file;
    DataStreamPtr   stream( rgm.openResource( file.getName(), file.getGroup() ) );
    BOOST_REQUIRE( stream->isReadable() );

    VGears::FLevelFileSerializer    ser;
    ser.importFLevelFile( stream, &file );
    BOOST_CHECK_EQUAL( 757890, stream->tell() );

    VGears::BackgroundFilePtr       background   ( file.getBackground() );
    VGears::CameraMatrixFilePtr     camera_matrix( file.getCameraMatrix() );
    VGears::ModelListFilePtr   model_list   ( file.getModelList() );
    VGears::PaletteFilePtr          palette      ( file.getPalette() );
    VGears::WalkmeshFilePtr         walkmesh     ( file.getWalkmesh() );

    BOOST_REQUIRE(background != nullptr);
    BOOST_REQUIRE(camera_matrix != nullptr);
    BOOST_REQUIRE(model_list != nullptr);
    BOOST_REQUIRE(palette != nullptr);
    BOOST_REQUIRE(walkmesh != nullptr);
    BOOST_CHECK_EQUAL( "reference.background", background->getName() );
    BOOST_CHECK_EQUAL( "reference.cam_matrix", camera_matrix->getName() );
    BOOST_CHECK_EQUAL( "reference.model_list", model_list->getName() );
    BOOST_CHECK_EQUAL( "reference.palette"   , palette->getName() );
    BOOST_CHECK_EQUAL( "reference.walkmesh"  , walkmesh->getName() );
    Matrix3 m3_expected(
        1, -0, -0
       ,0, -1, -0
       ,0, -0, -1
    );
    BOOST_CHECK_EQUAL( m3_expected, camera_matrix->getMatrix() );
    BOOST_CHECK_EQUAL( Vector3( -1, 2, -3 ), camera_matrix->getPosition() );
    BOOST_CHECK_EQUAL( 2, camera_matrix->getCount() );
    BOOST_CHECK_EQUAL( 12345, camera_matrix->getFocalLength() );


    Ogre::Image *image( background->createImage( palette ) );
    // TODO: FIX ME ON OSX
    //image->save( file.getName() + ".png" );
    delete image;

    VGears::FLevelFilePtr lzs_file = fmgr.load( "reference_compressed.flevel", "General" ).staticCast<VGears::FLevelFile>();
    BOOST_CHECK(lzs_file->getPalette() != nullptr);
    BOOST_CHECK(lzs_file->getBackground() != nullptr);

    image = lzs_file->getBackground()->createImage( lzs_file->getPalette() );
    // TODO: FIX ME ON OSX
    //image->save( lzs_file->getName() + ".png" );
    delete image;

    lzs_file.reset();
    logMgr.destroyLog( "Default Log" );
    stream->close();
}
