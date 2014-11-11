/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <exception>
#include <stdexcept>


struct vm_runtime_error;
struct vm_logic_error;
/** Thrown if bytecode attempts to access a register that doesn't exist. */
struct vm_bad_register;
/** Exception thrown for jumping to or calling an invalid instruction pointer. */
struct vm_invalid_instruction_pointer;
/** Thrown if pop is called when ebp == esp */
struct vm_stack_underflow;
/** Thrown if accessing a stack index < 0. */
struct vm_stack_access_error;
/** Used when peek/poke/memmove something outside the bounds of a block. */
struct vm_memory_access_error;
/** Thrown if the permissions on a memory block don't satisfy the access */
/** requested for a block. */
struct vm_memory_permission_error;
/**
 * Exception specifically used when there's an attempt to peek, poke, memdup,
 * or memmove the zero block or any other operation not permitted on the zero
 * block.
 */
struct vm_null_access_error;
/** Exception thrown when a CALL instruction has an argc < 0 or > esp. */
struct vm_invalid_argument_count;
/** Exception thrown for using a thread with the wrong process. */
struct vm_wrong_process;
/** Thrown if bytecode contains an unrecognized opcode. */
struct vm_bad_opcode;
/** Generic unit loading consistency error */
struct vm_bad_unit;
/** Generic unit loading IO error */
struct vm_unit_io_error;
/**
 * Exception thrown specifically for units that are of an unsupported
 * bytecode version.
 */
struct vm_unsupported_unit_version;


#define VM_DECLARE_EXCEPTION(KLASSNAME, SUPERKLASSNAME) \
  struct KLASSNAME : public SUPERKLASSNAME { \
    KLASSNAME (char const *what_arg) : SUPERKLASSNAME(what_arg) { /* nop */ } \
    KLASSNAME (std::string const &what_arg) : SUPERKLASSNAME(what_arg) { /* nop */ } \
  }


// Base VM exception classes
VM_DECLARE_EXCEPTION(vm_runtime_error, std::runtime_error);
VM_DECLARE_EXCEPTION(vm_logic_error, std::logic_error);

VM_DECLARE_EXCEPTION(vm_bad_register, vm_logic_error);
VM_DECLARE_EXCEPTION(vm_invalid_instruction_pointer, vm_logic_error);
VM_DECLARE_EXCEPTION(vm_stack_underflow, vm_logic_error);
VM_DECLARE_EXCEPTION(vm_stack_access_error, vm_logic_error);
VM_DECLARE_EXCEPTION(vm_memory_access_error, vm_logic_error);
VM_DECLARE_EXCEPTION(vm_memory_permission_error, vm_memory_access_error);
VM_DECLARE_EXCEPTION(vm_null_access_error, vm_memory_access_error);
VM_DECLARE_EXCEPTION(vm_invalid_argument_count, vm_logic_error);
VM_DECLARE_EXCEPTION(vm_wrong_process, vm_logic_error);

VM_DECLARE_EXCEPTION(vm_bad_opcode, vm_runtime_error);
VM_DECLARE_EXCEPTION(vm_unit_io_error, vm_runtime_error);
VM_DECLARE_EXCEPTION(vm_bad_unit, vm_runtime_error);

VM_DECLARE_EXCEPTION(vm_unsupported_unit_version, vm_bad_unit);

#undef VM_DECLARE_EXCEPTION
