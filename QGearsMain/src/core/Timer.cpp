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

#include "core/ConfigVar.h"
#include "core/Timer.h"

ConfigVar cv_timer_scale_game(
  "timer_scale_game", "Timer speed for game related things", "1"
);

template<>Timer *Ogre::Singleton<Timer>::msSingleton = nullptr;


Timer::Timer():
  system_time_total_(0),
  system_time_delta_(0),
  game_time_total_(0),
  game_time_delta_( 0 ),
  game_timer_( 0 )
{}

float Timer::GetSystemTimeTotal(){return system_time_total_;}

float Timer::GetSystemTimeDelta(){return system_time_delta_;}

float Timer::GetGameTimeTotal(){return game_time_total_;}

float Timer::GetGameTimeDelta(){return game_time_delta_;}

void Timer::AddTime( const float time ){
    system_time_delta_ = time;
    system_time_total_ += system_time_delta_;
    game_time_delta_ = time * cv_timer_scale_game.GetF();
    game_time_total_ += game_time_delta_;
    if (game_timer_ > 0){
        game_timer_ -= time;
        if (game_timer_ < 0) game_timer_ = 0;
    }
}

void Timer::SetGameTimer(const float timer){game_timer_ = timer;}

int Timer::GetGameTimer() const{return (int) game_timer_;}
