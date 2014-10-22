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
  int64_t const ip;

  uint64_t litflag() const;
  vm_opcode opcode() const;
  vm_value operator [] (int64_t index) const;
};
