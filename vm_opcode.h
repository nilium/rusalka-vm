/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __VM_OPCODE_H__
#define __VM_OPCODE_H__

#include <cstdint>
#include <iostream>


/* _L -> reg op literal */
enum opcode_t : uint16_t
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
