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


/**
 * vm_value is the sole value type used in Rusalka.
 *
 * It may be one of several primitive types, including an error code type, a
 * 64-bit unsigned or signed integer, a 64-bit float, a static data reference,
 * or possibly any other not-yet-defined type. In addition, a value may be
 * undefined, in which case any operations on it are either an error or also
 * undefined.
 */
struct vm_value final
{
  /** The epsilon used in fcmp tests. */
  static constexpr double EPSILON = 1.0e-17;

  /** The result of calling vm_value::fcmp. */
  enum fcmp_result : int
  {
    LESS = -1,
    EQUAL = 0,
    GREATER = 1,
  };

  /** All possible vm_value types and some related info constants. */
  enum value_type : int32_t
  {
    /** An error code type code. */
    ERROR                 = -1, // currently unused -- has no defined behavior yet
    /** An undefined value type code. */
    UNDEFINED             = 0,  // type set for undefined operations/conversions

    // Arithmetic types
    /** Unsigned 64-bit integer type code. */
    UNSIGNED              = 1,
    /** Signed 64-bit integer type code. */
    SIGNED                = 2,
    /** 64-bit floating point type code. */
    FLOAT                 = 3,

    // Memory type
    /** Data reference type code. */
    DATA                  = 4,

    /** The minimum type supported for arithmetic. */
    MIN_ARITHMETIC        = UNSIGNED,
    /** The maximum type supported for arithmetic. */
    MAX_ARITHMETIC        = FLOAT,

    /** The minimum type supported for comparisons (all >= are comparable). */
    MIN_COMPARABLE        = UNSIGNED,

    // Range of reserved values for built-in types
    /** The minimum inclusive type code for built-in types. */
    MIN_BUILTIN           = -(1 << 16),
    /** The maximum inclusive type code for built-in types. */
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

  // fcmp only returns valid results for arithmetic types. For non-arithmetic types, the result is
  // always LESS.
  fcmp_result fcmp(vm_value other) const;
  fcmp_result fcmp(vm_value other, double epsilon = EPSILON) const;

  vm_value as(int32_t new_type) const;
  vm_value &convert(int32_t new_type);
};

using vm_fcmp_result = typename vm_value::fcmp_result;
using vm_value_type  = typename vm_value::value_type;


std::ostream &operator << (std::ostream &out, vm_value v);



/**
 * Convenience function to construct a vm_value from an arithmetic type.
 *
 * This function may be overloaded as needed to provide vm_value objects for
 * various other types.
 */
template <
  typename T,
  typename Decayed = typename std::decay<T>::type,
  typename std::enable_if<
    std::is_arithmetic<Decayed>::value || std::is_same<Decayed, vm_value>::value,
    bool
    >::type = true
  >
vm_value make_value(T &&x)
{
  return vm_value { std::forward<T>(x) };
}


// A few static assertions so some guarantees can be made about the vm_value
// type, since it needs to have a few particular attributes (e.g., memcpy-able,
// trivial, and so on -- it cannot be too expensive to allocate its storage).
static_assert(std::is_trivial<vm_value>::value,
  "Value must be trivial");

static_assert(std::is_standard_layout<vm_value>::value,
  "Value must be standard layout");

static_assert(std::is_trivially_assignable<vm_value, vm_value>::value,
  "vm_value must be trivially assignable to itself");
