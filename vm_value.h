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


struct value_t final
{
  enum : uint64_t {
    NAN_MASK             = ~0x8000000000000000ULL,
    NAN_BITS             =  0x7FF8000000000000ULL,
    SPECIAL_BITS         =  0xFFFB000000000000ULL,
    SPECIAL_MASK         =  0xFFFF000000000000ULL,
    SPECIAL_CONTENT_MASK = ~SPECIAL_MASK,
  };

  union {
    uint64_t bits_;
    double  value;
  };

  value_t() = default;
  value_t(value_t const &v) = default;

  explicit value_t(double v)   : value(v) {}
  explicit value_t(float v)    : value((double)v) {}
  explicit value_t(int64_t v)  : value((double)v) {}
  explicit value_t(uint64_t v) : value((double)v) {}
  explicit value_t(int32_t v)  : value((double)v) {}
  explicit value_t(uint32_t v) : value((double)v) {}
  explicit value_t(int16_t v)  : value((double)v) {}
  explicit value_t(uint16_t v) : value((double)v) {}
  explicit value_t(int8_t v)   : value((double)v) {}
  explicit value_t(uint8_t v)  : value((double)v) {}

  double   f64() const  { return value; }
  float    f32() const  { return (float)value; }
  int64_t  i64() const  { return (int64_t)value; }
  uint64_t ui64() const { return (uint64_t)value; }
  int32_t  i32() const  { return (int32_t)value; }
  uint32_t ui32() const { return (uint32_t)value; }
  int16_t  i16() const  { return (int16_t)value; }
  uint16_t ui16() const { return (uint16_t)value; }
  int8_t   i8() const   { return (int8_t)value; }
  uint8_t  ui8() const  { return (uint8_t)value; }

  void set(double v)   { value = v; }
  void set(float v)    { value = (double)v; }
  void set(int64_t v)  { value = (double)v; }
  void set(uint64_t v) { value = (double)v; }
  void set(int32_t v)  { value = (double)v; }
  void set(uint32_t v) { value = (double)v; }
  void set(int16_t v)  { value = (double)v; }
  void set(uint16_t v) { value = (double)v; }
  void set(int8_t v)   { value = (double)v; }
  void set(uint8_t v)  { value = (double)v; }

  operator double() const   { return value; }
  operator float() const    { return (float)value; }
  operator uint64_t() const { return (uint64_t)value; }
  operator int64_t() const  { return (int64_t)value; }
  operator uint32_t() const { return (uint32_t)value; }
  operator int32_t() const  { return (int32_t)value; }
  operator int16_t() const  { return (int16_t)value; }
  operator uint16_t() const { return (uint16_t)value; }
  operator int8_t() const   { return (int8_t)value; }
  operator uint8_t() const  { return (uint8_t)value; }

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

  bool is_nan() const { return (bits_ & NAN_MASK) == NAN_BITS; }
  bool is_special() const { return (bits_ & SPECIAL_MASK) == SPECIAL_BITS; }
  uint64_t special_bits() const { return bits_ & ~(SPECIAL_MASK); }

  static value_t special(uint64_t sig48);

  bool operator == (value_t other) const { return value == other.value; }
  bool operator != (value_t other) const { return !(*this == other); }
  bool operator <= (value_t other) const { return value <= other.value; }
  bool operator < (value_t other) const { return value < other.value; }
  bool operator >= (value_t other) const { return value >= other.value; }
  bool operator > (value_t other) const { return value > other.value; }
};


std::ostream &operator << (std::ostream &out, value_t v);


template <class T>
value_t make_value(T x)
{
  return value_t { x };
}


static_assert(std::is_trivial<value_t>::value,
  "Value must be trivial");

static_assert(std::is_standard_layout<value_t>::value,
  "Value must be standard layout");


#endif /* end __VM_VALUE_H__ include guard */
