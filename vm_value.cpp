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
#include <limits>
#include <cmath>


std::ostream &operator << (std::ostream &out, vm_value v)
{
    out << "(#";

    switch (v.type) {
    case vm_value::ERROR:
      out << "error: " << std::hex << std::showbase << v.u64_;
      break;

    case vm_value::UNDEFINED:
      out << "undefined";
      break;

    case vm_value::UNSIGNED:
      out << "uint: " << std::hex << std::showbase << v.u64_;
      break;

    case vm_value::SIGNED:
      out << "int: "<< std::dec << std::noshowbase << v.s64_;
      break;

    case vm_value::FLOAT:
      out << "float: " << std::dec << std::noshowbase << v.f64_;
      break;

    case vm_value::DATA:
      out << "data: " << std::dec << std::noshowbase << v.s64_;
      break;

    default:
      out << v.type << ": " << std::hex << std::showbase << v.u64_;
      break;
    }

    return out << ")";
}


vm_value vm_value::undefined()
{
  return vm_value { UNDEFINED, 0ull };
}


vm_value vm_value::error()
{
  return vm_value { ERROR, 0ull };
}


vm_value vm_value::nan()
{
  return vm_value { FLOAT, std::numeric_limits<double>::quiet_NaN() };
}


vm_value vm_value::infinity()
{
  return vm_value { FLOAT, std::numeric_limits<double>::infinity() };
}


vm_value vm_value::epsilon()
{
  return vm_value { FLOAT, std::numeric_limits<double>::epsilon() };
}


double vm_value::f64() const
{
  switch (type) {
  case UNSIGNED: return static_cast<double>(u64_);
  case DATA:
  case SIGNED: return static_cast<double>(s64_);
  default:
  case FLOAT: return f64_;
  }
}


int64_t vm_value::i64() const
{
  switch (type) {
  case UNSIGNED: return static_cast<int64_t>(u64_);
  default:
  case DATA:
  case SIGNED: return s64_;
  case FLOAT: return static_cast<int64_t>(f64_);
  }
}


uint64_t vm_value::ui64() const
{
  switch (type) {
  default:
  case UNSIGNED: return u64_;
  case DATA:
  case SIGNED: return static_cast<uint64_t>(s64_);
  case FLOAT: return static_cast<uint64_t>(f64_);
  }
}


vm_value vm_value::as(int32_t new_type) const {
  if (new_type == type) {
    return *this;
  }

  switch (new_type) {
  case SIGNED: // want signed
    switch (type) {
    case UNSIGNED: return vm_value { static_cast<uint64_t>(s64_) };
    case SIGNED:   return *this;
    case FLOAT:    return vm_value { static_cast<double>(s64_) };
    default:       return undefined();
    }

  case UNSIGNED:
    switch (type) {
    case UNSIGNED: return *this;
    case SIGNED:   return vm_value { static_cast<int64_t>(u64_) };
    case FLOAT:    return vm_value { static_cast<uint64_t>(u64_) };
    default:       return undefined();
    }

  case FLOAT:
    switch (type) {
    case UNSIGNED: return vm_value { static_cast<double>(u64_) };
    case SIGNED:   return vm_value { static_cast<double>(s64_) };
    case FLOAT:    return *this;
    default:       return undefined();
    }

  default:
    return undefined();
  } // new_type
}


vm_value &vm_value::convert(int32_t new_type)
{
  if (type != new_type) {
    *this = as(new_type);
  }
  return *this;
}


auto vm_value::fcmp(vm_value other, double epsilon) const -> fcmp_result {
  vm_value const self = as(FLOAT);
  other.convert(FLOAT);
  if (self.type != FLOAT || other.type != FLOAT) {
    return LESS;
  }

  double const alpha = self.f64_ - other.f64_;

  if (alpha > epsilon) {
    return GREATER;
  } else if (alpha >= -epsilon) {
    return EQUAL;
  } else {
    return LESS;
  }
}


auto vm_value::fcmp(vm_value other) const -> fcmp_result {
  // Note: this currently just aliases fcmp(other, epsilon) but may be replaced
  // down the line with something that can be inlined more easily.
  return fcmp(other, vm_value::EPSILON);
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
  auto new_type = std::max(type, rhs.type);
  convert(new_type);
  rhs.convert(new_type);
  switch (new_type) {
  case UNSIGNED: u64_ += rhs.u64_; break;
  case SIGNED:   s64_ += rhs.s64_; break;
  case FLOAT:    f64_ += rhs.f64_; break;
  default: break;
  }
  return *this;
}


vm_value &vm_value::operator -= (vm_value rhs)
{
  auto new_type = std::max(type, rhs.type);
  convert(new_type);
  rhs.convert(new_type);
  switch (new_type) {
  case UNSIGNED: u64_ -= rhs.u64_; break;
  case SIGNED:   s64_ -= rhs.s64_; break;
  case FLOAT:    f64_ -= rhs.f64_; break;
  default: break;
  }
  return *this;
}


vm_value &vm_value::operator *= (vm_value rhs)
{
  auto new_type = std::max(type, rhs.type);
  convert(new_type);
  rhs.convert(new_type);
  switch (new_type) {
  case UNSIGNED: u64_ *= rhs.u64_; break;
  case SIGNED:   s64_ *= rhs.s64_; break;
  case FLOAT:    f64_ *= rhs.f64_; break;
  default: break;
  }
  return *this;
}


