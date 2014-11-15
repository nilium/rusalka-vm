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


/**
 * A VM op.
 *
 * Acts as a wrapper interface for a particular VM unit and an instruction
 * pointer to allow access to that instruction's opcode and operands.
 */
class vm_op
{
public:
  /** The VM unit the op is bound to. */
  vm_unit const &unit;
  /** The instruction pointer into the VM unit. */
  int64_t const ip;

  /** The litflag mask for the op. */
  uint64_t litflag() const;
  /** The op's opcode. */
  vm_opcode opcode() const;
  /** Subscript operator to access the op's operands / arguments. */
  vm_value operator [] (int64_t index) const;
};
