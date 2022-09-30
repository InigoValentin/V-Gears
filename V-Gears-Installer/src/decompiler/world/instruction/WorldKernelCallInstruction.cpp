
/*
 * V-Gears
 * Copyright (C) 2022 V-Gears Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include "decompiler/world/instruction/WorldKernelCallInstruction.h"
#include "decompiler/world/WorldEngine.h"
#include "decompiler/world/WorldCodeGenerator.h"
#include "decompiler/world/WorldDisassembler.h"

void FF7::WorldKernelCallInstruction::ProcessInst(
    Function& function, ValueStack &stack, Engine *engine, CodeGenerator *code_gen
){
    std::string func;
    switch (_opcode){
        case 0x203: func = "return;"; break;
        case 0x317: func = "TriggerBattle(" + stack.pop()->getString() + ");"; break;
        case 0x324:
            // x, y, w, h
            func = "SetWindowDimensions("
              + stack.pop()->getString() + ", " + stack.pop()->getString() + ", "
              + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x32D: func = "WaitForWindowReady();"; break;
        case 0x325: func = "SetWindowMessage(" + stack.pop()->getString() + ");"; break;
        case 0x333:
            func = "Unknown333("
              + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x308:
            func = "SetActiveEntityMeshCoordinates("
              + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x309:
            func = "SetActiveEntityMeshCoordinatesInMesh("
              + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x32e: func = "WaitForMessageAcknowledge();"; break;
        case 0x32c:
            // Mode, Permanency.
            func = "SetWindowParameters("
              + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x318:
            func = "EnterFieldScene("
              + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x348:
            func = "FadeIn(" + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x33b:
            func = "FadeOut(" + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x310:
            func = "SetActivePoint("
              + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x311:
            func = "SetLightMeshCoordinates("
              + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x312:
            func = "SetLightMeshCoordinatesInMesh("
              + stack.pop()->getString() + ", " + stack.pop()->getString() + ");";
            break;
        case 0x31D: func = "PlaySoundEffect(" + stack.pop()->getString() + ");"; break;
        case 0x328: func = "SetActiveEntityDirection(" + stack.pop()->getString() + ");"; break;
        case 0x336: // Honor walk mesh.
        case 0x303:
            func = "SetActiveEntityMovespeed(" + stack.pop()->getString() + ");";
            break;
        case 0x304:
            func = "SetActiveEntityDirectionAndFacing(" + stack.pop()->getString() + ");";
            break;
        case 0x32b: func = "SetBattleLock(" + stack.pop()->getString() + ");"; break;
        case 0x305: func = "SetWaitFrames(" + stack.pop()->getString() + ");"; break;
        case 0x33e: func = "Unknown_AKAO(" + stack.pop()->getString() + ");"; break;
        case 0x306: func = "Wait();"; break;
        case 0x350: func = "SetMeteorTexture(" + stack.pop()->getString() + ");"; break;
        case 0x34b:
            {
                std::string type = stack.pop()->getString();
                switch (std::stoi(type)){
                    case 0: type = "yellow"; break;
                    case 1: type = "green"; break;
                    case 2: type = "blue"; break;
                    case 3: type = "black"; break;
                    case 4: type = "gold"; break;
                }
                func = "SetChocoboType(" + type + ");";
            }
            break;
        case 0x34c:
            {
                std::string type = stack.pop()->getString();
                switch (std::stoi(type)){
                    case 0: type = "red"; break;
                    case 1: type = "blue"; break;
                    // HACK - not really supported, but works.
                    case -1: type = "gold"; break;
                    case -2: type = "black"; break;
                }
                func = "SetSubmarineColor(" + type + ");";
            }
            break;
        case 0x349:
            {
                std::string type = stack.pop()->getString();
                std::string comment = "// ";
                switch (std::stoi(type)){
                    case 0: comment += "before temple of the ancients,"; break;
                    case 1: comment += "after temple of the ancients"; break;
                    case 2: comment += " after ultimate weapon appears,"; break;
                    case 3: comment += "after mideel"; break;
                    case 4: comment += "after ultimate weapon killed"; break;
                    default: comment += "??"; break;
                }
                func = "SetWorldProgress(" + type + "); "  +comment;
            }
            break;
        case 0x300:
            {
                std::string type = stack.pop()->getString();
                std::string comment = "// ";
                try{
                    switch (std::stoi(type)){
                        case 0: comment += "Cloud"; break;
                        case 1: comment += "Tifa?"; break;
                        case 2: comment += "Cid?"; break;
                        case 3: comment += "Ultimate Weapon"; break;
                        case 4: comment += "Unknown"; break;
                        case 5: comment += "Unknown"; break;
                        case 6: comment += "Unknown"; break;
                        case 7: comment += "Unknown"; break;
                        case 8: comment += "Unknown"; break;
                        case 9: comment += "Unknown"; break;
                        case 10: comment += "Unknown"; break;
                        case 11: comment += "Highwind"; break;
                        case 12: comment += "Unknown"; break;
                        case 13: comment += "Submarine"; break;
                        case 14: comment += "Unknown"; break;
                        case 15: comment += "Unknown"; break;
                        case 16: comment += "Unknown"; break;
                        case 17: comment += "Unknown"; break;
                        case 18: comment += "Unknown"; break;
                        case 19: comment += "Chocobo"; break;
                        case 20: comment += "Unknown"; break;
                        case 21: comment += "Unknown"; break;
                        case 22: comment += "Unknown"; break;
                        case 23: comment += "Unknown"; break;
                        case 24: comment += "Unknown"; break;
                        case 25: comment += "Unknown"; break;
                        default: comment += "??"; break;
                    }
                }
                catch (std::invalid_argument&){}
                func = "LoadModel(" + type + "); " + comment;
            }
            break;
        case 0x307: func = "SetControlLock(" + stack.pop()->getString() + ");"; break;
        case 0x30c: func = "EnterVehicle();"; break;
        default: func = "kernel_unknown_" + AddressValue(_opcode).getString() + "();"; break;
    }
    code_gen->AddOutputLine(func);
}