vm_value &vm_value::operator %= (vm_value rhs)
{
  auto new_type = std::max(type, rhs.type);
  convert(new_type);
  rhs.convert(new_type);
  switch (new_type) {
  case UNSIGNED: u64_ %= rhs.u64_; break;
  case SIGNED:   s64_ %= rhs.s64_; break;
  case FLOAT:    f64_ = std::fmod(f64_, rhs.f64_); break;
  default: break;
  }
  return *this;
}


vm_value &vm_value::operator &= (vm_value rhs)
{
  convert(UNSIGNED);
  rhs.convert(UNSIGNED);
  if (type == UNSIGNED) {
    u64_ &= rhs.u64_;
  }
  return *this;
}


vm_value &vm_value::operator |= (vm_value rhs)
{
  convert(UNSIGNED);
  rhs.convert(UNSIGNED);
  if (type == UNSIGNED) {
    u64_ |= rhs.u64_;
  }
  return *this;
}


vm_value &vm_value::operator ^= (vm_value rhs)
{

  auto new_type = std::min(
    std::min(type, static_cast<int32_t>(SIGNED)),
    std::max(static_cast<int32_t>(UNSIGNED), rhs.type)
  );

  convert(new_type);
  rhs.convert(new_type);

  if (type == UNSIGNED || type == SIGNED) {
    u64_ ^= rhs.u64_;
  }
  return *this;
}


vm_value &vm_value::operator <<= (vm_value rhs)
{
  auto new_type = std::min(
    std::min(type, static_cast<int32_t>(SIGNED)),
    std::max(static_cast<int32_t>(UNSIGNED), rhs.type)
  );

  convert(new_type);
  rhs.convert(new_type);

  if (type == rhs.type) {
    switch (type) {
    case UNSIGNED: u64_ <<= rhs.u64_; break;
    case SIGNED: s64_ <<= rhs.s64_; break;
    default: break;
    }
  } else {
    convert(UNDEFINED);
  }
  return *this;
}


vm_value &vm_value::operator >>= (vm_value rhs)
{
  auto new_type = std::min(
    std::min(type, static_cast<int32_t>(SIGNED)),
    std::max(static_cast<int32_t>(UNSIGNED), rhs.type)
  );

  convert(new_type);
  rhs.convert(new_type);

  if (type == rhs.type) {
    switch (type) {
    case UNSIGNED: u64_ >>= rhs.u64_; break;
    case SIGNED: s64_ >>= rhs.s64_; break;
    default: break;
    }
  } else {
    convert(UNDEFINED);
  }
  return *this;
}


vm_value vm_value::operator - () const
{
  switch (type) {
  case UNSIGNED: return vm_value { -u64_ };
  case SIGNED: return vm_value { -s64_ };
  case FLOAT: return vm_value { -f64_ };
  default: return vm_value::undefined();
  }
}


vm_value vm_value::operator ~ () const
{
  switch (type) {
  case SIGNED:
  case UNSIGNED:
  case FLOAT:
    return vm_value { ~ as(UNSIGNED).u64_ };

  default:
    return vm_value::undefined();
  }
}


template <template <typename T> class Predicate, bool is_equality_test>
static bool logical_compare_value(vm_value lhs, vm_value rhs)
{
  int32_t const min_type = std::min(lhs.type, rhs.type);

  if (min_type < vm_value::MIN_COMPARABLE) {
    return false;
  }

  // Values after arithmetic types can only be tested for equality
  int32_t const max_type = std::max(lhs.type, rhs.type);

  if (vm_value::MAX_ARITHMETIC < max_type && !is_equality_test) {
    return false;
  }

  switch (max_type) {
  default:
  case vm_value::UNSIGNED: return (Predicate<uint64_t>{})(lhs.u64_, rhs.u64_);
  case vm_value::SIGNED:   return (Predicate<int64_t>{})(lhs.s64_, rhs.s64_);
  case vm_value::FLOAT:    return (Predicate<double>{})(lhs.f64_, rhs.f64_);
  }
}


bool vm_value::operator <= (vm_value rhs) const
{
  return logical_compare_value<std::less_equal, false>(*this, rhs);
}


bool vm_value::operator < (vm_value rhs) const
{
  return logical_compare_value<std::less, false>(*this, rhs);
}


bool vm_value::operator >= (vm_value rhs) const
{
  return logical_compare_value<std::greater_equal, false>(*this, rhs);
}


bool vm_value::operator > (vm_value rhs) const
{
  return logical_compare_value<std::greater, false>(*this, rhs);
}


bool vm_value::operator == (vm_value rhs) const
{
  return logical_compare_value<std::equal_to, true>(*this, rhs);
}


bool vm_value::operator != (vm_value rhs) const
{
  return logical_compare_value<std::not_equal_to, true>(*this, rhs);
}


bool vm_value::operator == (std::nullptr_t) const
{
  return type >= MIN_COMPARABLE && type != FLOAT && u64_ == 0;
}


bool vm_value::operator != (std::nullptr_t) const
{
  return type >= MIN_COMPARABLE && type != FLOAT && u64_ != 0;
}


bool vm_value::operator ! () const
{
  switch (type) {
  case FLOAT:
    return !f64_;
  case UNDEFINED:
  case ERROR:
    return true;
  default:
    return u64_ == 0ull;
  }
}


vm_value::operator bool () const
{
  switch (type) {
  case FLOAT:
    return !!f64_;
  case UNDEFINED:
  case ERROR:
    return false;
  default:
    return u64_ != 0ull;
  }
}


bool vm_value::is_nan() const
{
  return type == FLOAT && std::isnan(f64_);
}


bool vm_value::is_infinity() const
{
  return type == FLOAT && std::isinf(f64_);
}
