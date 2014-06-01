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


value_t value_t::special(uint64_t sig48)
{
  value_t result;
  result.bits_ = SPECIAL_BITS | (sig48 & SPECIAL_CONTENT_MASK);
  return result;
}

