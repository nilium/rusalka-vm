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

#include "vm_value.h"
#include <ios>
#include <iomanip>
#include <cmath>


std::ostream &operator << (std::ostream &out, value_t v)
{
  return
    out
    << "<"
    << std::dec << v.f64() << ", "
    << std::hex << std::showbase
    << v.i32()
    << std::dec << std::noshowbase
    << ">";
}


value_t value_t::cast(type_t ntype) const {
  switch (ntype) {
  case SIGNED:
    switch (type) {
    case UNSIGNED:
    case SIGNED:   return *this;
    case FLOAT:    return value_t { static_cast<int64_t>(f64_) };
    }
  case UNSIGNED:
    switch (type) {
    case UNSIGNED:
    case SIGNED:   return *this;
    case FLOAT:    return value_t { static_cast<uint64_t>(f64_) };
    }
  case FLOAT:
    switch (type) {
    case UNSIGNED: return value_t { static_cast<double>(u64_) };
    case SIGNED:   return value_t { static_cast<double>(s64_) };
    case FLOAT:    return *this;
    }
  }
}


value_t &value_t::force_type(type_t ntype)
{
  if (type != ntype) {
    *this = cast(ntype);
  }
  return *this;
}


vm_fcmp_result_t value_t::fcmp(value_t other) const {
  double const alpha = cast(FLOAT).f64_ - other.force_type(FLOAT).f64_;
  if (alpha > VM_FCMP_EPSILON) {
    return VM_FCMP_GREATER;
  } else if (alpha >= -VM_FCMP_EPSILON) {
    return VM_FCMP_EQUAL;
  } else {
    return VM_FCMP_LESS;
  }
}

value_t value_t::operator + (value_t const &rhs) const
{
  value_t lhs = *this;
  lhs += rhs;
  return lhs;
}


value_t value_t::operator - (value_t const &rhs) const
{
  value_t lhs = *this;
  lhs -= rhs;
  return lhs;
}


value_t value_t::operator * (value_t const &rhs) const
{
  value_t lhs = *this;
  lhs *= rhs;
  return lhs;
}


value_t value_t::operator % (value_t const &rhs) const
{
  value_t lhs = *this;
  lhs %= rhs;
  return lhs;
}


value_t value_t::operator & (value_t const &rhs) const
{
  value_t lhs = *this;
  lhs &= rhs;
  return lhs;
}


value_t value_t::operator | (value_t const &rhs) const
{
  value_t lhs = *this;
  lhs |= rhs;
  return lhs;
}


value_t value_t::operator ^ (value_t const &rhs) const
{
  value_t lhs = *this;
  lhs ^= rhs;
  return lhs;
}


value_t value_t::operator << (value_t const &rhs) const
{
  value_t lhs = *this;
  lhs <<= rhs;
  return lhs;
}


value_t value_t::operator >> (value_t const &rhs) const
{
  value_t lhs = *this;
  lhs >>= rhs;
  return lhs;
}


value_t &value_t::operator += (value_t rhs)
{
  type_t ntype = std::max(type, rhs.type);
  force_type(ntype);
  rhs.force_type(ntype);
  switch (ntype) {
  case UNSIGNED: u64_ += rhs.u64_; break;
  case SIGNED:   s64_ += rhs.s64_; break;
  case FLOAT:    f64_ += rhs.f64_; break;
  }
  return *this;
}


value_t &value_t::operator -= (value_t rhs)
{
  type_t ntype = std::max(type, rhs.type);
  force_type(ntype);
  rhs.force_type(ntype);
  switch (ntype) {
  case UNSIGNED: u64_ -= rhs.u64_; break;
  case SIGNED:   s64_ -= rhs.s64_; break;
  case FLOAT:    f64_ -= rhs.f64_; break;
  }
  return *this;
}


value_t &value_t::operator *= (value_t rhs)
{
  type_t ntype = std::max(type, rhs.type);
  force_type(ntype);
  rhs.force_type(ntype);
  switch (ntype) {
  case UNSIGNED: u64_ *= rhs.u64_; break;
  case SIGNED:   s64_ *= rhs.s64_; break;
  case FLOAT:    f64_ *= rhs.f64_; break;
  }
  return *this;
}


value_t &value_t::operator %= (value_t rhs)
{
  type_t ntype = std::max(type, rhs.type);
  force_type(ntype);
  rhs.force_type(ntype);
  switch (ntype) {
  case UNSIGNED: u64_ %= rhs.u64_; break;
  case SIGNED:   s64_ %= rhs.s64_; break;
  case FLOAT:    f64_ = std::fmod(f64_, rhs.f64_); break;
  }
  return *this;
}


value_t &value_t::operator &= (value_t rhs)
{
  force_type(UNSIGNED);
  rhs.force_type(UNSIGNED);
  u64_ &= rhs.u64_;
  return *this;
}


value_t &value_t::operator |= (value_t rhs)
{
  force_type(UNSIGNED);
  rhs.force_type(UNSIGNED);
  u64_ |= rhs.u64_;
  return *this;
}


value_t &value_t::operator ^= (value_t rhs)
{
  force_type(UNSIGNED);
  rhs.force_type(UNSIGNED);
  u64_ ^= rhs.u64_;
  return *this;
}


value_t &value_t::operator <<= (value_t rhs)
{
  type_t ntype = std::min(std::min(SIGNED, rhs.type), type);
  force_type(ntype);
  switch (ntype) {
  case UNSIGNED: u64_ <<= rhs.u64_; break;
  case SIGNED: s64_ <<= rhs.s64_; break;
  case FLOAT: break;
  }
  return *this;
}


value_t &value_t::operator >>= (value_t rhs)
{
  type_t ntype = std::min(std::min(SIGNED, rhs.type), type);
  force_type(ntype);
  rhs.force_type(ntype);
  switch (ntype) {
  case UNSIGNED: u64_ >>= rhs.u64_; break;
  case SIGNED: s64_ >>= rhs.s64_; break;
  case FLOAT: break;
  }
  return *this;
}


value_t value_t::operator - () const
{
  switch (type) {
  case UNSIGNED: return value_t { -u64_ };
  case SIGNED: return value_t { -s64_ };
  case FLOAT: return value_t { -f64_ };
  }
}


value_t value_t::operator ~ () const
{
  return value_t { ~ cast(UNSIGNED).u64_ };
}


bool    value_t::operator ! () const
{
  return u64_ == 0;
}


        value_t::operator bool () const
{
  return u64_ != 0;
}
