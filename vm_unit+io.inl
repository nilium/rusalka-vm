/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include "vm_unit+chunk_types.inl"


/**
 * Reads a fixed-length string from the given input stream.
 *
 * The resulting string may contain null characters.
 */
std::string
read_string(std::istream &input, int32_t length)
{
  std::string result;
  result.resize(length, '\0');
  input.read(&result[0], length);
  return result;
}



/**
 * Basic read_primitive template function. Reads a value of type T from a
 * stream and returns it. T should, ideally, be standard layout and fairly
 * small (i.e., 8 bytes or less). However, this can be used for any standard
 * layout struct as well, it's just not advisable without a specific
 * implementation for that type.
 */
template <typename T>
T read_primitive(std::istream &input)
{
  T value {};
  input.read((char *)&value, sizeof value);
  return value;
}



/**
 * Reads a vm_value from the stream.
 */
template <>
vm_value read_primitive<vm_value>(std::istream &input)
{
  return vm_value { read_primitive<double>(input) };
}



/**
 * Reads a vm_chunk_header from the stream.
 */
template <>
vm_chunk_header read_primitive<vm_chunk_header>(std::istream &input)
{
  return vm_chunk_header {
    read_primitive<vm_chunk_id>(input),
    read_primitive<int32_t>(input)
  };
}



/**
 * Reads a vm_table_header from the stream.
 */
template <>
vm_table_header read_primitive<vm_table_header>(std::istream &input)
{
  return vm_table_header {
    read_primitive<vm_chunk_header>(input),
    read_primitive<int32_t>(input)
  };
}



/**
 * Reads a table of unknown contents from the stream.
 *
 * The table is read by reading the table header and then iteratively calling
 * the given `func` as many times as there are entries in the table.
 *
 * It does not advance the read pointer for the stream for each row, so it's
 * assumed that `func` will do this.
 */
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



/**
 * Reads a table of unknown contents from the stream. Same as the other
 * read_table implementation except that `init` will be called prior to
 * iteration if the table header was successfully read and had a matching
 * chunk ID.
 */
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



/**
 * Same as read_table (sans init), except returns the table header by receiving
 * a reference to some storage for a vm_table_header object.
 */
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



/**
 * Reads a vm_label object from a stream.
 *
 * A label is defined as a 32-bit address and 32-bit length followed by a name
 * string with the previously-read length.
 */
vm_label read_label(std::istream &input)
{
  int64_t const address = static_cast<int64_t>(read_primitive<int32_t>(input));
  int64_t const length = static_cast<int64_t>(read_primitive<int32_t>(input));
  std::string name = read_string(input, length);
  return vm_label { std::move(name), address };
}



/**
 * Reads a variable-length string (an LString) from the stream. An LString is
 * prefixed by a 32-bit integer defining its length. The string may contain
 * null characters.
 */
std::string read_lstring(std::istream &input)
{
  std::string result;
  int64_t length = static_cast<int64_t>(read_primitive<int32_t>(input));
  result.resize(length, '\0');
  input.read(&result[0], length);
  return result;
}
