/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include <cfenv>
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>

#include "vm_exception.h"
#include "vm_state.h"
#include "hash.h"


// Null memory block.
vm_state::memblock const vm_state::NO_BLOCK {
  0,       // size
  0,       // flags
  nullptr  // block
};



/**
 * Invokes the defined callback with the given parameters.
 *
 * If argc is <= 0 or argv is null, argv is replaced with a single undefined
 * value and argc is forced to 0.
 */
vm_value vm_state::callback_info::invoke(vm_thread &thread, int argc, vm_value const *argv) const
{
  if (!callback) {
    return vm_value::undefined();
  }

  vm_value const no_args = vm_value::undefined();

  if (argv == nullptr || argc <= 0) {
    argv = &no_args;
    argc = 0;
  }

  return callback(thread, argc, argv, context);
}



/**
 * Destructor.
 */
vm_state::~vm_state()
{
  release_all_memblocks();
}



/**
 * Set the VM's unit to a copy of the given unit.
 */
void vm_state::set_unit(vm_unit const &unit)
{
  reset_state();
  _unit = unit;
  prepare_unit();
}



/**
 * Pass ownership of the given unit's data to the VM. This invalidates the
 * original unit.
 */
void vm_state::set_unit(vm_unit &&unit)
{
  reset_state();
  _unit = std::forward<vm_unit &&>(unit);
  prepare_unit();
}



/**
 * Resets basic VM state to nothing. Doesn't touch the unit, but does require
 * a unit to be re-assigned (so that the unit can be prepared). set_unit calls
 * this to reset anything set by a previous unit.
 *
 * It is an error to call this (or otherwise change units) when there are live
 * VM threads.
 */
void vm_state::reset_state()
{
  if (_threads.size() > 0) {
    throw new vm_logic_error("Attempt to reset state with loaded VM threads.");
  }

  release_all_memblocks();
  _block_counter = 1;

  _source_size = 0;
  _callbacks.resize(0);
}



/**
 * Prepares the VM for its current unit by allocating static memory blocks and
 * resizing the callback vector to hold as many callbacks as are needed.
 */
void vm_state::prepare_unit()
{
  _source_size = _unit.instructions.size();

  _callbacks.resize(_unit.imports.size());
  std::fill(_callbacks.begin(), _callbacks.end(), callback_info { nullptr, nullptr });

  vm_unit::data_id_ary_t new_ids;
  new_ids.resize(_unit._data_blocks.size(), 0);

  _unit.each_data([&](int64_t index, int64_t id, int64_t size, void const *ptr, bool &stop) {
    int64_t new_id = realloc_block_with_flags(VM_NULL_BLOCK, size, VM_MEM_SOURCE_DATA);
    auto found = get_block_info(new_id);
    if (!found.ok) {
      return;
    }

    std::memcpy(found.value.block, ptr, size);
    new_ids[index] = new_id;
  });

  _unit.relocate_static_data(new_ids);
}



/**
 * Binds a predefined named callback to a function.
 * @param  name     The callback name.
 * @param  length   The length of the name string.
 * @param  function The function to bind the callback name to.
 * @param  context  An opaque context pointer that will always be passed to the
 *   callback function.
 * @return          A result indicating whether the function was bound or not.
 *   If the result's first member (bool) is true, the second (int64_t) is set
 *   to the callback's instruction pointer (a negative value for callback
 *   functions).
 */
vm_bound_fn_t vm_state::bind_callback(const char *name, int length, vm_callback_t *function, void *context)
{
  uint64_t name_key = hash64(name, static_cast<size_t>(length));
  auto imported = _unit.imports.find(name_key);
  if (imported != _unit.imports.cend()) {
    const int64_t idx = -(imported->second + 1);
    _callbacks.at(idx) = callback_info { function, context };
    return vm_bound_fn_t { true, imported->second };
  }

  return  vm_bound_fn_t { false, 0 };
}



/**
 * Binds a predefined name callback to a function. Length of name is determined
 * using std::strlen.
 * @param  name     The callback name.
 * @param  function The function to bind the callback name to.
 * @param  context  An opaque context pointer that will always be passed to the
 *   callback function.
 * @return          A result indicating whether the function was bound or not.
 *   If the result's first member (bool) is true, the second (int64_t) is set
 *   to the callback's instruction pointer (a negative value for callback
 *   functions).
 */
vm_bound_fn_t vm_state::bind_callback(const char *name, vm_callback_t *function, void *context)
{
  return bind_callback(name, std::strlen(name), function, context);
}


