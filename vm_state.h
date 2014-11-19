/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <map>
#include <vector>

#include "_types.h"
#include "vm_unit.h"


class vm_thread;


enum vm_memblock_flags : uint32_t
{
  VM_MEM_NO_PERMISSIONS = 0x0,
  VM_MEM_READABLE       = 0x1,
  VM_MEM_WRITABLE       = 0x2,
  VM_MEM_STATIC         = 0x4,
  VM_MEM_READ_WRITE     = VM_MEM_READABLE | VM_MEM_WRITABLE,
  VM_MEM_SOURCE_DATA    = VM_MEM_STATIC | VM_MEM_READABLE,
};


constexpr int64_t VM_NULL_BLOCK = 0;


class vm_state
{
  /**
   * A memory block for a particular VM state. Only encodes the block's size,
   * pointer, and read/write flags to determine thread permissions for the
   * block.
   */
  struct memblock
  {
    /** The size, in bytes, of the memory block. */
    int64_t size;
    /**
     * Read/write flags associated with the block.
     * @see vm_memblock_flags.
     */
    uint32_t flags;
    /**
     * Read-write pointer to the memory block.
     *
     * Memory may not actually be writable, but must be when stored in the
     * memblock to quell the heart of C++.
     */
    void *block;
  };

  /**
   * Per-callback info. Stories only a VM callback and a context to pass to the
   * callback.
   */
  struct callback_info
  {
    vm_callback_t *callback;
    void *context;

    /**
     * Invokes the callback on the given thread with argc number of arguments
     * from argv.
     * @param  thread The thread to call the function on.
     * @param  argc   Number of arguments. Must be > 0.
     * @param  argv   Arguments. Must be a pointer to an array of `argc`
     *                vm_value objects if argc > 0, otherwise may be null. Its
     *                values will be copied.
     * @return        The value returned by the invoked function via the RP
     *                register.
     */
    vm_value invoke(vm_thread &thread, int argc, vm_value const *argv) const;
  };

  /** The result of searching for a memblock. */
  using found_memblock_t = vm_find_result<memblock>;
  /** A map of memblock names (integers) to their memblock info. */
  using memblock_map_t   = std::map<int64_t, memblock>;
  /** Collection used for the stack. Currently a vector of values. */
  using stack_t          = std::vector<vm_value>;
  /** Collection used for storing callback info. */
  using callbacks_t      = std::vector<callback_info>;
  /** A pointer to a thread allocated for a state. */
  using thread_pointer_t = std::unique_ptr<vm_thread>;
  /** A collection of thread pointers allocated for a state. */
  using thread_stores_t  = std::vector<thread_pointer_t>;

  /** The zero or null block constant. Has a null pointer and zero size. */
  static memblock const NO_BLOCK;

  /** All threads allocated to the state. */
  thread_stores_t _threads {};
  /** All callbacks allocated to the state. */
  callbacks_t _callbacks {};
  /** All memory blocks allocated to the state. */
  memblock_map_t _blocks {};
  /** Internal block name counter used for allocations. */
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
  vm_bound_fn_t bind_callback(const char *name, vm_callback_t *function, void *context = nullptr);

  vm_thread &thread_by_index(int64_t thread_index);
  vm_thread const &thread_by_index(int64_t thread_index) const;

  vm_thread &make_thread(size_t stack_size = 8192);
  vm_thread &fork_thread(vm_thread const &thread);

};
