/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __VM_UNIT_IO_INL__
#define __VM_UNIT_IO_INL__


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
chunk_header_t read_primitive<chunk_header_t>(std::istream &input)
{
  return chunk_header_t {
    read_primitive<chunk_id_t>(input),
    read_primitive<int32_t>(input)
  };
}


template <>
table_header_t read_primitive<table_header_t>(std::istream &input)
{
  return table_header_t {
    read_primitive<chunk_header_t>(input),
    read_primitive<int32_t>(input)
  };
}


template <typename Func>
bool read_table(std::istream &input, chunk_id_t id, Func &&func)
{
  table_header_t const itable = read_primitive<table_header_t>(input);

  if (itable.header.id == id) {
    for (int32_t counter = 0; counter < itable.count; ++counter) {
      func(counter);
    }

    return true;
  }

  return false;
}


template <typename InitFunc, typename Func>
bool read_table(std::istream &input, chunk_id_t id, InitFunc &&init, Func &&func)
{
  table_header_t const itable = read_primitive<table_header_t>(input);

  if (itable.header.id == id) {
    init(itable.count);

    for (int32_t counter = 0; counter < itable.count; ++counter) {
      func(counter);
    }

    return true;
  }

  return false;
}


template <typename Func>
bool read_table(
  std::istream &input,
  chunk_id_t id,
  table_header_t &header_out,
  Func &&func
  )
{
  table_header_t const itable = read_primitive<table_header_t>(input);
  header_out = itable;

  if (itable.header.id == id) {
    for (int32_t counter = 0; counter < itable.count; ++counter) {
      func(counter);
    }

    return true;
  }

  return false;
}


label_t read_label(std::istream &input)
{
  int32_t const address = read_primitive<int32_t>(input);
  int32_t const length = read_primitive<int32_t>(input);
  std::string name = read_string(input, length);
  return label_t { std::move(name), address };
}


std::string read_lstring(std::istream &input)
{
  std::string result;
  int32_t length = read_primitive<int32_t>(input);
  result.resize(length, '\0');
  input.read(&result[0], length);
  return result;
}


#endif /* end __VM_UNIT_IO_INL__ include guard */
