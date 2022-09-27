/*
-----------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2013-08-11 Tobias Peters <tobias.peters@kreativeffekt.at>

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
#include <cstddef>

#define BOOST_TEST_MODULE VGearsPFile
#include <boost/test/unit_test.hpp>

#include "data/VGearsPFile.h"

BOOST_AUTO_TEST_CASE( header_size )
{
    BOOST_CHECK_EQUAL( 0x18, sizeof( VGears::PFile::PolygonDefinition ) );
    BOOST_CHECK_EQUAL( 0x38, sizeof( VGears::PFile::Group ) );
}

BOOST_AUTO_TEST_CASE( polygon_offset )
{
    BOOST_CHECK_EQUAL( 0x02, offsetof( VGears::PFile::PolygonDefinition, vertex ) );
    BOOST_CHECK_EQUAL( 0x08, offsetof( VGears::PFile::PolygonDefinition, normal ) );
    BOOST_CHECK_EQUAL( 0x0E, offsetof( VGears::PFile::PolygonDefinition, edge ) );
}

BOOST_AUTO_TEST_CASE( group_offset )
{
    BOOST_CHECK_EQUAL( 0x00, offsetof( VGears::PFile::Group, primitive_type ) );
    BOOST_CHECK_EQUAL( 0x04, offsetof( VGears::PFile::Group, polygon_start_index ) );
    BOOST_CHECK_EQUAL( 0x08, offsetof( VGears::PFile::Group, num_polygons ) );
    BOOST_CHECK_EQUAL( 0x0C, offsetof( VGears::PFile::Group, vertex_start_index ) );
    BOOST_CHECK_EQUAL( 0x10, offsetof( VGears::PFile::Group, num_vertices ) );
    BOOST_CHECK_EQUAL( 0x14, offsetof( VGears::PFile::Group, edge_start_index ) );
    BOOST_CHECK_EQUAL( 0x18, offsetof( VGears::PFile::Group, num_edges ) );
    BOOST_CHECK_EQUAL( 0x2C, offsetof( VGears::PFile::Group, texture_coordinate_start_index ) );
    BOOST_CHECK_EQUAL( 0x30, offsetof( VGears::PFile::Group, has_texture ) );
    BOOST_CHECK_EQUAL( 0x34, offsetof( VGears::PFile::Group, texture_index ) );
}
