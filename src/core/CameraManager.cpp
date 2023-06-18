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

#include <iostream>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OIS/OIS.h>
#include "core/CameraManager.h"
#include "core/CameraManagerCommands.h"
#include "common/VGearsApplication.h"
#include "core/ConfigVar.h"
#include "core/InputManager.h"
#include "core/Logger.h"

ConfigVar cv_cam_speed("camera_speed", "Camera speed", "0.02");

/**
 * Camera manager singleton.
 */
template<>CameraManager* Ogre::Singleton<CameraManager>::msSingleton = nullptr;

CameraManager::CameraManager():
  battle_(false),
  camera_free_(false),
  camera_free_rotate_(false),
  d2_position_(Ogre::Vector3::ZERO),
  d2_orientation_(Ogre::Quaternion::ZERO),
  d2_fov_(0),
  d2_scroll_(Ogre::Vector2::ZERO)
{
    LOG_TRIVIAL("CameraManager started.");
    InitCommands();
    camera_ = Ogre::Root::getSingleton().getSceneManager("Scene")
      ->createCamera("Camera");
    position_initial_
      = Ogre::Vector3(camera_->getPosition().x, camera_->getPosition().y, camera_->getPosition().z);
    orientation_initial_ = Ogre::Quaternion(
      camera_->getOrientation().w, camera_->getOrientation().x,
      camera_->getOrientation().z, camera_->getOrientation().z
    );
    camera_->setNearClipDistance(0.001f);
    camera_->setFarClipDistance(1000.0f);
    //camera_->setFixedYawAxis(true, Ogre::Vector3::UNIT_Y);
    //camera_->setPosition(Ogre::Vector3(0, 0, 0));
    Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()
      ->setPosition(Ogre::Vector3(0, 0, 0));
    //camera_->lookAt(Ogre::Vector3(0, 0, 0));
    Ogre::Root::getSingleton().getSceneManager("Scene")->getRootSceneNode()
      ->lookAt(Ogre::Vector3(0, 0, 0), Ogre::Node::TS_LOCAL);
    viewport_ = VGears::Application::getSingleton().getRenderWindow()
      ->addViewport(camera_, 0);
    viewport_->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
    camera_->setAspectRatio(
      Ogre::Real(viewport_->getActualWidth())
      / Ogre::Real(viewport_->getActualHeight())
    );
}

CameraManager::~CameraManager(){LOG_TRIVIAL("CameraManager finished.");}

void CameraManager::Input(
  const VGears::Event& event, Ogre::Real time_since_last_frame
){
    if (camera_free_ == true){
        float speed = cv_cam_speed.GetF() * time_since_last_frame;
        if(
          InputManager::getSingleton().IsButtonPressed(OIS::KC_RSHIFT)
          || InputManager::getSingleton().IsButtonPressed(OIS::KC_LSHIFT)
        ){
            speed *= 4;
        }

        Ogre::SceneNode* rootScene =
          Ogre::Root::getSingleton().getSceneManager("Scene")
            ->getRootSceneNode();

        if (event.type == VGears::ET_KEY_IMPULSE && event.param1 == OIS::KC_W){
            //camera_->moveRelative(Ogre::Vector3(0, 0, -speed));
            rootScene->translate(
              Ogre::Vector3(0, 0, -speed), Ogre::Node::TS_LOCAL
            );
        }
        else if (
          event.type == VGears::ET_KEY_IMPULSE && event.param1 == OIS::KC_A
        ){
            //camera_->moveRelative(Ogre::Vector3(-speed, 0, 0));
            rootScene->translate(
              Ogre::Vector3(-speed, 0, 0), Ogre::Node::TS_LOCAL
            );
        }
        else if (
          event.type == VGears::ET_KEY_IMPULSE && event.param1 == OIS::KC_S
        ){
            //camera_->moveRelative(Ogre::Vector3(0, 0, speed));
            rootScene->translate(
              Ogre::Vector3(0, 0, speed), Ogre::Node::TS_LOCAL
            );
        }
        else if (
          event.type == VGears::ET_KEY_IMPULSE && event.param1 == OIS::KC_D
        ){
            //camera_->moveRelative(Ogre::Vector3(speed, 0, 0));
            rootScene->translate(
              Ogre::Vector3(speed, 0, 0), Ogre::Node::TS_LOCAL
            );
        }
        else if (
          event.type == VGears::ET_MOUSE_PRESS && event.param1 == OIS::MB_Right
        ){
            camera_free_rotate_ = true;
        }
        else if (
          event.type == VGears::ET_MOUSE_RELEASE
          && event.param1 == OIS::MB_Right
        ){
            camera_free_rotate_ = false;
        }
        else if (
          event.type == VGears::ET_MOUSE_MOVE && camera_free_rotate_ == true
        ){
            //camera_->rotate(
            //  Ogre::Vector3::UNIT_Z,
            //  Ogre::Radian(Ogre::Degree(-event.param1 * 0.13f))
            //);
            rootScene->rotate(
              Ogre::Vector3::UNIT_Z,
              Ogre::Radian(Ogre::Degree(-event.param1 * 0.13f))
            );
            //camera_->pitch(Ogre::Degree(-event.param2 * 0.13f));
            rootScene->pitch(Ogre::Degree(-event.param2 * 0.13f));
        }
    }
}

