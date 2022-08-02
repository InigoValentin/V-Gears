#include "data/worldmap/MapFile.h"
#include "data/worldmap/MapFileSerializer.h"

namespace QGears {

/*static*/ const String WorldMapFile::RESOURCE_TYPE( "QGearsWorldMap7File" );

WorldMapFile::~WorldMapFile()
{

}

void WorldMapFile::loadImpl()
{
    MapFileSerializer serializer;
    Ogre::DataStreamPtr stream( openResource() );
    serializer.importMapFile( stream, *this );
}

void WorldMapFile::unloadImpl()
{

}

} // namespace QGears
