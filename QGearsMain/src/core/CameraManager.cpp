#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OIS.h>

#include "core/CameraManager.h"
#include "core/CameraManagerCommands.h"
#include "common/QGearsApplication.h"
#include "core/ConfigVar.h"
#include "core/InputManager.h"
#include "core/Logger.h"


ConfigVar cv_cam_speed("camera_speed", "Camera speed", "0.02");
template<>CameraManager* Ogre::Singleton<CameraManager>::msSingleton = nullptr;

/**
 * Camera manager constructor.
 *
 * Initializes the camera with default parameters attached to the root scene
 * manager.
 */
CameraManager::CameraManager():
  m_CameraFree(false),
  m_CameraFreeRotate(false),
  m_2DPosition(Ogre::Vector3::ZERO),
  m_2DOrientation(Ogre::Quaternion::ZERO),
  m_2DFOV(0),
  m_2DScroll(Ogre::Vector2::ZERO)
{
    LOG_TRIVIAL("CameraManager started.");
    InitCommands();
    m_Camera = Ogre::Root::getSingleton().getSceneManager("Scene")->createCamera("Camera");
    m_Camera->setNearClipDistance(0.001f);
    m_Camera->setFarClipDistance(1000.0f);
    //m_Camera->setPosition(Ogre::Vector3(0, 0, 0));
    Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->setPosition(Ogre::Vector3(0, 0, 0));
    //m_Camera->lookAt(Ogre::Vector3(0, 0, 0));
    Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->lookAt(Ogre::Vector3(0, 0, 0), Ogre::Node::TS_LOCAL);
    m_Viewport = QGears::Application::getSingleton().getRenderWindow()->addViewport(m_Camera, 0);
    m_Viewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
    m_Camera->setAspectRatio(Ogre::Real(m_Viewport->getActualWidth()) / Ogre::Real(m_Viewport->getActualHeight()));
}

/**
 * Camera Manager destructor.
 */
CameraManager::~CameraManager(){
    LOG_TRIVIAL("CameraManager finished.");
}

/**
 * Handles cammera actions.
 *
 * Handles the scene manager camera actions based on events.
 *
 * @param event[in] Event that triggers the caera action.
 * @param timeSinceLastFrame[in] For speed calculation.
 */
void CameraManager::Input(const QGears::Event& event, Ogre::Real timeSinceLastFrame){
    if(m_CameraFree == true){
        float speed = cv_cam_speed.GetF() * timeSinceLastFrame;
        if(
          InputManager::getSingleton().IsButtonPressed(OIS::KC_RSHIFT)
          || InputManager::getSingleton().IsButtonPressed(OIS::KC_LSHIFT)
        ){
            speed *= 4;
        }

        if (event.type == QGears::ET_KEY_IMPULSE && event.param1 == OIS::KC_W){
            //m_Camera->moveRelative(Ogre::Vector3(0, 0, -speed));
            Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->translate(Ogre::Vector3(0, 0, -speed), Ogre::Node::TS_LOCAL);
        }
        else if (event.type == QGears::ET_KEY_IMPULSE && event.param1 == OIS::KC_A){
            //m_Camera->moveRelative(Ogre::Vector3(-speed, 0, 0));
            Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->translate(Ogre::Vector3(-speed, 0, 0), Ogre::Node::TS_LOCAL);
        }
        else if (event.type == QGears::ET_KEY_IMPULSE && event.param1 == OIS::KC_S){
            //m_Camera->moveRelative(Ogre::Vector3(0, 0, speed));
            Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->translate(Ogre::Vector3(0, 0, speed), Ogre::Node::TS_LOCAL);
        }
        else if (event.type == QGears::ET_KEY_IMPULSE && event.param1 == OIS::KC_D){
            //m_Camera->moveRelative(Ogre::Vector3(speed, 0, 0));
            Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->translate(Ogre::Vector3(speed, 0, 0), Ogre::Node::TS_LOCAL);
        }
        else if (event.type == QGears::ET_MOUSE_PRESS && event.param1 == OIS::MB_Right){
            m_CameraFreeRotate = true;
        }
        else if (event.type == QGears::ET_MOUSE_RELEASE && event.param1 == OIS::MB_Right){
            m_CameraFreeRotate = false;
        }
        else if (event.type == QGears::ET_MOUSE_MOVE && m_CameraFreeRotate == true){
            //m_Camera->rotate(Ogre::Vector3::UNIT_Z, Ogre::Radian(Ogre::Degree(-event.param1 * 0.13f)));
            Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->rotate(Ogre::Vector3::UNIT_Z, Ogre::Radian(Ogre::Degree(-event.param1 * 0.13f)));
            //m_Camera->pitch(Ogre::Degree(-event.param2 * 0.13f));
            Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->pitch(Ogre::Degree(-event.param2 * 0.13f));
        }
    }
}

/**
 * Triggered when updated.
 *
 * Unused.
 */
void CameraManager::Update(){}

/**
 * Trigered when the viewport is resized.
 *
 * Resets the aspect ratio.
 */
