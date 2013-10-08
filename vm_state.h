/*
  Copyright (c) 2013 Noel Raymond Cower.

  This file is part of Rusalka VM.

  Rusalka VM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Rusalka VM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Rusalka VM.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __VM_STATE_H__
#define __VM_STATE_H__

#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include <array>
#include <bitset>
#include <fstream>
#include <map>
#include <vector>

#include "vm_op.h"
#include "vm_value.h"
#include "vm_opcode.h"
#include "vm_source.h"
#include "vm_function.h"


/*** Macros to configure the VM ***/

/**
  Log each instruction and its arguments as it's executed. Does not log the
  contents of registers referenced by arguments.
*/
// #define LOG_OP_INFO

/** Log per-instruction changes to the stack and state. */
// #define LOG_STATE_CHANGES

/** Log the final registers and stack */
// #define LOG_FINAL_STATE

/**
  If defined, restores call argument registers on return to their original
  values, otherwise there is no attempt to restore either.
*/
// #define VM_PRESERVE_CALL_ARGUMENT_REGISTERS

/**
  If defined, all registers are preserved on call and restored on return.
  This disables VM_PRESERVE_CALL_ARGUMENT_REGISTERS. Otherwise, only registers
  0 through 12 are preserved, with the exception of the return register (3) as
  it's considered nonvolatile. Registers 13 through 31 are nonvolatile and must
  be preserved by the callee if used.
*/
// #define VM_PRESERVE_PRESERVE_ALL_REGISTERS_ON_CALL

/**
  If defined, replaces methods like reg(), stack(), etc. with macros.
*/
// #define VM_USE_DATA_ACCESS_MACROS


class vm_source_t;
class vm_state_t;


typedef value_t (vm_callback_t)(vm_state_t &vm, int32_t argc, const value_t *argv);


enum memblock_flags_t : uint32_t {
  VM_MEM_NO_PERMISSIONS = 0x0,
  VM_MEM_READABLE = 0x1,
  VM_MEM_WRITABLE = 0x2,
  VM_MEM_READ_WRITE = VM_MEM_READABLE | VM_MEM_WRITABLE,
  VM_MEM_SOURCE_DATA = 0x4 | VM_MEM_READABLE,
};


class vm_state_t {
  enum {
    REGISTER_COUNT = 32,
    REGISTER_SIZE = sizeof(value_t)
  };

  struct memblock_t {
    uint32_t size;
    uint32_t flags;
    void *block;
  };

  using memblock_map_t = std::map<uint32_t, memblock_t>;

  // std::array<value_t, REGISTER_COUNT> _registers;
  value_t _registers[REGISTER_COUNT];
  std::vector<value_t> _stack;
  std::vector<vm_callback_t *> _callbacks;
  memblock_map_t _blocks;
  uint32_t _block_counter;
  uint32_t _sequence;
  uint32_t _trap = 0;

  uint32_t unused_block_id();

  source_t _source;
  uint32_t _source_size;

  #ifdef VM_USE_DATA_ACCESS_MACROS
  #define reg(X) _registers[X]
  #define ip() _registers[0].i32
  #define ebp() _registers[1].i32
  #define esp() _registers[2].ui32
  #define rp() _registers[3]
  #define stack(X) _stack[ebp() + X]
  #endif

  template <class T, class... ARGS>
  uint32_t load_registers(int32_t index, T &&first, ARGS&&... args) {
    reg(index) = value_of(std::forward<T>(first));
    return load_registers(index + 1, std::forward<ARGS>(args)...);
  }

  template <class T>
  uint32_t load_registers(int32_t index, T &&first) {
    reg(index) = value_of(std::forward<T>(first));
    return index + 1;
  }

  uint32_t load_registers(int32_t index) {
    return index;
  }

  static value_t value_of(double d) {
    value_t v;
    v.f32 = static_cast<float>(d);
    return v;
  }

