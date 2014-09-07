/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once


struct vm_chunk_header
{
  vm_chunk_id  id;
  int32_t     byte_size;
};


struct vm_version_chunk
{
  vm_chunk_header  header;
  int32_t         version;
};


struct vm_table_header
{
  vm_chunk_header  header;
  int32_t         count;
};


struct vm_label
{
  std::string name;
  int32_t address;
};
