#ifndef __VM_EXCEPTION_H__
#define __VM_EXCEPTION_H__

#include <exception>


#define VM_DECLARE_EXCEPTION(KLASSNAME, SUPERKLASSNAME) \
  struct KLASSNAME : public SUPERKLASSNAME { \
    KLASSNAME (char const *what_arg) : SUPERKLASSNAME(what_arg) { /* nop */ } \
    KLASSNAME (std::string const &what_arg) : SUPERKLASSNAME(what_arg) { /* nop */ } \
  }

// Base VM exception classes
VM_DECLARE_EXCEPTION(vm_runtime_error, std::runtime_error);
VM_DECLARE_EXCEPTION(vm_logic_error, std::logic_error);

// Thrown if bytecode attempts to access a register that doesn't exist.
VM_DECLARE_EXCEPTION(vm_bad_register, vm_logic_error);
// Thrown if pop is called when ebp == esp
VM_DECLARE_EXCEPTION(vm_stack_underflow, vm_logic_error);
// Used when peek/poke/memmove something outside the bounds of a block.
VM_DECLARE_EXCEPTION(vm_memory_access_error, vm_logic_error);
// Exception specifically used when there's an attempt to
// peek/poke/memdup/memmove block zero.
VM_DECLARE_EXCEPTION(vm_null_access_error, vm_memory_access_error);

// Generic unit loading error
VM_DECLARE_EXCEPTION(vm_bad_unit, vm_runtime_error);
// Exception thrown specifically for units that are of an unsupported
// bytecode version.
VM_DECLARE_EXCEPTION(vm_unsupported_unit_version, vm_bad_unit);

#endif /* end __VM_EXCEPTION_H__ include guard */