void CameraManager::Update(){}

void CameraManager::OnResize(){
    camera_->setAspectRatio(
      Ogre::Real(viewport_->getActualWidth())
      / Ogre::Real(viewport_->getActualHeight())
    );
}

void CameraManager::SetCameraFree(const bool enable){
    camera_free_ = enable;
    if (camera_free_ == true){
        camera_->resetFrustumExtents();
        CameraManager::getSingleton().GetCurrentCamera()->setFOVy(
          Ogre::Radian(Ogre::Math::PI / 4.0f)
        );
    }
    else{
        // Set2DScroll sets position and orientation
        Set2DScroll(Get2DScroll());
        // TODO
        //CameraManager::getSingleton().GetCurrentCamera()->setPosition(
        // d2_position_
        //);
        //Ogre::Root::getSingleton().getSceneManager("Scene")
        //  ->getRootSceneNode()->setPosition(d2_position_);
        //CameraManager::getSingleton().GetCurrentCamera()->setOrientation(
        // d2_orientation_
        //);
        //Ogre::Root::getSingleton().getSceneManager("Scene")
        //  ->getRootSceneNode()->setOrientation(d2_orientation_);
        //CameraManager::getSingleton().GetCurrentCamera()->setFOVy(d2_fov_);
        //Set2DScroll(Get2DScroll());
    }
}


void CameraManager::Set2DCamera(
  const Ogre::Vector3 position,
  const Ogre::Quaternion orientation, const Ogre::Radian fov
){
    d2_position_ = position;
    d2_orientation_ = orientation;
    d2_fov_ = fov;
    // TODO
    CameraManager::getSingleton().GetCurrentCamera()->setPosition(d2_position_);
    //Ogre::Root::getSingleton().getSceneManager("Scene")
    //  ->getRootSceneNode()->setPosition(d2_position_);
    CameraManager::getSingleton().GetCurrentCamera()->setOrientation(
      d2_orientation_
    );
    //Ogre::Root::getSingleton().getSceneManager("Scene")
    //  ->getRootSceneNode()->setOrientation(d2_orientation_);
    CameraManager::getSingleton().GetCurrentCamera()->setFOVy(d2_fov_);
    Set2DScroll(Ogre::Vector2::ZERO);
}

void CameraManager::StartBattleCamera(
  const Ogre::Vector3 position, const Ogre::Vector3 orientation
){
    if (battle_){
        LOG_ERROR("Tried to start battle camera but the CameraManager is already in battle mode");
        return;
    }
    battle_ = true;
    std::cout << "CAMERA BATTLE START: " << camera_->getPosition().x << ", " << camera_->getPosition().y << ", " << camera_->getPosition().z << ", "
        << camera_->getOrientation().w << ", "<< camera_->getOrientation().x << ", " << camera_->getOrientation().y << ", "
        << camera_->getOrientation().z << std::endl;
    position_backup_ = Ogre::Vector3(
      camera_->getPosition().x, camera_->getPosition().y,
      camera_->getPosition().z
    );
    orientation_backup_ = Ogre::Quaternion(
      camera_->getOrientation().w, camera_->getOrientation().x,
      camera_->getOrientation().y, camera_->getOrientation().z
    );
    camera_->setPosition(position);
    //CameraManager::getSingleton().GetCurrentCamera()->setFixedYawAxis(true, Ogre::Vector3::UNIT_Y);
    camera_->lookAt(Ogre::Vector3(orientation.x, orientation.y, camera_->getPosition().z));
    camera_->lookAt(orientation);


    std::cout << "CAMERA BATTLE START FINISH: " << camera_->getPosition().x << ", " << camera_->getPosition().y << ", " << camera_->getPosition().z << ", "
            << camera_->getOrientation().w << ", "<< camera_->getOrientation().x << ", " << camera_->getOrientation().y << ", "
            << camera_->getOrientation().z << std::endl;
}

