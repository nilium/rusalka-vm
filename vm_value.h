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
  static constexpr double EPSILON = 1.0e-17;

  enum fcmp_result : int
  {
    LESS = -1,
    EQUAL = 0,
    GREATER = 1,
  };


  enum value_type : int32_t
  {
    ERROR                 = -1, // currently unused -- has no defined behavior yet
    UNDEFINED             = 0,  // type set for undefined operations/conversions

    // Arithmetic types
    UNSIGNED              = 1,
    SIGNED                = 2,
    FLOAT                 = 3,

    // Memory type
    DATA                  = 4,

    MIN_ARITHMETIC        = UNSIGNED,
    MAX_ARITHMETIC        = FLOAT,

    MIN_COMPARABLE        = UNSIGNED,

    // Range of reserved values for built-in types
    MIN_BUILTIN           = -(1 << 16),
    MAX_BUILTIN           = 1 << 16,
  };


  union {
    double   f64_;
    int64_t  s64_;
    uint64_t u64_;
  };
  int32_t    type;

  // Constants

  // Undefined result constant
  static vm_value undefined();
  // Error type constant (currently unused)
  static vm_value error();

  // FLOAT constants
  static vm_value nan();
  static vm_value infinity();
  static vm_value epsilon();

  //// Constructors

  vm_value() = default;
  vm_value(vm_value const &v) = default;

  template <
      typename T,
      typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, bool>::type = true
      >
  explicit vm_value(T v)
  : s64_(static_cast<T>(v))
  , type(SIGNED)
  {
    /* nop */
  }

  template <
      typename T,
      typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, bool>::type = true
      >
  explicit vm_value(T v)
  : u64_(static_cast<T>(v))
  , type(UNSIGNED)
  {
    /* nop */
  }

  template <
      typename T,
      typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true
      >
  explicit vm_value(T v)
  : f64_(static_cast<T>(v))
  , type(FLOAT)
  {
    /* nop */
  }

  vm_value(int32_t _type, uint64_t value)
  : u64_(value)
  , type(_type)
  {
    /* nop */
  }

  vm_value(int32_t _type, int64_t value)
  : s64_(value)
  , type(_type)
  {
    /* nop */
  }

  vm_value(int32_t _type, double value)
  : f64_(value)
  , type(_type)
  {
    /* nop */
  }


  //// Assignment

  vm_value &operator = (vm_value const &) = default;


  template <typename T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, bool>::type = true>
  void set(T si_value)
  {
    s64_ = static_cast<int64_t>(si_value);
    type = SIGNED;
  }

  template <typename T, typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, bool>::type = true>
  void set(T ui_value)
  {
    u64_ = static_cast<uint64_t>(ui_value);
    type = UNSIGNED;
  }

  template <typename T, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
  void set(T fp_value)
  {
    f64_ = static_cast<double>(fp_value);
    type = FLOAT;
  }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value &operator = (T value)
  {
    set(value);
    return *this;
  }


  //// Conversions

  bool is_arithmetic() const
  {
    return MIN_ARITHMETIC <= type && type <= MAX_ARITHMETIC;
  }

  bool is_undefined() const { return type == UNDEFINED; }
  bool is_defined() const { return !is_undefined(); }
  bool is_error() const { return type == ERROR; }

  // float types
  bool is_nan() const;
  bool is_infinity() const;

  double   f64() const;
  int64_t  i64() const;
  uint64_t ui64() const;

  operator float () const { return static_cast<float>(f64()); }
  operator double () const { return f64(); }
  operator uint64_t () const { return ui64(); }
  operator uint32_t () const { return static_cast<uint32_t>(ui64()); }
  operator uint16_t () const { return static_cast<uint16_t>(ui64()); }
  operator uint8_t () const { return static_cast<uint8_t>(ui64()); }
  operator int64_t () const { return i64(); }
  operator int32_t () const { return static_cast<int32_t>(i64()); }
  operator int16_t () const { return static_cast<int16_t>(i64()); }
  operator int8_t () const { return static_cast<int8_t>(i64()); }


  //// Operator overloads

  vm_value operator +   (vm_value rhs) const;
  vm_value operator -   (vm_value rhs) const;
  vm_value operator *   (vm_value rhs) const;
  vm_value operator %   (vm_value rhs) const;
  vm_value operator &   (vm_value rhs) const;
  vm_value operator |   (vm_value rhs) const;
  vm_value operator ^   (vm_value rhs) const;
  vm_value operator <<  (vm_value rhs) const;
  vm_value operator >>  (vm_value rhs) const;


  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value operator +   (T rhs) const { return (*this) +  vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value operator -   (T rhs) const { return (*this) -  vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value operator *   (T rhs) const { return (*this) *  vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value operator %   (T rhs) const { return (*this) %  vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value operator &   (T rhs) const { return (*this) &  vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value operator |   (T rhs) const { return (*this) |  vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value operator ^   (T rhs) const { return (*this) ^  vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value operator <<  (T rhs) const { return (*this) << vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value operator >>  (T rhs) const { return (*this) >> vm_value(rhs); }


  //// value-on-right operators

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend T operator +   (T lhs, vm_value rhs) { return lhs + static_cast<T>(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend T operator -   (T lhs, vm_value rhs) { return lhs - static_cast<T>(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend T operator *   (T lhs, vm_value rhs) { return lhs * static_cast<T>(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend T operator %   (T lhs, vm_value rhs) { return lhs % static_cast<T>(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend T operator &   (T lhs, vm_value rhs) { return lhs & static_cast<T>(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend T operator |   (T lhs, vm_value rhs) { return lhs | static_cast<T>(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend T operator ^   (T lhs, vm_value rhs) { return lhs ^ static_cast<T>(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend T operator <<  (T lhs, vm_value rhs) { return lhs << static_cast<T>(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend T operator >>  (T lhs, vm_value rhs) { return lhs >> static_cast<T>(rhs); }


  //// Assignment operators

  vm_value &operator +=   (vm_value rhs);
  vm_value &operator -=   (vm_value rhs);
  vm_value &operator *=   (vm_value rhs);
  vm_value &operator %=   (vm_value rhs);
  vm_value &operator &=   (vm_value rhs);
  vm_value &operator |=   (vm_value rhs);
  vm_value &operator ^=   (vm_value rhs);
  vm_value &operator <<=  (vm_value rhs);
  vm_value &operator >>=  (vm_value rhs);


  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value &operator +=   (T rhs) { return (*this) += vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value &operator -=   (T rhs) { return (*this) -= vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value &operator *=   (T rhs) { return (*this) *= vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value &operator %=   (T rhs) { return (*this) %= vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value &operator &=   (T rhs) { return (*this) &= vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value &operator |=   (T rhs) { return (*this) |= vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value &operator ^=   (T rhs) { return (*this) ^= vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value &operator <<=  (T rhs) { return (*this) <<= vm_value(rhs); }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  vm_value &operator >>=  (T rhs) { return (*this) >>= vm_value(rhs); }


  //// Unary operators

  vm_value operator - () const;
  vm_value operator ~ () const; // Implicit u64 conversion

  bool operator !    () const;
       operator bool () const;


  //// Comparison operators

  bool operator == (vm_value rhs) const;
  bool operator != (vm_value rhs) const;
  bool operator <= (vm_value rhs) const;
  bool operator < (vm_value rhs) const;
  bool operator >= (vm_value rhs) const;
  bool operator > (vm_value rhs) const;

  // Only returns true/false for comparable non-FLOAT values.
  bool operator == (std::nullptr_t) const;
  bool operator != (std::nullptr_t) const;

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  bool operator == (T value) const { return (*this)  ==  vm_value { value }; }
  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  bool operator != (T value) const { return (*this)  !=  vm_value { value }; }
  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  bool operator <= (T value) const { return (*this)  <=  vm_value { value }; }
  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  bool operator < (T value)  const { return (*this)  <   vm_value { value }; }
  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  bool operator >= (T value) const { return (*this)  >=  vm_value { value }; }
  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  bool operator > (T value)  const { return (*this)  >   vm_value { value }; }

  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend bool operator == (T lhs, vm_value rhs) { return vm_value { lhs }  ==  rhs; }
  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend bool operator != (T lhs, vm_value rhs) { return vm_value { lhs }  !=  rhs; }
  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend bool operator <= (T lhs, vm_value rhs) { return vm_value { lhs }  <=  rhs; }
  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend bool operator < (T lhs, vm_value rhs)  { return vm_value { lhs }  <  rhs; }
  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend bool operator >= (T lhs, vm_value rhs) { return vm_value { lhs }  >=  rhs; }
  template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
  friend bool operator > (T lhs, vm_value rhs)  { return vm_value { lhs }  >  rhs; }

  friend bool operator == (std::nullptr_t lhs, vm_value rhs) { return rhs == nullptr; }
  friend bool operator != (std::nullptr_t lhs, vm_value rhs) { return rhs != nullptr; }

  fcmp_result fcmp(vm_value other) const;
  fcmp_result fcmp(vm_value other, double epsilon = EPSILON) const;

  vm_value as(int32_t new_type) const;
  vm_value &convert(int32_t new_type);
};

using vm_fcmp_result = typename vm_value::fcmp_result;
using vm_value_type = typename vm_value::value_type;


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

static_assert(std::is_trivially_assignable<vm_value, vm_value>::value,
  "vm_value must be trivially assignable to itself");

#pragma GCC visibility pop
