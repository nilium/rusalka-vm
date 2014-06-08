/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "vm_value.h"
#include <ios>
#include <iomanip>


std::ostream &operator << (std::ostream &out, value_t v)
{
  return
    out
    << "<"
    << std::dec << v.f64() << ", "
    << std::hex << std::showbase
    << v.i32()
    << std::dec << std::noshowbase
    << ">";
}
