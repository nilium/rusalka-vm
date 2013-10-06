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

#ifndef __VM_VALUE_H__
#define __VM_VALUE_H__

#include <cstdint>
#include <iostream>


union value_t {
  int32_t i32;
  uint32_t ui32;
  float f32;

  operator float() const { return f32; }
  operator int() const { return (int)i32; }
  operator long() const { return (long)i32; }
  operator unsigned int() const { return (unsigned int)ui32; }
  operator unsigned long() const { return (unsigned long)ui32; }
};

std::ostream &operator << (std::ostream &out, value_t v);

#endif /* end __VM_VALUE_H__ include guard */
