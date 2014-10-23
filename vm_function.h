/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <cstdint>
#include <utility>

#include "vm_value.h"


class vm_thread;


template <class... ARGS>
vm_value vm_invoke_function(vm_thread &thread, int64_t pointer, ARGS &&... args);


class vm_function
{
  vm_thread &_thread;
  int64_t _pointer;

  vm_function(vm_thread &thread, int64_t pointer)
  : _thread(thread)
  , _pointer(pointer)
  {
    // nop
  }

  friend vm_thread;

public:
  vm_function(const vm_function &other) = default;
  vm_function() = delete;

  template <class... ARGS>
  vm_value operator()(ARGS&&... args);
  vm_value operator() ();
};



template <class... ARGS>
vm_value vm_function::operator()(ARGS&&... args)
{
  return vm_invoke_function(_thread, _pointer, std::forward<ARGS>(args)...);
}
