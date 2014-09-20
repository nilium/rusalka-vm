/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include "vm_value.h"
#include <ios>
#include <iomanip>
#include <cmath>


static constexpr vm_value::value_type max_type(vm_value::value_type lhs, vm_value::value_type rhs)
{
  return lhs >= rhs ? lhs : rhs;
}


std::ostream &operator << (std::ostream &out, vm_value v)
{
  return
    out
    << "<"
    << std::dec << v.f64() << ", "
    << std::hex << std::showbase
    << v.i64()
    << std::dec << std::noshowbase
    << ">";
}


double vm_value::f64() const
{
  switch (type) {
  case UNSIGNED: return static_cast<double>(u64_);
  case SIGNED: return static_cast<double>(s64_);
  case FLOAT: return f64_;
  }
}


int64_t vm_value::i64() const
{
  switch (type) {
  case UNSIGNED: return static_cast<int64_t>(u64_);
  case SIGNED: return s64_;
  case FLOAT: return static_cast<int64_t>(f64_);
  }
}


uint64_t vm_value::ui64() const
{
  switch (type) {
  case UNSIGNED: return u64_;
  case SIGNED: return static_cast<uint64_t>(s64_);
  case FLOAT: return static_cast<uint64_t>(f64_);
  }
}


vm_value vm_value::as(value_type new_type) const {
  switch (new_type) {

  case SIGNED: // want signed
    switch (type) {
    case UNSIGNED: return vm_value { static_cast<uint64_t>(s64_) };
    case SIGNED:   return *this;
    case FLOAT:    return vm_value { static_cast<double>(s64_) };
    }

  case UNSIGNED:
    switch (type) {
    case UNSIGNED: return *this;
    case SIGNED:   return vm_value { static_cast<int64_t>(u64_) };
    case FLOAT:    return vm_value { static_cast<uint64_t>(u64_) };
    }

  case FLOAT:
    switch (type) {
    case UNSIGNED: return vm_value { static_cast<double>(u64_) };
    case SIGNED:   return vm_value { static_cast<double>(s64_) };
    case FLOAT:    return *this;
    }

  } // new_type
}


vm_value &vm_value::convert(value_type new_type)
{
  if (type != new_type) {
    *this = as(new_type);
  }
  return *this;
}


auto vm_value::fcmp(vm_value other, double epsilon) const -> fcmp_result {
  double const alpha = as(FLOAT).f64_ - other.convert(FLOAT).f64_;
  if (alpha > epsilon) {
    return GREATER;
  } else if (alpha >= -epsilon) {
    return EQUAL;
  } else {
    return LESS;
  }
}


auto vm_value::fcmp(vm_value other) const -> fcmp_result {
  static constexpr double epsilon = vm_value::EPSILON;
  double const alpha = as(FLOAT).f64_ - other.convert(FLOAT).f64_;
  if (alpha > epsilon) {
    return GREATER;
  } else if (alpha >= -epsilon) {
    return EQUAL;
  } else {
    return LESS;
  }
}


vm_value vm_value::operator + (vm_value rhs) const
{
  vm_value lhs = *this;
  lhs += rhs;
  return lhs;
}


vm_value vm_value::operator - (vm_value rhs) const
{
  vm_value lhs = *this;
  lhs -= rhs;
  return lhs;
}


vm_value vm_value::operator * (vm_value rhs) const
{
  vm_value lhs = *this;
  lhs *= rhs;
  return lhs;
}


vm_value vm_value::operator % (vm_value rhs) const
{
  vm_value lhs = *this;
  lhs %= rhs;
  return lhs;
}


vm_value vm_value::operator & (vm_value rhs) const
{
  vm_value lhs = *this;
  lhs &= rhs;
  return lhs;
}


vm_value vm_value::operator | (vm_value rhs) const
{
  vm_value lhs = *this;
  lhs |= rhs;
  return lhs;
}


vm_value vm_value::operator ^ (vm_value rhs) const
{
  vm_value lhs = *this;
  lhs ^= rhs;
  return lhs;
}


vm_value vm_value::operator << (vm_value rhs) const
{
  vm_value lhs = *this;
  lhs <<= rhs;
  return lhs;
}


vm_value vm_value::operator >> (vm_value rhs) const
{
  vm_value lhs = *this;
  lhs >>= rhs;
  return lhs;
}


vm_value &vm_value::operator += (vm_value rhs)
{
  value_type new_type = std::max(type, rhs.type);
  convert(new_type);
  rhs.convert(new_type);
  switch (new_type) {
  case UNSIGNED: u64_ += rhs.u64_; break;
  case SIGNED:   s64_ += rhs.s64_; break;
  case FLOAT:    f64_ += rhs.f64_; break;
  }
  return *this;
}


