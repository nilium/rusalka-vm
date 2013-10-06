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

#ifndef __VM_OP_H__
#define __VM_OP_H__

#include <type_traits>

#include "vm_opcode.h"
#include "vm_value.h"


struct op_t {
  opcode_t opcode;
  value_t argv[5];
};

static_assert(sizeof(opcode_t) == sizeof(value_t),
  "the size of opcode_t must be the same as value_t");

// static_assert(sizeof(op_t) == sizeof(opcode_t),
//   "the size of op_t must be that of opcode_t");

static_assert(offsetof(op_t, argv) == sizeof(opcode_t),
  "offset of op_t::argv must be the same as the size of op_t");

#endif /* end __VM_OP_H__ include guard */
