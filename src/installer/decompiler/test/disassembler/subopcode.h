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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DEC_TEST_DISASM_SUBOPCODE_H
#define DEC_TEST_DISASM_SUBOPCODE_H

#include "decompiler/simple_disassembler.h"

class SubOpcodeDisassembler : public SimpleDisassembler {
public:
	SubOpcodeDisassembler(InstVec &insts);
	void doDisassemble() override;
};

class SubOpcodeFakeInstruction : public Instruction {
public:
    virtual void ProcessInst(Function&, ValueStack&, Engine*, CodeGenerator*) override {}
};

#endif
