/*
 *          Copyright Noel Cower 2013 - 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <cstdint>
#include <iostream>

#pragma GCC visibility push(default)


struct vm_value final
{
  double  value;

  vm_value() = default;
  vm_value(vm_value const &v) = default;

  explicit vm_value(double v)   : value(v) {}
  explicit vm_value(float v)    : value((double)v) {}
  explicit vm_value(int64_t v)  : value((double)v) {}
  explicit vm_value(uint64_t v) : value((double)v) {}
  explicit vm_value(int32_t v)  : value((double)v) {}
  explicit vm_value(uint32_t v) : value((double)v) {}
  explicit vm_value(int16_t v)  : value((double)v) {}
  explicit vm_value(uint16_t v) : value((double)v) {}
  explicit vm_value(int8_t v)   : value((double)v) {}
  explicit vm_value(uint8_t v)  : value((double)v) {}

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

  vm_value &operator = (double v)    { set(v); return *this; }
  vm_value &operator = (float v)     { set(v); return *this; }
  vm_value &operator = (int64_t v)   { set(v); return *this; }
  vm_value &operator = (uint64_t v)  { set(v); return *this; }
  vm_value &operator = (int32_t v)   { set(v); return *this; }
  vm_value &operator = (uint32_t v)  { set(v); return *this; }
  vm_value &operator = (int16_t v)   { set(v); return *this; }
  vm_value &operator = (uint16_t v)  { set(v); return *this; }
  vm_value &operator = (int8_t v)    { set(v); return *this; }
  vm_value &operator = (uint8_t v)   { set(v); return *this; }

  bool operator == (vm_value other) const { return value == other.value; }
  bool operator != (vm_value other) const { return !(*this == other); }
  bool operator <= (vm_value other) const { return value <= other.value; }
  bool operator < (vm_value other) const { return value < other.value; }
  bool operator >= (vm_value other) const { return value >= other.value; }
  bool operator > (vm_value other) const { return value > other.value; }
};


std::ostream &operator << (std::ostream &out, vm_value v);


template <class T>
vm_value make_value(T x)
{
  return vm_value { x };
}


static_assert(std::is_trivial<vm_value>::value,
  "Value must be trivial");

static_assert(std::is_standard_layout<vm_value>::value,
  "Value must be standard layout");


#pragma GCC visibility pop