void CameraManager::OnResize(){
    m_Camera->setAspectRatio(
      Ogre::Real(m_Viewport->getActualWidth()) / Ogre::Real(m_Viewport->getActualHeight())
    );
}

/**
 * Enables or disables the free camera.
 *
 * A free camera moves depending on player input, not according to a script.
 * For example, a camera following the PC is a free camera.
 *
 * @param enable[in] True to enable free camera, false to disable it.
 */
void CameraManager::SetCameraFree(const bool enable){
    m_CameraFree = enable;
    if(m_CameraFree == true){
        m_Camera->resetFrustumExtents();
        CameraManager::getSingleton().GetCurrentCamera()->setFOVy(Ogre::Radian(Ogre::Math::PI / 4.0f));
    }
    else{
        // Set2DScroll sets position and orientation
        Set2DScroll(Get2DScroll());
        // TODO
        //CameraManager::getSingleton().GetCurrentCamera()->setPosition(m_2DPosition);
        //Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->setPosition(m_2DPosition);
        //CameraManager::getSingleton().GetCurrentCamera()->setOrientation(m_2DOrientation);
        //Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->setOrientation(m_2DOrientation);
        //CameraManager::getSingleton().GetCurrentCamera()->setFOVy(m_2DFOV);
        //Set2DScroll(Get2DScroll());
    }
}

/**
 * Sets the camera properties.
 *
 * @param position[in] Camera coordinates.
 * @param orientation[in] Camera rotation, in quaternion format.
 * @param fov Field of view radians.
 */
void CameraManager::Set2DCamera(const Ogre::Vector3 position, const Ogre::Quaternion orientation, const Ogre::Radian fov){
    m_2DPosition = position;
    m_2DOrientation = orientation;
    m_2DFOV = fov;
    // TODO
    CameraManager::getSingleton().GetCurrentCamera()->setPosition(m_2DPosition);
    //Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->setPosition(m_2DPosition);
    CameraManager::getSingleton().GetCurrentCamera()->setOrientation(m_2DOrientation);
    //Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()->setOrientation(m_2DOrientation);
    CameraManager::getSingleton().GetCurrentCamera()->setFOVy(m_2DFOV);
    Set2DScroll(Ogre::Vector2::ZERO);
}

/**
 * Sets the camera scroll.
 *
 * Moves the camera to the desired position. If the camera is in free mode, it
 * sets the position, but it won't actually move the camera.
 *
 * @param position[in] Position to scroll the camera to.
 */
void CameraManager::Set2DScroll(const Ogre::Vector2& position){

    m_2DScroll = position;
    if(m_CameraFree == true) return;
    float width = static_cast<float>(m_Viewport->getActualWidth());
    float height = static_cast<float>(m_Viewport->getActualHeight());
    float left, right, top, bottom;
    m_Camera->resetFrustumExtents();
    Ogre::RealRect frustrumRect = m_Camera->getFrustumExtents();
    //m_Camera->getFrustumExtents(left, right, top, bottom);
    left = frustrumRect.left;
    right = frustrumRect.right;
    top = frustrumRect.top;
    bottom = frustrumRect.bottom;
    float move_x = ((right - left) / width) * position.x;
    float move_y = ((bottom - top) / height) * -position.y;
    m_Camera->setFrustumExtents(left - move_x, right - move_x, top + move_y, bottom + move_y);
}

/**
 * Retireves the camera position.
 *
 * @return The camera current position, relative to the absolute origin.
 */
const Ogre::Vector2& CameraManager::Get2DScroll() const{
    return m_2DScroll;
}

/**
 * @TODO Document this properly.
 */
const Ogre::Vector3 CameraManager::ProjectPointToScreen(const Ogre::Vector3& point){
    Ogre::Vector3 view = m_Camera->getViewMatrix() * point;
    float z = view.z;
    view = m_Camera->getProjectionMatrix() * view;
    view.z = z;
    float width = static_cast<float>(m_Viewport->getActualWidth() / 2);
    float height = static_cast<float>(m_Viewport->getActualHeight() / 2);
    view.x = width + view.x * width;
    view.y = height - view.y * height;
    return view;
}

/**
 * Retrieves the camera.
 *
 * @return The current camera.
 */
Ogre::Camera* CameraManager::GetCurrentCamera() {
    return m_Camera;
}

/**
 * Retrieves the viewport.
 *
 * @return The camera viewport.
 */
Ogre::Viewport* CameraManager::getViewport(){
    return m_Viewport;
}

/**
 * Enables or disables the camera wireframe.
 *
 * @TODO Description of what this actually does
 *
 * @param enable @TODO
 */
void CameraManager::EnableWireFrame(bool enable){
    if (m_Camera){
        //m_Camera->setPolygonMode(Ogre::PM_WIREFRAME);
        //m_Camera->rotate( Ogre::Vector3::UNIT_Z, Ogre::Radian( Ogre::Degree( 180 ) ) );
        m_Camera->setFarClipDistance(900000000);
    }
}
