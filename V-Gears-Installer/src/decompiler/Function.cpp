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

#include "decompiler/Function.h"

/**
 * Constructor.
 *
 * Required for use with STL, should not be called manually.
 */
Function::Function(): start_addr(0), end_addr(0), num_instructions(0){}

/**
 * Constructor.
 *
 * @param start_addr[in] Address of the first instruction in the function.
 * @param end_addr[in] Address of the last instruction in the function
 */
Function::Function(uint32 start_addr, uint32 end_addr)
  : start_addr(start_addr), end_addr(end_addr), num_instructions(0){}
