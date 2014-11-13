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


// Semi-internal.
template <class... ARGS>
vm_value vm_invoke_function(vm_thread &thread, int64_t pointer, ARGS &&... args);


/**
 * Invokable function type for Rusalka.
 *
 * vm_function is bound to a specific thread and, given an instruction pointer,
 * can be used to call a VM function (whether it's a host callback or bytecode
 * function) similar to a normal function. Automagically converts its arguments
 * to VM values.
 */
class vm_function
{
  /** The thread the function is bound to. */
  vm_thread &_thread;
  /**
   * The instruction pointer of the function.
   *
   * If negative, points to a host callback.
   */
  int64_t _pointer;

  vm_function(vm_thread &thread, int64_t pointer)
  : _thread(thread)
  , _pointer(pointer)
  {
    // nop
  }

  friend vm_thread;

public:
  /** Copy ctor. */
  vm_function(const vm_function &other) = default;

  vm_function() = delete;

  /**
   * Call operator. Given any number of arguments, will attempt to convert them
   * all to VM value types and call the function with those after pushing them
   * onto the stack.
   */
  template <class... ARGS>
  vm_value operator()(ARGS&&... args);
  /**
   * Argumentless call operator. Does what you think it does.
   */
  vm_value operator() ();
};



template <class... ARGS>
vm_value vm_function::operator()(ARGS&&... args)
{
  return vm_invoke_function(_thread, _pointer, std::forward<ARGS>(args)...);
}
