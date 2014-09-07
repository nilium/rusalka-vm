/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include <cfenv>
#include <cmath>
#include <iostream>
#include <iomanip>

#include "vm_exception.h"
#include "vm_state.h"


/*
  TODO: handle proper stack unwinding on trap somehow. Currently, it can really
  screw with the VM if there are nested run() calls (e.g., an imported function
  calls back into the vm before returning).

  Could be done by throwing an object that the VM can catch and use to unwind.
  Maybe a counter that's decremented and re-thrown to handle each nesting.

  Not sure if it would be possible to then continue execution normally. I
  haven't decided yet if trap should be the equivalent of yielding to the host
  or if it should be for signalling that the VM cannot continue to run due to
  some error. In the latter case, it might be a good idea to completely reset
  the stack and all registers and then return the trapped value.

  Don't know -- for now, the TRAP instruction is iffy and kind of just grinds
  the VM to a halt and makes it hard to recover.
*/


vm_state::memblock const vm_state::NO_BLOCK { 0, 0, nullptr };



vm_state::~vm_state()
{
  release_all_memblocks();
}



void vm_state::set_unit(vm_unit const &unit)
{
  _unit = unit;
  prepare_unit();
}



void vm_state::set_unit(vm_unit &&unit)
{
  _unit = std::forward<vm_unit &&>(unit);
  prepare_unit();
}



void vm_state::prepare_unit()
{
  release_all_memblocks();
  _block_counter = 1;
  _source_size = _unit.instructions.size();
  _callbacks.resize(_unit.imports.size());
  std::fill(_callbacks.begin(), _callbacks.end(), callback_info { nullptr, nullptr });
  vm_unit::data_id_ary_t new_ids;
  new_ids.resize(_unit._data_blocks.size(), 0);
  _unit.each_data([&](int32_t index, int32_t id, int32_t size, void const *ptr, bool &stop) {
    int32_t new_id = realloc_block_with_flags(VM_NULL_BLOCK, size, VM_MEM_SOURCE_DATA);
    auto found = get_block_info(new_id);
    if (!found.ok) {
      return;
    }

    std::memcpy(found.value.block, ptr, size);
    new_ids[index] = new_id;
  });
  _unit.relocate_static_data(new_ids);
}



vm_bound_fn_t vm_state::bind_callback(const char *name, int length, vm_callback_t *function, void *context)
{
  std::string name_str(name, length);
  auto imported = _unit.imports.find(name_str);
  if (imported != _unit.imports.cend()) {
    const int32_t idx = -(imported->second + 1);
    _callbacks.at(idx) = callback_info { function, context };
    return vm_bound_fn_t { true, imported->second };
  }

  return  vm_bound_fn_t { false, 0 };
}



void vm_state::release_all_memblocks() noexcept
{
  for (auto kvpair : _blocks) {
    if (!(kvpair.second.flags & VM_MEM_STATIC) && kvpair.second.block) {
      std::free(kvpair.second.block);
    }
  }
  _blocks.clear();
}



int32_t vm_state::unused_block_id()
{
  auto end = _blocks.end();
  while (_blocks.find(_block_counter) != end || _block_counter == 0) {
    ++_block_counter;
  }
  return _block_counter++;
}



int32_t vm_state::realloc_block_with_flags(int32_t block_id, int32_t size, uint32_t flags)
{
  void *src = nullptr;
  if (block_id != 0) {
    memblock_map_t::iterator iter = _blocks.find(block_id);

    if (iter == _blocks.cend()) {
      throw vm_memory_access_error("No block found for given block_id");
    }

    src = iter->second.block;
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



int32_t vm_state::realloc_block(int32_t block_id, int32_t size)
{
  return realloc_block_with_flags(block_id, size, VM_MEM_WRITABLE | VM_MEM_READABLE);
}



int32_t vm_state::duplicate_block(int32_t block_id)
{
  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter != _blocks.cend()) {
    const auto entry = iter->second;
    if (entry.flags & VM_MEM_READABLE) {
      int32_t new_block_id = realloc_block(VM_NULL_BLOCK, entry.size);
      void *new_block = get_block(new_block_id, VM_MEM_WRITABLE);
      std::memcpy(new_block, entry.block, entry.size);
      return new_block_id;
    }
  }
  return 0;
}



int32_t vm_state::block_size(int32_t block_id) const
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



void vm_state::free_block(int32_t block_id)
{
  if (block_id == 0) {
    throw vm_null_access_error("Attempt to free null block");
  }

  memblock_map_t::const_iterator iter = _blocks.find(block_id);
  if (iter == _blocks.cend()) {
    throw vm_memory_access_error("Attempt to free nonexistent block");
  } else if (iter->second.flags & VM_MEM_STATIC) {
    throw vm_memory_permission_error("Attempt to free static memory block");
  }

  std::free(iter->second.block);
  _blocks.erase(iter);
}



auto vm_state::get_block_info(int32_t block_id) const -> found_memblock_t {
  auto const block_iter = _blocks.find(block_id);
  if (block_iter == _blocks.end()) {
    return { false, NO_BLOCK };
  }
  return { true, block_iter->second };
}



void *vm_state::get_block(int32_t block_id, uint32_t permissions)
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



const void *vm_state::get_block(int32_t block_id, uint32_t permissions) const
{
  if (permissions == VM_MEM_NO_PERMISSIONS) {
    throw vm_memory_permission_error("No permissions provided -- request is impossible");
  } else if (block_id == 0) {
    return nullptr;
  }

  auto block = _blocks.at(block_id);
  if (!(block.flags & permissions)) {
    throw vm_memory_permission_error("Attempt to access block with inadequate permissions");
  }

  return block.block;
}



vm_found_fn_t vm_state::find_function_pointer(const char *name) const
{
  const std::string str_name((name));
  vm_unit::label_table_t::const_iterator iter = _unit.imports.find(name);
  if (iter == _unit.imports.cend() &&
      (iter = _unit.exports.find(name)) == _unit.exports.cend()) {
    return vm_found_fn_t { false, 0 };
  }
  return vm_found_fn_t { true, iter->second };
}



bool vm_state::check_block_bounds(int32_t block_id, int32_t offset, int32_t size) const
{
  int32_t const bsize = block_size(block_id);
  int32_t const end = offset + size;

  return
    offset >= 0 &&
    size >= 0 &&
    size <= bsize &&
    end <= bsize &&
    end >= offset;
}



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



void vm_state::destroy_thread(int32_t index)
{
  _threads.at(index).reset(nullptr);
}



vm_thread &vm_state::thread_by_index(int32_t thread_index)
{
  return *_threads[thread_index];
}



vm_thread const &vm_state::thread_by_index(int32_t thread_index) const
{
  return *_threads[thread_index];
}



vm_thread &vm_state::make_thread(size_t stack_size)
{
  thread_pointer_t ptr { new vm_thread(*this, stack_size) };
  vm_thread *raw = ptr.get();
  load_thread(std::move(ptr));
  return *raw;
}



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
