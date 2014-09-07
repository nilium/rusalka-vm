/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#pragma GCC visibility push(hidden)

class vm_op;

#pragma GCC visibility pop


#pragma GCC visibility push(default)

class vm_thread;
class vm_state;
struct vm_value;


template <typename T>
struct vm_find_result {
  bool ok;
  T value;
};

using vm_found_fn_t = vm_find_result<int32_t>;
using vm_bound_fn_t = vm_found_fn_t;


/// VM callback -- given a thread, argument count, and array of argument values.
using vm_callback_t = vm_value (vm_thread &vm, int32_t argc, vm_value const *argv, void *context);


#pragma GCC visibility pop