vm_value &vm_value::operator -= (vm_value rhs)
{
  value_type new_type = std::max(type, rhs.type);
  convert(new_type);
  rhs.convert(new_type);
  switch (new_type) {
  case UNSIGNED: u64_ -= rhs.u64_; break;
  case SIGNED:   s64_ -= rhs.s64_; break;
  case FLOAT:    f64_ -= rhs.f64_; break;
  }
  return *this;
}


vm_value &vm_value::operator *= (vm_value rhs)
{
  value_type new_type = std::max(type, rhs.type);
  convert(new_type);
  rhs.convert(new_type);
  switch (new_type) {
  case UNSIGNED: u64_ *= rhs.u64_; break;
  case SIGNED:   s64_ *= rhs.s64_; break;
  case FLOAT:    f64_ *= rhs.f64_; break;
  }
  return *this;
}


vm_value &vm_value::operator %= (vm_value rhs)
{
  value_type new_type = std::max(type, rhs.type);
  convert(new_type);
  rhs.convert(new_type);
  switch (new_type) {
  case UNSIGNED: u64_ %= rhs.u64_; break;
  case SIGNED:   s64_ %= rhs.s64_; break;
  case FLOAT:    f64_ = std::fmod(f64_, rhs.f64_); break;
  }
  return *this;
}


vm_value &vm_value::operator &= (vm_value rhs)
{
  convert(UNSIGNED);
  rhs.convert(UNSIGNED);
  u64_ &= rhs.u64_;
  return *this;
}


vm_value &vm_value::operator |= (vm_value rhs)
{
  convert(UNSIGNED);
  rhs.convert(UNSIGNED);
  u64_ |= rhs.u64_;
  return *this;
}


vm_value &vm_value::operator ^= (vm_value rhs)
{
  convert(UNSIGNED);
  rhs.convert(UNSIGNED);
  u64_ ^= rhs.u64_;
  return *this;
}


vm_value &vm_value::operator <<= (vm_value rhs)
{
  value_type new_type = std::min(std::min(SIGNED, rhs.type), type);
  convert(new_type);
  switch (new_type) {
  case UNSIGNED: u64_ <<= rhs.u64_; break;
  case SIGNED: s64_ <<= rhs.s64_; break;
  case FLOAT: break;
  }
  return *this;
}


vm_value &vm_value::operator >>= (vm_value rhs)
{
  value_type new_type = std::min(std::min(SIGNED, rhs.type), type);
  convert(new_type);
  rhs.convert(new_type);
  switch (new_type) {
  case UNSIGNED: u64_ >>= rhs.u64_; break;
  case SIGNED: s64_ >>= rhs.s64_; break;
  case FLOAT: break;
  }
  return *this;
}


vm_value vm_value::operator - () const
{
  switch (type) {
  case UNSIGNED: return vm_value { -u64_ };
  case SIGNED: return vm_value { -s64_ };
  case FLOAT: return vm_value { -f64_ };
  }
}


vm_value vm_value::operator ~ () const
{
  return vm_value { ~ as(UNSIGNED).u64_ };
}


bool vm_value::operator <= (vm_value rhs) const
{
  value_type req_type = max_type(type, rhs.type);
  vm_value lhs = as(req_type);
  rhs.convert(req_type);

  switch (req_type) {
  case UNSIGNED: return lhs.u64_ <= rhs.u64_;
  case SIGNED: return lhs.s64_ <= rhs.s64_;
  case FLOAT:  return lhs.f64_ <= rhs.f64_;
  }
}


bool vm_value::operator < (vm_value rhs) const
{
  value_type req_type = max_type(type, rhs.type);
  vm_value lhs = as(req_type);
  rhs.convert(req_type);

  switch (req_type) {
  case UNSIGNED: return lhs.u64_ < rhs.u64_;
  case SIGNED: return lhs.s64_ < rhs.s64_;
  case FLOAT:  return lhs.f64_ < rhs.f64_;
  }
}


bool vm_value::operator >= (vm_value rhs) const
{
  value_type req_type = max_type(type, rhs.type);
  vm_value lhs = as(req_type);
  rhs.convert(req_type);

  switch (req_type) {
  case UNSIGNED: return lhs.u64_ >= rhs.u64_;
  case SIGNED: return lhs.s64_ >= rhs.s64_;
  case FLOAT:  return lhs.f64_ >= rhs.f64_;
  }
}


bool vm_value::operator > (vm_value rhs) const
{
  value_type req_type = max_type(type, rhs.type);
  vm_value lhs = as(req_type);
  rhs.convert(req_type);

  switch (req_type) {
  case UNSIGNED: return lhs.u64_ > rhs.u64_;
  case SIGNED: return lhs.s64_ > rhs.s64_;
  case FLOAT:  return lhs.f64_ > rhs.f64_;
  }
}


bool vm_value::operator ! () const
{
  return u64_ == 0;
}


vm_value::operator bool () const
{
  return u64_ != 0;
}
