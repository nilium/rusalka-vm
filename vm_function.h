/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __VM_FUNCTION_H__
#define __VM_FUNCTION_H__


#include <cstdint>
#include <utility>

#include "vm_value.h"


template <typename VM_TYPE>
class vm_function_t
{
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
  value_t operator()(ARGS&&... args)
  {
    return _vm.call_function(_pointer, std::forward<ARGS>(args)...);
  }

  value_t operator()()
  {
    return _vm.call_function(_pointer, 0);
  }
};


#endif /* end __VM_FUNCTION_H__ include guard */