void CameraManager::EndBattleCamera(){
    if (!battle_){
        LOG_ERROR("Tried to end battle camera but the CameraManager is not in battle mode");
        return;
    }
    battle_ = false;
        std::cout << "CAMERA BATTLE END: " << camera_->getPosition().x << ", " << camera_->getPosition().y << ", " << camera_->getPosition().z << ", "
            << camera_->getOrientation().w << ", "<< camera_->getOrientation().x << ", " << camera_->getOrientation().y << ", "
            << camera_->getOrientation().z << std::endl;
    CameraManager::getSingleton().GetCurrentCamera()->setPosition(
      Ogre::Vector3(position_backup_.x, position_backup_.y, position_backup_.z)
    );
    CameraManager::getSingleton().GetCurrentCamera()->setOrientation(Ogre::Quaternion(
      orientation_backup_.w, orientation_backup_.x, orientation_backup_.y, orientation_backup_.z
    ));
    std::cout << "CAMERA BATTLE END FINISH: " << camera_->getPosition().x << ", " << camera_->getPosition().y << ", " << camera_->getPosition().z << ", "
                << camera_->getOrientation().w << ", "<< camera_->getOrientation().x << ", " << camera_->getOrientation().y << ", "
                << camera_->getOrientation().z << std::endl;
}

void CameraManager::Set2DScroll(const Ogre::Vector2& position){
    d2_scroll_ = position;
    if(camera_free_ == true) return;
    float width = static_cast<float>(viewport_->getActualWidth());
    float height = static_cast<float>(viewport_->getActualHeight());
    float left, right, top, bottom;
    camera_->resetFrustumExtents();
    Ogre::RealRect frustrumRect = camera_->getFrustumExtents();
    //camera_->getFrustumExtents(left, right, top, bottom);
    left = frustrumRect.left;
    right = frustrumRect.right;
    top = frustrumRect.top;
    bottom = frustrumRect.bottom;
    float move_x = ((right - left) / width) * position.x;
    float move_y = ((bottom - top) / height) * -position.y;
    camera_->setFrustumExtents(
      left - move_x, right - move_x, top + move_y, bottom + move_y
    );
}

const Ogre::Vector2& CameraManager::Get2DScroll() const{return d2_scroll_;}

const Ogre::Vector3 CameraManager::ProjectPointToScreen(
  const Ogre::Vector3& point
){
    Ogre::Vector3 view = camera_->getViewMatrix() * point;
    float z = view.z;
    view = camera_->getProjectionMatrix() * view;
    view.z = z;
    float width = static_cast<float>(viewport_->getActualWidth() / 2);
    float height = static_cast<float>(viewport_->getActualHeight() / 2);
    view.x = width + view.x * width;
    view.y = height - view.y * height;
    return view;
}

Ogre::Camera* CameraManager::GetCurrentCamera(){return camera_;}

Ogre::Viewport* CameraManager::getViewport(){return viewport_;}

void CameraManager::EnableWireFrame(bool enable){
    if (camera_){
        //camera_->setPolygonMode(Ogre::PM_WIREFRAME);
        //camera_->rotate(
        //  Ogre::Vector3::UNIT_Z, Ogre::Radian(Ogre::Degree(180))
        //);
        camera_->setFarClipDistance(900000000);
    }
}

void CameraManager::ScriptSetCamera(
  const int x, const int y, const int z, const int d_x, const int d_y, const int d_z
){
    Ogre::Vector3 position = Ogre::Vector3(x, y, z);
    camera_->setPosition(position);
    //Ogre::Quaternion orientation = position.getRotationTo(Ogre::Vector3(d_x, d_y, d_z));
    //camera_->setOrientation(Ogre::Quaternion(orientation_initial_.w, orientation_initial_.x, orientation_initial_.y, orientation_initial_.z));
    //CameraManager::getSingleton().GetCurrentCamera()->setFixedYawAxis(false);
    //camera_->setOrientation(orientation);
    //camera_->roll(orientation.getRoll());
    //camera_->yaw(orientation.getYaw());
    //camera_->pitch(orientation.getPitch());
    camera_->lookAt(Ogre::Vector3(d_x, d_y, camera_->getPosition().z));
    camera_->lookAt(Ogre::Vector3(d_x, d_y, d_z));
}
