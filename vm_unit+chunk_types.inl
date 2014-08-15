/*
 *          Copyright Noel Cower 2014.
 *
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __VM_UNIT_CHUNK_TYPES_INL__
#define __VM_UNIT_CHUNK_TYPES_INL__


struct chunk_header_t
{
  vm_chunk_id  id;
  int32_t     byte_size;
};


struct version_chunk_t
{
  chunk_header_t  header;
  int32_t         version;
};


struct table_header_t
{
  chunk_header_t  header;
  int32_t         count;
};


struct label_t
{
  std::string name;
  int32_t address;
};


#endif /* end __VM_UNIT_CHUNK_TYPES_INL__ include guard */
