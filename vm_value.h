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

#ifndef __VM_VALUE_H__
#define __VM_VALUE_H__

#include <cstdint>
#include <iostream>


constexpr double VM_FCMP_EPSILON = 1.0e-10;


enum vm_fcmp_result_t : int
{
  VM_FCMP_LESS    = -1,
  VM_FCMP_EQUAL   = 0,
  VM_FCMP_GREATER = 1,
};


struct value_t final
{
  enum type_t {
    UNSIGNED = 0,
    SIGNED   = 1,
    FLOAT    = 2,
  } type;

  union {
    double   f64_;
    int64_t  s64_;
    uint64_t u64_;
  };

  value_t() = default;
  value_t(value_t const &v) = default;

  explicit value_t(double v)   : type(FLOAT),    f64_(v) {}
  explicit value_t(int64_t v)  : type(SIGNED),   s64_(v) {}
  explicit value_t(uint64_t v) : type(UNSIGNED), u64_(v) {}
  explicit value_t(float v)    : f64_(static_cast<double>(v)) {}
  explicit value_t(int32_t v)  : s64_(static_cast<int64_t>(v)) {}
  explicit value_t(uint32_t v) : u64_(static_cast<uint64_t>(v)) {}
  explicit value_t(int16_t v)  : s64_(static_cast<int64_t>(v)) {}
  explicit value_t(uint16_t v) : u64_(static_cast<uint64_t>(v)) {}
  explicit value_t(int8_t v)   : s64_(static_cast<int64_t>(v)) {}
  explicit value_t(uint8_t v)  : u64_(static_cast<uint64_t>(v)) {}

  double   f64() const  { return f64_; }
  float    f32() const  { return (float)f64_; }
  int64_t  i64() const  { return (int64_t)f64_; }
  uint64_t ui64() const { return (uint64_t)f64_; }
  int32_t  i32() const  { return (int32_t)f64_; }
  uint32_t ui32() const { return (uint32_t)f64_; }
  int16_t  i16() const  { return (int16_t)f64_; }
  uint16_t ui16() const { return (uint16_t)f64_; }
  int8_t   i8() const   { return (int8_t)f64_; }
  uint8_t  ui8() const  { return (uint8_t)f64_; }

  void set(double v)   { f64_ = v; }
  void set(int64_t v)  { s64_ = v; }
  void set(uint64_t v) { u64_ = v; }
  void set(float v)    { f64_ = static_cast<double>(v); }
  void set(int32_t v)  { s64_ = static_cast<int64_t>(v); }
  void set(uint32_t v) { u64_ = static_cast<uint64_t>(v); }
  void set(int16_t v)  { s64_ = static_cast<int64_t>(v); }
  void set(uint16_t v) { u64_ = static_cast<uint64_t>(v); }
  void set(int8_t v)   { s64_ = static_cast<int64_t>(v); }
  void set(uint8_t v)  { u64_ = static_cast<uint64_t>(v); }

  operator double() const   { return f64_; }
  operator int64_t() const  { return s64_; }
  operator uint64_t() const { return u64_; }
  operator float() const    { return static_cast<float>(f64_); }
  operator int32_t() const  { return static_cast<int32_t>(s64_); }
  operator uint32_t() const { return static_cast<uint32_t>(u64_); }
  operator int16_t() const  { return static_cast<int16_t>(s64_); }
  operator uint16_t() const { return static_cast<uint16_t>(u64_); }
  operator int8_t() const   { return static_cast<int8_t>(s64_); }
  operator uint8_t() const  { return static_cast<uint8_t>(u64_); }

  value_t &operator = (double v)    { set(v); return *this; }
  value_t &operator = (float v)     { set(v); return *this; }
  value_t &operator = (int64_t v)   { set(v); return *this; }
  value_t &operator = (uint64_t v)  { set(v); return *this; }
  value_t &operator = (int32_t v)   { set(v); return *this; }
  value_t &operator = (uint32_t v)  { set(v); return *this; }
  value_t &operator = (int16_t v)   { set(v); return *this; }
  value_t &operator = (uint16_t v)  { set(v); return *this; }
  value_t &operator = (int8_t v)    { set(v); return *this; }
  value_t &operator = (uint8_t v)   { set(v); return *this; }

  value_t operator + (value_t const &rhs) const;
  value_t operator - (value_t const &rhs) const;
  value_t operator * (value_t const &rhs) const;
  value_t operator % (value_t const &rhs) const;
  value_t operator & (value_t const &rhs) const;
  value_t operator | (value_t const &rhs) const;
  value_t operator ^ (value_t const &rhs) const;
  value_t operator << (value_t const &rhs) const;
  value_t operator >> (value_t const &rhs) const;

  value_t &operator += (value_t rhs);
  value_t &operator -= (value_t rhs);
  value_t &operator *= (value_t rhs);
  value_t &operator %= (value_t rhs);
  value_t &operator &= (value_t rhs);
  value_t &operator |= (value_t rhs);
  value_t &operator ^= (value_t rhs);
  value_t &operator <<= (value_t rhs);
  value_t &operator >>= (value_t rhs);

  value_t operator - () const;
  value_t operator ~ () const;
  bool    operator ! () const;
          operator bool () const;

  bool operator == (value_t other) const { return u64_ == other.u64_; }
  bool operator != (value_t other) const { return !(*this == other); }
  bool operator <= (value_t other) const { return value <= other.value; }
  bool operator < (value_t other) const { return value < other.value; }
  bool operator >= (value_t other) const { return value >= other.value; }
  bool operator > (value_t other) const { return value > other.value; }

  vm_fcmp_result_t fcmp(value_t other) const;

  value_t cast(type_t ntype) const;
  value_t &force_type(type_t ntype);

};


std::ostream &operator << (std::ostream &out, value_t v);


template <class T>
value_t make_value(T x)
{
  return value_t { x };
}


static_assert(
  std::is_standard_layout<value_t>::value,
  "value_t must be standard layout"
  );

static_assert(
  std::is_trivial<value_t>::value,
  "value_t must be trivial"
  );

static_assert(
  std::is_trivially_copyable<value_t>::value,
  "value_t must be trivially copyable"
  );

static_assert(
  std::is_trivially_assignable<value_t, value_t>::value,
  "value_t must be trivially assignable to itself"
  );

#endif /* end __VM_VALUE_H__ include guard */