/**
 * Releases all non-static memory allocated by the VM.
 */
void vm_state::release_all_memblocks() noexcept
{
  for (auto kvpair : _blocks) {
    if (!(kvpair.second.flags & VM_MEM_STATIC) && kvpair.second.block) {
      std::free(kvpair.second.block);
    }
  }
  _blocks.clear();
}



/**
 * Yields an unused block ID. Subsequent calls return the same value unless the
 * ID is consumed by the VM memory allocator.
 *
 * Returns zero (0) upon error.
 */
int64_t vm_state::unused_block_id()
{
  auto end = _blocks.end();
  auto const start_counter = _block_counter;
  while (_blocks.find(_block_counter) != end || _block_counter == 0) {
    ++_block_counter;
    if (_block_counter == start_counter) {
      return 0;
    }
  }
  return _block_counter;
}



/**
 * Reallocates a block ID with the given flags. The block may be zero, in which
 * case the result is a new block.
 * @param  block_id The block ID to reallocate. May be 0. If block_id points to
 *   a static memory block, a new block ID will be allocated. For other blocks,
 *   the block ID may be reused.
 * @param  size     The size of the block to allocate. Must be greater than zero.
 * @param  flags    Permissions to allocate the block with.
 *   May not be VM_MEM_NO_PERMISSIONS.
 * @return          The block ID for the reallocated block. May be the same as
 *   the input ID or a new ID.
 */
int64_t vm_state::realloc_block_with_flags(int64_t block_id, int64_t size, uint32_t flags)
{
  void *src = nullptr;

  if (size <= 0) {
    throw new vm_logic_error("Attempt to allocate block with size <= 0.");
  } else if (flags == VM_MEM_NO_PERMISSIONS) {
    throw new vm_memory_permission_error("Attempt to allocate block with no set permissions.");
  }

  if (block_id != 0) {
    memblock_map_t::iterator iter = _blocks.find(block_id);

    if (iter == _blocks.cend()) {
      throw vm_memory_access_error("No block found for given block_id");
    }

    src = iter->second.block;

    if (iter->second.flags & VM_MEM_STATIC) {
      throw vm_memory_permission_error("Attempt to reallocate static memory block.");
    }
  } else {
    block_id = unused_block_id();
  }

  memblock block {
    size,
    flags,
    std::realloc(src, static_cast<size_t>(size))
  };

  if (block.block == nullptr) {
    throw vm_memory_access_error("Unable to reallocate block");
  }

  _blocks[block_id] = block;
  return block_id;
}



/**
 * Simplified realloc_block function. Given a block ID, reallocates the block
 * (or for block_id 0, allocates a new block) of the given size (> 0).
 */
int64_t vm_state::realloc_block(int64_t block_id, int64_t size)
{
  return realloc_block_with_flags(block_id, size, VM_MEM_WRITABLE | VM_MEM_READABLE);
}



/**
 * Given a block ID, allocates a new block with read-write permissions and
 * copies all data from block_id to the new block. Returns the newly allocated
 * block ID.
 * @param  block_id The block to copy.
 * @return          The new block ID. Returns 0 if block_id does not exist or
 *   could not be copied.
 */
int64_t vm_state::duplicate_block(int64_t block_id)
{
  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    const auto entry = iter->second;
    if (entry.flags & VM_MEM_READABLE) {
      int64_t new_block_id = realloc_block(VM_NULL_BLOCK, entry.size);
      void *new_block = get_block(new_block_id, VM_MEM_WRITABLE);
      std::memcpy(new_block, entry.block, entry.size);
      return new_block_id;
    }
  }
  return 0;
}



/**
 * Returns the size of the given block_id, or 0 if the block does not exist.
 */
int64_t vm_state::block_size(int64_t block_id) const
{
  if (block_id == 0) {
    return 0;
  }

  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    return iter->second.size;
  }
  return 0;
}



/**
 * Frees the given block_id and any memory held by it. It is an error to free a
 * non-zero block ID that does not exist or a static block ID. Freeing block ID
 * 0 is a no-op.
 */
void vm_state::free_block(int64_t block_id)
{
  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter == _blocks.cend()) {
    throw vm_memory_access_error("Attempt to free nonexistent block");
  } else if (iter->second.flags & VM_MEM_STATIC) {
    throw vm_memory_permission_error("Attempt to free static memory block");
  }

  std::free(iter->second.block);
  _blocks.erase(iter);
}



