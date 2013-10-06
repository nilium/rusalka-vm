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

#include "vm_opcode.h"

const uint32_t g_opcode_argc[OP_COUNT] = {
#define INSTRUCTION(OPCODE, ASM_NAME, CODE, NUM_ARGS, ARG_INFO... ) NUM_ARGS,
#include "vm_instructions.h"
#undef INSTRUCTION
};

std::ostream &operator << (std::ostream &out, const opcode_t &v) {
  switch (v) {
  #define INSTRUCTION(OPCODE, ASM_NAME, CODE, ARGS, ARG_INFO... ) case OPCODE: return out << #OPCODE;
  #include "vm_instructions.h"
  #undef INSTRUCTION
  case OP_COUNT: {
    // throw std::runtime_error("invalid opcode");
    return out << "INVALID OPCODE";
  }
  }
}
