/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __VM_STATE_H__
#define __VM_STATE_H__

#include <climits>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include <array>
#include <bitset>
#include <fstream>
#include <map>
#include <memory>
#include <vector>

#include "_types.h"
#include "vm_op.h"
#include "vm_value.h"
#include "vm_opcode.h"
#include "vm_function.h"
#include "vm_unit.h"
#include "vm_thread.h"


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


class vm_state;
class vm_thread;


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


class vm_state
{
  struct memblock_t
  {
    int32_t size;
    uint32_t flags;
    void *block;
  };

  struct callback_info
  {
    vm_callback_t *callback;
    void *context;

    value_t invoke(vm_thread &thread, int argc, value_t const *argv) const {
      return callback(thread, argc, argv, context);
    }
  };

  using found_memblock = vm_find_result<memblock_t>;
  using memblock_map_t = std::map<int32_t, memblock_t>;
  using stack_t = std::vector<value_t>;
  using callbacks_t = std::vector<callback_info>;
  using thread_pointer = std::unique_ptr<vm_thread>;
  using thread_stores = std::vector<thread_pointer>;

  static memblock_t const NO_BLOCK;

  thread_stores _threads {};
  callbacks_t _callbacks {};
  memblock_map_t _blocks {};
  int32_t _block_counter = 1;

  int32_t unused_block_id();
  void release_all_memblocks() noexcept;

  vm_unit_t _unit;
  int32_t _source_size;

  void prepare_unit();

  friend class vm_thread;

public:
  vm_state() = default;
  ~vm_state();

  /**
    Sets the VM's source object. Must be called before running the VM and must
    not be called afterward. The VM takes ownership of the source data -- the
    object is moved and the original object is invalid.
  */
  void set_unit(vm_unit_t const &unit);
  void set_unit(vm_unit_t &&unit);

private:
  bool check_block_bounds(int32_t block_id, int32_t offset, int32_t size) const;
  void load_thread(thread_pointer &&thread);
  void destroy_thread(int32_t thread_index);

  int32_t realloc_block_with_flags(int32_t block_id, int32_t size, uint32_t flags);
  // Returns the block for the given ID -- does not do flag checking of any kind.
  found_memblock get_block_info(int32_t block_id) const;

public:
  int32_t realloc_block(int32_t block, int32_t size);
  int32_t alloc_block(int32_t size) { return realloc_block(0, size); }
  void free_block(int32_t block_id);
  int32_t block_size(int32_t block_id) const;
  int32_t duplicate_block(int32_t block_id);
  void *get_block(int32_t block_id, uint32_t permissions);
  const void *get_block(int32_t block_id, uint32_t permissions) const;

  vm_found_fn find_function_pointer(const char *name) const;

  vm_bound_fn bind_callback(const char *name, int length, vm_callback_t *function, void *context = nullptr);
  vm_bound_fn bind_callback(const char *name, vm_callback_t *function, void *context = nullptr)
  {
    return bind_callback(name, std::strlen(name), function, context);
  }

  vm_thread &thread_by_index(int32_t thread_index);
  vm_thread const &thread_by_index(int32_t thread_index) const;

  vm_thread &make_thread(size_t stack_size = 8192);
  vm_thread &fork_thread(vm_thread const &thread);

};


#endif /* end __VM_STATE_H__ include guard */