/**
 * Attempts to get info for the given block ID.
 */
auto vm_state::get_block_info(int64_t block_id) const -> found_memblock_t {
  auto const block_iter = _blocks.find(block_id);
  if (block_iter == _blocks.end()) {
    return { false, NO_BLOCK };
  }
  return { true, block_iter->second };
}



/**
 * Looks up a block by its ID, returning it if it's requested with adequate
 * permissions. It's an error to request a block without adequate permissions
 * (e.g., requesting a static block with write permissions is an error).
 * Requesting a block and specifying no permissions is also an error, as no
 * block will match this.
 *
 * Returns nullptr for blocks that are not found and block ID 0.
 */
void *vm_state::get_block(int64_t block_id, uint32_t permissions)
{
  if (permissions == VM_MEM_NO_PERMISSIONS) {
    throw vm_memory_permission_error("No permissions provided -- request is impossible");
  } else if (block_id == 0) {
    return nullptr;
  }

  auto found_block = get_block_info(block_id);
  if (!found_block.ok) {
    return nullptr;
  } else if (!(found_block.value.flags & permissions)) {
    throw vm_memory_permission_error("Attempt to access block with inadequate permissions");
  }

  return found_block.value.block;
}



/**
 * Const form of get_block.
 */
const void *vm_state::get_block(int64_t block_id, uint32_t permissions) const
{
  if (permissions == VM_MEM_NO_PERMISSIONS) {
    throw vm_memory_permission_error("No permissions provided -- request is impossible");
  } else if (block_id == 0) {
    return nullptr;
  }

  auto found_block = get_block_info(block_id);
  if (!found_block.ok) {
    return nullptr;
  } else if (!(found_block.value.flags & permissions)) {
    throw vm_memory_permission_error("Attempt to access block with inadequate permissions");
  }

  return found_block.value.block;
}



/**
 * Looks up a function pointer ID by name.
 */
vm_found_fn_t vm_state::find_function_pointer(const char *name) const
{
  uint64_t name_key = hash64(name, std::strlen(name));
  vm_unit::label_table_t::const_iterator iter = _unit.imports.find(name_key);
  if (iter == _unit.imports.cend() &&
      (iter = _unit.exports.find(name_key)) == _unit.exports.cend()) {
    return vm_found_fn_t { false, 0 };
  }
  return vm_found_fn_t { true, iter->second };
}



/**
 * Checks whether an offset with a size are within a block's bounds. Always
 * returns true for a size and offset of zero.
 */
bool vm_state::check_block_bounds(int64_t block_id, int64_t offset, int64_t size) const
{
  int64_t const bsize = block_size(block_id);
  int64_t const end = offset + size;

  return
    offset >= 0 &&
    size >= 0 &&
    size <= bsize &&
    end <= bsize &&
    end >= offset;
}



/**
 * Given a thread, takes ownership of its resources and adds it to the VM. The
 * original thread object is invalidated.
 */
void vm_state::load_thread(thread_pointer_t &&thread)
{
  auto const end = _threads.end();
  auto first_null = std::find(_threads.begin(), end, nullptr);

  if (first_null == end) {
    _threads.emplace_back(std::move(thread));
  } else {
    *first_null = std::move(thread);
  }
}



/**
 * Destroys the thread at the given index.
 */
void vm_state::destroy_thread(int64_t index)
{
  _threads.at(index).reset(nullptr);
}



/**
 * Gets a thread by its index.
 */
vm_thread &vm_state::thread_by_index(int64_t thread_index)
{
  return *_threads[thread_index];
}



/**
 * Const form of thread_by_index.
 */
vm_thread const &vm_state::thread_by_index(int64_t thread_index) const
{
  return *_threads[thread_index];
}



/**
 * Allocates a new thread with a given initial stack size.
 */
vm_thread &vm_state::make_thread(size_t stack_size)
{
  thread_pointer_t ptr { new vm_thread(*this, stack_size) };
  vm_thread *raw = ptr.get();
  load_thread(std::move(ptr));
  return *raw;
}



/**
 * Forks a given thread, returning a reference to the new thread. The returned
 * thread may be run to resume execution from where it left off.
 */
vm_thread &vm_state::fork_thread(vm_thread const &thread)
{
  if (&thread._process != this) {
    throw vm_wrong_process("Thread process doesn't match this process.");
  }

  thread_pointer_t ptr { new vm_thread(thread) };
  vm_thread *raw = ptr.get();
  load_thread(std::move(ptr));
  return *raw;
}
