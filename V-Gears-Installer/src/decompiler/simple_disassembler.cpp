/* ScummVM Tools
 *
 * ScummVM Tools is the legal property of its developers, whose
 * names are too numerous to list here. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "decompiler/simple_disassembler.h"

SimpleDisassembler::SimpleDisassembler(InstVec &insts) 
    : Disassembler(insts) 
{

}

static inline unsigned int Nib2(unsigned int v)
{
    return (v & 0xF);
}

static inline unsigned int Nib1(unsigned int v)
{
    return (v >> 4) & 0xF;
}

// [8s]
// [8u]
// [16s]
// [16u]
// [32s]
// [32u]
// [4,4]
// [8u...8u]

// NBwwB
// NBdB when N == 0
// [ [8u EQ 0], [8u][8s][16s][8u] , [8u][8s][32u][8s] ]
// j prefix = jump/label

void SimpleDisassembler::readParams(InstPtr inst, const char *typeString)
{
    // Handle [] blocks as working on an individual element (i.e a BYTE,WORD etc)
    // this syntax allows picking of nibbles and bit fields into their own parameters.
    while (*typeString)
    {
        std::string typeStr(typeString, 1);
        if (typeStr == "N") // Read nibbles 
        {
            const uint8 byte = stream_->ReadU8();

            //inst->GetParams().push_back(new IntValue(Nib1(byte), false));
            //inst->GetParams().push_back(new IntValue(Nib2(byte), false));
            //inst->_params.push_back(new IntValue(Nib1(byte), false));
            //inst->_params.push_back(new IntValue(Nib2(byte), false));
            inst->AddParam(new IntValue(Nib1(byte), false));
            inst->AddParam(new IntValue(Nib2(byte), false));

            address_++;
        }
        else if (typeStr == "U")
        {
            const uint8 byte = stream_->ReadU8();
            //inst->GetParams().push_back( new IntValue((byte >> 5) & 0x7, false));
            //inst->GetParams().push_back( new IntValue((byte & 0x1F), false));
            //inst->_params.push_back( new IntValue((byte >> 5) & 0x7, false));
            //inst->_params.push_back( new IntValue((byte & 0x1F), false));
            inst->AddParam( new IntValue((byte >> 5) & 0x7, false));
            inst->AddParam( new IntValue((byte & 0x1F), false));
            address_++;
        }
        else
        {
            //inst->GetParams().push_back(readParameter(inst, typeStr));
            //inst->_params.push_back(readParameter(inst, typeStr));
            inst->AddParam(readParameter(inst, typeStr));
        }
        typeString++;
    }
}

void SimpleDisassembler::readParams(InstPtr inst, const char *typeString, const std::vector<std::string>& params)
{
    while (*typeString)
    {
        std::string typeStr(typeString, 1);
        if (typeStr == "N") // Read nibbles 
        {
         
            address_++;
        }
        else if (typeStr == "U")
        {
            address_++;
        }
        else
        {
           // inst->_params.push_back(readParameter(inst, typeStr));
        }
        typeString++;
    }
}

ValuePtr SimpleDisassembler::readParameter(InstPtr inst, std::string type) {
    ValuePtr retval = NULL;

    if (type == "b") // signed byte
    {
        retval = new IntValue(stream_->ReadS8(), true);
        address_++;
    }
    else if (type == "B") // unsigned byte
    {
        retval = new IntValue((uint32)stream_->ReadU8(), false);
        address_++;
    }
    else if (type == "s") // 16-bit signed integer (short), little-endian
    {
        retval = new IntValue(stream_->ReadS16(), true);
        address_ += 2;
    }
    else if (type == "w") // 16-bit unsigned integer (word), little-endian
    {
        retval = new IntValue((uint32)stream_->ReadU16(), false);
        address_ += 2;
    }
    else if (type == "i") // 32-bit signed integer (int), little-endian
    {
        retval = new IntValue(stream_->ReadS32(), true);
        address_ += 4;
    }
    else if (type == "d") // 32-bit unsigned integer (dword), little-endian
    {
        retval = new IntValue(stream_->ReadU32(), false);
        address_ += 4;
    }
    else
    {
        throw UnknownOpcodeParameterException(type);
    }
    return retval;
}

