/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <pthread.h>


class vm_rwlock
{
  bool              inited_;
  bool              invalid_;
  pthread_rwlock_t  lock_;

  void ensure_init();
  void destroy() noexcept;
  void validate();

public:
  class wr_lockable;
  class rd_lockable;

  vm_rwlock();
  ~vm_rwlock();

  vm_rwlock(vm_rwlock const &) = delete;
  vm_rwlock(vm_rwlock &&) = delete;

  vm_rwlock &operator = (vm_rwlock const &) = delete;
  vm_rwlock &operator = (vm_rwlock &&) = delete;

  bool lock_read();
  bool lock_write();
  bool unlock() noexcept;

  wr_lockable write_lockable() { return wr_lockable { *this }; }
  rd_lockable read_lockable() { return rd_lockable { *this }; }

  class wr_lockable
  {
    vm_rwlock &rwlock_;
  public:
    wr_lockable(vm_rwlock &rwlock) : rwlock_(rwlock) { /* nop */ }
    bool lock() { return rwlock_.lock_write(); }
    bool unlock() noexcept { return rwlock_.unlock(); }
  };


  class rd_lockable
  {
    vm_rwlock &rwlock_;
  public:
    rd_lockable(vm_rwlock &rwlock) : rwlock_(rwlock) { /* nop */ }
    bool lock() { return rwlock_.lock_read(); }
    bool unlock() noexcept { return rwlock_.unlock(); }
  };
};
