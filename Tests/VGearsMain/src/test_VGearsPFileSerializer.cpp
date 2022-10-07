/*
-----------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2013-08-10 Tobias Peters <tobias.peters@kreativeffekt.at>

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
#include <fstream>

#define BOOST_TEST_MODULE VGearsPFileSerializer
#include <boost/test/unit_test.hpp>
#include <Ogre.h>

#include "data/VGearsPFileSerializer.h"

BOOST_AUTO_TEST_CASE( header_size )
{
    BOOST_CHECK_EQUAL( 0x80, sizeof( VGears::PFileSerializer::Header ) );
}

BOOST_AUTO_TEST_CASE( header_offset )
{
    BOOST_CHECK_EQUAL( 0x00, offsetof( VGears::PFileSerializer::Header, version                 ) );
    BOOST_CHECK_EQUAL( 0x08, offsetof( VGears::PFileSerializer::Header, vertex_type             ) );
    BOOST_CHECK_EQUAL( 0x0C, offsetof( VGears::PFileSerializer::Header, num_vertices            ) );
    BOOST_CHECK_EQUAL( 0x10, offsetof( VGears::PFileSerializer::Header, num_normals             ) );
    BOOST_CHECK_EQUAL( 0x14, offsetof( VGears::PFileSerializer::Header, num_unknown1            ) );
    BOOST_CHECK_EQUAL( 0x18, offsetof( VGears::PFileSerializer::Header, num_texture_coordinates ) );
    BOOST_CHECK_EQUAL( 0x1C, offsetof( VGears::PFileSerializer::Header, num_vertex_colors       ) );
    BOOST_CHECK_EQUAL( 0x20, offsetof( VGears::PFileSerializer::Header, num_edges               ) );
    BOOST_CHECK_EQUAL( 0x24, offsetof( VGears::PFileSerializer::Header, num_polygons            ) );
    BOOST_CHECK_EQUAL( 0x30, offsetof( VGears::PFileSerializer::Header, num_materials           ) );
    BOOST_CHECK_EQUAL( 0x34, offsetof( VGears::PFileSerializer::Header, num_groups              ) );
    BOOST_CHECK_EQUAL( 0x38, offsetof( VGears::PFileSerializer::Header, num_bboxes              ) );
    BOOST_CHECK_EQUAL( 0x3C, offsetof( VGears::PFileSerializer::Header, norm_index_table_flag   ) );
}

BOOST_AUTO_TEST_CASE( read_file )
{
    class TestPFile : public VGears::PFile
    {
    public:
        TestPFile() : VGears::PFile( NULL, "", 0, "" ) {}
        size_t getCalculatedSize() const { return calculateSize(); }
    };

    const char* file_name( "reference.p" );
    std::ifstream *ifs(  OGRE_NEW_T( std::ifstream, Ogre::MEMCATEGORY_GENERAL )( file_name, std::ifstream::binary ) );
    BOOST_REQUIRE( ifs->is_open() );
    Ogre::DataStreamPtr stream( OGRE_NEW Ogre::FileStreamDataStream( ifs ) );
    BOOST_REQUIRE( stream->isReadable() );

	Ogre::LogManager        logMgr;
    VGears::PFileSerializer ser;
    TestPFile               file;

    logMgr.createLog( "Default Log", true, true, true );

    ser.importPFile( stream, &file );

    logMgr.destroyLog( "Default Log" );

    BOOST_CHECK_EQUAL( 119, file.getVertices().size() );
    BOOST_CHECK_EQUAL(  90, file.getNormals().size() );
    BOOST_CHECK_EQUAL(  14, file.getTextureCoordinates().size() );
    BOOST_CHECK_EQUAL( 119, file.getVertexColors().size() );
    BOOST_CHECK_EQUAL( 291, file.getEdges().size() );
    BOOST_CHECK_EQUAL( 180, file.getPolygonDefinitions().size() );
    BOOST_CHECK_EQUAL( 180, file.getPolygonColors().size() );
    BOOST_CHECK_EQUAL(   4, file.getGroups().size() );
    BOOST_CHECK_EQUAL(   1, file.getBBoxes().size() );

    ifs->close();
}