  static value_t value_of(float f) {
    value_t v;
    v.f32 = f;
    return v;
  }

  static value_t value_of(int32_t i) {
    value_t v;
    v.i32 = i;
    return v;
  }

  static value_t value_of(uint32_t u) {
    value_t v;
    v.ui32 = u;
    return v;
  }

public:
  vm_state_t(size_t stackSize);
  ~vm_state_t();

  /**
    Sets the VM's source object. Must be called before running the VM and must
    not be called afterward. The VM takes ownership of the source data -- the
    object is moved and the original object is invalid.
  */
  void set_source(source_t &&source);

private:
  void exec(const op_t &op);
  bool run(int32_t from_ip);
  bool run();

  /**
    RESERVED REGISTERS:
    0 -> ip
    1 -> ebp
    2 -> esp
    3 -> return value
  **/

  int32_t fetch();

  #ifndef VM_USE_DATA_ACCESS_MACROS
  int32_t ip() const { return _registers[0].i32; }
  int32_t ebp() const { return _registers[1].i32; }
  uint32_t esp() const { return _registers[2].ui32; }
  value_t rp() const { return _registers[3]; }

  int32_t &ip() { return _registers[0].i32; }
  int32_t &ebp() { return _registers[1].i32; }
  uint32_t &esp() { return _registers[2].ui32; }
  value_t &rp() { return _registers[3]; }

  value_t &reg(int32_t off) { return _registers[off]; }
  value_t reg(int32_t off) const { return _registers[off]; }

  value_t stack(int32_t off) const { return _stack[ebp() + off]; }
  value_t &stack(int32_t off) { return _stack[ebp() + off]; }
  #endif

  void push(uint32_t bits);
  void pop(uint32_t bits, bool shrink = true);

  void exec_call(int32_t instr, uint32_t args_mask);

public:
  uint32_t alloc(uint32_t size);
  void free(uint32_t block_id);
  uint32_t block_size(uint32_t block_id) const;
  uint32_t duplicate_block(uint32_t block_id);
  void *get_block(uint32_t block_id, uint32_t permissions);
  const void *get_block(uint32_t block_id, uint32_t permissions) const;

  void dump_registers(size_t count = REGISTER_COUNT) const;
  void dump_stack(size_t until = SIZE_MAX) const;

  std::pair<bool, int32_t> find_function_pointer(const char *name) const;

  void bind_callback(const char *name, vm_callback_t *function);

  template <class... ARGS>
  value_t call_function(const char *name, ARGS&&... args) {
    const auto pointer = find_function_pointer(name);
    // if (!pointer.first) throw std::runtime_error("no such function");
    return call_function(pointer.second, args...);
  }

  template <class... ARGS>
  value_t call_function(int32_t pointer, ARGS&&... args) {
    const uint32_t argc = load_registers(4, std::forward<ARGS>(args)...) - 4;
    return call_function_nt(pointer, argc);
  }

  value_t call_function(const char *name) {
    const auto pointer = find_function_pointer(name);
    // if (!pointer.first) throw std::runtime_error("no such function");

    return call_function_nt(pointer.second, 0);
  }

  value_t call_function_nt(int32_t pointer) { return call_function_nt(pointer, 0); }

  value_t call_function_nt(const char *name, uint32_t argc, const value_t *argv);
  value_t call_function_nt(int32_t pointer, uint32_t argc, const value_t *argv);
  value_t call_function_nt(int32_t pointer, uint32_t mask);

  void set_callback(uint32_t id, vm_callback_t *callback);

  vm_function_t<vm_state_t> function(const char *name);
  vm_function_t<vm_state_t> function(int32_t pointer);
};

#if defined(VM_USE_DATA_ACCESS_MACROS) && !defined(VM_KEEP_DATA_MACROS)
#undef reg
#undef stack
#undef ip
#undef ebp
#undef esp
#undef rp
#endif

#endif /* end __VM_STATE_H__ include guard */
