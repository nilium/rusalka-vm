/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <type_traits>

#include "vm_opcode.h"
#include "vm_value.h"


class vm_unit;


class vm_op
{
public:
  vm_unit const &unit;
  int32_t const ip;

  uint16_t litflag() const;
  vm_opcode opcode() const;
  vm_value operator [] (int32_t index) const;
};

// static_assert(sizeof(vm_opcode) == sizeof(vm_value),
//   "the size of vm_opcode must be the same as vm_value");

// static_assert(sizeof(vm_op) == sizeof(vm_opcode),
//   "the size of vm_op must be that of vm_opcode");

// static_assert(offsetof(vm_op, argv) == sizeof(vm_opcode),
//   "offset of vm_op::argv must be the same as the size of vm_op");
