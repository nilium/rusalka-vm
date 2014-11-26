/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once


/**
 * A chunk header as stored in a file. Not packed the way it would be in a unit
 * file, though.
 */
struct vm_chunk_header
{
  vm_chunk_id  id;
  int32_t      byte_size;
};



/**
 * A version chunk, as stored in a file. Not packed the way it would be in a
 * unit.
 */
struct vm_version_chunk
{
  vm_chunk_header  header;
  int32_t          version;
};



/**
 * A table chunk header, as stored in a unit file. Not packed.
 */
struct vm_table_header
{
  vm_chunk_header  header;
  int32_t          count;
};



/**
 * A label entry from reading a label out of a unit file.
 */
struct vm_label
{
  std::string name;
  int64_t     address;
};
