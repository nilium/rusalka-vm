/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __VM_OP_H__
#define __VM_OP_H__

#include <type_traits>

#include "vm_opcode.h"
#include "vm_value.h"


class vm_unit_t;


class vm_op
{
public:
  vm_unit_t const &unit;
  int32_t const ip;

  uint16_t litflag() const;
  opcode_t opcode() const;
  vm_value operator [] (int32_t index) const;
};

// static_assert(sizeof(opcode_t) == sizeof(vm_value),
//   "the size of opcode_t must be the same as vm_value");

// static_assert(sizeof(vm_op) == sizeof(opcode_t),
//   "the size of vm_op must be that of opcode_t");

// static_assert(offsetof(vm_op, argv) == sizeof(opcode_t),
//   "offset of vm_op::argv must be the same as the size of vm_op");

#endif /* end __VM_OP_H__ include guard */
