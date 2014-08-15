#include "vm_function.h"
#include "vm_thread.h"


vm_value vm_function::operator() ()
{
  return _thread.call_function(_pointer, 0);
}
