/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <cstdint>
#include <iostream>


/* _L -> reg op literal */
/**
 * vm_opcode defines all opcodes used in the Rusalka VM as unsigned 16-bit
 * integers.
 *
 * It's not expected that Rusalka will ever exceed 16 bits for an opcode.
 *
 * Depends on `vm_instructions.h`.
 */
enum vm_opcode : uint16_t
{
#define INSTRUCTION(OPCODE, ASM_NAME, CODE, NUM_ARGS, ARG_INFO... ) OPCODE = CODE,
#include "vm_instructions.h"
#undef INSTRUCTION
  /** The total number of opcodes defined by Rusalka. */
  OP_COUNT
};


/**
 * Constant list of how many operands a given opcode takes.
 *
 * Indexed by vm_opcode.
 */
extern const int32_t g_opcode_argc[OP_COUNT];


/**
 * Writes an opcode's name to the output stream.
 */
std::ostream &operator << (std::ostream &out, vm_opcode v);

/** Returns whether a given opcode takes a litflag. */
bool opcode_has_litflag(vm_opcode op);
