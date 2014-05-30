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
#include "vm_function.h"
#include "vm_unit.h"


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


class vm_state_t;


using vm_callback_t = value_t(vm_state_t &vm, int32_t argc, value_t const *argv);
using vm_fn_find_result_t = std::pair<bool, int32_t>;


enum memblock_flags_t : uint32_t
{
  VM_MEM_NO_PERMISSIONS = 0x0,
  VM_MEM_READABLE = 0x1,
  VM_MEM_WRITABLE = 0x2,
  VM_MEM_STATIC = 0x4,
  VM_MEM_READ_WRITE = VM_MEM_READABLE | VM_MEM_WRITABLE,
  VM_MEM_SOURCE_DATA = VM_MEM_STATIC | VM_MEM_READABLE,
};

constexpr int32_t VM_NULL_BLOCK = 0;


class vm_state_t
{
  enum
  {
    R_NONVOLATILE_REGISTERS = 8,

    R_IP = 0,
    R_EBP = 1,
    R_ESP = 2,
    R_RP = 3,

    R_FIRST_NONVOLATILE,
    R_LAST_NONVOLATILE = R_FIRST_NONVOLATILE + (R_NONVOLATILE_REGISTERS - 1),

    R_FIRST_VOLATILE,

    REGISTER_COUNT = 256,

    R_VOLATILE_REGISTERS = REGISTER_COUNT - R_FIRST_VOLATILE,
  };

  struct memblock_t
  {
    int32_t size;
    uint32_t flags;
    void *block;
  };

  using memblock_map_t = std::map<int32_t, memblock_t>;
  using stack_t = std::vector<value_t>;
  using callbacks_t = std::vector<vm_callback_t *>;

  // std::array<value_t, REGISTER_COUNT> _registers;
  value_t _registers[REGISTER_COUNT];

  stack_t _stack;
  callbacks_t _callbacks;
  memblock_map_t _blocks;
  int32_t _block_counter;
  int32_t _sequence;
  int32_t _trap = 0;

  int32_t unused_block_id();
  void release_all_memblocks();

  vm_unit_t _unit;
  int32_t _source_size;

  template <class T, class... ARGS>
  int32_t load_registers(int32_t index, T &&first, ARGS&&... args)
  {
    push(make_value(std::forward<T>(first)));
    return load_registers(index + 1, std::forward<ARGS>(args)...);
  }

  template <class T>
  int32_t load_registers(int32_t index, T &&first)
  {
    push(make_value(std::forward<T>(first)));
    return index + 1;
  }

  int32_t load_registers(int32_t index) const
  {
    return index;
  }

  void prepare_unit();

public:
  vm_state_t(size_t stackSize = 8192);
  ~vm_state_t();

  /**
    Sets the VM's source object. Must be called before running the VM and must
    not be called afterward. The VM takes ownership of the source data -- the
    object is moved and the original object is invalid.
  */
  void set_unit(vm_unit_t const &unit);
  void set_unit(vm_unit_t &&unit);

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

  value_t ip() const { return _registers[R_IP]; }
  value_t &ip() { return _registers[R_IP]; }

  value_t ebp() const { return _registers[R_EBP]; }
  value_t &ebp() { return _registers[R_EBP]; }

  value_t esp() const { return _registers[R_ESP]; }
  value_t &esp() { return _registers[R_ESP]; }

  value_t rp() const { return _registers[R_RP]; }
  value_t &rp() { return _registers[R_RP]; }

  value_t reg(int32_t off) const;
  value_t &reg(int32_t off);

  value_t stack(int32_t off) const;
  value_t &stack(int32_t off);

  void push(value_t value);
  value_t pop(bool copy_only = false);

  void exec_call(int32_t instr, int32_t argc);

  bool check_block_bounds(int32_t block_id, int32_t offset, int32_t size) const;

public:
  int32_t realloc_block(int32_t block, int32_t size);
  int32_t alloc_block(int32_t size) { return realloc_block(0, size); }
  void free_block(int32_t block_id);
  int32_t block_size(int32_t block_id) const;
  int32_t duplicate_block(int32_t block_id);
  void *get_block(int32_t block_id, uint32_t permissions);
  const void *get_block(int32_t block_id, uint32_t permissions) const;

  void dump_registers(size_t count = REGISTER_COUNT) const;
  void dump_stack(size_t until = SIZE_MAX) const;

  vm_fn_find_result_t find_function_pointer(const char *name) const;

  void bind_callback(const char *name, int length, vm_callback_t *function);
  void bind_callback(const char *name, vm_callback_t *function)
  {
    bind_callback(name, std::strlen(name), function);
  }

  template <class... ARGS>
  value_t call_function(const char *name, ARGS&&... args)
  {
    const auto pointer = find_function_pointer(name);
    // if (!pointer.first) throw std::runtime_error("no such function");
    return call_function(pointer.second, args...);
  }

  template <class... ARGS>
  value_t call_function(int32_t pointer, ARGS&&... args)
  {
    const int32_t argc = load_registers(4, std::forward<ARGS>(args)...) - 4;
    return call_function_nt(pointer, argc);
  }

  value_t call_function(const char *name)
  {
    const auto pointer = find_function_pointer(name);
    // if (!pointer.first) throw std::runtime_error("no such function");

    return call_function_nt(pointer.second, 0);
  }

  value_t call_function_nt(int32_t pointer) { return call_function_nt(pointer, 0); }

  value_t call_function_nt(const char *name, int32_t argc, const value_t *argv);
  value_t call_function_nt(int32_t pointer, int32_t argc, const value_t *argv);
  value_t call_function_nt(int32_t pointer, int32_t argc);

  void set_callback(int32_t id, vm_callback_t *callback);

  vm_function_t<vm_state_t> function(const char *name);
  vm_function_t<vm_state_t> function(int32_t pointer);

  value_t deref(value_t input, value_t flag, uint32_t mask = ~0u) const;
};


#endif /* end __VM_STATE_H__ include guard */
