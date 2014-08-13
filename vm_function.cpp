#include "vm_function.h"
#include "vm_thread.h"


value_t vm_function_t::operator() ()
{
  return _thread.call_function(_pointer, 0);
}
