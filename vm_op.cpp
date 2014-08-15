/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "vm_op.h"
#include "vm_unit.h"


opcode_t vm_op::opcode() const
{
  return unit.instructions[ip].opcode;
}


vm_value vm_op::operator [] (int32_t index) const
{
  int32_t argv_base = unit.instructions[ip].arg_pointer;
  return unit.instruction_argv[argv_base + index];
}


uint16_t vm_op::litflag() const
{
  return unit.instructions[ip].litflag;
}
