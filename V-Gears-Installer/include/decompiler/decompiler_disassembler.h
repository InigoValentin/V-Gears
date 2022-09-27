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

#ifndef DEC_DISASSEMBLER_H
#define DEC_DISASSEMBLER_H

#include <iostream>
#include <vector>

#include "instruction.h"
#include "common/BinaryReader.h"
#include "unknown_opcode_exception.h"
#include "objectFactory.h"

/**
 * Base class for disassemblers.
 */
class Disassembler {
protected:
	std::unique_ptr<BinaryReader> stream_;                         ///< Used to perform file I/O.
	InstVec &_insts;                              ///< Container for disassembled instructions.
	uint32 address_base_;                          ///< Base address where the script starts.

	/**
	 * Performs disassembly.
	 *
	 * @throws UnknownOpcodeException on unknown opcode. May throw std::exception on other failures.
	 */
	virtual void DoDisassemble() = 0;

	/**
	 * Outputs the disassembled code.
	 *
	 * @param output The std::ostream to output to.
	 */
	virtual void DoDumpDisassembly(std::ostream &output);

public:
	/**
	 * Constructor for Disassembler.
	 *
	 * @param insts Reference to the vector in which disassembled instructions should be placed.
	 */
	Disassembler(InstVec &insts);
    virtual ~Disassembler() = default;

	/**
	 * Open a file for disassembly.
	 *
	 * @param filename The file to disassemble.
	 */
	virtual void Open(const char *filename);

	/**
	 * Request disassembled instructions.
	 *
	 * @return An std::vector containing the disassembled instructions.
	 */
	void Disassemble();

	/**
	 * Outputs the disassembled code. Disassembles code if this has not already been done.
	 *
	 * @param output The std::ostream to output to.
	 */
	void DumpDisassembly(std::ostream &output);
};
#endif
