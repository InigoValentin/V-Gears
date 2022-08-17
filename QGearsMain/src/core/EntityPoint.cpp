#include "core/ConfigVar.h"
#include "core/DebugDraw.h"
#include "core/EntityPoint.h"
#include "core/ScriptManager.h"


ConfigVar cv_debug_point("debug_point", "Draw point debug info", "false");


EntityPoint::EntityPoint(const Ogre::String& name):
    name_(name),
    position_(Ogre::Vector3::ZERO),
    rotation_(0)
{
}


EntityPoint::~EntityPoint()
{
}


void
EntityPoint::UpdateDebug()
{
    if(cv_debug_point.GetB() == true)
    {
        DEBUG_DRAW.SetColour(Ogre::ColourValue::White);
        Ogre::Vector3 point1 = position_;
        point1.z += 0.5f;
        Ogre::Vector3 point2 = position_;
        point2.z -= 0.5f;
        DEBUG_DRAW.Line3d(point1, point2);
        DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.CENTER);
        DEBUG_DRAW.SetFadeDistance(30, 40);
        DEBUG_DRAW.Text(position_, 0, 0, name_);
    }
}


const Ogre::String&
EntityPoint::GetName() const
{
    return name_;
}


void
EntityPoint::SetPosition(const Ogre::Vector3& point)
{
    position_ = point;

}


const Ogre::Vector3&
EntityPoint::GetPosition() const
{
    return position_;
}


void
EntityPoint::ScriptGetPosition() const
{
    ScriptManager::getSingleton().AddValueToStack(position_.x);
    ScriptManager::getSingleton().AddValueToStack(position_.y);
    ScriptManager::getSingleton().AddValueToStack(position_.z);
}


void
EntityPoint::SetRotation(const float rotation)
{
    rotation_ = rotation;
}


float
EntityPoint::GetRotation() const
{
    return rotation_;
}


float
EntityPoint::ScriptGetRotation() const
{
    return rotation_;
}
