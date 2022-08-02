#ifndef _WORLD_MAP_MODULE_H_
#define _WORLD_MAP_MODULE_H_

#include <OgreSingleton.h>
#include "common/TypeDefine.h"

namespace QGears {

class WorldMapModule : public Ogre::Singleton< WorldMapModule >
{
public:
    void Init();
};

} // namespace QGears

#endif // _WORLD_MAP_MODULE_H_
