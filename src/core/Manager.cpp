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
#include <OgreRoot.h>
#include "Manager.h"
#include "Logger.h"


Manager::Manager(): module_(Module::FIELD), prev_module_(Module::FIELD), paused_(false){}

Manager::~Manager(){}

Manager::Module Manager::GetModule(){return module_;}

bool Manager::IsModule(Manager::Module module){return (module_ == module);}

bool Manager::IsFieldModule(){return (module_ == Module::FIELD);}

bool Manager::IsBattleModule(){return (module_ == Module::BATTLE);}

bool Manager::IsWorldModule(){return (module_ == Module::WORLD);}

void Manager::SetModule(Manager::Module module){
    if (module == Module::FIELD){
        ClearBattle();
        ClearWorld();
    }
    else if (module == Module::WORLD){
        ClearBattle();
        ClearField();
    }
    else if (module == Module::BATTLE){
        prev_module_ = module_;
    }
    module_ = module;
}

void Manager::SetFieldModule(){SetModule(Module::FIELD);}

void Manager::SetBattleModule(){SetModule(Module::BATTLE);}

void Manager::SetWorldModule(){SetModule(Module::WORLD);}

void Manager::SetPreviousModule(){
    if (module_ != Module::BATTLE)
        LOG_WARNING(
          "Called Manager::SetPreviousModule without battle module active. Doing nothing"
          + "Current module is " + std::to_string(module_)
        );
    else if(module_ = prev_module_)
        LOG_WARNING(
          "Called Manager::SetPreviousModule but there is no previous module. Doing nothing"
          + "Current module is " + std::to_string(module_)
        );
    else module_ = prev_module_;
    return;
}

void Manager::Update(){
    UpdateDebug();
    if (paused_ == true) return;
    Update(module_);
}


void Manager::Update(Module module){
    switch (module){
        case Module::FIELD:
            UpdateField();
            break;
        case Module::BATTLE:
            UpdateBattle();
            break;
        case Module::WORLD:
            UpdateWorld();
            break;
    }
}

void Manager::Clear(){Clear(module_);}

void Manager::Clear(Module module){
    switch (module){
        case Module::FIELD:
            ClearField();
            break;
        case Module::BATTLE:
            ClearBattle();
            break;
        case Module::WORLD:
            ClearWorld();
            break;
    }
}

void Manager::ClearAll(){
    ClearField();
    ClearBattle();
    ClearWorld();
}

void Manager::ScriptSetPaused(const bool paused){paused_ = paused;}
