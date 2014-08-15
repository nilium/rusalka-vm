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


class vm_thread;


template <class... ARGS>
vm_value vm_invoke_function(vm_thread &thread, int32_t pointer, ARGS &&... args);


class vm_function_t
{
  vm_thread &_thread;
  int32_t _pointer;

  vm_function_t(vm_thread &thread, int32_t pointer)
  : _thread(thread)
  , _pointer(pointer)
  {
    // nop
  }

  friend vm_thread;

public:
  vm_function_t(const vm_function_t &other) = default;
  vm_function_t() = delete;

  template <class... ARGS>
  vm_value operator()(ARGS&&... args);
  vm_value operator() ();
};



template <class... ARGS>
vm_value vm_function_t::operator()(ARGS&&... args)
{
  return vm_invoke_function(_thread, _pointer, std::forward<ARGS>(args)...);
}


#endif /* end __VM_FUNCTION_H__ include guard */
