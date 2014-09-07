/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stdexcept>
#include <cerrno>
#include <cstring>

#include "vm_rwlock.h"


vm_rwlock::vm_rwlock()
: inited_(false)
, invalid_(false)
{
  ensure_init();
}


vm_rwlock::~vm_rwlock()
{
  destroy();
}


bool vm_rwlock::lock_read()
{
  validate();
  if (inited_) {
    int const err = pthread_rwlock_rdlock(&lock_);
    if (err != 0) {
      throw std::runtime_error(std::strerror(err));
    }
    return true;
  } else {
    throw std::runtime_error("Lock is not initialized");
  }
}


bool vm_rwlock::lock_write()
{
  validate();
  if (inited_) {
    int const err = pthread_rwlock_wrlock(&lock_);
    if (err != 0) {
      throw std::runtime_error(std::strerror(err));
    }
    return true;
  } else {
    throw std::runtime_error("Lock is not initialized");
  }
}


bool vm_rwlock::unlock() noexcept
{
  if (!invalid_ && inited_) {
    return pthread_rwlock_unlock(&lock_) == 0;
  }
  return false;
}


void vm_rwlock::ensure_init()
{
  validate();

  if (!inited_) {
    int const err = pthread_rwlock_init(&lock_, NULL);
    if (err != 0 && err != EAGAIN) {
      throw std::runtime_error(std::strerror(err));
    }
    inited_ = true;
  }
}


void vm_rwlock::destroy() noexcept
{
  if (!invalid_ && inited_) {
    int err = pthread_rwlock_destroy(&lock_);

    while (err != 0 && err == EBUSY) {
      err = pthread_rwlock_wrlock(&lock_); // ignore EDEADLK
      if (err == EINVAL) {
        invalid_ = true;
        return;
      }

      err = pthread_rwlock_unlock(&lock_);
      if (err == 0) {
        err = pthread_rwlock_destroy(&lock_);
      }
    }

    inited_ = false;
  }
}


void vm_rwlock::validate()
{
  if (invalid_) {
    throw std::runtime_error("Lock is no longer valid (may have been moved or destroyed)");
  }
}

