/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include "vm_unit+chunk_types.inl"


std::string
read_string(std::istream &input, int32_t length)
{
  std::string result;
  result.resize(length, '\0');
  input.read(&result[0], length);
  return result;
}


template <typename T>
T read_primitive(std::istream &input)
{
  T value {};
  input.read((char *)&value, sizeof value);
  return value;
}


template <>
vm_value read_primitive<vm_value>(std::istream &input)
{
  return vm_value { read_primitive<double>(input) };
}


template <>
vm_chunk_header read_primitive<vm_chunk_header>(std::istream &input)
{
  return vm_chunk_header {
    read_primitive<vm_chunk_id>(input),
    read_primitive<int32_t>(input)
  };
}


template <>
vm_table_header read_primitive<vm_table_header>(std::istream &input)
{
  return vm_table_header {
    read_primitive<vm_chunk_header>(input),
    read_primitive<int32_t>(input)
  };
}


template <typename Func>
bool read_table(std::istream &input, vm_chunk_id id, Func &&func)
{
  vm_table_header const itable = read_primitive<vm_table_header>(input);

  if (itable.header.id == id) {
    for (int counter = 0; counter < itable.count; ++counter) {
      func(counter);
    }

    return true;
  }

  return false;
}


template <typename InitFunc, typename Func>
bool read_table(std::istream &input, vm_chunk_id id, InitFunc &&init, Func &&func)
{
  vm_table_header const itable = read_primitive<vm_table_header>(input);

  if (itable.header.id == id) {
    init(itable.count);

    for (int counter = 0; counter < itable.count; ++counter) {
      func(counter);
    }

    return true;
  }

  return false;
}


template <typename Func>
bool read_table(
  std::istream &input,
  vm_chunk_id id,
  vm_table_header &header_out,
  Func &&func
  )
{
  vm_table_header const itable = read_primitive<vm_table_header>(input);
  header_out = itable;

  if (itable.header.id == id) {
    for (int counter = 0; counter < itable.count; ++counter) {
      func(counter);
    }

    return true;
  }

  return false;
}


vm_label read_label(std::istream &input)
{
  int64_t const address = static_cast<int64_t>(read_primitive<int32_t>(input));
  int64_t const length = static_cast<int64_t>(read_primitive<int32_t>(input));
  std::string name = read_string(input, length);
  return vm_label { std::move(name), address };
}


std::string read_lstring(std::istream &input)
{
  std::string result;
  int64_t length = static_cast<int64_t>(read_primitive<int32_t>(input));
  result.resize(length, '\0');
  input.read(&result[0], length);
  return result;
}

