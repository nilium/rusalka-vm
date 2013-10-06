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

#ifndef __VM_FUNCTION_H__
#define __VM_FUNCTION_H__


#include <cstdint>
#include <utility>

#include "vm_value.h"


template <typename VM_TYPE>
class vm_function_t {
  VM_TYPE &_vm;
  int32_t _pointer;

  vm_function_t(VM_TYPE &vm, int32_t pointer)
  : _vm(vm)
  , _pointer(pointer)
  {
    // nop
  }

  friend VM_TYPE;

public:
  vm_function_t(const vm_function_t &other) = default;
  vm_function_t() = delete;

  template <class... ARGS>
  value_t operator()(ARGS&&... args) {
    return _vm.call_function(_pointer, std::forward<ARGS>(args)...);
  }

  value_t operator()() {
    return _vm.call_function(_pointer, 0);
  }
};


#endif /* end __VM_FUNCTION_H__ include guard */
