/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <climits>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
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


class vm_thread;


enum vm_memblock_flags : uint32_t
{
  VM_MEM_NO_PERMISSIONS = 0x0,
  VM_MEM_READABLE = 0x1,
  VM_MEM_WRITABLE = 0x2,
  VM_MEM_STATIC = 0x4,
  VM_MEM_READ_WRITE = VM_MEM_READABLE | VM_MEM_WRITABLE,
  VM_MEM_SOURCE_DATA = VM_MEM_STATIC | VM_MEM_READABLE,
};

constexpr int64_t VM_NULL_BLOCK = 0;


class vm_state
{
  struct memblock
  {
    int64_t size;
    uint32_t flags; // todo: reorder to remove padding bytes
    void *block;
  };

  struct callback_info
  {
    vm_callback_t *callback;
    void *context;

    vm_value invoke(vm_thread &thread, int argc, vm_value const *argv) const {
      return callback(thread, argc, argv, context);
    }
  };

  using found_memblock_t = vm_find_result<memblock>;
  using memblock_map_t = std::map<int64_t, memblock>;
  using stack_t = std::vector<vm_value>;
  using callbacks_t = std::vector<callback_info>;
  using thread_pointer_t = std::unique_ptr<vm_thread>;
  using thread_stores_t = std::vector<thread_pointer_t>;

  static memblock const NO_BLOCK;

  thread_stores_t _threads {};
  callbacks_t _callbacks {};
  memblock_map_t _blocks {};
  int64_t _block_counter = 1;

  int64_t unused_block_id();
  void release_all_memblocks() noexcept;

  vm_unit _unit;
  int64_t _source_size;

  void reset_state();
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
  void set_unit(vm_unit const &unit);
  void set_unit(vm_unit &&unit);

private:
  bool check_block_bounds(int64_t block_id, int64_t offset, int64_t size) const;
  void load_thread(thread_pointer_t &&thread);
  void destroy_thread(int64_t thread_index);

  int64_t realloc_block_with_flags(int64_t block_id, int64_t size, uint32_t flags);
  // Returns the block for the given ID -- does not do flag checking of any kind.
  found_memblock_t get_block_info(int64_t block_id) const;

public:
  int64_t realloc_block(int64_t block, int64_t size);
  int64_t alloc_block(int64_t size) { return realloc_block(0, size); }
  void free_block(int64_t block_id);
  int64_t block_size(int64_t block_id) const;
  int64_t duplicate_block(int64_t block_id);
  void *get_block(int64_t block_id, uint32_t permissions);
  const void *get_block(int64_t block_id, uint32_t permissions) const;

  vm_found_fn_t find_function_pointer(const char *name) const;

  vm_bound_fn_t bind_callback(const char *name, int length, vm_callback_t *function, void *context = nullptr);
  vm_bound_fn_t bind_callback(const char *name, vm_callback_t *function, void *context = nullptr)
  {
    return bind_callback(name, std::strlen(name), function, context);
  }

  vm_thread &thread_by_index(int64_t thread_index);
  vm_thread const &thread_by_index(int64_t thread_index) const;

  vm_thread &make_thread(size_t stack_size = 8192);
  vm_thread &fork_thread(vm_thread const &thread);

};

