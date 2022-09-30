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

#include <boost/format.hpp>
#include "decompiler/world/WorldDisassembler.h"
#include "decompiler/world/WorldEngine.h"
#include "decompiler/world/instruction/WorldBinaryEqualStackInstruction.h"
#include "decompiler/world/instruction/WorldCondJumpInstruction.h"
#include "decompiler/world/instruction/WorldKernelCallInstruction.h"
#include "decompiler/world/instruction/WorldLoadBankInstruction.h"
#include "decompiler/world/instruction/WorldLoadInstruction.h"
#include "decompiler/world/instruction/WorldNoOutputInstruction.h"
#include "decompiler/world/instruction/WorldStackInstruction.h"
#include "decompiler/world/instruction/WorldStoreInstruction.h"
#include "decompiler/world/instruction/WorldSubStackInstruction.h"
#include "decompiler/world/instruction/WorldUncondJumpInstruction.h"

FF7::WorldDisassembler::WorldDisassembler(WorldEngine*, InstVec &insts, int script_number)
  : SimpleDisassembler(insts), script_number_(script_number){}

FF7::WorldDisassembler::~WorldDisassembler(){}

/*
00000202: reset stack (0)
00000203: push byte from bank 0 897 (2)
00000205: jump if false 25 (0)

00000207: reset stack (0)
00000208: push byte from bank 0 897 (2)
0000020a: push byte from bank 0 897 (2)
0000020c: push constant 1 (2)
0000020e: push sub (2)
0000020f: write bank (0)

00000210: reset stack (0)
00000211: push byte from bank 0 897 (2)
00000213: push constant 0 (2)
00000215: push equal (2)
00000216: jump if false 25 (0)

00000218: unknown 320 (0)
00000219: return (0)

if(!Read(897))
{
    Write(897-1)
    if (!(Read(897) == 0))
    {
        unknown320()
    }
}

*/
void FF7::WorldDisassembler::DoDisassemble(){
    struct WorldScriptData{
        uint16 type = 0;
        uint16 instruction_pointer = 0;
    };
    std::vector<WorldScriptData> world_data;
    world_data.reserve(256);
    for (int i = 0; i < 256; i ++){
        WorldScriptData tmp;
        tmp.type = this->stream_->ReadU16();
        tmp.instruction_pointer = this->stream_->ReadU16();
        world_data.emplace_back(tmp);
    }
    int i = this->script_number_;
    for (int i = 1; i < 2; i ++){ //for (int i = 1; i < 2; i ++){
        WorldScriptData data = world_data[i];
        this->stream_->Seek((256 * sizeof(int)) + (data.instruction_pointer*sizeof(uint16)));
        this->address_ = this->stream_->GetPosition();
        bool end = false;
        for (;;){ //    while (this->_f.pos() != (int)this->_f.size())
            uint32 full_opcode = 0;
            uint16 opcode = this->stream_->ReadU16();
            std::string opcode_prefix;
            if (end) break;
            if (opcode == 0x203) end = true;
            if (opcode >= 0x204 && opcode < 0x300)
                ParseOpcode(opcode, "reset stack", new WorldKernelCallInstruction(), 0, "");
            else{
                switch (opcode){
                    case 0x100:
                        ParseOpcode(
                          opcode, "reset stack", new WorldNoOutputInstruction(), 0, ""
                        );
                        break;
                    case 0x201:
                        ParseOpcode(
                          opcode, "jump if false", new WorldCondJumpInstruction(), 0, "w"
                        );
                        break;
                    case 0x203:
                        ParseOpcode(opcode, "return", new WorldKernelCallInstruction(), 0, "");
                        break;
                    case 0x200:
                        ParseOpcode(opcode, "jump", new WorldUncondJumpInstruction(), 0, "w");
                        break;

                    case 0x118:
                        ParseOpcode(
                          opcode, "push byte from bank 0", new WorldLoadBankInstruction(), 2, "w"
                        );
                        break;
                    case 0x110:
                        ParseOpcode(opcode, "push constant", new WorldLoadInstruction(), 2, "w");
                        break;
                    case 0x119:
                        ParseOpcode(
                          opcode, "push byte from bank 1", new WorldLoadBankInstruction(), 2, "w"
                        );
                        break;
                    case 0x11c:
                        ParseOpcode(
                          opcode, "push word from bank 0", new WorldLoadBankInstruction(), 2, "w"
                        );
                        break;
                    case 0x114:
                        ParseOpcode(
                          opcode, "push bit from bank 0", new WorldLoadBankInstruction(), 2, "w"
                        );
                        break;
                    case 0x11b:
                        ParseOpcode(
                          opcode, "push special", new WorldLoadBankInstruction(), 2, "w"
                        );
                        break;
                    case 0x11f:
                        ParseOpcode(opcode, "push special", new WorldLoadInstruction(), 2, "w");
                        break;
                    case 0x117:
                        ParseOpcode(
                          opcode, "push special", new WorldLoadBankInstruction(), 2, "w"
                        );
                        break;

                    case 0x0e0:
                        ParseOpcode(opcode, "write bank", new WorldStoreInstruction(), 0, "");
                        break;
                    case 0x019:
                        ParseOpcode(
                          opcode, "push distance from active entity to entity by model id",
                          new WorldKernelCallInstruction(), 2, ""
                        );
                        break;
                    case 0x018:
                        ParseOpcode(
                          opcode, "push distance from active entity to light",
                          new WorldKernelCallInstruction(), 2, ""
                        );
                        break;

                    case 0x061:
                        ParseOpcode(opcode, "push greater", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x0b0:
                        ParseOpcode(opcode, "push logicand", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x062:
                        ParseOpcode(opcode, "push lessequal", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x030:
                        ParseOpcode(opcode, "push mul", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x051:
                        ParseOpcode(opcode, "push shr", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x080:
                        ParseOpcode(opcode, "push and", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x0a0:
                        ParseOpcode(opcode, "push or", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x070:
                        ParseOpcode(opcode, "push equal", new WorldBinaryEqualStackInstruction(), 2, "");
                        break;
                    case 0x041:
                        ParseOpcode(opcode, "push sub", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x063:
                        ParseOpcode(
                          opcode, "push greaterequal", new WorldSubStackInstruction(), 2, ""
                        );
                        break;
                    case 0x017:
                        ParseOpcode(opcode, "push logicnot", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x060:
                        ParseOpcode(opcode, "push less", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x0c0:
                        ParseOpcode(opcode, "push logicor", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x040:
                        ParseOpcode(opcode, "push add", new WorldSubStackInstruction(), 2, "");
                        break;
                    case 0x015:
                        ParseOpcode(opcode, "push neg", new WorldSubStackInstruction(), 2, "");
                        break;

                    case 0x320:
                        ParseOpcode(
                          opcode, "unknown 320", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x34a:
                        ParseOpcode(
                          opcode, "unknown 34a", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x334:
                        ParseOpcode(
                          opcode, "unknown 334", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x33a:
                        ParseOpcode(
                          opcode, "unknown 33a", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x31c:
                        ParseOpcode(
                          opcode, "unknown 31c", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x32f:
                        ParseOpcode(
                          opcode, "unknown 32f", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x339:
                        ParseOpcode(
                          opcode, "unknown 339", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x30d:
                        ParseOpcode(
                          opcode, "unknown 30d", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x354:
                        ParseOpcode(
                          opcode, "unknown 354", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x332:
                        ParseOpcode(
                          opcode, "unknown 332", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x31f:
                        ParseOpcode(
                          opcode, "unknown 31f", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x329:
                        ParseOpcode(
                          opcode, "unknown 329", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x32a:
                        ParseOpcode(
                          opcode, "unknown 32a", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x34d:
                        ParseOpcode(
                          opcode, "unknown 3d4", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x34e:
                        ParseOpcode(
                          opcode, "unknown 3de", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x30a:
                        ParseOpcode(
                          opcode, "unknown 30a", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x333:
                        ParseOpcode(
                          opcode, "unknown 333", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x331:
                        ParseOpcode(
                          opcode, "unknown 331", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x319:
                        ParseOpcode(
                          opcode, "unknown 319", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x353:
                        ParseOpcode(
                          opcode, "unknown 353", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x33e:
                        ParseOpcode(
                          opcode, "unknown 33e", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x321:
                        ParseOpcode(
                          opcode, "unknown 321", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;

                    case 0x31d:
                        ParseOpcode(
                          opcode, "play sound effect", new WorldKernelCallInstruction(), 2, "w"
                        );
                        break;
                    case 0x336:
                        ParseOpcode(
                          opcode, "set active entity movespeed (honor walkmesh)",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x328:
                        ParseOpcode(
                          opcode, "set active entity direction",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x33d:
                        ParseOpcode(
                          opcode, "set field entry point2?",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x351:
                        ParseOpcode(
                          opcode, "set music volume", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x352:
                        ParseOpcode(
                          opcode, "shake camera on / off",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x34f:
                        ParseOpcode(
                          opcode, "set active entity y position",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x30b:
                        ParseOpcode(
                          opcode, "set active entity y offset",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x347:
                        ParseOpcode(
                          opcode, "move active entity to entity by model id ?",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x30e:
                        ParseOpcode(
                          opcode, "active entity play animation",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x305:
                        ParseOpcode(
                          opcode, "set wait frames", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x306:
                        ParseOpcode(opcode, "wait?", new WorldKernelCallInstruction(), 0, "");
                        break;
                    case 0x307:
                        ParseOpcode(
                          opcode, "set control lock", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x303:
                        ParseOpcode(
                          opcode, "set active entity movespeed",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x32c:
                        ParseOpcode(
                          opcode, "set window parameters",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x32d:
                        ParseOpcode(
                          opcode, "wait for window ready",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x324:
                        ParseOpcode(
                          opcode, "set window dimensions",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x325:
                        ParseOpcode(
                          opcode, "set window message", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x32e:
                        ParseOpcode(
                          opcode, "wait for message acknowledge",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x32b:
                        ParseOpcode(
                          opcode, "set battle lock", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x326:
                        ParseOpcode(
                          opcode, "set window prompt", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x327:
                        ParseOpcode(
                          opcode, "wait for prompt acknowledge ?",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x318:
                        ParseOpcode(
                          opcode, "enter field scene", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x33b:
                        ParseOpcode(
                          opcode, "fade out?", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x348:
                        ParseOpcode(
                          opcode, "fade in ?", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x330:
                        ParseOpcode(
                          opcode, "set active entity", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x350:
                        ParseOpcode(
                          opcode, "set meteor texture on/off",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x34b:
                        ParseOpcode(
                          opcode, "set chocobo type", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x34c:
                        ParseOpcode(
                          opcode, "set submarine color", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x349:
                        ParseOpcode(
                          opcode, "set world progress", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x300:
                        ParseOpcode(
                          opcode, "load model", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x302:
                        ParseOpcode(
                          opcode, "set player entity", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x30c:
                        ParseOpcode(
                          opcode, "enter vehicle?", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x308:
                        ParseOpcode(
                          opcode, "set active entity mesh coordinates",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x309:
                        ParseOpcode(
                          opcode, "set active entity coordinates in mesh",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x304:
                        ParseOpcode(
                          opcode, "set active entity direction & facing",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x310:
                        ParseOpcode(
                          opcode, "set active point", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x311:
                        ParseOpcode(
                          opcode, "set point mesh coordinates",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x312:
                        ParseOpcode(
                          opcode, "set point coordinates in mesh",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x313:
                        ParseOpcode(
                          opcode, "set point terrain BGR",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x314:
                        ParseOpcode(
                          opcode, "set point dropoff parameters",
                          new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x315:
                        ParseOpcode(
                          opcode, "set point sky BGR", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x316:
                        ParseOpcode(
                          opcode, "set point BGR3?", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    case 0x317:
                        ParseOpcode(
                          opcode, "trigger battle", new WorldKernelCallInstruction(), 0, ""
                        );
                        break;
                    default:
                        throw UnknownOpcodeException(this->address_, opcode);
                }
            }
            this->address_ += 2;
        }
        for (auto& instruction : this->insts_) instruction->_address = instruction->_address / 2;
    }
}

