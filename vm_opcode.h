/*
  Copyright (c) 2013 Noel Raymond Cower.

  This file is part of Rusalka VM.

  Rusalka VM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Rusalka VM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Rusalka VM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __VM_OPCODE_H__
#define __VM_OPCODE_H__

#include <cstdint>
#include <iostream>


/* _L -> reg op literal */
enum opcode_t : int32_t
{
#define INSTRUCTION(OPCODE, ASM_NAME, CODE, NUM_ARGS, ARG_INFO... ) OPCODE = CODE,
#include "vm_instructions.h"
#undef INSTRUCTION
  OP_COUNT
};


extern const int32_t g_opcode_argc[OP_COUNT];


std::ostream &operator << (std::ostream &out, opcode_t v);
bool opcode_has_litflag(opcode_t op);

#endif /* end __VM_OPCODE_H__ include guard */
