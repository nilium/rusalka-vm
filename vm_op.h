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


class op_t
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

// static_assert(sizeof(op_t) == sizeof(opcode_t),
//   "the size of op_t must be that of opcode_t");

// static_assert(offsetof(op_t, argv) == sizeof(opcode_t),
//   "offset of op_t::argv must be the same as the size of op_t");

#endif /* end __VM_OP_H__ include guard */
