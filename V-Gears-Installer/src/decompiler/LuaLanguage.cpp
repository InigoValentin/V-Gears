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

#include "decompiler/LuaLanguage.h"
#include <sstream>
#include <boost/format.hpp>

const std::string LuaLanguage::LoopBreak(){return "break";}

const std::string LuaLanguage::LoopContinue(){
    // LUA has no continue keyword
    return "-- TODO continue not supported in LUA!";
}

std::string LuaLanguage::Goto(uint32 target){
    std::stringstream s;
    s << boost::format("goto label_0x%X") % target;
    return s.str();
}

const std::string LuaLanguage::DoLoopHeader(){return "repeat";}

std::string LuaLanguage::DoLoopFooter(bool before_expr){
    if (before_expr) return "until (";
    return ")";
}

std::string LuaLanguage::If(bool before_expr){
    if (before_expr) return "if (";
    return ") then";
}

std::string LuaLanguage::WhileHeader(bool before_expr){
    if (before_expr) return "while (";
    return ") do";
}

const std::string LuaLanguage::FunctionCallArgumentSeperator(){return ",";}

const std::string LuaLanguage::FunctionCallBegin(){return "(";}

const std::string LuaLanguage::FunctionCallEnd(){return ")";}

std::string LuaLanguage::Label(uint32 addr){
    std::stringstream s;
    s << boost::format("::label_0x%X::") % addr;
    return s.str();
}

const std::string LuaLanguage::Else(){return "else";}

std::string LuaLanguage::StartBlock(CONTEXT context){return "";}

std::string LuaLanguage::EndBlock(CONTEXT context){
    // For the final else, an end it's not needed before it
    if (context == TO_ELSE_BLOCK) return "";
    return "end";
}

const std::string LuaLanguage::LineTerminator(){return "";}
