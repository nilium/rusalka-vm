/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "vm_function.h"
#include "vm_thread.h"


vm_value vm_function::operator() ()
{
  return _thread.call_function(_pointer, 0);
}
